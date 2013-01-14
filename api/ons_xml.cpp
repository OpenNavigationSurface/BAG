//************************************************************************
//      File:    ons_xml.cpp
//
//      Open Navigation Surface Working Group, 2005
//
//      - Initial implementation
//        Bill Lamey, 7/20/2005
//
//************************************************************************

// ONS includes
#include "ons_xml_error_handler.h"
#include "ons_xml.h"
#include "bag_metadata_def.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <float.h>
#include <string>
#include <sstream>
#include <algorithm>

// XERCES includes.
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMWriter.hpp>
#include <iostream>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/sax/SAXParseException.hpp>

namespace
{

// constant locations/names.
static const char *ons_schema_location = "ISO19139/bag/bag.xsd";
static const char *ons_schema_nameSpace = "http://www.opennavsurf.org/schema/bag";

// flag used to force the validation to succeed when parsing a BAG XML file or mem buffer.
// Bill Lamey July 21, 2005 - This is set to false temoporarily as there are some problems validating the 
//          MD_Georectified portion of the XML file.  The MD_Georectified section is crucial
//          as it contains information about he projected corner coordinates of the grid, 
//          pixel location (center, lower left, etc), cell geometry (cell, node), etc.  Once
//          the validation errors have been resolved, this falg should be removed.  That is,
//          the library should ALWAYS force the validation of the 19115 schema.
static bool ons_force_schema_validation = true;

//************************************************************************
/*!
\brief Retrieve the value from the given node.

\param node
    \li The node to retrieve the value from.
\return
    \li String containing the value of the given node.
*/
//************************************************************************
std::string getValueFromNode(DOMNode &node)
{
    DOMNode *pChildNode = node.getFirstChild();
    if (pChildNode == NULL)
        return std::string();

    char *pTmpStr = XMLString::transcode(pChildNode->getNodeValue());
    std::string retStr(pTmpStr);
    XMLString::release(&pTmpStr);

    return retStr;
}

//************************************************************************
/*!
\brief Retrieve the metadata version.

    Currently we only have two metadata versions. The first was used
    upto and including version 1.4.0 of the library. The second is used
    for 1.5.0 and above.

\param version
    \li The BAG version string.
\return
    \li The metadata version.
*/
//************************************************************************
int getMetadataVersion(const char *version)
{
    const double verNum = atof(version);

    if (verNum < 1.5)
        return 1;

    return 2;
}

}   //namespace

//! Structure used to hold the XML parser and error handler.
struct bagXMLHandle
{
    bagXMLHandle()
        :parser(NULL),
        errHandler(NULL)
    {
    }

    ~bagXMLHandle()
    {
        if (parser != NULL)
            delete parser;

        if (errHandler != NULL)
            delete errHandler;
    }

    //! Pointer to the current document parser.
    XercesDOMParser *parser;
    //! Pointer to the current error handler.
    BAGMetaDataErrorHandler *errHandler;
};

//! This a simpe class to allow transcoding of a wide character buffer to an ascii buffer.
class wideString
{
public :
//Construction/Destruction
    wideString(const XMLCh* const toTranscode)
    {
        // Call the private transcoding method
        fLocalForm = XMLString::transcode(toTranscode);
    }

    ~wideString()
    {
        XMLString::release(&fLocalForm);
    }

//Methods
    const char* localForm() const
    {
        return fLocalForm;
    }

private :
    //! This is the local code page form of the string.
    char*   fLocalForm;
};

//! This a simpe class to allow transcoding of an ascii buffer to wide character buffer.
class asciiString
{
public:
    //! Constructor
    asciiString()
    : m_wBuffer(NULL)
    {
    }

    //! Constructor
    asciiString(const char *buffer)
    : m_wBuffer(NULL)
    {
        if (buffer != NULL)
            m_wBuffer = XMLString::transcode(buffer);
    }

    //! Destructor
    ~asciiString()
    {
        if (m_wBuffer != NULL)
            XMLString::release(&m_wBuffer);
    }

    //! Cast type operator.
    operator XMLCh*() const
    {
        return m_wBuffer;
    }

    //! Assignment operator.
    asciiString & operator=(const char *buffer)
    {
        if (m_wBuffer != NULL)
        {
            XMLString::release(&m_wBuffer);
            m_wBuffer = NULL;
        }

        if (buffer != NULL)
            m_wBuffer = XMLString::transcode(buffer);

        return *this;
    }

private:

    //! The wide character buffer.
    XMLCh *m_wBuffer;
};

//************************************************************************
//      Method name:    operator<<()
//
//      
//      - Initial implementation
//        Bill Lamey, 7/19/2005
//
//************************************************************************
//! Stream operator for the string.
/*!
\param target
    \li The stream to be display to.
\param toDump
    \li The string to be dumped.
\return
    \li reference to the input stream.
*/
//************************************************************************
inline XERCES_STD_QUALIFIER ostream& operator<<(
    XERCES_STD_QUALIFIER ostream& target,
    const wideString& toDump
    )
{
    target << toDump.localForm();
    return target;
}
//************************************************************************
//      Implementaiton of the BAGMetaDataErrorHandler class.
//************************************************************************

//************************************************************************
//      Method name:    warning()
//
//      
//      - Initial implementation
//        Bill Lamey, 7/19/2005
//
//************************************************************************
//! Display a warning that was passed in from the parsing.
/*!
\param toCatch
    \li The exception to be displayed.
*/
//************************************************************************
void BAGMetaDataErrorHandler::warning(
    const SAXParseException& toCatch
    )
{
    XERCES_STD_QUALIFIER cerr << "Warning at file \"" << wideString(toCatch.getSystemId())
		 << "\", line " << toCatch.getLineNumber()
		 << ", column " << toCatch.getColumnNumber()
         << "\n   Message: " << wideString(toCatch.getMessage()) << XERCES_STD_QUALIFIER endl;
}

//************************************************************************
//      Method name:    error()
//
//      
//      - Initial implementation
//        Bill Lamey, 7/19/2005
//
//************************************************************************
//! Display an error passed in from the parser.
/*!
\param toCatch
    \li The exception to be displayed.
*/
//************************************************************************
void BAGMetaDataErrorHandler::error(
    const SAXParseException& toCatch
    )
{
    fSawErrors = true;
    XERCES_STD_QUALIFIER cerr << "Error at file \"" << wideString(toCatch.getSystemId())
		 << "\", line " << toCatch.getLineNumber()
		 << ", column " << toCatch.getColumnNumber()
         << "\n   Message: " << wideString(toCatch.getMessage()) << XERCES_STD_QUALIFIER endl;
}

//************************************************************************
//      Method name:    fatalError()
//
//      
//      - Initial implementation
//        Bill Lamey, 7/19/2005
//
//************************************************************************
//! Display a fatal error passed in from the parser.
/*!
\param toCatch
    \li The exception to be displayed.
*/
//************************************************************************
void BAGMetaDataErrorHandler::fatalError(
    const SAXParseException& toCatch
    )
{
    fSawErrors = true;
    fSawFatals = true;
    XERCES_STD_QUALIFIER cerr << "Fatal Error at file \"" << wideString(toCatch.getSystemId())
		 << "\", line " << toCatch.getLineNumber()
		 << ", column " << toCatch.getColumnNumber()
         << "\n   Message: " << wideString(toCatch.getMessage()) << XERCES_STD_QUALIFIER endl;
}

//************************************************************************
//      Method name:    resetErrors()
//
//      
//      - Initial implementation
//        Bill Lamey, 7/19/2005
//
//************************************************************************
//! Reset the internal flag indicating error.
//************************************************************************
void BAGMetaDataErrorHandler::resetErrors(
    )
{
    fSawErrors = false;
    fSawFatals = false;
}


//************************************************************************
//    Implementaiton of the ONS routines - Non exported functions
//************************************************************************

//************************************************************************
//      Method name:    bagGetXMLNodeByName()
//
//      
//      - Initial implementation
//        Bill Lamey, 7/21/2005
//
//************************************************************************
//! Get an XML node by parsing an XPath.
/*!
\param parent
    \li The parent node to begin looking from.
\param pathName
    \li The full XPath of the element to be searched for.
\return
    \li The DOMNode matching the input path on success, NULL if the path does
        not exist in the DOM tree.
*/
//************************************************************************
DOMNode *bagGetXMLNodeByName(
    DOMNode *parent,
    const char *pathName
    )
{
    if (parent == NULL)
        return NULL;

    // stip out the first section of the path.
    const char *endPos = strchr(pathName, '/');

    asciiString nodeName;
    if (endPos)
    {
        char nodeNameBuffer[256];
        size_t len = endPos - pathName;
        strncpy(nodeNameBuffer, pathName, len);
        nodeNameBuffer[len] = '\0';

        nodeName = nodeNameBuffer;
    }
    else
        nodeName = pathName;

    DOMNodeList *pList = parent->getChildNodes();

    for (u32 i = 0; i < pList->getLength(); i++)
    {
        // get the node.
        DOMNode *pChild = pList->item(i);
        if (pChild)
        {
            const XMLCh *pCurrNodeName = pChild->getNodeName();
            if (XMLString::compareString(nodeName, pCurrNodeName) == 0)
            {
                if (endPos == NULL)
                {
                    // if we are at the end, return the node.
                    return pChild;
                }
                else
                {
                    // recurse.
                    DOMNode *pRet = bagGetXMLNodeByName(pChild, endPos + 1);
                    if (pRet)
                    {
                        return pRet;
                    }
                }
            }
        }
    }

    // not found.
    return NULL;
}

//************************************************************************
//      Method name:    bagGetMetadata()
//
//      - Initial implementation
//        Mike Van Duzee, 1/26/2012
//
//************************************************************************
//! Retrieve the bagMetaData from the given source.
/*!
\param source
    \li The input source object to be parsed.
\param error
    \li Modified to contain a non-zero value if the funtion fails, modified
    to contain zero if the function succeeds.
\return
    \li A valid bagMetaData handle on success, NULL on failure.  If the method
        fails, the value of error will be non-zero.
*/
//************************************************************************
bagMetaData bagGetMetadata(
    const InputSource &source,
    Bool doValidation,
    bagError *error
    )
{
    // Reset the error code.
    *error = 0;

    bagMetaData metaData = NULL;

    //  Create our parser, then attach an error handler to the parser.
    //  The parser will call back to methods of the ErrorHandler if it
    //  discovers errors during the course of parsing the XML document.
    metaData = new bagXMLHandle;
    metaData->parser = new XercesDOMParser;
    metaData->parser->setDoNamespaces(true);
    metaData->parser->setValidationSchemaFullChecking(false);
    metaData->parser->setCreateEntityReferenceNodes(false);
    metaData->parser->setIncludeIgnorableWhitespace(false);
    metaData->parser->setValidationScheme(XercesDOMParser::Val_Never);
    metaData->errHandler = new BAGMetaDataErrorHandler();
    metaData->parser->setErrorHandler(metaData->errHandler);

    if (doValidation == True)
    {
        // Check environment.
        const char *onsHome = getenv("BAG_HOME");
        if (onsHome == NULL)
        {
            delete metaData;
         
            *error = BAG_METADTA_NO_HOME;
            return NULL;
        }

        //Build the full path to the schema location.
        std::string schemaFile(onsHome);
        schemaFile += "/";
        schemaFile += ons_schema_location;

        // make sure the main schema file exists.
        struct stat fStat;
        if (stat(schemaFile.c_str(), &fStat))
        {
            delete metaData;
         
            *error = BAG_METADTA_SCHEMA_FILE_MISSING;
            return NULL;
        }

        // make sure that any spaces in the file name are replaced with %20 so that Xerces will parse correctly.
        size_t pos = 0;
        while((pos = schemaFile.find(" ", pos)) != std::string::npos)
        {
            schemaFile.replace(pos, 1, "%20");
            pos += 3;
        }

        //Format the schema entry.
        std::string schemaEntry(ons_schema_nameSpace);
        schemaEntry += " ";
        schemaEntry += schemaFile;

        //  Create our parser, then attach an error handler to the parser.
        //  The parser will call back to methods of the ErrorHandler if it
        //  discovers errors during the course of parsing the XML document.
        metaData->parser->setValidationScheme(XercesDOMParser::Val_Always);
        metaData->parser->setDoSchema(ons_force_schema_validation);
        metaData->parser->setExternalSchemaLocation(schemaEntry.c_str());
    }

    //  Parse the XML file, catching any XML exceptions that might propogate
    //  out of it.
    try
    {
        metaData->parser->parse(source);

        if (doValidation == True)
        {
            DOMDocument* xmlDoc = metaData->parser->getDocument();
        
            // The schema validation will pass even if the MD_DataIdentification element was
            // used, so lets verify that we have the correct bag version.
            DOMNodeList* nodeList = xmlDoc->getElementsByTagName((const asciiString)"bag:BAG_DataIdentification");
            if (nodeList == NULL || nodeList->getLength() == 0)
            {
                delete metaData;

                *error = BAG_METADTA_VALIDATE_FAILED;
                return NULL;
            }

            // The schema validation will pass even if the LI_ProcessStep element was
            // used, so lets verify that we have the correct bag version.
            nodeList = xmlDoc->getElementsByTagName((const asciiString)"bag:BAG_ProcessStep");
            if (nodeList == NULL || nodeList->getLength() == 0)
            {
                delete metaData;

                *error = BAG_METADTA_VALIDATE_FAILED;
                return NULL;
            }
        }
    }
    catch (const OutOfMemoryException&)
    {
        XERCES_STD_QUALIFIER cerr << "OutOfMemoryException" << XERCES_STD_QUALIFIER endl;
        *error = BAG_METADTA_PARSE_MEM_EXCEPTION;
    }
    catch (const XMLException& e)
    {
        XERCES_STD_QUALIFIER cerr << "An error occurred during parsing\n   Message: "
             << wideString(e.getMessage()) << XERCES_STD_QUALIFIER endl;
        *error = BAG_METADTA_PARSE_EXCEPTION;
    }
    catch (const DOMException& e)
    {
        const u32 maxChars = 2047;
        XMLCh errText[maxChars + 1];

        XERCES_STD_QUALIFIER cerr << "\nDOM Error parsing " << wideString(source.getSystemId()) << XERCES_STD_QUALIFIER endl
             << "DOMException code is:  " << e.code << XERCES_STD_QUALIFIER endl;

        if (DOMImplementation::loadDOMExceptionMsg(e.code, errText, maxChars))
             XERCES_STD_QUALIFIER cerr << "Message is: " << wideString(errText) << XERCES_STD_QUALIFIER endl;
        *error = BAG_METADTA_PARSE_DOM_EXCEPTION;
    }
    catch (...)
    {
        XERCES_STD_QUALIFIER cerr << "An error occurred during parsing\n " << XERCES_STD_QUALIFIER endl;
        *error = BAG_METADTA_PARSE_UNK_EXCEPTION;
    }

    // Check for validation errors if the parse was successful.
    if (*error == 0 && doValidation == True)
    {
        // determine if there were any validation errors.
        bool validateErrors = metaData->errHandler->getSawErrors();
        bool fatalErrors = metaData->errHandler->getFatalErrors();

        if (validateErrors && ons_force_schema_validation)
            *error = BAG_METADTA_VALIDATE_FAILED;
        else if (fatalErrors)
            *error = BAG_METADTA_PARSE_EXCEPTION;
    }

    // if an error occured, delete the metadata.
    if (*error)
    {
        delete metaData;
        metaData = NULL;
    }

    return metaData;
}

//************************************************************************
//    Implementaiton of the ONS routines - Exported functions
//************************************************************************

