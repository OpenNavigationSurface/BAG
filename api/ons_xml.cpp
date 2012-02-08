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


// constant locations/names.
static const char *ons_schema_location = "ISO19139/smXML/metadataEntity.xsd";
static const char *ons_schema_nameSpace = "http://metadata.dgiwg.org/smXML";

// flag used to force the validation to succeed when parsing a BAG XML file or mem buffer.
// Bill Lamey July 21, 2005 - This is set to false temoporarily as there are some problems validating the 
//          MD_Georectified portion of the XML file.  The MD_Georectified section is crucial
//          as it contains information about he projected corner coordinates of the grid, 
//          pixel location (center, lower left, etc), cell geometry (cell, node), etc.  Once
//          the validation errors have been resolved, this falg should be removed.  That is,
//          the library should ALWAYS force the validation of the 19115 schema.
static bool ons_force_schema_validation = true;

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
        delete parser;
        delete errHandler;
    }

    //! Pointer to the current document parser.
    XercesDOMParser *parser;
    //! Pointer to the current error handler.
    BAGMetaDataErrorHandler *errHandler;
};

//! This is a simple class that lets us do easy (though not terribly efficient) transcoding of XMLCh data to local code page for display.
class StrX
{
public :
//Construction/Destruction
    StrX(const XMLCh* const toTranscode)
    {
        // Call the private transcoding method
        fLocalForm = XMLString::transcode(toTranscode);
    }
    ~StrX()
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
    const StrX& toDump
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
    XERCES_STD_QUALIFIER cerr << "Warning at file \"" << StrX(toCatch.getSystemId())
		 << "\", line " << toCatch.getLineNumber()
		 << ", column " << toCatch.getColumnNumber()
         << "\n   Message: " << StrX(toCatch.getMessage()) << XERCES_STD_QUALIFIER endl;
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
    XERCES_STD_QUALIFIER cerr << "Error at file \"" << StrX(toCatch.getSystemId())
		 << "\", line " << toCatch.getLineNumber()
		 << ", column " << toCatch.getColumnNumber()
         << "\n   Message: " << StrX(toCatch.getMessage()) << XERCES_STD_QUALIFIER endl;
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
    XERCES_STD_QUALIFIER cerr << "Fatal Error at file \"" << StrX(toCatch.getSystemId())
		 << "\", line " << toCatch.getLineNumber()
		 << ", column " << toCatch.getColumnNumber()
         << "\n   Message: " << StrX(toCatch.getMessage()) << XERCES_STD_QUALIFIER endl;
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

    XMLCh *pNodeName = NULL;
    if (endPos)
    {
        char nodeName[256];
        size_t len = endPos - pathName;
        strncpy(nodeName, pathName, len);
        nodeName[len] = '\0';
        pNodeName = XMLString::transcode(nodeName);
    }
    else
        pNodeName = XMLString::transcode(pathName);

    DOMNodeList *pList = parent->getChildNodes();

    for (u32 i = 0; i < pList->getLength(); i++)
    {
        // get the node.
        DOMNode *pChild = pList->item(i);
        if (pChild)
        {
            const XMLCh *pCurrNodeName = pChild->getNodeName();
            if (XMLString::compareString(pNodeName, pCurrNodeName) == 0)
            {
                // release the string 
                XMLString::release(&pNodeName);
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
                        return pRet;
                }
            }
        }

    }

    // not found.
    return NULL;
}

