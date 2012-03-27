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
//      Method name:    bagGetUncertantyType()
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
bagError bagGetUncertantyType(
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