//************************************************************************
//      Method name:    ons_free_metadata()
//
//      
//      - Initial implementation
//        Bill Lamey, 7/19/2005
//
//************************************************************************
//! Free the memory associated with the specified handle and the handle itself.
/*!
\param metaData
    \li The metadata to free.
*/
//************************************************************************
void bagFreeMetadata(
    bagMetaData metaData
    )
{
    delete metaData;
}

//************************************************************************
//      Method name:    bagInitMetadata()
//
//      
//      - Initial implementation
//        Bill Lamey, 7/20/2005
//
//************************************************************************
//! Initialize the metadata library.
/*!
        This function initializes the metadata library by initializing the XML
        subsystem for the xerces parser.

\return
    \li 0 if the function is successful, non-zero if the function fails.
*/
//************************************************************************
bagError bagInitMetadata()
try
{
    XMLPlatformUtils::Initialize();
    return 0;
}
catch(const XMLException &toCatch)
{
    XERCES_STD_QUALIFIER cerr << "Error during Xerces-c Initialization.\n"
         << "  Exception message:"
         << wideString(toCatch.getMessage()) << XERCES_STD_QUALIFIER endl;
    return BAG_METADTA_INIT_FAILED;
}

//************************************************************************
//      Method name:    bagTermMetadata()
//
//      
//      - Initial implementation
//        Bill Lamey, 7/20/2005
//
//************************************************************************
//! Terminate the metadata library.
/*!
        This function terminates the usage of the metadata library by calling
        the termination routine of the xerces sub-system.

\return
    \li 0 if the function is successful, non-zero if the function fails.
*/
//************************************************************************
bagError bagTermMetadata()
{
    XMLPlatformUtils::Terminate();
    return 0;
}

//************************************************************************
//      Method name:    bagGetMetadataBuffer()
//
//      - Initial implementation
//        Mike Van Duzee, 1/26/2012
//
//************************************************************************
//! Retrieve the bagMetaData from the input XML buffer.
/*!
\param buffer
    \li The character buffer to be parsed.
\param bufferSize
    \li The size of the input buffer.
\param doValidation
    \li true if the metadata should be validated against the schema,
    false for no validation.
\param error
    \li Modified to contain a non-zero value if the funtion fails, modified
    to contain zero if the function succeeds.
\return
    \li A valid bagMetaData handle on success, NULL on failure.  If the method
        fails, the value of error will be non-zero.
*/
//************************************************************************
bagMetaData bagGetMetadataBuffer(char *buffer, s32 bufferSize, Bool doValidation, bagError *error)
{
    // generate the input source.
    const char *bufferid = "BAG Meta Data";
    MemBufInputSource memSource((const XMLByte *)buffer, bufferSize, bufferid);

    // call the validation routine.
    return bagGetMetadata(memSource, doValidation, error);
}

//************************************************************************
//      Method name:    bagGetMetadataFile()
//
//      - Initial implementation
//        Mike Van Duzee, 1/26/2012
//
//************************************************************************
//! Retrieve the bagMetaData from the input file.
/*!
\param fileName
    \li The full path and file name of the file to be parsed.
\param doValidation
    \li true if the metadata should be validated against the schema,
    false for no validation.
\param error
    \li Modified to contain a non-zero value if the funtion fails, modified
    to contain zero if the function succeeds.
\return
    \li A valid bagMetaData handle on success, NULL on failure.  If the method
        fails, the value of error will be non-zero.
*/
//************************************************************************
bagMetaData bagGetMetadataFile(char *fileName, Bool doValidation, bagError *error)
{
    // generate the input source.
    asciiString fName(fileName);
    LocalFileInputSource  fileSource(fName);

    // call the validation routine.
    return bagGetMetadata(fileSource, doValidation, error);
}

//************************************************************************
//      Method name:    bagGetCellDims()
//
//      
//      - Initial implementation
//        Bill Lamey, 7/19/2005
//
//************************************************************************
//! Get the cell dimensions from the metadata.
/*!
\param metaData
    \li The input meta data handle.
\param version  
    \li The bag dataset version number.
\param nRows
    \li Modified to contain the number of rows in the metadata.
\param nCols
    \li Modified to contain the number of columns in the metadata.
\return
    \li 0 if the function is successful, non-zero if the function fails.
*/
//************************************************************************
bagError bagGetCellDims(
    bagMetaData metaData,
    const char *version,
    u32 *nRows,
    u32 *nCols
    )
{
    if (metaData == NULL)
        return BAG_METADTA_INVALID_HANDLE;

    //Figure out what version of metadata we have.
    const int metaVer = getMetadataVersion(version);

    // get the number of dimensions.
    const std::string dimensionStr = (metaVer == 1) ?
        "smXML:MD_Metadata/spatialRepresentationInfo/smXML:MD_Georectified/numberOfDimensions" :
        "gmi:MI_Metadata/gmd:spatialRepresentationInfo/gmd:MD_Georectified/gmd:numberOfDimensions/gco:Integer";

    DOMNode *pDimNode = bagGetXMLNodeByName(metaData->parser->getDocument(), dimensionStr.c_str());
    if (pDimNode == NULL)
        return BAG_METADTA_INVLID_DIMENSIONS;

    const std::string value = getValueFromNode(*pDimNode);
    const s32 numDims = atoi(value.c_str());

    // validate the number of dimensions.
    if (numDims != 2)
        return BAG_METADTA_INVLID_DIMENSIONS;

    *nRows = 0;
    *nCols = 0;

    const char rowStr[] = "row";
    const char colStr[] = "column";

    const asciiString dimStr = (metaVer == 1) ?
        "smXML:MD_Dimension" : "gmd:MD_Dimension";

    const std::string dimNameStr = (metaVer == 1) ?
        "dimensionName" : "gmd:dimensionName/gmd:MD_DimensionNameTypeCode";

    const std::string dimSizeStr = (metaVer == 1) ?
        "dimensionSize" : "gmd:dimensionSize/gco:Integer";

    //Find all of the dimension nodes.
    DOMNodeList *pNodeList = metaData->parser->getDocument()->getElementsByTagName(dimStr);
    for (XMLSize_t i = 0; i < pNodeList->getLength(); i++)
    {
        DOMNode *pNode = pNodeList->item(i);
        if (pNode == NULL)
            return BAG_METADTA_INVLID_DIMENSIONS;

        //Get the dimension name node.
        DOMNode *pNameNode = bagGetXMLNodeByName(pNode, dimNameStr.c_str());
        if (pNameNode == NULL)
            return BAG_METADTA_INVLID_DIMENSIONS;

        //Get the dimension size node.
        DOMNode *pSizeNode = bagGetXMLNodeByName(pNode, dimSizeStr.c_str());
        if (pSizeNode == NULL)
            return BAG_METADTA_INVLID_DIMENSIONS;

        const std::string name = getValueFromNode(*pNameNode);
        const std::string size = getValueFromNode(*pSizeNode);

        if (strcmp(name.c_str(), rowStr) == 0)
            *nRows = atoi(size.c_str());
        else if (strcmp(name.c_str(), colStr) == 0)
            *nCols = atoi(size.c_str());
    }

    return 0;
}

//************************************************************************
//      Method name:    bagGetGeoCover()
//
//      
//      - Initial implementation
//        Bill Lamey, 7/19/2005
//
//************************************************************************
//! Get the geographic cover of the BAG stored in the metadata.
/*!
\param metaData
    \li The handle to the metadata.
\param llx
    \li Modified to contain western most longitude on success, if the function
    fails the contents are unmodified.
\param lly
    \li Modified to contain southern most latitiude on success, if the function
    fails the contents are unmodified.
\param urx
    \li Modified to contain eastern most longitude on success, if the function
    fails the contents are unmodified.
\param ury
    \li Modified to contain northern most latitude on success, if the function
    fails the contents are unmodified.
\return
    \li 0 if the function is successfull, non-zero if the function fails.
*/
//************************************************************************
bagError bagGetGeoCover(bagMetaData metaData, f64 *llLat, f64 *llLong, f64 *urLat, f64 *urLong)
{
    if (metaData == NULL)
        return BAG_METADTA_INVALID_HANDLE;


    f64 latS = DBL_MAX, latN = DBL_MAX, lngE = DBL_MAX, lngW = DBL_MAX;
    const char * pGeoBox = "smXML:MD_Metadata/identificationInfo/smXML:BAG_DataIdentification/extent/smXML:EX_Extent/geographicElement/smXML:EX_GeographicBoundingBox";

    // define the needed strings.
    const asciiString pllx("westBoundLongitude");
    const asciiString purx("eastBoundLongitude");
    const asciiString plly("southBoundLatitude");
    const asciiString pury("northBoundLatitude");

    // get the nodes in turn.
    DOMNode *pGeoNode = bagGetXMLNodeByName(metaData->parser->getDocument(), pGeoBox);
    if (pGeoNode == NULL)
        return BAG_METADTA_INCOMPLETE_COVER;

    DOMNodeList *pChild = pGeoNode->getChildNodes();

    // get the root node.
    for (u32 i = 0; i < pChild->getLength(); i++)
    {
        DOMNode *pGeoNode = pChild->item(i);
        if (pGeoNode)
        {
            if (XMLString::compareString(pGeoNode->getNodeName(), pllx) == 0)
            {
                const std::string value = getValueFromNode(*pGeoNode);
                lngW = atof(value.c_str());
            }
            else if (XMLString::compareString(pGeoNode->getNodeName(), plly) == 0)
            {
                const std::string value = getValueFromNode(*pGeoNode);
                latS = atof(value.c_str());
            }
            else if (XMLString::compareString(pGeoNode->getNodeName(), purx) == 0)
            {
                const std::string value = getValueFromNode(*pGeoNode);
                lngE = atof(value.c_str());
            }
            else if (XMLString::compareString(pGeoNode->getNodeName(), pury) == 0)
            {
                const std::string value = getValueFromNode(*pGeoNode);
                latN = atof(value.c_str());
            }
        }
    }

    // make sure that each element was set correctly.
    if (latS == DBL_MAX || latN == DBL_MAX || lngE == DBL_MAX || lngW == DBL_MAX)
        return BAG_METADTA_INCOMPLETE_COVER;

    // copy the extracted values.
    *llLat = latS;
    *llLong = lngW;
    *urLat = latN;
    *urLong = lngE;

    return 0;
}

//************************************************************************
//      Method name:    bagGetProjectedCover()
//
//      
//      - Initial implementation
//        Bill Lamey, 7/20/2005
//
//************************************************************************
//! Get the projected (ground) extents of the BAG.
/*!
\param metaData
    \li The input meta data handle.
\param version  
    \li The bag dataset version number.
\param llx
    \li Modified to contain the lower left X coordinate in projected units.
\param lly
    \li Modified to contain the lower left Y coordinate in projected units.
\param urx
    \li Modified to contain the upper right X coordinate in projected units.
\param ury
    \li Modified to contain the upper right Y coordinate in projected units.
\return
    \li 0 if the function is successful, non-zero if the function fails.
*/
//************************************************************************
bagError bagGetProjectedCover(
    bagMetaData metaData,
    const char *version,
    f64 *llx,
    f64 *lly,
    f64 *urx,
    f64 *ury
    )
{
    if (metaData == NULL)
        return BAG_METADTA_INVALID_HANDLE;

    //Figure out what version of metadata we have.
    const int metaVer = getMetadataVersion(version);

    f64 lx = DBL_MAX, ly = DBL_MAX, rx = DBL_MAX, ry = DBL_MAX;

    const std::string geoBoxStr = (metaVer == 1) ?
        "smXML:MD_Metadata/spatialRepresentationInfo/smXML:MD_Georectified/cornerPoints/gml:Point/gml:coordinates" :
        "gmi:MI_Metadata/gmd:spatialRepresentationInfo/gmd:MD_Georectified/gmd:cornerPoints/gml:Point/gml:coordinates";

    DOMNode *pGMLCoord = bagGetXMLNodeByName(metaData->parser->getDocument(), geoBoxStr.c_str());
    if (pGMLCoord == NULL)
        return BAG_METADTA_INCOMPLETE_COVER;

    const std::string value = getValueFromNode(*pGMLCoord);

    // the string is 2 item tuples seperated by spaces.
    lx = atof(value.c_str());
    const char *tmpPos = strchr(value.c_str(), ',');
    ly = atof(tmpPos+1);
    tmpPos = strchr(tmpPos+1, ' ');
    rx = atof(tmpPos+1);
    tmpPos = strchr(tmpPos, ',');
    ry = atof(tmpPos+1);

    if (lx == DBL_MAX || ly == DBL_MAX || rx == DBL_MAX || ry == DBL_MAX)
        return BAG_METADTA_INCOMPLETE_COVER;

    *llx = lx;
    *lly = ly;
    *urx = rx;
    *ury = ry;

    return 0;
}

//************************************************************************
//      Method name:    bagGetGridSpacing()
//
//      
//      - Initial implementation
//        Bill Lamey, 7/19/2005
//
//************************************************************************
//! Get the grid spacings from the metadata.
/*!
\param metaData
    \li The handle to the metadata.
\param version  
    \li The bag dataset version number.
\param dx
    \li Modified to contain the node spacing along the X axis.
\param dy
    \li Modified to contain the node spacing along the Y axis.
\return
    \li 0 if the function is successfull, non-zero if the function fails.
*/
//************************************************************************
bagError bagGetGridSpacing(
    bagMetaData metaData,
    const char *version,
    f64 *dx,
    f64 *dy
    )
{
    if (metaData == NULL)
        return BAG_METADTA_INVALID_HANDLE;

    *dx = 0.0;
    *dy = 0.0;

    //Figure out what version of metadata we have.
    const int metaVer = getMetadataVersion(version);

    const char rowStr[] = "row";
    const char colStr[] = "column";

    const asciiString dimStr = (metaVer == 1) ?
        "smXML:MD_Dimension" : "gmd:MD_Dimension";

    const std::string dimNameStr = (metaVer == 1) ?
        "dimensionName" : "gmd:dimensionName/gmd:MD_DimensionNameTypeCode";

    const std::string resolutionStr = (metaVer == 1) ?
        "resolution/smXML:Measure/smXML:value" : "gmd:resolution/gco:Measure";

    //Find all of the dimension nodes.
    DOMNodeList *pNodeList = metaData->parser->getDocument()->getElementsByTagName(dimStr);
    for (XMLSize_t i = 0; i < pNodeList->getLength(); i++)
    {
        DOMNode *pNode = pNodeList->item(i);
        if (pNode == NULL)
            return BAG_METADTA_RESOLUTION_MISSING;

        //Get the dimension name node.
        DOMNode *pNameNode = bagGetXMLNodeByName(pNode, dimNameStr.c_str());
        if (pNameNode == NULL)
            return BAG_METADTA_RESOLUTION_MISSING;

        //Get the resolution node.
        DOMNode *pResNode = bagGetXMLNodeByName(pNode, resolutionStr.c_str());
        if (pResNode == NULL)
            return BAG_METADTA_RESOLUTION_MISSING;

        const std::string name = getValueFromNode(*pNameNode);
        const std::string resolution = getValueFromNode(*pResNode);

        if (strcmp(name.c_str(), rowStr) == 0)
            *dy = atof(resolution.c_str());
        else if (strcmp(name.c_str(), colStr) == 0)
            *dx = atof(resolution.c_str());
    }

    if (*dx == 0.0 || *dy == 0.0)
        return BAG_METADTA_RESOLUTION_MISSING;

    return 0;
}