//************************************************************************
//      Method name:    bagValidateMetadata()
//
//      
//      - Initial implementation
//        Bill Lamey, 7/20/2005
//
//************************************************************************
//! Validate the input XML file against the BAG schema.
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
bagMetaData bagValidateMetadata(
    const InputSource &source,
    bagError *error
    )
{
    // Reset the error code.
    *error = 0;

    bagMetaData metaData = NULL;

    // Check environment.
    char *onsHome = getenv("BAG_HOME");

    if (onsHome == NULL)
    {
        *error = BAG_METADTA_NO_HOME;
        return  metaData;
    }

    // create the name of the schema file.
    size_t totalLen = strlen(onsHome) + strlen(ons_schema_location) + 2;
    char *schemaFile = new char [totalLen];
    sprintf(schemaFile, "%s/%s", onsHome, ons_schema_location);

    // make sure the main schema file exists.
    struct stat fStat;
    if (stat(schemaFile, &fStat))
    {
        delete [] schemaFile;
        *error = BAG_METADTA_SCHEMA_FILE_MISSING;
        return NULL;
    }

    // make sure that any spaces in the file name are replaced with %20 so that Xerces will parse correctly.
    size_t i, idx = 0, numSpaces = 0;
    size_t length = strlen(schemaFile);

    // count the number of spaces in the string.
    for (i = 0; i < length; i++)
    {
        if (schemaFile[i] == ' ')
            numSpaces++;
    }

    // create the buffer for the new file name (with spaces replaced with %20)
    char *newFileName = new char[strlen(schemaFile) + numSpaces * 3 + 1];

    // replace spaces.
    for (i = 0; i < length; i++)
    {
        if (schemaFile[i] == ' ')
        {
            newFileName[idx++] = '%';
            newFileName[idx++] = '2';
            newFileName[idx++] = '0';
        }
        else
            newFileName[idx++] = schemaFile[i];
    }

    // terminate the string.
    newFileName[idx] = '\0';

    totalLen = strlen(newFileName) + strlen(ons_schema_location) + 2;
    char *schemaEntry = new char[strlen(ons_schema_nameSpace) + totalLen + 2];

    sprintf(schemaEntry, "%s %s", ons_schema_nameSpace, newFileName);
    delete [] newFileName;

    //  Create our parser, then attach an error handler to the parser.
    //  The parser will call back to methods of the ErrorHandler if it
    //  discovers errors during the course of parsing the XML document.
    metaData = new bagXMLHandle;
    metaData->parser = new XercesDOMParser;
    metaData->parser->setValidationScheme(XercesDOMParser::Val_Always);
    metaData->parser->setDoNamespaces(true);
    metaData->parser->setDoSchema(ons_force_schema_validation);
    metaData->parser->setValidationSchemaFullChecking(false);
    metaData->parser->setCreateEntityReferenceNodes(false);
    metaData->parser->setExternalSchemaLocation(schemaEntry);
    metaData->parser->setIncludeIgnorableWhitespace(false);

    // clean up srings;
    delete [] schemaEntry;
    delete [] schemaFile;

    metaData->errHandler = new BAGMetaDataErrorHandler();
    metaData->parser->setErrorHandler(metaData->errHandler);

    //  Parse the XML file, catching any XML exceptions that might propogate
    //  out of it.
    try
    {
        metaData->parser->parse(source);
    }
    catch (const OutOfMemoryException&)
    {
        XERCES_STD_QUALIFIER cerr << "OutOfMemoryException" << XERCES_STD_QUALIFIER endl;
        *error = BAG_METADTA_PARSE_MEM_EXCEPTION;
    }
    catch (const XMLException& e)
    {
        XERCES_STD_QUALIFIER cerr << "An error occurred during parsing\n   Message: "
             << StrX(e.getMessage()) << XERCES_STD_QUALIFIER endl;
        *error = BAG_METADTA_PARSE_EXCEPTION;
    }
    catch (const DOMException& e)
    {
        const u32 maxChars = 2047;
        XMLCh errText[maxChars + 1];

        XERCES_STD_QUALIFIER cerr << "\nDOM Error parsing " << StrX(source.getSystemId()) << XERCES_STD_QUALIFIER endl
             << "DOMException code is:  " << e.code << XERCES_STD_QUALIFIER endl;

        if (DOMImplementation::loadDOMExceptionMsg(e.code, errText, maxChars))
             XERCES_STD_QUALIFIER cerr << "Message is: " << StrX(errText) << XERCES_STD_QUALIFIER endl;
        *error = BAG_METADTA_PARSE_DOM_EXCEPTION;
    }
    catch (...)
    {
        XERCES_STD_QUALIFIER cerr << "An error occurred during parsing\n " << XERCES_STD_QUALIFIER endl;
        *error = BAG_METADTA_PARSE_UNK_EXCEPTION;
    }

    // Check for validation errors if the parse was successful.
    if (*error == 0)
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
    \li 
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
bagError bagInitMetadata(
    )
{
    // Initialize the XML4C2 system
    try
    {
        XMLPlatformUtils::Initialize();
    }

    catch(const XMLException &toCatch)
    {
        XERCES_STD_QUALIFIER cerr << "Error during Xerces-c Initialization.\n"
             << "  Exception message:"
             << StrX(toCatch.getMessage()) << XERCES_STD_QUALIFIER endl;
        return BAG_METADTA_INIT_FAILED;
    }


    return 0;
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
bagError bagTermMetadata(
    )
{
    XMLPlatformUtils::Terminate();
    return 0;
}

//************************************************************************
//      Method name:    bagValidateMetadataBuffer()
//
//      
//      - Initial implementation
//        Bill Lamey, 7/19/2005
//
//************************************************************************
//! Validate the input XML memory buffer against the BAG schema.
/*!
\param buffer
    \li The character buffer to be parsed.
\param bufferSize
    \li The size of the input buffer.
\param error
    \li Modified to contain a non-zero value if the funtion fails, modified
    to contain zero if the function succeeds.
\return
    \li A valid bagMetaData handle on success, NULL on failure.  If the method
        fails, the value of error will be non-zero.
*/
//************************************************************************
bagMetaData bagValidateMetadataBuffer(
    char *buffer,
    s32 bufferSize,
    bagError *error
    )
{
    // generate the input source.
    const char *bufferid = "BAG Meta Data";
    MemBufInputSource memSource((const XMLByte *)buffer, bufferSize, bufferid);

    // call the validation routine.
    return bagValidateMetadata(memSource, error);
}

//************************************************************************
//      Method name:    bagValidateMetadataFile()
//
//      
//      - Initial implementation
//        Bill Lamey, 7/19/2005
//
//************************************************************************
//! Validate the input XML file against the BAG schema.
/*!
\param fileName
    \li The full path and file name of the file to be parsed.
\param error
    \li Modified to contain a non-zero value if the funtion fails, modified
    to contain zero if the function succeeds.
\return
    \li A valid bagMetaData handle on success, NULL on failure.  If the method
        fails, the value of error will be non-zero.
*/
//************************************************************************
bagMetaData bagValidateMetadataFile(char *fileName, bagError *error)
{
    // generate the input source.
    XMLCh *fName = XMLString::transcode(fileName);
    LocalFileInputSource  fileSource(fName);
    XMLString::release(&fName);

    // call the validation routine.
    return bagValidateMetadata(fileSource, error);
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
    u32 *nRows,
    u32 *nCols
    )
{
    if (metaData == NULL)
        return BAG_METADTA_INVALID_HANDLE;


    // get the number of dimensions.
    const char *pDimensions = "smXML:MD_Metadata/spatialRepresentationInfo/smXML:MD_Georectified/numberOfDimensions";
    DOMNode *pDimNode = bagGetXMLNodeByName(metaData->parser->getDocument(), pDimensions);

    s32 numDims = 0;
    DOMNode *pChildNode = pDimNode->getFirstChild();
    if (pChildNode)
    {
        const XMLCh *pStr = pChildNode->getNodeValue();
        char *numDimStr = XMLString::transcode(pStr);
        numDims = atoi(numDimStr);
        XMLString::release(&numDimStr);
    }

    // validate the number of dimensions.
    if (numDims != 2)
        return BAG_METADTA_INVLID_DIMENSIONS;

    *nRows = 0;
    *nCols = 0;

    // get the root node.
    XMLCh *pDimName = XMLString::transcode("dimensionName");
    XMLCh *pDimSize = XMLString::transcode("dimensionSize");
    XMLCh *pRow = XMLString::transcode("row");
    XMLCh *pCol = XMLString::transcode("column");

    DOMNodeList *pChild = metaData->parser->getDocument()->getElementsByTagName(pDimName);
    for (u32 i = 0; i < pChild->getLength(); i++)
    {
        DOMNode *pNode = pChild->item(i);
     
        if (pNode)
        {
            DOMNodeList *pChildList2 = pNode->getChildNodes();

            for (u32 j = 0; j < pChildList2->getLength(); j++)
            {
                DOMNode *pChildNode = pChildList2->item(j);

                const XMLCh *pStr = pChildNode->getNodeValue();

                // get the next sibling from our parent.
                DOMNode *pSibling = pNode->getNextSibling();

                u32 dim = 0;
                bool foundDim = false;
                while (pSibling != NULL)
                {
                    // is this the dimension node....
                    if (XMLString::compareString(pSibling->getNodeName(), pDimSize) == 0)
                    {
                        // should only be 1 child with the dimension value.
                        DOMNode *pDimNode = pSibling->getFirstChild();
                        if (pDimNode)
                        {
                            char *nRowsStr = XMLString::transcode(pDimNode->getNodeValue());
                            dim = atoi(nRowsStr);
                            XMLString::release(&nRowsStr);
                            foundDim = true;
                        }
                        // terminate the loop.
                        pSibling = NULL;
                    }
                    else
                        pSibling = pSibling->getNextSibling();

                }

                if (foundDim)
                {
                    // check the name.
                    if (XMLString::compareString(pStr, pRow) == 0)
                        *nRows = dim;
                    else if (XMLString::compareString(pStr, pCol) == 0)
                        *nCols = dim;
                }
            }
        }
    }

    // release strings.
    XMLString::release(&pDimSize);
    XMLString::release(&pDimName);
    XMLString::release(&pRow);
    XMLString::release(&pCol);

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
    XMLCh *pllx = XMLString::transcode("westBoundLongitude");
    XMLCh *purx = XMLString::transcode("eastBoundLongitude");
    XMLCh *plly = XMLString::transcode("southBoundLatitude");
    XMLCh *pury = XMLString::transcode("northBoundLatitude");

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
                // should only be 1 child with the dimension value.
                DOMNode *pTmpNode = pGeoNode->getFirstChild();
                if (pTmpNode)
                {
                    char *pTmpStr = XMLString::transcode(pTmpNode->getNodeValue());
                    lngW = atof(pTmpStr);
                    XMLString::release(&pTmpStr);
                }
            }
            else if (XMLString::compareString(pGeoNode->getNodeName(), plly) == 0)
            {
                // should only be 1 child with the dimension value.
                DOMNode *pTmpNode = pGeoNode->getFirstChild();
                if (pTmpNode)
                {
                    char *pTmpStr = XMLString::transcode(pTmpNode->getNodeValue());
                    latS = atof(pTmpStr);
                    XMLString::release(&pTmpStr);
                }
            }
            else if (XMLString::compareString(pGeoNode->getNodeName(), purx) == 0)
            {
                // should only be 1 child with the dimension value.
                DOMNode *pTmpNode = pGeoNode->getFirstChild();
                if (pTmpNode)
                {
                    char *pTmpStr = XMLString::transcode(pTmpNode->getNodeValue());
                    lngE = atof(pTmpStr);
                    XMLString::release(&pTmpStr);
                }
            }
            else if (XMLString::compareString(pGeoNode->getNodeName(), pury) == 0)
            {
                // should only be 1 child with the dimension value.
                DOMNode *pTmpNode = pGeoNode->getFirstChild();
                if (pTmpNode)
                {
                    char *pTmpStr = XMLString::transcode(pTmpNode->getNodeValue());
                    latN = atof(pTmpStr);
                    XMLString::release(&pTmpStr);
                }
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

    // release strings.
    XMLString::release(&pllx);
    XMLString::release(&purx);
    XMLString::release(&plly);
    XMLString::release(&pury);

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
    f64 *llx,
    f64 *lly,
    f64 *urx,
    f64 *ury
    )
{
    if (metaData == NULL)
        return BAG_METADTA_INVALID_HANDLE;

    f64 lx = DBL_MAX, ly = DBL_MAX, rx = DBL_MAX, ry = DBL_MAX;

    const char * pGeoBox = "smXML:MD_Metadata/spatialRepresentationInfo/smXML:MD_Georectified/cornerPoints/gml:Point/gml:coordinates";

    DOMNode *pGMLCoord = bagGetXMLNodeByName(metaData->parser->getDocument(), pGeoBox);

    if (pGMLCoord == NULL)
        return BAG_METADTA_INCOMPLETE_COVER;

    DOMNode *pValue = pGMLCoord->getFirstChild();
    if (pValue)
    {
        const XMLCh *pTmpStr = pValue->getNodeValue();
        if (pTmpStr)
        {
            char *pPointStr = XMLString::transcode(pTmpStr);

            // the string is 2 item tuples seperated by spaces.
            lx = atof(pPointStr);
            char *tmpPos = strchr(pPointStr, ',');
            ly = atof(tmpPos+1);
            tmpPos = strchr(tmpPos+1, ' ');
            rx = atof(tmpPos+1);
            tmpPos = strchr(tmpPos, ',');
            ry = atof(tmpPos+1);
            XMLString::release(&pPointStr);
        }
    }

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
    f64 *dx,
    f64 *dy
    )
{
    if (metaData == NULL)
        return BAG_METADTA_INVALID_HANDLE;

    *dx = 0.0;
    *dy = 0.0;

    // get the root node.
    XMLCh *pDimName = XMLString::transcode("dimensionName");
    XMLCh *pRow = XMLString::transcode("row");
    XMLCh *pCol = XMLString::transcode("column");
    XMLCh *pRes = XMLString::transcode("resolution");

    DOMNodeList *pChild = metaData->parser->getDocument()->getElementsByTagName(pDimName);
    for (u32 i = 0; i < pChild->getLength(); i++)
    {
        DOMNode *pNode = pChild->item(i);
     
        if (pNode)
        {
            DOMNodeList *pChildList2 = pNode->getChildNodes();

            for (u32 j = 0; j < pChildList2->getLength(); j++)
            {
                DOMNode *pChildNode = pChildList2->item(j);

                const XMLCh *pStr = pChildNode->getNodeValue();

                // get the next sibling from our parent.
                DOMNode *pSibling = pNode->getNextSibling();

                f64 res = 0.0;
                bool found = false;
                while (pSibling != NULL)
                {
                    // is this the dimension node....
                    if (XMLString::compareString(pSibling->getNodeName(), pRes) == 0)
                    {
                        // should only be 1 child with the dimension value.
                        DOMNode *pSMLRes = pSibling->getFirstChild();
                        if (pSMLRes)
                        {
                            DOMNode *pValue = pSMLRes->getFirstChild();

                            if (pValue)
                            {
                                pValue = pValue->getFirstChild();

                                if (pValue)
                                {
                                    char *valStr = XMLString::transcode(pValue->getNodeValue());
                                    res = atof(valStr);
                                    XMLString::release(&valStr);
                                    found = true;
                                }
                            }
                        }
                        // terminate the loop.
                        pSibling = NULL;
                    }
                    else
                        pSibling = pSibling->getNextSibling();

                }

                if (found)
                {
                    // check the name.
                    if (XMLString::compareString(pStr, pRow) == 0)
                        *dy = res;
                    else if (XMLString::compareString(pStr, pCol) == 0)
                        *dx = res;
                }
            }
        }
    }

    // release strings.
    XMLString::release(&pRes);
    XMLString::release(&pDimName);
    XMLString::release(&pRow);
    XMLString::release(&pCol);

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
    XMLCh *pZone = XMLString::transcode("zone");
    XMLCh *pStdPar = XMLString::transcode("standardParallel");
    XMLCh *pCenterMer = XMLString::transcode("longitudeOfCentralMeridian");
    XMLCh *pLatProjOrig = XMLString::transcode("latitudeOfProjectionOrigin");
    XMLCh *pFalseEast = XMLString::transcode("falseEasting");
    XMLCh *pFalseNorth = XMLString::transcode("falseNorthing");
    XMLCh *pScaleFactAtEq = XMLString::transcode("scaleFactorAtEquator");
    XMLCh *pHeightOfPers = XMLString::transcode("heightOfProspectivePointAboveSurface");
    XMLCh *pLongProjCenter = XMLString::transcode("longitudeOfProjectionCenter");
    XMLCh *pLatProjCenter = XMLString::transcode("latitudeOfProjectionCenter");
    XMLCh *pScaleFactAtCenterLine = XMLString::transcode("scaleFactorAtCenterLine");
    XMLCh *pStVertLongFromPole = XMLString::transcode("straightVerticalLongitudeFromPole");
    XMLCh *pScaleFactAtProjOrig = XMLString::transcode("scaleFactorAtProjectionOrigin");

    bool valuesFound = false;

    DOMNode *pProjIdNode = bagGetXMLNodeByName(metaData->parser->getDocument(), pProjIdLocation);

    if (pProjIdNode)
    {
        DOMNode *pTmpNode = pProjIdNode->getFirstChild();
        if (pTmpNode)
        {
            char *pTmpStr = XMLString::transcode(pTmpNode->getNodeValue());

            if (strlen(pTmpStr) > projIdLen)
            {
                strncpy(projId, pTmpStr, sizeof(char) * projIdLen);
                projId[projIdLen - 1] = '\0';
            }
            else
            {
                strcpy(projId, pTmpStr);
            }

            XMLString::release(&pTmpStr);
            valuesFound = true;
        }
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

            if (XMLString::compareString(pStr, pZone) == 0)
            {
                // should only be 1 child with the value.
                DOMNode *pTmpNode = pParamNode->getFirstChild();
                if (pTmpNode)
                {
                    char *pTmpStr = XMLString::transcode(pTmpNode->getNodeValue());
                    *zone = atoi(pTmpStr);
                    XMLString::release(&pTmpStr);
                    valuesFound = true;
                }
            }
            else if (XMLString::compareString(pStr, pStdPar) == 0)
            {
                // should only be 1 child with the value.
                DOMNode *pTmpNode = pParamNode->getFirstChild();
                if (pTmpNode)
                {
                    char *pTmpStr = XMLString::transcode(pTmpNode->getNodeValue());
                    *standrardParallel = atof(pTmpStr);
                    XMLString::release(&pTmpStr);
                    valuesFound = true;
                }
            }
            else if (XMLString::compareString(pStr, pCenterMer) == 0)
            {
                // should only be 1 child with the value.
                DOMNode *pTmpNode = pParamNode->getFirstChild();
                if (pTmpNode)
                {
                    char *pTmpStr = XMLString::transcode(pTmpNode->getNodeValue());
                    *centralMeridian = atof(pTmpStr);
                    XMLString::release(&pTmpStr);
                    valuesFound = true;
                }
            }
            else if (XMLString::compareString(pStr, pLatProjOrig) == 0)
            {
                // should only be 1 child with the value.
                DOMNode *pTmpNode = pParamNode->getFirstChild();
                if (pTmpNode)
                {
                    char *pTmpStr = XMLString::transcode(pTmpNode->getNodeValue());
                    *latitudeOfOrigin = atof(pTmpStr);
                    XMLString::release(&pTmpStr);
                    valuesFound = true;
                }
            }
            else if (XMLString::compareString(pStr, pFalseEast) == 0)
            {
                // should only be 1 child with the value.
                DOMNode *pTmpNode = pParamNode->getFirstChild();
                if (pTmpNode)
                {
                    char *pTmpStr = XMLString::transcode(pTmpNode->getNodeValue());
                    *falseEasting = atof(pTmpStr);
                    XMLString::release(&pTmpStr);
                    valuesFound = true;
                }
            }
            else if (XMLString::compareString(pStr, pFalseNorth) == 0)
            {
                // should only be 1 child with the value.
                DOMNode *pTmpNode = pParamNode->getFirstChild();
                if (pTmpNode)
                {
                    char *pTmpStr = XMLString::transcode(pTmpNode->getNodeValue());
                    *falseNorthing = atof(pTmpStr);
                    XMLString::release(&pTmpStr);
                    valuesFound = true;
                }
            }
            else if (XMLString::compareString(pStr, pScaleFactAtEq) == 0)
            {
                // should only be 1 child with the value.
                DOMNode *pTmpNode = pParamNode->getFirstChild();
                if (pTmpNode)
                {
                    char *pTmpStr = XMLString::transcode(pTmpNode->getNodeValue());
                    *scaleFactAtEq = atof(pTmpStr);
                    XMLString::release(&pTmpStr);
                    valuesFound = true;
                }
            }
            else if (XMLString::compareString(pStr, pHeightOfPers) == 0)
            {
                // should only be 1 child with the value.
                DOMNode *pTmpNode = pParamNode->getFirstChild();
                if (pTmpNode)
                {
                    char *pTmpStr = XMLString::transcode(pTmpNode->getNodeValue());
                    *heightOfPersPoint = atof(pTmpStr);
                    XMLString::release(&pTmpStr);
                    valuesFound = true;
                }
            }
            else if (XMLString::compareString(pStr, pLongProjCenter) == 0)
            {
                // should only be 1 child with the value.
                DOMNode *pTmpNode = pParamNode->getFirstChild();
                if (pTmpNode)
                {
                    char *pTmpStr = XMLString::transcode(pTmpNode->getNodeValue());
                    *longOfProjCenter = atof(pTmpStr);
                    XMLString::release(&pTmpStr);
                    valuesFound = true;
                }
            }
            else if (XMLString::compareString(pStr, pLatProjCenter) == 0)
            {
                // should only be 1 child with the value.
                DOMNode *pTmpNode = pParamNode->getFirstChild();
                if (pTmpNode)
                {
                    char *pTmpStr = XMLString::transcode(pTmpNode->getNodeValue());
                    *latOfProjCenter = atof(pTmpStr);
                    XMLString::release(&pTmpStr);
                    valuesFound = true;
                }
            }
            else if (XMLString::compareString(pStr, pScaleFactAtCenterLine) == 0)
            {
                // should only be 1 child with the value.
                DOMNode *pTmpNode = pParamNode->getFirstChild();
                if (pTmpNode)
                {
                    char *pTmpStr = XMLString::transcode(pTmpNode->getNodeValue());
                    *scaleAtCenterLine = atof(pTmpStr);
                    XMLString::release(&pTmpStr);
                    valuesFound = true;
                }
            }
            else if (XMLString::compareString(pStr, pStVertLongFromPole) == 0)
            {
                // should only be 1 child with the value.
                DOMNode *pTmpNode = pParamNode->getFirstChild();
                if (pTmpNode)
                {
                    char *pTmpStr = XMLString::transcode(pTmpNode->getNodeValue());
                    *vertLongFromPole = atof(pTmpStr);
                    XMLString::release(&pTmpStr);
                    valuesFound = true;
                }
            }
            else if (XMLString::compareString(pStr, pScaleFactAtProjOrig) == 0)
            {
                // should only be 1 child with the value.
                DOMNode *pTmpNode = pParamNode->getFirstChild();
                if (pTmpNode)
                {
                    char *pTmpStr = XMLString::transcode(pTmpNode->getNodeValue());
                    *scaleAtProjOrigin = atof(pTmpStr);
                    XMLString::release(&pTmpStr);
                    valuesFound = true;
                }
            }
        }
    }

    // releae all the strings.
    XMLString::release(&pZone);
    XMLString::release(&pStdPar);
    XMLString::release(&pCenterMer);
    XMLString::release(&pLatProjOrig);
    XMLString::release(&pFalseEast);
    XMLString::release(&pFalseNorth);
    XMLString::release(&pScaleFactAtEq);
    XMLString::release(&pHeightOfPers);
    XMLString::release(&pLongProjCenter);
    XMLString::release(&pLatProjCenter);
    XMLString::release(&pScaleFactAtCenterLine);
    XMLString::release(&pStVertLongFromPole);
    XMLString::release(&pScaleFactAtProjOrig);

    bagError error = 0;
    if (!valuesFound)
        error = BAG_METADTA_NO_PROJECTION_INFO;

    return error;
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

    bagError error = 0;
    if (pEllipIdNode)
    {
        DOMNode *pTmpNode = pEllipIdNode->getFirstChild();
        if (pTmpNode)
        {
            char *pTmpStr = XMLString::transcode(pTmpNode->getNodeValue());

            if (strlen(pTmpStr) > bufferSize)
            {
                strncpy(buffer, pTmpStr, sizeof(char) * bufferSize);
                buffer[bufferSize - 1] = '\0';
            }
            else
            {
                strcpy(buffer, pTmpStr);
            }

            XMLString::release(&pTmpStr);
        }
    }

    return error;
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
    const char * pEllipIdLocation = "smXML:MD_Metadata/referenceSystemInfo/smXML:MD_CRS/verticalDatum/smXML:RS_Identifier/code";

    DOMNode *pEllipIdNode = bagGetXMLNodeByName(metaData->parser->getDocument(), pEllipIdLocation);

    bagError error = 0;
    if (pEllipIdNode)
    {
        DOMNode *pTmpNode = pEllipIdNode->getFirstChild();
        if (pTmpNode)
        {
            char *pTmpStr = XMLString::transcode(pTmpNode->getNodeValue());

            if (strlen(pTmpStr) > bufferSize)
            {
                strncpy(buffer, pTmpStr, sizeof(char) * bufferSize);
                buffer[bufferSize - 1] = '\0';
            }
            else
            {
                strcpy(buffer, pTmpStr);
            }

            XMLString::release(&pTmpStr);
        }
    }

    return error;
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

    bagError error = 0;
    if (pEllipIdNode)
    {
        DOMNode *pTmpNode = pEllipIdNode->getFirstChild();
        if (pTmpNode)
        {
            char *pTmpStr = XMLString::transcode(pTmpNode->getNodeValue());

            if (strlen(pTmpStr) > bufferSize)
            {
                strncpy(buffer, pTmpStr, sizeof(char) * bufferSize);
                buffer[bufferSize - 1] = '\0';
            }
            else
            {
                strcpy(buffer, pTmpStr);
            }

            XMLString::release(&pTmpStr);
        }
    }

    return error;
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
\param uncrtType
    \li Modified to contain the type of uncertainty represented in this BAG.
    See BAG_UNCERT_TYPE in bag.h for a complete listing.
\return
    \li Error code.
*/
//************************************************************************
bagError bagGetUncertantyType(
    bagMetaData metaData,
    u32 *uncrtType
    )
{
    bagError error = BAG_SUCCESS;
    *uncrtType = Unknown_Uncert;

    const char * pUncrtType = "smXML:MD_Metadata/identificationInfo/smXML:BAG_DataIdentification/verticalUncertaintyType";
    DOMNode *pUncrtNode = bagGetXMLNodeByName(metaData->parser->getDocument(), pUncrtType);

    if (pUncrtNode)
    {
        DOMNode *pTmpNode = pUncrtNode->getFirstChild();
        if (pTmpNode)
        {
            char *pTmpStr = XMLString::transcode(pTmpNode->getNodeValue());
            if (pTmpStr)
            {
                // convert the string to data type.

                //  Generic version so we don't need to check for WIN32

                char *lower = new char[strlen (pTmpStr) + 1];

                for (unsigned int i = 0 ; i < strlen (pTmpStr) ; i++) lower[i] = tolower (pTmpStr[i]);
                lower[strlen (pTmpStr)] = 0;

                if (!strcmp(lower, "raw std dev"))
                    *uncrtType = Raw_Std_Dev;
                else if (!strcmp(lower, "cube std dev"))
                    *uncrtType = CUBE_Std_Dev;
                else if (!strcmp(lower, "product uncert"))
                    *uncrtType = Product_Uncert;
                else if (!strcmp(lower, "historical std dev"))
                    *uncrtType = Historical_Std_Dev;
            }

            XMLString::release(&pTmpStr);
        }
    }
    else
        error = BAG_METADTA_UNCRT_MISSING;

    return error;
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
\param depthCorrectionType
    \li Modified to contain the type of depth correction represented in this BAG.
    See BAG_DEPTH_CORRECTION_TYPES in bag.h for a complete listing.
\return
    \li Error code.
*/
//************************************************************************
bagError bagGetDepthCorrectionType(
    bagMetaData metaData,
    u32 *depthCorrectionType
    )
{
    bagError error = BAG_SUCCESS;
    *depthCorrectionType = (unsigned int) NULL_GENERIC;

    const char * pDepthCorrectionType = "smXML:MD_Metadata/identificationInfo/smXML:BAG_DataIdentification/depthCorrectionType";
    DOMNode *pDepthCorrectionNode = bagGetXMLNodeByName(metaData->parser->getDocument(), pDepthCorrectionType);

    if (pDepthCorrectionNode)
    {
        DOMNode *pTmpNode = pDepthCorrectionNode->getFirstChild();
        if (pTmpNode)
        {
            char *pTmpStr = XMLString::transcode(pTmpNode->getNodeValue());
            if (pTmpStr)
            {
                // convert the string to data type.

                //  Generic version so we don't need to check for WIN3

                char *lower = new char[strlen (pTmpStr) + 1];

                for (unsigned int i = 0 ; i < strlen (pTmpStr) ; i++) lower[i] = tolower (pTmpStr[i]);
                lower[strlen (pTmpStr)] = 0;

                if (!strcmp(lower, "true depth"))
                    *depthCorrectionType = True_Depth;
                else if (!strcmp(lower, "nominal at 1500 m/s"))
                    *depthCorrectionType = Nominal_Depth_Meters;
                else if (!strcmp(lower, "nominal at 4800 ft/s"))
                    *depthCorrectionType = Nominal_Depth_Feet;
                else if (!strcmp(lower, "corrected via carter's tables"))
                    *depthCorrectionType = Corrected_Carters;
                else if (!strcmp(lower, "corrected via matthew's tables"))
                    *depthCorrectionType = Corrected_Matthews;
                else if (!strcmp(lower, "unknown"))
                    *depthCorrectionType = Unknown_Correction;
            }

            XMLString::release(&pTmpStr);
        }
    }
    else
        error = BAG_METADTA_DPTHCORR_MISSING;

    return error;
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
    bagError error = BAG_SUCCESS;
    *nodeGroupType = (u8)0;

    const char * pNodeType = "smXML:MD_Metadata/identificationInfo/smXML:BAG_DataIdentification/nodeGroupType";
    DOMNode *pNodeGroupNode = bagGetXMLNodeByName(metaData->parser->getDocument(), pNodeType);

    if (pNodeGroupNode)
    {
        DOMNode *pTmpNode = pNodeGroupNode->getFirstChild();
        if (pTmpNode)
        {
            char *pTmpStr = XMLString::transcode(pTmpNode->getNodeValue());
            if (pTmpStr)
            {
                // convert the string to data type.

                //  Generic version so we don't need to check for WIN3

                char *lower = new char[strlen (pTmpStr) + 1];

                for (unsigned int i = 0 ; i < strlen (pTmpStr) ; i++) lower[i] = tolower (pTmpStr[i]);
                lower[strlen (pTmpStr)] = 0;

                if (!strcmp(lower, "cube solution"))
                    *nodeGroupType = CUBE_Solution;
                else if (!strcmp(lower, "product solution"))
                    *nodeGroupType = Product_Solution;
                else
                    *nodeGroupType = Unknown_Solution;
            }

            XMLString::release(&pTmpStr);
        }
    }

    return error;
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
    bagError error = BAG_SUCCESS;
    *nodeGroupType = (u8) 0;

    const char * pNodeType = "smXML:MD_Metadata/identificationInfo/smXML:BAG_DataIdentification/elevationSolutionGroupType";
    DOMNode *pElevationSolutionNode = bagGetXMLNodeByName(metaData->parser->getDocument(), pNodeType);

    if (pElevationSolutionNode)
    {
        DOMNode *pTmpNode = pElevationSolutionNode->getFirstChild();
        if (pTmpNode)
        {
            char *pTmpStr = XMLString::transcode(pTmpNode->getNodeValue());
            if (pTmpStr)
            {
                // convert the string to data type.

                //  Generic version so we don't need to check for WIN3

                char *lower = new char[strlen (pTmpStr) + 1];

                for (unsigned int i = 0 ; i < strlen (pTmpStr) ; i++) lower[i] = tolower (pTmpStr[i]);
                lower[strlen (pTmpStr)] = 0;

                if (!strcmp(lower, "cube solution"))
                    *nodeGroupType = CUBE_Solution;
                else if (!strcmp(lower, "product solution"))
                    *nodeGroupType = Product_Solution;
                else
                    *nodeGroupType = Unknown_Solution;
            }

            XMLString::release(&pTmpStr);
        }
    }

    return error;
}