//************************************************************************
//      Method name:    bagGetXMLBuffer()
//
//      
//      - Initial implementation
//        Bill Lamey, 7/20/2005
//
//************************************************************************
//! Get the XML stream as a raw character buffer.
/*!
\param metaData
    \li The input meta data handle.
\param buffer
    \li The buffer to recieve the XML data.
\param maxBufferSize
    \li The maximum size of buffer.  Will be modified to contain the number
    of characters actually written to buffer.
\return
    \li 0 if the function is successful, non-zero if the function fails.
*/
//************************************************************************
bagError bagGetXMLBuffer(
    bagMetaData metaData,
    char *buffer,
    u32 *maxBufferSize
    )
{
    if (metaData == NULL)
        return BAG_METADTA_INVALID_HANDLE;

    // get a serializer, an instance of DOMWriter
    XMLCh tempStr[100];
    XMLString::transcode("LS", tempStr, 99);
    DOMImplementation *impl          = DOMImplementationRegistry::getDOMImplementation(tempStr);
    DOMWriter         *theSerializer = ((DOMImplementationLS*)impl)->createDOMWriter();

    // Create the memory target.
    MemBufFormatTarget memTarget(4096);  

    // get the DOM representation
    DOMNode *doc = metaData->parser->getDocument();

    // do the serialization through DOMWriter::writeNode();
    theSerializer->writeNode(&memTarget, *doc);

    // delete the serializer.
    delete theSerializer;

    if (*maxBufferSize <= memTarget.getLen())
        return BAG_METADTA_INSUFFICIENT_BUFFER;

    // return the number of bytes written into the buffer.
    *maxBufferSize = memTarget.getLen();

    // extract the contents of the memory format target into the user supplied buffer.
    memcpy(buffer, memTarget.getRawBuffer(), sizeof(XMLByte) * memTarget.getLen());
    return 0;
}

//************************************************************************
//      Method name:    bagGetVertDatum()
//
//      
//      - Initial implementation
//        Webb McDonald -- Fri Jul 30 12:01:41 2010
//
//************************************************************************
//! Get the vertical datum identifier from the meta data.
/*!
\param metaData
    \li The meta data to be searched.
\param buffer
    \li Modified to contain the horizontal datum id from the meta data.
\param bufferSize
    \li The maximum size of buffer.
\return
    \li 0 if the function succeeds, non-zero if the function fails.
*/
//************************************************************************
bagError bagGetVertDatum(
    bagMetaData metaData,
    char *buffer,
    u32 bufferSize
    )
{
    buffer[0] = '\0';

    //First try the custom vertical datum location.
    const char * pEllipIdLocation = "smXML:MD_Metadata/referenceSystemInfo/smXML:MD_CRS/verticalDatum/smXML:RS_Identifier/code";

    DOMNode *pEllipIdNode = bagGetXMLNodeByName(metaData->parser->getDocument(), pEllipIdLocation);
    if (pEllipIdNode)
    {
        std::string vDatumName = getValueFromNode(*pEllipIdNode);
        if (vDatumName.size() > bufferSize)
            vDatumName.resize(bufferSize);

        strcpy(buffer, vDatumName.c_str());
    }

    //If we did not find it, then try the more standard location.
    if (strlen(buffer) == 0)
    {
        const char codeLoc[] = "datum/smXML:RS_Identifier/code";

        DOMNodeList* nodeList = metaData->parser->getDocument()->getElementsByTagName((const asciiString)"smXML:MD_CRS");
        if (nodeList == NULL)
            return BAG_SUCCESS;

        //The first reference system node will be horizontal, and the second will be vertical.
        //So if we don't have 2, we must not have a vertical system.
        if (nodeList->getLength() < 2)
            return BAG_SUCCESS;

        DOMNode *pVerticalNode = nodeList->item(1);
        if (pVerticalNode == NULL)
            return BAG_SUCCESS;

        //Find the code node.
        DOMNode *pCodeNode = bagGetXMLNodeByName(pVerticalNode, codeLoc);
        if (pCodeNode == NULL)
            return BAG_SUCCESS;

        //Get the vertical datum name from the node.
        std::string vDatumName = getValueFromNode(*pCodeNode);
        if (vDatumName.size() > bufferSize)
            vDatumName.resize(bufferSize);

        strcpy(buffer, vDatumName.c_str());
    }

    return BAG_SUCCESS;
}

//************************************************************************
//      Method name:    bagGetUncertaintyType()
//
//      
//      - Initial implementation
//        Bill Lamey, 20/01/2006
//
//************************************************************************
//! Get the type of uncertainty represented by the Uncertainty layer in the BAG file.
/*!
\param metaData
    \li The handle to the meta data structure.
\param version  
    \li The bag dataset version number.
\param uncrtType
    \li Modified to contain the type of uncertainty represented in this BAG.
    See BAG_UNCERT_TYPE in bag.h for a complete listing.
\return
    \li Error code.
*/
//************************************************************************
bagError bagGetUncertaintyType(
    bagMetaData metaData,
    const char *version,
    u32 *uncrtType
    )
{
    *uncrtType = Unknown_Uncert;

    //Figure out what version of metadata we have.
    const int metaVer = getMetadataVersion(version);

    const std::string uncertStr = (metaVer == 1) ? 
        "smXML:MD_Metadata/identificationInfo/smXML:BAG_DataIdentification/verticalUncertaintyType" :
        "gmi:MI_Metadata/gmd:identificationInfo/bag:BAG_DataIdentification/bag:verticalUncertaintyType/bag:BAG_VertUncertCode";

    DOMNode *pUncrtNode = bagGetXMLNodeByName(metaData->parser->getDocument(), uncertStr.c_str());
    if (pUncrtNode == NULL)
        return BAG_METADTA_UNCRT_MISSING;

    std::string value = getValueFromNode(*pUncrtNode);
    if (value.empty())
        return BAG_METADTA_UNCRT_MISSING;

    std::transform(value.begin(), value.end(), value.begin(), ::tolower);

    if (!strcmp(value.c_str(), "raw std dev") || !strcmp(value.c_str(), "rawstddev"))
        *uncrtType = Raw_Std_Dev;
    else if (!strcmp(value.c_str(), "cube std dev") || !strcmp(value.c_str(), "cubestddev"))
        *uncrtType = CUBE_Std_Dev;
    else if (!strcmp(value.c_str(), "product uncert") || !strcmp(value.c_str(), "productuncert"))
        *uncrtType = Product_Uncert;
    else if (!strcmp(value.c_str(), "average tpe") || !strcmp(value.c_str(), "averagetpe"))
        *uncrtType = Average_TPE;
    else if (!strcmp(value.c_str(), "historical std dev") || !strcmp(value.c_str(), "historicalstddev"))
        *uncrtType = Historical_Std_Dev;

    return 0;
}

//************************************************************************
//      Method name:    bagGetDepthCorrectionType()
//
//      
//      - Initial implementation
//        NAVOCEANO, 06/06/2008
//
//************************************************************************
//! Get the type of depth correction type represented by the depth layer in the BAG file.
/*!
\param metaData
    \li The handle to the meta data structure.
\param version  
    \li The bag dataset version number.
\param depthCorrectionType
    \li Modified to contain the type of depth correction represented in this BAG.
    See BAG_DEPTH_CORRECTION_TYPES in bag.h for a complete listing.
\return
    \li Error code.
*/
//************************************************************************
bagError bagGetDepthCorrectionType(
    bagMetaData metaData,
    const char *version,
    u32 *depthCorrectionType
    )
{
    *depthCorrectionType = (unsigned int) NULL_GENERIC;

    //Figure out what version of metadata we have.
    const int metaVer = getMetadataVersion(version);

    const std::string depthCorrectStr = (metaVer == 1) ? 
        "smXML:MD_Metadata/identificationInfo/smXML:BAG_DataIdentification/depthCorrectionType" :
        "gmi:MI_Metadata/gmd:identificationInfo/bag:BAG_DataIdentification/bag:depthCorrectionType/bag:BAG_DepthCorrectCode";

    DOMNode *pDepthCorrectionNode = bagGetXMLNodeByName(metaData->parser->getDocument(), depthCorrectStr.c_str());
    if (pDepthCorrectionNode == NULL)
        return BAG_METADTA_DPTHCORR_MISSING;

    std::string value = getValueFromNode(*pDepthCorrectionNode);
    std::transform(value.begin(), value.end(), value.begin(), ::tolower);

    if (!strcmp(value.c_str(), "true depth") || !strcmp(value.c_str(), "truedepth"))
        *depthCorrectionType = True_Depth;
    else if (!strcmp(value.c_str(), "nominal at 1500 m/s") || !strcmp(value.c_str(), "nominaldepthmetre"))
        *depthCorrectionType = Nominal_Depth_Meters;
    else if (!strcmp(value.c_str(), "nominal at 4800 ft/s") || !strcmp(value.c_str(), "nominaldepthfeet"))
        *depthCorrectionType = Nominal_Depth_Feet;
    else if (!strcmp(value.c_str(), "corrected via carter's tables") || !strcmp(value.c_str(), "correctedcarters"))
        *depthCorrectionType = Corrected_Carters;
    else if (!strcmp(value.c_str(), "corrected via matthew's tables") || !strcmp(value.c_str(), "correctedmatthews"))
        *depthCorrectionType = Corrected_Matthews;
    else if (!strcmp(value.c_str(), "unknown"))
        *depthCorrectionType = Unknown_Correction;
            
    return 0;
}

//************************************************************************
//      Method name:    bagGetProjectionParams()
//
//      
//      - Initial implementation
//        Bill Lamey, 7/20/2005
//
//************************************************************************
//! Get the projection parameters from the metadata.
/*!
        This function will retrieve all of the projection information stored in
        the metadata.

\param metaData
    \li The input metadata hanlde to extract the information from.
\param projId
    \li Modified to contain the projection identifier.  Buffer must be at least 5 characters long.  May not be NULL.
\param projIdLen
    \li The size of the projId buffer.
\param zone
    \li Modified to contain the UTM zone number if applicable.  May not be NULL.
\param standrardParallel
    \li 
\param standrardParallel2
    \li
\param centralMeridian
    \li 
\param latitudeOfOrigin
    \li 
\param falseEasting
    \li 
\param falseNorthing
    \li 
\param scaleFactAtEq
    \li 
\param heightOfPersPoint
    \li 
\param longOfProjCenter
    \li 
\param latOfProjCenter
    \li 
\param scaleAtCenterLine
    \li 
\param vertLongFromPole
    \li 
\param scaleAtProjOrigin
    \li 
\return
    \li 0 if the function is successful, non-zero if the function fails.
*/
//************************************************************************
bagError bagGetProjectionParams(
    bagMetaData metaData,
    char *projId,
    size_t projIdLen,
    s32 *zone,
    f64 *standrardParallel,
    f64 *standrardParallel2,
    f64 *centralMeridian,
    f64 *latitudeOfOrigin,
    f64 *falseEasting,
    f64 *falseNorthing,
    f64 *scaleFactAtEq,
    f64 *heightOfPersPoint,
    f64 *longOfProjCenter,
    f64 *latOfProjCenter,
    f64 *scaleAtCenterLine,
    f64 *vertLongFromPole,
    f64 *scaleAtProjOrigin
    )
{
    if (metaData == NULL)
        return BAG_METADTA_INVALID_HANDLE;

    const char * pProjIdLocation = "smXML:MD_Metadata/referenceSystemInfo/smXML:MD_CRS/projection/smXML:RS_Identifier/code";
    const char * pProjLocation = "smXML:MD_Metadata/referenceSystemInfo/smXML:MD_CRS/projectionParameters/smXML:MD_ProjectionParameters";

    // Needed strings.
    const asciiString zoneStr("zone");
    const asciiString stdParStr("standardParallel");
    const asciiString centerMerStr("longitudeOfCentralMeridian");
    const asciiString latProjOrigStr("latitudeOfProjectionOrigin");
    const asciiString falseEastStr("falseEasting");
    const asciiString falseNorthStr("falseNorthing");
    const asciiString scaleFactAtEqStr("scaleFactorAtEquator");
    const asciiString heightOfPersStr("heightOfProspectivePointAboveSurface");
    const asciiString longProjCenterStr("longitudeOfProjectionCenter");
    const asciiString latProjCenterStr("latitudeOfProjectionCenter");
    const asciiString scaleFactAtCenterLineStr("scaleFactorAtCenterLine");
    const asciiString stVertLongFromPoleStr("straightVerticalLongitudeFromPole");
    const asciiString scaleFactAtProjOrigStr("scaleFactorAtProjectionOrigin");

    bool valuesFound = false;

    DOMNode *pProjIdNode = bagGetXMLNodeByName(metaData->parser->getDocument(), pProjIdLocation);
    if (pProjIdNode)
    {
        std::string value = getValueFromNode(*pProjIdNode);
        if (value.size() > projIdLen)
            value.resize(projIdLen);

        strcpy(projId, value.c_str());
        valuesFound = true;
    }

    // get all of the projection parameters.
    DOMNode *pProjNode = bagGetXMLNodeByName(metaData->parser->getDocument(), pProjLocation);
    if (pProjNode)
    {
        DOMNodeList *pParamList = pProjNode->getChildNodes();
        for (u32 j = 0; j < pParamList->getLength(); j++)
        {
            // check the string.
            DOMNode *pParamNode = pParamList->item(j);

            const XMLCh *pStr = pParamNode->getNodeName();

            if (XMLString::compareString(pStr, zoneStr) == 0)
            {
                std::string value = getValueFromNode(*pParamNode);
                *zone = atof(value.c_str());
                valuesFound = true;
            }
            else if (XMLString::compareString(pStr, stdParStr) == 0)
            {
                // may be 1 or 2 children for this value.
                DOMNode *pTmpNode = pParamNode->getFirstChild();
                if (pTmpNode)
                {
                    char *pTmpStr = XMLString::transcode(pTmpNode->getNodeValue());
                    *standrardParallel = atof(pTmpStr);
                    XMLString::release(&pTmpStr);
                    valuesFound = true;

                    DOMNode *pTmpNode2 = pTmpNode->getNextSibling();
                    if (pTmpNode2 != NULL)
                    {
                        char *pTmpStr2 = XMLString::transcode(pTmpNode2->getNodeValue());
                        *standrardParallel2 = atof(pTmpStr2);
                        XMLString::release(&pTmpStr2);
                    }
                }
            }
            else if (XMLString::compareString(pStr, centerMerStr) == 0)
            {
                std::string value = getValueFromNode(*pParamNode);
                *centralMeridian = atof(value.c_str());
                valuesFound = true;
            }
            else if (XMLString::compareString(pStr, latProjOrigStr) == 0)
            {
                std::string value = getValueFromNode(*pParamNode);
                *latitudeOfOrigin = atof(value.c_str());
                valuesFound = true;
            }
            else if (XMLString::compareString(pStr, falseEastStr) == 0)
            {
                std::string value = getValueFromNode(*pParamNode);
                *falseEasting = atof(value.c_str());
                valuesFound = true;
            }
            else if (XMLString::compareString(pStr, falseNorthStr) == 0)
            {
                std::string value = getValueFromNode(*pParamNode);
                *falseNorthing = atof(value.c_str());
                valuesFound = true;
            }
            else if (XMLString::compareString(pStr, scaleFactAtEqStr) == 0)
            {
                std::string value = getValueFromNode(*pParamNode);
                *scaleFactAtEq = atof(value.c_str());
                valuesFound = true;
            }
            else if (XMLString::compareString(pStr, heightOfPersStr) == 0)
            {
                std::string value = getValueFromNode(*pParamNode);
                *heightOfPersPoint = atof(value.c_str());
                valuesFound = true;
            }
            else if (XMLString::compareString(pStr, longProjCenterStr) == 0)
            {
                std::string value = getValueFromNode(*pParamNode);
                *longOfProjCenter = atof(value.c_str());
                valuesFound = true;
            }
            else if (XMLString::compareString(pStr, latProjCenterStr) == 0)
            {
                std::string value = getValueFromNode(*pParamNode);
                *latOfProjCenter = atof(value.c_str());
                valuesFound = true;
            }
            else if (XMLString::compareString(pStr, scaleFactAtCenterLineStr) == 0)
            {
                std::string value = getValueFromNode(*pParamNode);
                *scaleAtCenterLine = atof(value.c_str());
                valuesFound = true;
            }
            else if (XMLString::compareString(pStr, stVertLongFromPoleStr) == 0)
            {
                std::string value = getValueFromNode(*pParamNode);
                *vertLongFromPole = atof(value.c_str());
                valuesFound = true;
            }
            else if (XMLString::compareString(pStr, scaleFactAtProjOrigStr) == 0)
            {
                std::string value = getValueFromNode(*pParamNode);
                *scaleAtProjOrigin = atof(value.c_str());
                valuesFound = true;
            }
        }
    }

    bagError error = 0;
    if (!valuesFound)
        error = BAG_METADTA_NO_PROJECTION_INFO;

    return error;
}

//************************************************************************
//      Method name:    bagGetHorizDatum()
//
//      
//      - Initial implementation
//        Bill Lamey, 7/21/2005
//
//************************************************************************
//! Get the horizontal datum identifier from the meta data.
/*!
\param metaData
    \li The meta data to be searched.
\param buffer
    \li Modified to contain the horizontal datum id from the meta data.
\param bufferSize
    \li The maximum size of buffer.
\return
    \li 0 if the function succeeds, non-zero if the function fails.
*/
//************************************************************************
bagError bagGetHorizDatum(
    bagMetaData metaData,
    char *buffer,
    u32 bufferSize
    )
{
    const char * pEllipIdLocation = "smXML:MD_Metadata/referenceSystemInfo/smXML:MD_CRS/datum/smXML:RS_Identifier/code";

    DOMNode *pEllipIdNode = bagGetXMLNodeByName(metaData->parser->getDocument(), pEllipIdLocation);
    if (pEllipIdNode)
    {
        std::string value = getValueFromNode(*pEllipIdNode);
        if (value.size() > bufferSize)
            value.resize(bufferSize);

        strcpy(buffer, value.c_str());
    }

    return 0;
}

//************************************************************************
//      Method name:    bagGetEllipsoid()
//
//      
//      - Initial implementation
//        Webb McDonald -- Fri Jul 30 12:01:41 2010
//
//************************************************************************
//! Get the ellipsoid identifier from the meta data.
/*!
\param metaData
    \li The meta data to be searched.
\param buffer
    \li Modified to contain the horizontal datum id from the meta data.
\param bufferSize
    \li The maximum size of buffer.
\return
    \li 0 if the function succeeds, non-zero if the function fails.
*/
//************************************************************************
bagError bagGetEllipsoid(
    bagMetaData metaData,
    char *buffer,
    u32 bufferSize
    )
{
    const char * pEllipIdLocation = "smXML:MD_Metadata/referenceSystemInfo/smXML:MD_CRS/ellipsoid/smXML:RS_Identifier/code";

    DOMNode *pEllipIdNode = bagGetXMLNodeByName(metaData->parser->getDocument(), pEllipIdLocation);
    if (pEllipIdNode)
    {
        std::string value = getValueFromNode(*pEllipIdNode);
        if (value.size() > bufferSize)
            value.resize(bufferSize);

        strcpy(buffer, value.c_str());
    }

    return 0;
}

//************************************************************************
//      Method name:    bagGetNodeGroupType()
//
//      
//      - Initial implementation
//        Webb McDonald -- Tue Dec 20 14:21:02 2011
//
//************************************************************************
//! Get the type of node group type in the BAG file.
/*!
\param metaData
    \li The handle to the meta data structure.
\param nodeGroupType
    \li Modified to contain the type of node group in this BAG.
    See BAG_OPT_GROUP_TYPES in bag.h for a complete listing.
\return
    \li Error code.
*/
//************************************************************************
bagError bagGetNodeGroupType(
    bagMetaData metaData,
    u8 *nodeGroupType
    )
{
    *nodeGroupType = (u8) 0;

    const char * pNodeType = "gmi:MI_Metadata/gmd:identificationInfo/bag:BAG_DataIdentification/bag:nodeGroupType/bag:BAG_OptGroupCode";
    DOMNode *pNodeGroupNode = bagGetXMLNodeByName(metaData->parser->getDocument(), pNodeType);
    
    if (pNodeGroupNode != NULL)
    {
        std::string value = getValueFromNode(*pNodeGroupNode);
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
    
        if (!strcmp(value.c_str(), "cube"))
            *nodeGroupType = CUBE_Solution;
        else if (!strcmp(value.c_str(), "product"))
            *nodeGroupType = Product_Solution;
        else if (!strcmp(value.c_str(), "average"))
            *nodeGroupType = Average_TPE_Solution;
        else
            *nodeGroupType = Unknown_Solution;
    }
    
    return BAG_SUCCESS;
}

//************************************************************************
//      Method name:    bagGetElevationSolutionType()
//
//      
//      - Initial implementation
//        Webb McDonald -- Tue Dec 20 14:21:02 2011
//
//************************************************************************
//! Get the type of elevation solution group type in the BAG file.
/*!
\param metaData
    \li The handle to the meta data structure.
\param nodeGroupType
    \li Modified to contain the type of elevation solution group in this BAG.
    See BAG_OPT_GROUP_TYPES in bag.h for a complete listing.
\return
    \li Error code.
*/
//************************************************************************
bagError bagGetElevationSolutionType(
    bagMetaData metaData,
    u8 *nodeGroupType
    )
{
    *nodeGroupType = (u8) 0;

    const char * pNodeType = "gmi:MI_Metadata/gmd:identificationInfo/bag:BAG_DataIdentification/bag:elevationSolutionGroupType/bag:BAG_OptGroupCode";
    DOMNode *pElevationSolutionNode = bagGetXMLNodeByName(metaData->parser->getDocument(), pNodeType);
    
    if (pElevationSolutionNode != NULL)
    {
        std::string value = getValueFromNode(*pElevationSolutionNode);
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
    
        if (!strcmp(value.c_str(), "cube"))
            *nodeGroupType = CUBE_Solution;
        else if (!strcmp(value.c_str(), "product"))
            *nodeGroupType = Product_Solution;
        else if (!strcmp(value.c_str(), "average"))
            *nodeGroupType = Average_TPE_Solution;
        else
            *nodeGroupType = Unknown_Solution;
    }
    
    return BAG_SUCCESS;
}

//******************************************************************************
//      Method name:    bagHGetReferenceSystemV2()
//
//      - Initial implementation
//        Mike Van Duzee, 1/26/2012
//
//******************************************************************************
//! Retrieve the BAG's version 2 horizontal reference system. 
/*!
    The version 2 reference system is either an EPSG number or WKT (Well Known Text)
    definition.

    The output buffer will contain either a WKT definition, or an EPSG number. 
    If the output is EPSG the buffer will be in the following format:
    "EPSG:<number>"

\param metaData
    \li The handle to the meta data structure.
\param buffer
    \li Modified to contain the reference's system definition.
\param bufferSize
    \li The size of the	definition buffer passed in.
\return
    \li On success, a value of zero is returned. On failure an error code is returned.
*/
//******************************************************************************
bagError bagGetHReferenceSystemV2(
    bagMetaData metaData,
    char *buffer,
    u32 bufferSize
    )
{
    const char referenceLoc[] = "gmi:MI_Metadata/gmd:referenceSystemInfo/gmd:MD_ReferenceSystem/gmd:referenceSystemIdentifier/gmd:RS_Identifier";
    const char codeLoc[] = "gmd:code/gco:CharacterString";
    const char codePageLoc[] = "gmd:codeSpace/gco:CharacterString";

    //Find the reference node.
    DOMNode *pRefNode = bagGetXMLNodeByName(metaData->parser->getDocument(), referenceLoc);
    if (pRefNode == NULL)
        return BAG_METADTA_INVALID_HREF;

    //Find the code node.
    DOMNode *pCodeNode = bagGetXMLNodeByName(pRefNode, codeLoc);
    if (pCodeNode == NULL)
        return BAG_METADTA_INVALID_HREF;

    //Find the code page node.
    DOMNode *pCodePageNode = bagGetXMLNodeByName(pRefNode, codePageLoc);
    if (pCodePageNode == NULL)
        return BAG_METADTA_INVALID_HREF;

    //Get the code and code page values.
    std::string codeValue = getValueFromNode(*pCodeNode);
    std::string codePageValue = getValueFromNode(*pCodePageNode);

    //Make sure the codepage is all upper case.
    std::transform(codePageValue.begin(), codePageValue.end(), codePageValue.begin(), ::toupper);

    std::string defString;

    //If the code page is WKT then...
    if (strcmp(codePageValue.c_str(), "WKT") != 0)
    {
        defString = codePageValue;
        defString += ":";
    }

    defString += codeValue;

    //Make sure our string is not too large.
    if (defString.size() > bufferSize)
        defString.resize(bufferSize);

    strcpy(buffer, defString.c_str());

    return BAG_SUCCESS;
}

//******************************************************************************
//      Method name:    bagGetHReferenceSystemV1()
//
//      - Initial implementation
//        Mike Van Duzee, 1/26/2012
//
//******************************************************************************
//! Retrieve the BAG's version 1 horizontal reference system. 
/*!
    The version 1 reference system contains all of the individual projection
    parameters.

    The output buffer will contain a WKT definition.

\param metaData
    \li The handle to the meta data structure.
\param buffer
    \li Modified to contain the reference's system definition.
\param bufferSize
    \li The size of the	definition buffer passed in.
\return
    \li On success, a value of zero is returned. On failure an error code is returned.
*/
//******************************************************************************
bagError bagGetHReferenceSystemV1(bagMetaData metaData, char *buffer, u32 bufferSize)
{
    f64 scaleFactAtEq, scaleAtCenterLine, scaleAtProjOrigin, heightOfPersPoint;
    char projectionId[XML_ATTR_MAXSTR];
    char datumId[XML_ATTR_MAXSTR];

    bagLegacyReferenceSystem v1Def;
    memset(&v1Def, 0, sizeof(bagLegacyReferenceSystem));

    /* read the projection information */
    bagError error = bagGetProjectionParams(metaData, projectionId, XML_ATTR_MAXSTR,
        &v1Def.geoParameters.zone,
        &v1Def.geoParameters.std_parallel_1,
        &v1Def.geoParameters.std_parallel_2, 
        &v1Def.geoParameters.central_meridian,
        &v1Def.geoParameters.origin_latitude, 
        &v1Def.geoParameters.false_easting,
        &v1Def.geoParameters.false_northing,
        &scaleFactAtEq,                   /* dhf - used if Mercator coord sys */
        &heightOfPersPoint,               /* dhf - for space oblique mercator (not in bag) */
        &v1Def.geoParameters.longitude_of_centre,
        &v1Def.geoParameters.latitude_of_centre,
        &scaleAtCenterLine,               /* dhf - used if oblique mercator (not bag implemented) */
        &v1Def.geoParameters.longitude_down_from_pole,
        &scaleAtProjOrigin                /* used for polar stereographic & transverse mercator */
	);
    if (error)
        return error;

    /* retrieve the ellipsoid */
    error = bagGetEllipsoid(metaData, (char *)v1Def.geoParameters.ellipsoid, XML_ATTR_MAXSTR);
    if (error)
        return error;

    /* retrieve the horizontal datum */
    error = bagGetHorizDatum(metaData, datumId, XML_ATTR_MAXSTR);
    if (error)
        return error;

    /*convert the projection id to a supported type */
    v1Def.coordSys = bagCoordsys(projectionId);

    /* retrieve the vertical datum */
    error = bagGetEllipsoid(metaData, (char *)v1Def.geoParameters.vertical_datum, XML_ATTR_MAXSTR);
    if (error)
        return error;

    /* dhf */
    /* scaleFactAtEq - for mercator */
    /* scaleAtCenterLine - for oblique mercator (not supported) */
    /* scaleAtProjOrigin - for polar stereographic & transverse mercator */

    if ( v1Def.coordSys == Mercator )
    	v1Def.geoParameters.scale_factor = scaleFactAtEq;
    if ( v1Def.coordSys == Transverse_Mercator || v1Def.coordSys == Polar_Stereo )
    	v1Def.geoParameters.scale_factor = scaleAtProjOrigin;

    /* convert the datum type */
    v1Def.geoParameters.datum = bagDatumID(datumId);  

    error = bagLegacyToWkt(v1Def, buffer, bufferSize, NULL, 0);
    if (error)
        return error;

    return BAG_SUCCESS;
}

//******************************************************************************
//      Method name:    bagGetHReferenceSystem()
//
//      - Initial implementation
//        Mike Van Duzee, 1/26/2012
//
//******************************************************************************
//! Retrieve the BAG's horizontal reference system. 
/*!

    The output buffer will contain either a WKT definition, or an EPSG number. 
    If the output is EPSG the buffer will be in the following format:
    "EPSG:<number>"

\param metaData
    \li The handle to the meta data structure.
\param version  
    \li The bag dataset version number.
\param buffer
    \li Modified to contain the reference's system definition.
\param bufferSize
    \li The size of the	definition buffer passed in.
\return
    \li On success, a value of zero is returned. On failure an error code is returned.
*/
//******************************************************************************
bagError bagGetHReferenceSystem(
    bagMetaData metaData,
    const char *version,
    char *buffer,
    u32 bufferSize
    )
{
    bagError err = 0;

    //Figure out what version of metadata we have.
    const int metaVer = getMetadataVersion(version);

    if (metaVer == 1)
        err = bagGetHReferenceSystemV1(metaData, buffer, bufferSize);
    else
        err = bagGetHReferenceSystemV2(metaData, buffer, bufferSize);

    return err;
}

//******************************************************************************
//      Method name:    bagGetVReferenceSystemV2()
//
//      - Initial implementation
//        Mike Van Duzee, 1/26/2012
//
//******************************************************************************
//! Retrieve the BAG's version 2 vertical reference system. 
/*!
    The version 2 reference system is either an EPSG number or WKT (Well Known Text)
    definition.

    The output buffer will contain either a WKT definition, or an EPSG number. 
    If the output is EPSG the buffer will be in the following format:
    "EPSG:<number>"

\param metaData
    \li The handle to the meta data structure.
\param buffer
    \li Modified to contain the vertical reference's system definition.
\param bufferSize
    \li The size of the	definition buffer passed in.
\return
    \li On success, a value of zero is returned. On failure an error code is returned.
*/
//******************************************************************************
bagError bagGetVReferenceSystemV2(bagMetaData metaData, char *buffer, u32 bufferSize)
{
    const char codeLoc[] = "gmd:referenceSystemIdentifier/gmd:RS_Identifier/gmd:code/gco:CharacterString";
    const char codePageLoc[] = "gmd:referenceSystemIdentifier/gmd:RS_Identifier/gmd:codeSpace/gco:CharacterString";

    //Lets find the reference system nodes.
    asciiString dataIdent("gmd:MD_ReferenceSystem");
    DOMNodeList* nodeList = metaData->parser->getDocument()->getElementsByTagName(dataIdent);
    if (nodeList == NULL)
    {
        buffer[0] = '\0';
        return BAG_SUCCESS;
    }

    //The first reference system node will be horizontal, and the second will be vertical.
    //So if we don't have 2, we must not have a vertical system.
    if (nodeList->getLength() < 2)
    {
        buffer[0] = '\0';
        return BAG_SUCCESS;
    }

    DOMNode *pVerticalNode = nodeList->item(1);
    if (pVerticalNode == NULL)
    {
        buffer[0] = '\0';
        return BAG_SUCCESS;
    }

    //Find the code node.
    DOMNode *pCodeNode = bagGetXMLNodeByName(pVerticalNode, codeLoc);
    if (pCodeNode == NULL)
        return BAG_METADTA_INVALID_VREF;

    //Find the code page node.
    DOMNode *pCodePageNode = bagGetXMLNodeByName(pVerticalNode, codePageLoc);
    if (pCodePageNode == NULL)
        return BAG_METADTA_INVALID_VREF;

    //Get the code and code page values.
    std::string codeValue = getValueFromNode(*pCodeNode);
    std::string codePageValue = getValueFromNode(*pCodePageNode);

    //Make sure the codepage is all upper case.
    std::transform(codePageValue.begin(), codePageValue.end(), codePageValue.begin(), ::toupper);

    std::string defString;

    //If the code page is WKT then...
    if (strcmp(codePageValue.c_str(), "WKT") != 0)
    {
        defString = codePageValue;
        defString += ":";
    }

    defString += codeValue;

    //Make sure our string is not too large.
    if (defString.size() > bufferSize)
        defString.resize(bufferSize);

    strcpy(buffer, defString.c_str());

    return BAG_SUCCESS;
}

//******************************************************************************
//      Method name:    bagGetVReferenceSystemV1()
//
//      - Initial implementation
//        Mike Van Duzee, 1/26/2012
//
//******************************************************************************
//! Retrieve the BAG's version 1 vertical reference system. 
/*!
    The version 1 reference system contains simply contains the vertical
    datum name.

    If the vertical datum's name is "v_datum_1" the output definition
    will be in the following form:

    VERT_CS["v_datum_1",
        VERT_DATUM[v_datum_1, 2000]]

\param metaData
    \li The handle to the meta data structure.
\param buffer
    \li Modified to contain the vertical reference's system definition.
\param bufferSize
    \li The size of the	definition buffer passed in.
\return
    \li On success, a value of zero is returned. On failure an error code is returned.
*/
//******************************************************************************
bagError bagGetVReferenceSystemV1(bagMetaData metaData, char *buffer, u32 bufferSize)
{
    //First try to retrieve the custom 'verticalDatum' node value.
    bagLegacyReferenceSystem system;
    bagError error = bagGetVertDatum(metaData, (char *)system.geoParameters.vertical_datum, XML_ATTR_MAXSTR);
    if (error)
        return error;

    return bagLegacyToWkt(system, NULL, 0, buffer, bufferSize);
}

//******************************************************************************
//      Method name:    bagGetVReferenceSystem()
//
//      - Initial implementation
//        Mike Van Duzee, 1/26/2012
//
//******************************************************************************
//! Retrieve the BAG's vertical reference system. 
/*!

    The output buffer will contain either a WKT definition, or an EPSG number. 
    If the output is EPSG the buffer will be in the following format:
    "EPSG:<number>"

\param metaData
    \li The handle to the meta data structure.
\param version  
    \li The bag dataset version number.
\param buffer
    \li Modified to contain the reference's system definition.
\param bufferSize
    \li The size of the	definition buffer passed in.
\return
    \li On success, a value of zero is returned. On failure an error code is returned.
*/
//******************************************************************************
bagError bagGetVReferenceSystem(
    bagMetaData metaData,
    const char *version,
    char *buffer,
    u32 bufferSize
    )
{
    bagError err = 0;

    //Figure out what version of metadata we have.
    const int metaVer = getMetadataVersion(version);

    if (metaVer == 1)
        err = bagGetVReferenceSystemV1(metaData, buffer, bufferSize);
    else 
        err = bagGetVReferenceSystemV2(metaData, buffer, bufferSize);

    return err;
}

//*****************************************************************************
/*!
\ Function: bagGetContact

\brief  Populates a RESPONSIBLE_PARTY structure with "contact" information
		  retreived from a BAG metadata XML string.

\param  metaData 
	\li		The handle to the meta data structure.
\param	version
	\li		BAG version
\param  contact
	\li     pointer to structure to load with converted XML.
\return
	\li 0 if the function is successful, non-zero if the function fails.

\ Assumptions: Assumes that only one "contact" node exists in the XML.
*/
//*****************************************************************************

bagError bagGetContact( bagMetaData metaData, const char *version, RESPONSIBLE_PARTY * contact )
{

     //Figure out what version of metadata we have.
    const int metaVer = getMetadataVersion(version);

	/* retrieve the INDIVIDUAL NAME from the Responsible Party info */
    const std::string IndivStr_name = (metaVer == 1) ? 
        "smXML:MD_Metadata/contact/smXML:CI_ResponsibleParty/individualName" :
        "gmi:MI_Metadata/gmd:contact/gmd:CI_ResponsibleParty/gmd:individualName/gco:CharacterString";

    DOMNode *pResponibleNameNode = bagGetXMLNodeByName(metaData->parser->getDocument(), IndivStr_name.c_str());
      
    if (pResponibleNameNode == NULL)
    {
		strcpy((char*)contact->individualName,"\0");
	}
	else
	{
		std::string value = getValueFromNode(*pResponibleNameNode);
		if (value.empty())
			strcpy((char*)contact->individualName,"\0");
		else
			strcpy((char*)contact->individualName, value.c_str());
	}
	
	/* retrieve the ROLE of the responsible party */
	const std::string roleStr_name = (metaVer == 1) ? 
        "smXML:MD_Metadata/contact/smXML:CI_ResponsibleParty/role" :
        "gmi:MI_Metadata/gmd:contact/gmd:CI_ResponsibleParty/gmd:role/gmd:CI_RoleCode";

    DOMNode *pRoleNode = bagGetXMLNodeByName(metaData->parser->getDocument(), roleStr_name.c_str());
      
	if (pRoleNode != NULL)
	{
		const std::string roleName = getValueFromNode(*pRoleNode);	

		if (roleName.empty())
			strcpy((char*)contact->role,"\0");
		else
			strcpy((char*)contact->role, roleName.c_str());
	}
	else 
		strcpy((char*)contact->role, "\0");

	/* retrieve the ORGANISATION_NAME from the responsible party */
	const std::string OrgStr_name = (metaVer == 1) ? 
        "smXML:MD_Metadata/contact/smXML:CI_ResponsibleParty/organisationName" :
        "gmi:MI_Metadata/gmd:contact/gmd:CI_ResponsibleParty/gmd:organisationName/gco:CharacterString";

	DOMNode *pOrgNameNode = bagGetXMLNodeByName(metaData->parser->getDocument(), OrgStr_name.c_str());
    
	if(pOrgNameNode != NULL)
	{
		const std::string orgName = getValueFromNode(*pOrgNameNode);

		if (orgName.empty())
			strcpy((char*)contact->organisationName,"\0");
		else
			strcpy((char*)contact->organisationName, orgName.c_str());
	}
	else
		strcpy((char*)contact->organisationName, "\0");

	/* retrieve the POSITION NAME from the responsible party */
	const std::string positionStr_name = (metaVer == 1) ? 
        "smXML:MD_Metadata/contact/smXML:CI_ResponsibleParty/positionName" :
        "gmi:MI_Metadata/gmd:contact/gmd:CI_ResponsibleParty/gmd:positionName/gco:CharacterString";

    DOMNode *pPositionNode = bagGetXMLNodeByName(metaData->parser->getDocument(), positionStr_name.c_str());
      
	if(pPositionNode != NULL)
	{
		const std::string positionName = getValueFromNode(*pPositionNode);

		if (positionName.empty())
			strcpy((char*)contact->positionName, "\0");
		else
			strcpy((char*)contact->positionName, positionName.c_str());
	}
	else
		strcpy((char*)contact->positionName, "\0");

    return 0;

}

//*****************************************************************************
/*!
\ Function: bagGetLegalConstraints

\brief  Populates a LEGAL_CONSTRAINTS structure with MD_LegalConstraints information
		  retreived from a BAG metadata XML string.

\param  metaData 
	\li		The handle to the meta data structure. 
\param	version
	\li		BAG version
\param  legalConstraints
	\li     pointer to structure to load with converted XML.
\return
	\li 0 if the function is successful, non-zero if the function fails.

*/
//*****************************************************************************
bagError  bagGetLegalConstraints(  bagMetaData metaData, const char *version, LEGAL_CONSTRAINTS * legalConstraints )
{
	   
     //Figure out what version of metadata we have.
    const int metaVer = getMetadataVersion(version);


	/* retrieve the USE CONSTRAINTS from the legal contraints info */
    const std::string UseConstraintsStr_name = (metaVer == 1) ? 
        "smXML:MD_Metadata/metadataConstraints/smXML:MD_LegalConstraints/useConstraints" :
		"gmi:MI_Metadata/gmd:metadataConstraints/gmd:MD_LegalConstraints/gmd:useConstraints/gmd:MD_RestrictionCode";
    DOMNode *pUseConstraintsNode = bagGetXMLNodeByName(metaData->parser->getDocument(), UseConstraintsStr_name.c_str());
      
    if (pUseConstraintsNode == NULL)
    	strcpy((char*)legalConstraints->useConstraints,"\0");
	else
	{
		std::string value = getValueFromNode(*pUseConstraintsNode);
		if (value.empty())
			strcpy((char*)legalConstraints->useConstraints,"\0");
		else
			strcpy((char*)legalConstraints->useConstraints, value.c_str());
	}

	/* retrieve the OTHER CONSTRAINTS from the legal contraints info */
    const std::string OtherConstraintsStr_name = (metaVer == 1) ? 
        "smXML:MD_Metadata/metadataConstraints/smXML:MD_LegalConstraints/otherConstraints" :
		"gmi:MI_Metadata/gmd:metadataConstraints/gmd:MD_LegalConstraints/gmd:otherConstraints/gco:CharacterString";
    DOMNode *pOtherConstraintsNode = bagGetXMLNodeByName(metaData->parser->getDocument(), OtherConstraintsStr_name.c_str());
      
    if (pOtherConstraintsNode == NULL)
    	strcpy((char*)legalConstraints->otherConstraints,"\0");
	else
	{
		std::string value = getValueFromNode(*pOtherConstraintsNode);
		if (value.empty())
			strcpy((char*)legalConstraints->otherConstraints,"\0");
		else
			strcpy((char*)legalConstraints->otherConstraints, value.c_str());
	}

	return 0;	
}

//*****************************************************************************
/*!
\ Function: bagGetSecurityConstraints

\brief  Populates a SECURITY_CONSTRAINTS structure with MD_SecurityConstraints information
		  retreived from a BAG metadata XML string.

\param  metaData 
	\li		The handle to the meta data structure.
\param	version
	\li		BAG version
\param  securityConstraints
	\li     pointer to structure to load with converted XML.
\return
	\li 0 if the function is successful, non-zero if the function fails.

*/
//*****************************************************************************
bagError  bagGetSecurityConstraints(  bagMetaData metaData, const char *version, SECURITY_CONSTRAINTS *securityConstraints )
{
	signed short errorCode = 0;
    
     //Figure out what version of metadata we have.
    const int metaVer = getMetadataVersion(version);


	/* retrieve the CLASSIFICATION from the security contraints info */
    const std::string ClassStr_name = (metaVer == 1) ? 
        "smXML:MD_Metadata/metadataConstraints/smXML:MD_SecurityConstraints/classification" :
		"gmi:MI_Metadata/gmd:metadataConstraints/gmd:MD_SecurityConstraints/gmd:classification/gmd:MD_ClassificationCode";
    DOMNode *pClassNode = bagGetXMLNodeByName(metaData->parser->getDocument(), ClassStr_name.c_str());
      
    if (pClassNode == NULL)
    	strcpy((char*)securityConstraints->classification,"\0");
	else
	{
		std::string value = getValueFromNode(*pClassNode);
		if (value.empty())
			strcpy((char*)securityConstraints->classification,"\0");
		else
			strcpy((char*)securityConstraints->classification, value.c_str());
	}

	/* retrieve the USERS NOTE from the security contraints info */
    const std::string UserNoteStr_name = (metaVer == 1) ? 
        "smXML:MD_Metadata/metadataConstraints/smXML:MD_SecurityConstraints/userNote" :
		"gmi:MI_Metadata/gmd:metadataConstraints/gmd:MD_SecurityConstraints/gmd:userNote/gco:CharacterString";
    DOMNode *pUserNoteNode = bagGetXMLNodeByName(metaData->parser->getDocument(), UserNoteStr_name.c_str());
      
    if (pUserNoteNode == NULL)
    	strcpy((char*)securityConstraints->userNote,"\0");
	else
	{
		std::string value = getValueFromNode(*pUserNoteNode);
		if (value.empty())
			strcpy((char*)securityConstraints->userNote,"\0");
		else
			strcpy((char*)securityConstraints->userNote, value.c_str());
	}

	return 0;	
}



//*****************************************************************************
/*!
\ Function: bagGetDataQualityInfo

\brief  Populates a DATA_QUALITY_INFO structure with DQ_DataQuality information
		  retreived from a BAG metadata XML string.

\param  metaData 
	\li		The handle to the meta data structure. 
\param	version
	\li		BAG version
\param  dataQualityInfo
	\li     pointer to structure to load with converted XML.
\return
	\li 0 if the function is successful, non-zero if the function fails.

*/
//*****************************************************************************
bagError  bagGetDataQualityInfo(bagMetaData metaData, const char *version, DATA_QUALITY_INFO *dataQualityInfo)
{
	signed short errorCode = 0;
    
     //Figure out what version of metadata we have.
    const int metaVer = getMetadataVersion(version);

	

	const asciiString LineageStr = (metaVer == 1) ?
        "smXML:LI_Lineage" : "gmd:LI_Lineage";

	const std::string descriptionStr = (metaVer == 1) ?
        "processStep/smXML:BAG_ProcessStep/source/smXML:LI_Source/description" : 
		"gmd:processStep/bag:BAG_ProcessStep/gmd:source/gmd:LI_Source/gmd:description/gco:CharacterString";

    const std::string titleStr = (metaVer == 1) ?
        "processStep/smXML:BAG_ProcessStep/source/smXML:LI_Source/sourceCitation/smXML:CI_Citation/title" : 
		"gmd:processStep/bag:BAG_ProcessStep/gmd:source/gmd:LI_Source/gmd:sourceCitation/gmd:CI_Citation/gmd:title/gco:CharacterString";

	const std::string dateStr = (metaVer == 1) ?
        "processStep/smXML:BAG_ProcessStep/source/smXML:LI_Source/sourceCitation/smXML:CI_Citation/date/smXML:CI_Date/date" : 
		"gmd:processStep/bag:BAG_ProcessStep/gmd:source/gmd:LI_Source/gmd:sourceCitation/gmd:CI_Citation/gmd:date/gmd:CI_Date/gmd:date";

	const std::string dateTypeStr = (metaVer == 1) ?
        "processStep/smXML:BAG_ProcessStep/source/smXML:LI_Source/sourceCitation/smXML:CI_Citation/date/smXML:CI_Date/dateType" : 
		"gmd:processStep/bag:BAG_ProcessStep/gmd:source/gmd:LI_Source/gmd:sourceCitation/gmd:CI_Citation/gmd:date/gmd:CI_Date/gmd:dateType/gmd:CI_DateTypeCode";

	const std::string roleStr = (metaVer == 1) ?
        "processStep/smXML:BAG_ProcessStep/processor/smXML:CI_ResponsibleParty/role" : 
		"gmd:processStep/bag:BAG_ProcessStep/gmd:processor/gmd:CI_ResponsibleParty/gmd:role/gmd:CI_RoleCode";

	const std::string indivNameStr = (metaVer == 1) ?
        "processStep/smXML:BAG_ProcessStep/processor/smXML:CI_ResponsibleParty/individualName" : 
		"gmd:processStep/bag:BAG_ProcessStep/gmd:processor/gmd:CI_ResponsibleParty/gmd:individualName/gco:CharacterString";
   	
	const std::string dateTimeStr = (metaVer == 1) ?
        "processStep/smXML:BAG_ProcessStep/DateTime" : 
		"gmd:processStep/bag:BAG_ProcessStep/gmd:dateTime/gco:DateTime";

	const std::string ProcessDescriptStr = (metaVer == 1) ?
        "processStep/smXML:BAG_ProcessStep/description" : 
		"gmd:processStep/bag:BAG_ProcessStep/gmd:description/gco:CharacterString";

	const std::string TrackIDStr = (metaVer == 1) ?
        "processStep/smXML:BAG_ProcessStep/trackingId" : 
		"gmd:processStep/bag:BAG_ProcessStep/bag:trackingId/gco:CharacterString";
   
	
	//Find all of the lineage nodes.
    DOMNodeList *pNodeList = metaData->parser->getDocument()->getElementsByTagName(LineageStr);
    for (XMLSize_t i = 0; i < pNodeList->getLength(); i++)
    {
        DOMNode *pNode = pNodeList->item(i);
        if (pNode == NULL)
            return BAG_METADTA_INVLID_DIMENSIONS;

        //Get the dimension name node.
        DOMNode *pDescriptNode = bagGetXMLNodeByName(pNode, descriptionStr.c_str());
        if (pDescriptNode == NULL)
            return BAG_METADTA_INVLID_DIMENSIONS;

        // Get the dimension size node.
        DOMNode *pTitleNode = bagGetXMLNodeByName(pNode, titleStr.c_str());
        if (pTitleNode == NULL)
            return BAG_METADTA_INVLID_DIMENSIONS;

		//Get the dimension name node.
        DOMNode *pDateNode = bagGetXMLNodeByName(pNode, dateStr.c_str());
        if (pDateNode == NULL)
            return BAG_METADTA_INVLID_DIMENSIONS;

        // Get the dimension size node.
        DOMNode *pDateTypeNode = bagGetXMLNodeByName(pNode, dateTypeStr.c_str());
        if (pDateTypeNode == NULL)
            return BAG_METADTA_INVLID_DIMENSIONS;

		// Get the dimension size node.
        DOMNode *pRoleNode = bagGetXMLNodeByName(pNode, roleStr.c_str());
        if (pRoleNode == NULL)
            return BAG_METADTA_INVLID_DIMENSIONS;
		
		// Get the dimension size node.
        DOMNode *pIndivNameNode = bagGetXMLNodeByName(pNode, indivNameStr.c_str());
        if (pIndivNameNode == NULL)
            return BAG_METADTA_INVLID_DIMENSIONS;

		// Get the dimension size node.
        DOMNode *pDateTimeNode = bagGetXMLNodeByName(pNode, dateTimeStr.c_str());
        if (pDateTimeNode == NULL)
            return BAG_METADTA_INVLID_DIMENSIONS;

		// Get the dimension size node.
        DOMNode *pProcessDescriptNode = bagGetXMLNodeByName(pNode, ProcessDescriptStr.c_str());
        if (pProcessDescriptNode == NULL)
            return BAG_METADTA_INVLID_DIMENSIONS;

		
		// Get the dimension size node.
        DOMNode *pTrackIDNode = bagGetXMLNodeByName(pNode, TrackIDStr.c_str());
        if (pTrackIDNode == NULL)
            return BAG_METADTA_INVLID_DIMENSIONS;
		
        const std::string description = getValueFromNode(*pDescriptNode);
        const std::string title = getValueFromNode(*pTitleNode);
		const std::string date = getValueFromNode(*pDateNode);
        const std::string dateType = getValueFromNode(*pDateTypeNode);
		const std::string role = getValueFromNode(*pRoleNode);
		const std::string indivName = getValueFromNode(*pIndivNameNode);
		const std::string dateTime = getValueFromNode(*pDateTimeNode);
		const std::string processDescript = getValueFromNode(*pProcessDescriptNode);
		const std::string trackID = getValueFromNode(*pTrackIDNode);
		
		strcpy((char*)dataQualityInfo->lineageSources->description, description.c_str());
		strcpy((char*)dataQualityInfo->lineageSources->title, title.c_str());
		strcpy((char*)dataQualityInfo->lineageSources->date, date.c_str());
		strcpy((char*)dataQualityInfo->lineageSources->dateType, dateType.c_str());

		strcpy((char*)dataQualityInfo->lineageProcessSteps->processors->role, role.c_str());
		strcpy((char*)dataQualityInfo->lineageProcessSteps->processors->individualName, indivName.c_str());
		strcpy((char*)dataQualityInfo->lineageProcessSteps->dateTime, dateTime.c_str());
		strcpy((char*)dataQualityInfo->lineageProcessSteps->description, processDescript.c_str());
		strcpy((char*)dataQualityInfo->lineageProcessSteps->trackingId, trackID.c_str());
    }


	/* retrieve the level scope info */
    const std::string ScopeStr_name = (metaVer == 1) ? 
        "smXML:MD_Metadata/dataQualityInfo/smXML:DQ_DataQuality/scope/smXML:DQ_Scope/level" :
		"gmi:MI_Metadata/gmd:dataQualityInfo/gmd:DQ_DataQuality/gmd:scope/gmd:DQ_Scope/gmd:level/gmd:MD_ScopeCode";
    DOMNode *pScopeNode = bagGetXMLNodeByName(metaData->parser->getDocument(), ScopeStr_name.c_str());
      
    if (pScopeNode == NULL)
    	strcpy((char*)dataQualityInfo->scope,"\0");
	else 
	{
		std::string value = getValueFromNode(*pScopeNode);
		if (value.empty())
			strcpy((char*)dataQualityInfo->scope,"\0");
		else
			strcpy((char*)dataQualityInfo->scope, value.c_str());
	}

	
	dataQualityInfo->numberOfSources=1;
	dataQualityInfo->numberOfProcessSteps =1;

	
	return 0;	
}

//*****************************************************************************
/*!
\ Function: bagGetSpatialRepresentationInfo

\brief  Populates a SPATIAL_REPRESENTATION_INFO structure with spatialRepresentationInfo
		information retreived from a BAG metadata XML string.

\param  metaData 
	\li		The handle to the meta data structure. 
\param	version
	\li		BAG version
\param  spatial_rep_info
	\li     pointer to structure to load with converted XML.
\return
	\li 0 if the function is successful, non-zero if the function fails.

*/
//*****************************************************************************
extern bagError bagGetSpatialRepresentationInfo(bagMetaData metaData, const char *version, SPATIAL_REPRESENTATION_INFO *spatial_rep_info)
{

  signed short errorCode = 0;
  
    
     //Figure out what version of metadata we have.
    const int metaVer = getMetadataVersion(version);


	// get the number of dimensions.
    const std::string dimensionStr = (metaVer == 1) ?
        "smXML:MD_Metadata/spatialRepresentationInfo/smXML:MD_Georectified/numberOfDimensions" :
        "gmi:MI_Metadata/gmd:spatialRepresentationInfo/gmd:MD_Georectified/gmd:numberOfDimensions/gco:Integer";

    DOMNode *pDimNode = bagGetXMLNodeByName(metaData->parser->getDocument(), dimensionStr.c_str());
    if (pDimNode == NULL)
        return BAG_METADTA_INVLID_DIMENSIONS;

    const std::string value = getValueFromNode(*pDimNode);
    const s32 numDims = atoi(value.c_str());

    spatial_rep_info->numberOfDimensions = numDims;

    const char rowStr[] = "row";
    const char colStr[] = "column";

    const asciiString dimStr = (metaVer == 1) ?
        "smXML:MD_Dimension" : "gmd:MD_Dimension";

    const std::string dimNameStr = (metaVer == 1) ?
        "dimensionName" : "gmd:dimensionName/gmd:MD_DimensionNameTypeCode";

    const std::string dimSizeStr = (metaVer == 1) ?
        "dimensionSize" : "gmd:dimensionSize/gco:Integer";

	 const std::string resolutionStr = (metaVer == 1) ?
        "resolution/smXML:Measure/smXML:value" : "gmd:resolution/gco:Measure";

	 const asciiString GeoRectifiedStr = (metaVer == 1) ?
        "smXML:MD_Georectified" : "gmd:MD_Georectified";
	 

	 const std::string cellGeometryStr = (metaVer == 1) ?
        "cellGeometry" : "gmd:cellGeometry/gmd:MD_CellGeometryCode";

	 const std::string transParamAvailStr = (metaVer == 1) ?
        "transformationParameterAvailability" : "gmd:transformationParameterAvailability/gco:Boolean";

	 const std::string checkPtAvailStr = (metaVer == 1) ?
        "checkPointAvailability" : "gmd:checkPointAvailability/gco:Boolean";

	 
    //Find all of the dimension nodes.
    DOMNodeList *pNodeList = metaData->parser->getDocument()->getElementsByTagName(dimStr);
    for (XMLSize_t i = 0; i < pNodeList->getLength(); i++)
    {
        DOMNode *pNode = pNodeList->item(i);
        if (pNode == NULL)
            return BAG_METADTA_INVLID_DIMENSIONS;

        //Get the dimension name node.
        DOMNode *pNameNode = bagGetXMLNodeByName(pNode, dimNameStr.c_str());
        if (pNameNode == NULL)
            return BAG_METADTA_INVLID_DIMENSIONS;

        //Get the dimension size node.
        DOMNode *pSizeNode = bagGetXMLNodeByName(pNode, dimSizeStr.c_str());
        if (pSizeNode == NULL)
            return BAG_METADTA_INVLID_DIMENSIONS;

		//Get the resolution node.
        DOMNode *pResNode = bagGetXMLNodeByName(pNode, resolutionStr.c_str());
        if (pResNode == NULL)
            return BAG_METADTA_RESOLUTION_MISSING;

		

        const std::string name = getValueFromNode(*pNameNode);
        const std::string size = getValueFromNode(*pSizeNode);
        const std::string resolution = getValueFromNode(*pResNode);
		

       	/* dimension name and sizes */
		strcpy((char*)spatial_rep_info->dimensionName[i], name.c_str());
		spatial_rep_info->dimensionSize[i] = atoi(size.c_str());

		/* resolution values from dx dy*/
	    if (strcmp(name.c_str(), rowStr) == 0)
            spatial_rep_info->resolutionValue[i] = atof(resolution.c_str());
        else if (strcmp(name.c_str(), colStr) == 0)
            spatial_rep_info->resolutionValue[i] = atof(resolution.c_str());
		
		
	}

	//Find all of the dimension nodes.
    DOMNodeList *pNextNodeList = metaData->parser->getDocument()->getElementsByTagName(GeoRectifiedStr);
    for (XMLSize_t i = 0; i < pNodeList->getLength(); i++)
    {
        DOMNode *pNextNode = pNextNodeList->item(i);
        if (pNextNode == NULL)
            break;

		//Get the cell geometry node.
        DOMNode *pCellGeometryNode = bagGetXMLNodeByName(pNextNode, cellGeometryStr.c_str());
        if (pCellGeometryNode == NULL)
            return BAG_METADTA_RESOLUTION_MISSING;

		//Get the trans param availability node.
        DOMNode *pTransParamAvailNode = bagGetXMLNodeByName(pNextNode, transParamAvailStr.c_str());
        if (pTransParamAvailNode == NULL)
            return BAG_METADTA_RESOLUTION_MISSING;

		//Get the checkPointAvailability node.
        DOMNode *pCheckPtAvailabilityNode = bagGetXMLNodeByName(pNextNode, checkPtAvailStr.c_str());
        if (pCheckPtAvailabilityNode == NULL)
            return BAG_METADTA_RESOLUTION_MISSING;

		const std::string cellGeom = getValueFromNode(*pCellGeometryNode);
        const std::string transParamAvail = getValueFromNode(*pTransParamAvailNode);
        const std::string checkPtAvail = getValueFromNode(*pCheckPtAvailabilityNode);

		strcpy((char*)spatial_rep_info->cellGeometry, cellGeom.c_str());
		strcpy((char*)spatial_rep_info->transformationParameterAvailability, transParamAvail.c_str());
		strcpy((char*)spatial_rep_info->checkPointAvailability, checkPtAvail.c_str());
	}

	bagGetProjectedCover(metaData, version, &spatial_rep_info->llCornerX, &spatial_rep_info->llCornerY, 
		&spatial_rep_info->urCornerX, &spatial_rep_info->urCornerY);

	return 0;

}

//*****************************************************************************
/*!
\ Function: bagGetDateStamp

\brief  Populates a character string with dateStamp
		information retreived from a BAG metadata XML string.

\param  metaData 
	\li		The handle to the meta data structure. 
\param	version
	\li		BAG version
\param  dateString
	\li     pointer to character string to load with converted XML.
\return
	\li 0 if the function is successful, non-zero if the function fails.

*/
//*****************************************************************************

bagError  bagGetDateStamp(bagMetaData metaData, const char *version, char ** dateString)
{
	signed short errorCode = 0;
    
     //Figure out what version of metadata we have.
    const int metaVer = getMetadataVersion(version);


	/* retrieve the Description from the source info */
    const std::string DateStr_name = (metaVer == 1) ? 
        "smXML:MD_Metadata/dateStamp" :
		"gmi:MI_Metadata/gmd:dateStamp";
    DOMNode *pDateNode = bagGetXMLNodeByName(metaData->parser->getDocument(), DateStr_name.c_str());
      
    if (pDateNode == NULL)
    	strcpy((char*)dateString,"\0");
	else 
	{
		std::string value = getValueFromNode(*pDateNode);
		if (value.empty())
			strcpy((char*)dateString,"\0");
		else
			strcpy((char*)dateString, value.c_str());
	}

	return 0;

}

//*****************************************************************************
/*!
\ Function: bagGetFileIdentifier

\brief  Populates a character string with file indentifier
		information retreived from a BAG metadata XML string.

\param  metaData 
	\li		The handle to the meta data structure. 
\param	version
	\li		BAG version
\param  fileIdentStr
	\li     pointer to character string to load with converted XML.
\return
	\li 0 if the function is successful, non-zero if the function fails.

*/
//*****************************************************************************

extern bagError  bagGetFileIdentifier(bagMetaData metaData, const char *version, char ** fileIdentStr)
{
	signed short errorCode = 0;
    
     //Figure out what version of metadata we have.
    const int metaVer = getMetadataVersion(version);


	/* retrieve the Description from the source info */
    const std::string FileIdentifierStr_name = (metaVer == 1) ? 
        "smXML:MD_Metadata/fileIdentifier" :
		"gmi:MI_Metadata/gmd:fileIdentifier/gco:CharacterString";
    DOMNode *pFileIdentNode = bagGetXMLNodeByName(metaData->parser->getDocument(), FileIdentifierStr_name.c_str());
      
    if (pFileIdentNode == NULL)
    	strcpy((char*)fileIdentStr,"\0");
	else 
	{
		std::string value = getValueFromNode(*pFileIdentNode);
		if (value.empty())
			strcpy((char*)fileIdentStr,"\0");
		else
			strcpy((char*)fileIdentStr, value.c_str());
	}

	return 0;

}

//*****************************************************************************
/*!
\ Function: bagGetLanguage

\brief  Populates a character string with language
		information retreived from a BAG metadata XML string.

\param  metaData 
	\li		The handle to the meta data structure. 
\param	version
	\li		BAG version
\param  language
	\li     pointer to character string to load with converted XML.
\return
	\li 0 if the function is successful, non-zero if the function fails.

*/
//*****************************************************************************

bagError  bagGetLanguage(bagMetaData metaData, const char *version, char ** language )
{
	signed short errorCode = 0;
    
     //Figure out what version of metadata we have.
    const int metaVer = getMetadataVersion(version);


	/* retrieve the Description from the source info */
    const std::string LanguageStr_name = (metaVer == 1) ? 
        "smXML:MD_Metadata/language" :
		"gmi:MI_Metadata/gmd:language/gmd:LanguageCode";
    DOMNode *pLanguageNode = bagGetXMLNodeByName(metaData->parser->getDocument(), LanguageStr_name.c_str());
      
    if (pLanguageNode == NULL)
    	strcpy((char*)language,"\0");
	else 
	{
		std::string value = getValueFromNode(*pLanguageNode);
		if (value.empty())
			strcpy((char*)language,"\0");
		else
			strcpy((char*)language, value.c_str());
	}

	return 0;

}

//*****************************************************************************
/*!
\ Function: bagGetIdentificationInfo

\brief  Populates a IDENTIFICATION_INFO structure with BAG_DataIdentification
		information retreived from a BAG metadata XML string.

\param  metaData 
	\li		The handle to the meta data structure. 
\param	version
	\li		BAG version
\param  identification_info
	\li     pointer to structure to load with converted XML.
\return
	\li 0 if the function is successful, non-zero if the function fails.

*/
//*****************************************************************************

 bagError bagGetIdentificationInfo(bagMetaData metaData, const char *version, IDENTIFICATION_INFO *identification_info)
 {
	signed short errorCode = 0;
    
     //Figure out what version of metadata we have.
    const int metaVer = getMetadataVersion(version);


	/* retrieve the Description from the source info */
    const std::string TitleStr_name = (metaVer == 1) ? 
        "smXML:MD_Metadata/identificationInfo/smXML:BAG_DataIdentification/citation/smXML:CI_Citation/title" :
		"gmi:MI_Metadata/gmd:identificationInfo/bag:BAG_DataIdentification/gmd:citation/gmd:CI_Citation/gmd:title/gco:CharacterString";
    DOMNode *pTitleNode = bagGetXMLNodeByName(metaData->parser->getDocument(), TitleStr_name.c_str());
      
    if (pTitleNode == NULL)
    	strcpy((char*)identification_info->title,"\0");
	else 
	{
		std::string value = getValueFromNode(*pTitleNode);
		if (value.empty())
			strcpy((char*)identification_info->title,"\0");
		else
			strcpy((char*)identification_info->title, value.c_str());
	}

	/* retrieve the date from the source info */
    const std::string DateStr_name = (metaVer == 1) ? 
        "smXML:MD_Metadata/identificationInfo/smXML:BAG_DataIdentification/citation/smXML:CI_Citation/date/smXML:CI_Date/date" :
		"gmi:MI_Metadata/gmd:identificationInfo/bag:BAG_DataIdentification/gmd:citation/gmd:CI_Citation/gmd:date/gmd:CI_Date/gmd:date/gco:Date";
    DOMNode *pDateNode = bagGetXMLNodeByName(metaData->parser->getDocument(), DateStr_name.c_str());
      
    if (pDateNode == NULL)
    	strcpy((char*)identification_info->date,"\0");
	else 
	{
		std::string value = getValueFromNode(*pDateNode);
		if (value.empty())
			strcpy((char*)identification_info->date,"\0");
		else
			strcpy((char*)identification_info->date, value.c_str());
	}

	/* retrieve the date type from the source info */
    const std::string DateTypeStr_name = (metaVer == 1) ? 
        "smXML:MD_Metadata/identificationInfo/smXML:BAG_DataIdentification/citation/smXML:CI_Citation/date/smXML:CI_Date/dateType" :
		"gmi:MI_Metadata/gmd:identificationInfo/bag:BAG_DataIdentification/gmd:citation/gmd:CI_Citation/gmd:date/gmd:CI_Date/gmd:dateType/gmd:CI_DateTypeCode";
    DOMNode *pDateTypeNode = bagGetXMLNodeByName(metaData->parser->getDocument(), DateTypeStr_name.c_str());
      
    if (pDateTypeNode == NULL)
    	strcpy((char*)identification_info->dateType,"\0");
	else 
	{
		std::string value = getValueFromNode(*pDateTypeNode);
		if (value.empty())
			strcpy((char*)identification_info->dateType,"\0");
		else
			strcpy((char*)identification_info->dateType, value.c_str());
	}

	/* retrieve the individual name from the identification info */
    const std::string IndivNameStr_name = (metaVer == 1) ? 
        "smXML:MD_Metadata/identificationInfo/smXML:BAG_DataIdentification/citation/smXML:CI_Citation/citedResponsibleParty/smXML:CI_ResponsibleParty/individualName" :
		"gmi:MI_Metadata/gmd:identificationInfo/bag:BAG_DataIdentification/gmd:citation/gmd:CI_Citation/gmd:citedResponsibleParty/gmd:CI_ResponsibleParty/gmd:individualName/gco:CharacterString";
    DOMNode *pIndivNameNode = bagGetXMLNodeByName(metaData->parser->getDocument(), IndivNameStr_name.c_str());
      
    if (pIndivNameNode == NULL)
    	strcpy((char*)identification_info->responsibleParties->individualName,"\0");
	else 
	{
		std::string value = getValueFromNode(*pIndivNameNode);
		if (value.empty())
			strcpy((char*)identification_info->responsibleParties->individualName,"\0");
		else
			strcpy((char*)identification_info->responsibleParties->individualName, value.c_str());
	}

	/* retrieve the role from the identification info */
    const std::string RoleStr_name = (metaVer == 1) ? 
        "smXML:MD_Metadata/identificationInfo/smXML:BAG_DataIdentification/citation/smXML:CI_Citation/citedResponsibleParty/smXML:CI_ResponsibleParty/role" :
		"gmi:MI_Metadata/gmd:identificationInfo/bag:BAG_DataIdentification/gmd:citation/gmd:CI_Citation/gmd:citedResponsibleParty/gmd:CI_ResponsibleParty/gmd:role/gmd:CI_RoleCode";
    DOMNode *pRoleNode = bagGetXMLNodeByName(metaData->parser->getDocument(), RoleStr_name.c_str());
      
    if (pRoleNode == NULL)
    	strcpy((char*)identification_info->responsibleParties->role,"\0");
	else 
	{
		std::string value = getValueFromNode(*pRoleNode);
		if (value.empty())
			strcpy((char*)identification_info->responsibleParties->role,"\0");
		else
			strcpy((char*)identification_info->responsibleParties->role, value.c_str());
	}

	/* retrieve the abstract from the identification info */
    const std::string AbstractStr_name = (metaVer == 1) ? 
        "smXML:MD_Metadata/identificationInfo/smXML:BAG_DataIdentification/abstract" :
		"gmi:MI_Metadata/gmd:identificationInfo/bag:BAG_DataIdentification/gmd:abstract/gco:CharacterString";
    DOMNode *pAbstractNode = bagGetXMLNodeByName(metaData->parser->getDocument(), AbstractStr_name.c_str());
      
    if (pAbstractNode == NULL)
    	strcpy((char*)identification_info->abstract,"\0");
	else 
	{
		std::string value = getValueFromNode(*pAbstractNode);
		if (value.empty())
			strcpy((char*)identification_info->abstract,"\0");
		else
			strcpy((char*)identification_info->abstract, value.c_str());
	}

	/* retrieve the topic category from the identification info */
    const std::string TopicCatStr_name = (metaVer == 1) ? 
        "smXML:MD_Metadata/identificationInfo/smXML:BAG_DataIdentification/topicCategory" :
		"gmi:MI_Metadata/gmd:identificationInfo/bag:BAG_DataIdentification/gmd:topicCategory/gmd:MD_TopicCategoryCode";
    DOMNode *pTopicCatNode = bagGetXMLNodeByName(metaData->parser->getDocument(), TopicCatStr_name.c_str());
      
    if (pTopicCatNode == NULL)
    	strcpy((char*)identification_info->topicCategory,"\0");
	else 
	{
		std::string value = getValueFromNode(*pAbstractNode);
		if (value.empty())
			strcpy((char*)identification_info->topicCategory,"\0");
		else
			strcpy((char*)identification_info->topicCategory, value.c_str());
	}

	/* retrieve the status from the identification info */
    const std::string StatusStr_name = (metaVer == 1) ? 
        "smXML:MD_Metadata/identificationInfo/smXML:BAG_DataIdentification/status" :
		"gmi:MI_Metadata/gmd:identificationInfo/bag:BAG_DataIdentification/gmd:status/gmd:MD_ProgressCode";
    DOMNode *pStatusNode = bagGetXMLNodeByName(metaData->parser->getDocument(), StatusStr_name.c_str());
      
    if (pStatusNode == NULL)
    	strcpy((char*)identification_info->status,"\0");
	else 
	{
		std::string value = getValueFromNode(*pStatusNode);
		if (value.empty())
			strcpy((char*)identification_info->status,"\0");
		else
			strcpy((char*)identification_info->status, value.c_str());
	}

	/* retrieve the spatial representation type from the identification info */
    const std::string SpatialRepTypeStr_name = (metaVer == 1) ? 
        "smXML:MD_Metadata/identificationInfo/smXML:BAG_DataIdentification/spatialRepresentation" :
		"gmi:MI_Metadata/gmd:identificationInfo/bag:BAG_DataIdentification/gmd:spatialRepresentationType/gmd:MD_SpatialRepresentationTypeCode";
    DOMNode *pSpatialRepTypeNode = bagGetXMLNodeByName(metaData->parser->getDocument(), SpatialRepTypeStr_name.c_str());
      
    if (pSpatialRepTypeNode == NULL)
    	strcpy((char*)identification_info->spatialRepresentationType,"\0");
	else 
	{
		std::string value = getValueFromNode(*pSpatialRepTypeNode);
		if (value.empty())
			strcpy((char*)identification_info->spatialRepresentationType,"\0");
		else
			strcpy((char*)identification_info->spatialRepresentationType, value.c_str());
	}


		/* retrieve the language from the identification info */
    const std::string LanguageStr_name = (metaVer == 1) ? 
        "smXML:MD_Metadata/identificationInfo/smXML:BAG_DataIdentification/language" :
		"gmi:MI_Metadata/gmd:identificationInfo/bag:BAG_DataIdentification/gmd:language/gmd:LanguageCode";
    DOMNode *pLanguageNode = bagGetXMLNodeByName(metaData->parser->getDocument(), LanguageStr_name.c_str());
      
    if (pLanguageNode == NULL)
    	strcpy((char*)identification_info->language,"\0");
	else 
	{
		std::string value = getValueFromNode(*pLanguageNode);
		if (value.empty())
			strcpy((char*)identification_info->language,"\0");
		else
			strcpy((char*)identification_info->language, value.c_str());
	}

	/* retrieve the character set from the identification info */
    const std::string CharSetStr_name = (metaVer == 1) ? 
        "smXML:MD_Metadata/identificationInfo/smXML:BAG_DataIdentification/characterSet" :
		"gmi:MI_Metadata/gmd:identificationInfo/bag:BAG_DataIdentification/gmd:characterSet/gmd:MD_CharacterSetCode";
    DOMNode *pCharSetNode = bagGetXMLNodeByName(metaData->parser->getDocument(), CharSetStr_name.c_str());
      
    if (pCharSetNode == NULL)
    	strcpy((char*)identification_info->character_set,"\0");
	else 
	{
		std::string value = getValueFromNode(*pCharSetNode);
		if (value.empty())
			strcpy((char*)identification_info->character_set,"\0");
		else
			strcpy((char*)identification_info->character_set, value.c_str());
	}

	/* retrieve the west bound longitude from the identification info */
    const std::string WestBoundStr_name = (metaVer == 1) ? 
        "smXML:MD_Metadata/identificationInfo/smXML:BAG_DataIdentification/extent/smXML:EX_Extent/geographicElement/smXML:EX_GeographicBoundingBox/westBoundLongitude" :
		"gmi:MI_Metadata/gmd:identificationInfo/bag:BAG_DataIdentification/gmd:extent/gmd:EX_Extent/gmd:geographicElement/gmd:EX_GeographicBoundingBox/gmd:westBoundLongitude/gco:Decimal";
    DOMNode *pWestBoundNode = bagGetXMLNodeByName(metaData->parser->getDocument(), WestBoundStr_name.c_str());
      
    if (pWestBoundNode == NULL)
    	identification_info->westBoundingLongitude = 999;
	else 
	{
		std::string value = getValueFromNode(*pWestBoundNode);
		if (value.empty())
			identification_info->westBoundingLongitude = 999;
		else
			identification_info->westBoundingLongitude = atof( value.c_str());
	}

	/* retrieve the east bound loongitude from the identification info */
    const std::string EastBoundStr_name = (metaVer == 1) ? 
        "smXML:MD_Metadata/identificationInfo/smXML:BAG_DataIdentification/extent/smXML:EX_Extent/geographicElement/smXML:EX_GeographicBoundingBox/eastBoundLongitude" :
		"gmi:MI_Metadata/gmd:identificationInfo/bag:BAG_DataIdentification/gmd:extent/gmd:EX_Extent/gmd:geographicElement/gmd:EX_GeographicBoundingBox/gmd:eastBoundLongitude/gco:Decimal";
    DOMNode *pEastBoundNode = bagGetXMLNodeByName(metaData->parser->getDocument(), EastBoundStr_name.c_str());
      
    if (pEastBoundNode == NULL)
    	identification_info->eastBoundingLongitude = 999;
	else 
	{
		std::string value = getValueFromNode(*pEastBoundNode);
		if (value.empty())
			identification_info->eastBoundingLongitude = 999;
		else
			identification_info->eastBoundingLongitude = atof( value.c_str());
	}

	/* retrieve the north bound latitude from the identification info */
    const std::string NorthBoundStr_name = (metaVer == 1) ? 
        "smXML:MD_Metadata/identificationInfo/smXML:BAG_DataIdentification/extent/smXML:EX_Extent/geographicElement/smXML:EX_GeographicBoundingBox/northBoundLatitude" :
		"gmi:MI_Metadata/gmd:identificationInfo/bag:BAG_DataIdentification/gmd:extent/gmd:EX_Extent/gmd:geographicElement/gmd:EX_GeographicBoundingBox/gmd:northBoundLatitude/gco:Decimal";
    DOMNode *pNorthBoundNode = bagGetXMLNodeByName(metaData->parser->getDocument(), NorthBoundStr_name.c_str());
      
    if (pNorthBoundNode == NULL)
    	identification_info->northBoundingLatitude = 999;
	else 
	{
		std::string value = getValueFromNode(*pNorthBoundNode);
		if (value.empty())
			identification_info->northBoundingLatitude = 999;
		else
			identification_info->northBoundingLatitude = atof( value.c_str());
	}

	/* retrieve the south bound latitude from the identification info */
    const std::string SouthBoundStr_name = (metaVer == 1) ? 
        "smXML:MD_Metadata/identificationInfo/smXML:BAG_DataIdentification/extent/smXML:EX_Extent/geographicElement/smXML:EX_GeographicBoundingBox/southBoundLatitude" :
		"gmi:MI_Metadata/gmd:identificationInfo/bag:BAG_DataIdentification/gmd:extent/gmd:EX_Extent/gmd:geographicElement/gmd:EX_GeographicBoundingBox/gmd:southBoundLatitude/gco:Decimal";
    DOMNode *pSouthBoundNode = bagGetXMLNodeByName(metaData->parser->getDocument(), SouthBoundStr_name.c_str());
      
    if (pSouthBoundNode == NULL)
    	identification_info->southBoundingLatitude = 999;
	else 
	{
		std::string value = getValueFromNode(*pSouthBoundNode);
		if (value.empty())
			identification_info->southBoundingLatitude = 999;
		else
			identification_info->southBoundingLatitude = atof( value.c_str());
	}

	/* retrieve the vertical uncertainty from the identification info */
    const std::string VertUncStr_name = (metaVer == 1) ? 
        "smXML:MD_Metadata/identificationInfo/smXML:BAG_DataIdentification/verticalUncertaintyType" :
		"gmi:MI_Metadata/gmd:identificationInfo/bag:BAG_DataIdentification/bag:verticalUncertaintyType/bag:BAG_VertUncertCode";
    DOMNode *pVertUncNode = bagGetXMLNodeByName(metaData->parser->getDocument(), VertUncStr_name.c_str());
      
    if (pVertUncNode == NULL)
    	strcpy((char*)identification_info->verticalUncertaintyType,"\0");
	else 
	{
		std::string value = getValueFromNode(*pVertUncNode);
		if (value.empty())
			strcpy((char*)identification_info->verticalUncertaintyType,"\0");
		else
			strcpy((char*)identification_info->verticalUncertaintyType, value.c_str());
	}

	/* retrieve the depth correction type from the identification info */
    const std::string DepthCorrStr_name = (metaVer == 1) ? 
        "smXML:MD_Metadata/identificationInfo/smXML:BAG_DataIdentification/depthCorrectionType" :
		"gmi:MI_Metadata/gmd:identificationInfo/bag:BAG_DataIdentification/bag:depthCorrectionType/bag:BAG_DepthCorrectCode";
    DOMNode *pDepthCorrNode = bagGetXMLNodeByName(metaData->parser->getDocument(), DepthCorrStr_name.c_str());
      
    if (pDepthCorrNode == NULL)
    	strcpy((char*)identification_info->depthCorrectionType,"\0");
	else 
	{
		std::string value = getValueFromNode(*pDepthCorrNode);
		if (value.empty())
			strcpy((char*)identification_info->depthCorrectionType,"\0");
		else
			strcpy((char*)identification_info->depthCorrectionType, value.c_str());
	}

	/* retrieve the elevation solution group type from the identification info */
    const std::string ElevSolutionStr_name = (metaVer == 1) ? 
        "smXML:MD_Metadata/identificationInfo/smXML:BAG_DataIdentification/elevationSolutionGroupType" :
		"gmi:MI_Metadata/gmd:identificationInfo/bag:BAG_DataIdentification/bag:elevationSolutionGroupType/bag:BAG_OptGroupCode";
    DOMNode *pElevSolutionNode = bagGetXMLNodeByName(metaData->parser->getDocument(), ElevSolutionStr_name.c_str());
      
    if (pElevSolutionNode == NULL)
    	strcpy((char*)identification_info->elevationSolutionGroupType, "\0");
	else 
	{
		std::string value = getValueFromNode(*pElevSolutionNode);
		if (value.empty())
			strcpy((char*)identification_info->elevationSolutionGroupType,"\0");
		else
			strcpy((char*)identification_info->elevationSolutionGroupType, value.c_str());
	}

	/* retrieve the depth correction type from the identification info */
    const std::string NodeGroupStr_name = (metaVer == 1) ? 
        "smXML:MD_Metadata/identificationInfo/smXML:BAG_DataIdentification/nodeGroupType" :
		"gmi:MI_Metadata/gmd:identificationInfo/bag:BAG_DataIdentification/bag:nodeGroupType/bag:BAG_OptGroupCode";
    DOMNode *pNodeGroupNode = bagGetXMLNodeByName(metaData->parser->getDocument(), NodeGroupStr_name.c_str());
      
    if (pNodeGroupNode == NULL)
    	strcpy((char*)identification_info->nodeGroupType,"\0");
	else 
	{
		std::string value = getValueFromNode(*pNodeGroupNode);
		if (value.empty())
			strcpy((char*)identification_info->nodeGroupType,"\0");
		else
			strcpy((char*)identification_info->nodeGroupType, value.c_str());
	}

	
	
	return 0;

 }

 
//*****************************************************************************
/*!
\ Function: InitDataIdentificationInfo

\brief  Initializes a IDENTIFICATION_INFO structure.

\param  dataIdentificationInfo 
	\li		pointer to structure to be initialized.
\return
	\li 1 if the function is successful, non-zero if the function fails.

*/
//*****************************************************************************
bagError InitDataIdentificationInfo(IDENTIFICATION_INFO * dataIdentificationInfo)
{

    u8 status = 1;

    try
    {
		if(dataIdentificationInfo != NULL)
		{

   
			strcpy((char*)(*dataIdentificationInfo).title, "\0"); 
			strcpy((char*)(*dataIdentificationInfo).date, "\0"); 
			strcpy((char*)(*dataIdentificationInfo).dateType, "\0");  
			(*dataIdentificationInfo).abstract = (u8*)malloc(sizeof(char) * 8000);
			strcpy((char*)(*dataIdentificationInfo).abstract, "\0"); 
			strcpy((char*)(*dataIdentificationInfo).purpose, "\0"); 
			strcpy((char*)(*dataIdentificationInfo).status, "\0"); 
			strcpy((char*)(*dataIdentificationInfo).spatialRepresentationType, "\0"); 
			strcpy((char*)(*dataIdentificationInfo).language, ""); 
			strcpy((char*)(*dataIdentificationInfo).topicCategory, ""); 
			(*dataIdentificationInfo).westBoundingLongitude = INIT_VALUE;
			(*dataIdentificationInfo).eastBoundingLongitude = INIT_VALUE;   
			(*dataIdentificationInfo).southBoundingLatitude = INIT_VALUE;       
			(*dataIdentificationInfo).northBoundingLatitude = INIT_VALUE;
			strcpy((char*)(*dataIdentificationInfo).verticalUncertaintyType, "\0");
			strcpy((char*)(*dataIdentificationInfo).depthCorrectionType, "\0");
			strcpy((char*)(*dataIdentificationInfo).elevationSolutionGroupType, "\0");
			strcpy((char*)(*dataIdentificationInfo).nodeGroupType, "\0");

		}
		else
			throw (signed short) -1;

		if((*dataIdentificationInfo).responsibleParties != NULL)
		{

			for(u8 i = 0; i < MAX_CI_RESPONSIBLE_PARTIES; i++)
			{
				strcpy((char*)(*dataIdentificationInfo).responsibleParties[i].individualName, "\0");
				strcpy((char*)(*dataIdentificationInfo).responsibleParties[i].positionName, "\0");
				strcpy((char*)(*dataIdentificationInfo).responsibleParties[i].organisationName, "\0");
				strcpy((char*)(*dataIdentificationInfo).responsibleParties[i].contactInfo, "\0");
				strcpy((char*)(*dataIdentificationInfo).responsibleParties[i].role, "\0");
	         
			}
		}
		else 
			throw (signed short) -1;


			

			


    }               
    catch(signed short intException)
    {
		fprintf(stderr,"ERROR: Exception when attempting to intialize data structure. Exception message is: Null pointer\n");
        status = -1;
    }



    return status;


}



//*****************************************************************************
/*!
\ Function: InitLegalConstraints

\brief  Initializes a LEGAL_CONSTRAINTS structure.

\param  legalConstraints 
	\li		pointer to structure to be initialized.
\return
	\li 1 if the function is successful, non-zero if the function fails.

*/
//*****************************************************************************
bagError InitLegalConstraints(LEGAL_CONSTRAINTS * legalConstraints)
{

    u8 status = 1;

    try
    {
		if(legalConstraints != NULL)
		{
			strcpy( (char*)(*legalConstraints).useConstraints, "\0");
			strcpy( (char*)(*legalConstraints).otherConstraints,"\0");
		}
		else 
			throw (signed short) -1;
    }               
    catch(signed short intException)
    {
		fprintf(stderr,"ERROR: Exception when attempting to intialize data structure. Exception message is: Null pointer\n");
        status = -1;
    }




    return status;


}


//*****************************************************************************
/*!
\ Function: InitSecurityConstraints

\brief  Initializes a SECURITY_CONSTRAINTS structure.

\param  securityConstraints 
	\li		pointer to structure to be initialized.
\return
	\li 1 if the function is successful, non-zero if the function fails.

*/
//*****************************************************************************

bagError InitSecurityConstraints(SECURITY_CONSTRAINTS * securityConstraints)
{

    u8 status = 1;

    try
    {
		if(securityConstraints != NULL)
		{

			strcpy( (char*)(*securityConstraints).classification, "\0");
			/*allocate space first for the userNote */
			(*securityConstraints).userNote = (u8*)malloc(sizeof(char) * 4000);
			strcpy( (char*)(*securityConstraints).userNote,"\0");
		}
		else 
			throw (signed short) -1;
    }               
    catch(signed short intException)
    {
		fprintf(stderr,"ERROR: Exception when attempting to intialize data structure. Exception message is: Null pointer\n");
        status = -1;
    }


    return status;


}



//*****************************************************************************
/*!
\ Function: InitDataQualityInfo

\brief  Initializes a DATA_QUALITY_INFO structure.

\param  dataQualityInfo 
	\li		pointer to structure to be initialized.
\return
	\li 1 if the function is successful, non-zero if the function fails.

*/
//*****************************************************************************
bagError InitDataQualityInfo(DATA_QUALITY_INFO * dataQualityInfo)
{

    u8 status = 1;

    try
    {
		if(dataQualityInfo != NULL)
		{

			strcpy( (char*)(*dataQualityInfo).scope, "\0");

			(*dataQualityInfo).numberOfSources = 0;
			(*dataQualityInfo).numberOfProcessSteps = 0;


			(*dataQualityInfo).lineageSources = NULL;            
			(*dataQualityInfo).lineageProcessSteps = NULL;  

			(*dataQualityInfo).lineageSources = (SOURCE_INFO*)malloc(sizeof(SOURCE_INFO));
			strcpy( (char*)(*dataQualityInfo).lineageSources->description, "\0");
			strcpy( (char*)(*dataQualityInfo).lineageSources->title,"\0");
			strcpy( (char*)(*dataQualityInfo).lineageSources->date,"\0");
			strcpy( (char*)(*dataQualityInfo).lineageSources->dateType,"\0");
	    
			for(u8 i = 0; i < MAX_CI_RESPONSIBLE_PARTIES; i++)
			{
				strcpy((char*)(*dataQualityInfo).lineageSources->responsibleParties[i].individualName, "\0");
				strcpy((char*)(*dataQualityInfo).lineageSources->responsibleParties[i].positionName, "\0");
				strcpy((char*)(*dataQualityInfo).lineageSources->responsibleParties[i].organisationName, "\0");
				strcpy((char*)(*dataQualityInfo).lineageSources->responsibleParties[i].contactInfo, "\0");
				strcpy((char*)(*dataQualityInfo).lineageSources->responsibleParties[i].role, "\0");
	         
			}

			(*dataQualityInfo).lineageProcessSteps = (PROCESS_STEP_INFO*)malloc(sizeof(PROCESS_STEP_INFO));
			strcpy( (char*)(*dataQualityInfo).lineageProcessSteps->description, "\0");
			strcpy( (char*)(*dataQualityInfo).lineageProcessSteps->dateTime,"\0");
			strcpy( (char*)(*dataQualityInfo).lineageProcessSteps->trackingId,"\0");
	        
	    
			for(u8 i = 0; i < MAX_CI_RESPONSIBLE_PARTIES; i++)
			{
				strcpy((char*)(*dataQualityInfo).lineageProcessSteps->processors[i].individualName, "\0");
				strcpy((char*)(*dataQualityInfo).lineageProcessSteps->processors[i].positionName, "\0");
				strcpy((char*)(*dataQualityInfo).lineageProcessSteps->processors[i].organisationName, "\0");
				strcpy((char*)(*dataQualityInfo).lineageProcessSteps->processors[i].contactInfo, "\0");
				strcpy((char*)(*dataQualityInfo).lineageProcessSteps->processors[i].role, "\0");
	         
			}
		}
		else 
			throw (signed short) -1;
    }               
    catch(signed short intException)
    {
		fprintf(stderr,"ERROR: Exception when attempting to intialize data structure. Exception message is: Null pointer\n");
        status = -1;
    }



    return status;


}




//*****************************************************************************
/*!
\ Function: InitSpatialRepresentationInfo

\brief  Initializes a SPATIAL_REPRESENTATION_INFO structure.

\param  spatialRepresentationInfo 
	\li		pointer to structure to be initialized.
\return
	\li 1 if the function is successful, non-zero if the function fails.

*/
//*****************************************************************************
bagError InitSpatialRepresentationInfo(SPATIAL_REPRESENTATION_INFO * spatialRepresentationInfo)
{

    u8 status = 1;

    try
    {

		if(spatialRepresentationInfo != NULL)
		{

			(*spatialRepresentationInfo).numberOfDimensions = INIT_VALUE;

			for (u8 i = 0; i < 3; i++)
			{
				strcpy((char*)(*spatialRepresentationInfo).dimensionName[i],  "\0");
				(*spatialRepresentationInfo).dimensionSize[i] = 0; 
				(*spatialRepresentationInfo).resolutionValue[i]  = 0; 
			}
               

			strcpy((char*)(*spatialRepresentationInfo).cellGeometry, "\0");
			strcpy((char*)(*spatialRepresentationInfo).transformationParameterAvailability, "\0");
			strcpy((char*)(*spatialRepresentationInfo).checkPointAvailability, "\0");                

			(*spatialRepresentationInfo).llCornerX = INIT_VALUE;                                 
			(*spatialRepresentationInfo).llCornerY = INIT_VALUE;                                  
			(*spatialRepresentationInfo).urCornerX = INIT_VALUE;                               
			(*spatialRepresentationInfo).urCornerY = INIT_VALUE;  
		}

		else 
			throw (signed short) -1;
    }               
    catch(signed short intException)
    {
		fprintf(stderr,"ERROR: Exception when attempting to intialize data structure. Exception message is: Null pointer\n");
        status = -1;
    }



    return status;


}





/******************************************************************************************************
*
* Function: InitResponsibleParty
*
* Purpose:  Initialize all character arrays to empty string.
*
* Inputs:   responsibleParty - pointer to object to initialize.
*
* Returns:  1 = success
*          -1 = not successful.  An exception was encountered.
*
*******************************************************************************************************/

bagError InitResponsibleParty(RESPONSIBLE_PARTY * responsibleParty)
{

    u8 status = 1;

    try
    {
		if(responsibleParty != NULL)
		{

			strcpy( (char*)(*responsibleParty).individualName, "\0");
			strcpy( (char*)(*responsibleParty).organisationName, "\0");
			strcpy( (char*)(*responsibleParty).positionName, "\0");
			strcpy( (char*)(*responsibleParty).contactInfo, "\0");
			strcpy( (char*)(*responsibleParty).role, "\0");

		}

		else 
			throw (signed short) -1;
    }               
    catch(signed short intException)
    {
		fprintf(stderr,"ERROR: Exception when attempting to intialize data structure. Exception message is: Null pointer\n");
        status = -1;
    }


    return status;


}


