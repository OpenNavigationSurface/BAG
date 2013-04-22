//************************************************************************
//
//      Open Navigation Surface Working Group, 2013
//
//************************************************************************
#include "bag_metadata.h"
#include "bag_legacy.h"
#include "bag_errors.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>

#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/xmlschemas.h>

extern Bool initResponsibleParty(BAG_RESPONSIBLE_PARTY * responsibleParty);
extern Bool initSourceInfo(BAG_SOURCE * sourceInfo);

namespace
{
    
//! The location of the BAG home folder.
static std::string bagHomeFolder;

//! The relative path from the BAG home folder, to the Bag schema file.
static std::string ons_schema_location = "ISO19139/bag/bag.xsd";

//! Utility class to convert an encoded XML string.
class EncodedString
{
public:
    //************************************************************************
    //! Constructor
    /*!
    \param doc
        \li The XML document that the string is from.
    \param string
        \li The string to be converted.
    */
    //************************************************************************
    EncodedString(xmlDoc &doc, const char *string)
    {
        this->m_pEncodedString = xmlEncodeEntitiesReentrant(&doc, (const xmlChar *)string);
    }

    //************************************************************************
    //! Destructor.
    //************************************************************************
    ~EncodedString()
    {
        xmlFree(this->m_pEncodedString);
    }

    //************************************************************************
    //! Conversion operator.
    /*!
    \return 
        \li The encoded string.
    */
    //************************************************************************
    operator xmlChar*() const
    {
        return this->m_pEncodedString;
    }

private:
    //! The encoded string.
    xmlChar* m_pEncodedString;
};

//************************************************************************
//! Get the current setting for the BAG_HOME directory.
/*!
\return
    \li If the user has called setBagHomeDirectory(), then that
        value is returned, otherwise, getenv("BAG_HOME") is
        returned.
*/
//************************************************************************
std::string getBagHomeDirectory()
{
    //If the BAG home folder has been set, then return it.
    if (!bagHomeFolder.empty())
        return bagHomeFolder;

    //Else we will return the environment variable.
    return std::string(getenv("BAG_HOME"));
}

//************************************************************************
//! Set the location of the BAG 'home' directory.
/*!
\param homeFolder
    \li The new location for the BAG home directory.
*/
//************************************************************************
void setBagHomeDirectory(const u8 *homeFolder)
{
    bagHomeFolder = (const char *)homeFolder;
}

//************************************************************************
//! Convert a string to a double value.
/*!
\param value
    \li The intput string to be converted.
\return
    \li The doulbe value.
*/
//************************************************************************
double toDouble(const std::string &value)
{
    std::stringstream lineStream;
    lineStream.imbue(std::locale::classic());
    lineStream << value;

    double dblValue = 0.0;
    lineStream >> dblValue;

    return dblValue;
}

//************************************************************************
//! Get the name of a node.
/*!
\param value
    \li The intput string to be converted.
\return
    \li The doulbe value.
*/
//************************************************************************
std::string getNodeName(const xmlNode &node)
{
    std::string name;

    // append the namespace prefix
    const xmlNs* nameSpace = node.ns;
    if (nameSpace)
    {
        name = (const char*)nameSpace->prefix;
        name += ':';
    }

    name += (const char*)node.name;
    return name;
}

//************************************************************************
//! Find all the nodes matching the supplied xpath pattern.
/*!
\param relativeNode
    \li The node to start searching from.
\param searchString
    \li The pattern to be searched for.  The string conforms to the xpath
        searching syntax.  For detailed information, see http://www.w3schools.com/xpath/xpath_syntax.asp
\return
    \li The list of XML nodes that match \e searchString
*/
//************************************************************************
std::vector<const xmlNode*> findNodes(const xmlNode &relativeNode, const char *searchString)
{
    std::vector<const xmlNode*> retList;

    //Get the root node of the document.
    const xmlNode *pRoot = xmlDocGetRootElement(relativeNode.doc);
    if (pRoot == NULL)
        return std::vector<const xmlNode*>();

    //If the xPath context has not been initialized yet, do it now.
    xmlXPathContext *pContext = xmlXPathNewContext(relativeNode.doc);
    if (pContext == NULL)
        return std::vector<const xmlNode*>();

    pContext->node = const_cast<xmlNode*>(&relativeNode);

    //Register any namespaces with the xPath context.
    const xmlNs *xmlNameSpace = pRoot->nsDef;
    while (xmlNameSpace != NULL)
    {
        if (xmlNameSpace->prefix != NULL)
        {
            const int ret = xmlXPathRegisterNs(pContext, xmlNameSpace->prefix, xmlNameSpace->href);
            if (ret != 0)
            {
                //Error
                 xmlXPathFreeContext(pContext);
                return std::vector<const xmlNode*>();
            }
        }

        xmlNameSpace = xmlNameSpace->next;
    }

    //Encode the specified search string.
    const EncodedString encodedSearch(*relativeNode.doc, searchString);

    //Evaluate the expression.
    xmlXPathObject *pPathObject = xmlXPathEvalExpression(encodedSearch, pContext);
    if (pPathObject == NULL)
    {
        //Error
         xmlXPathFreeContext(pContext);
        return std::vector<const xmlNode*>();
    }

    //Add each value that was returned.
    if (pPathObject->nodesetval != NULL)
    {
        for (int i = 0; i < pPathObject->nodesetval->nodeNr; i++)
            retList.push_back(pPathObject->nodesetval->nodeTab[i]);
    }

    xmlXPathFreeObject(pPathObject);
    xmlXPathFreeContext(pContext);

    return retList;
}

//************************************************************************
//! Find a single node matching the given xpath pattern.
/*!
\param relativeNode
    \li The node to start searching from.
\param searchString
    \li The pattern to be searched for.  The string conforms to the xpath
        searching syntax.  For detailed information, see http://www.w3schools.com/xpath/xpath_syntax.asp
\return
    \li The first XML node that matches \e searchString
*/
//************************************************************************
const xmlNode* findNode(const xmlNode &relativeNode, const char *searchString)
{
    std::vector<const xmlNode*> retList = findNodes(relativeNode, searchString);
    if (retList.empty())
        return NULL;

    return retList.front();
}

//************************************************************************
//! Get the named property from an XML node.
/*!
\param current
    \li The node to retreive the property from.
\param propertyName
    \li The name of the property to be retreived.
\return
    \li The property maching \e propertyName from \e current.
*/
//************************************************************************
std::string getProperty(const xmlNode &current, const char *propertyName)
{
     // Retrieve the property.
    xmlChar * temp = xmlGetProp(const_cast<xmlNode *>(&current), EncodedString(*current.doc, propertyName));
    if(temp == NULL)
        return std::string();

    const std::string value((const char *)temp);

    // Free the memory allocated by xmlGetProp().
    xmlFree(temp);

    return value;
}

//************************************************************************
//! Get the contents of an XML node.
/*!
\param current
    \li The node to retreive the contents from.
\return
    \li The contents of \e current.
*/
//************************************************************************
std::string getContents(const xmlNode &current)
{
    std::string contents;

    // Get the children of the current element.
    const xmlNode * text = current.children;

    // Concatenate all the text elements.
    while(text != NULL)
    {
        if (text->type == XML_TEXT_NODE && text->content != NULL)
            contents += (const char *)text->content;

        text = text->next;
    }

    return contents;
}

//************************************************************************
//! Get the named property from an XML node as a string.
/*!
\param node
    \li The node to begin searching from.
\param searchPath
    \li The pattern to be searched for.  The string conforms to the xpath
        searching syntax.  For detailed information, see http://www.w3schools.com/xpath/xpath_syntax.asp
\param propertyName
    \li The name of the property to be retreived from the node maching 
        \e searchPath
\return
    \li The specified property as a string value.  NULL is returned if
        a node matching \e searchPath cannot be found.
*/
//************************************************************************
u8* getPropertyAsString(const xmlNode &node, const char *searchPath, const char * propertyName)
{
    const xmlNode *pNode = findNode(node, searchPath);
    if (pNode == NULL)
        return NULL;

    const std::string &value = getProperty(*pNode, propertyName);
    return (u8*)_strdup(value.c_str());
}

//************************************************************************
//! Get the contents of an XML node as a string.
/*!
\param node
    \li The node to begin searching from.
\param searchPath
    \li The pattern to be searched for.  The string conforms to the xpath
        searching syntax.  For detailed information, see http://www.w3schools.com/xpath/xpath_syntax.asp
\return
    \li The contents of \e node as a string value.  NULL is returned if
        a node matching \e searchPath cannot be found.
*/
//************************************************************************
u8* getContentsAsString(const xmlNode &node, const char *searchPath)
{
    const xmlNode *pNode = findNode(node, searchPath);
    if (pNode == NULL)
        return NULL;

    const std::string &value = getContents(*pNode);
    return (u8*)_strdup(value.c_str());
}

//************************************************************************
//! Get the contents of an XML node as an integer.
/*!
\param node
    \li The node to begin searching from.
\param searchPath
    \li The pattern to be searched for.  The string conforms to the xpath
        searching syntax.  For detailed information, see http://www.w3schools.com/xpath/xpath_syntax.asp
\return
    \li The contents of \e node as an integer value.  0 is returned if
        a node matching \e searchPath cannot be found.
*/
//************************************************************************
s32 getContentsAsInt(const xmlNode &node, const char *searchPath)
{
    const xmlNode *pNode = findNode(node, searchPath);
    if (pNode == NULL)
        return 0;

    const std::string &value = getContents(*pNode);
    return (s32)toDouble(value);
}

//************************************************************************
//! Get the contents of an XML node as a 64 bit floating point value.
/*!
\param node
    \li The node to begin searching from.
\param searchPath
    \li The pattern to be searched for.  The string conforms to the xpath
        searching syntax.  For detailed information, see http://www.w3schools.com/xpath/xpath_syntax.asp
\return
    \li The contents of \e node as a 64 bit floating point value. 
        0.0 is returned if a node matching \e searchPath cannot be found.
*/
//************************************************************************
f64 getContentsAsFloat(const xmlNode &node, const char *searchPath)
{
    const xmlNode *pNode = findNode(node, searchPath);
    if (pNode == NULL)
        return 0.0;

    const std::string &value = getContents(*pNode);
    return toDouble(value);
}

//************************************************************************
//! Get the contents of an XML node as a boolean value.
/*!
\param node
    \li The node to begin searching from.
\param searchPath
    \li The pattern to be searched for.  The string conforms to the xpath
        searching syntax.  For detailed information, see http://www.w3schools.com/xpath/xpath_syntax.asp
\return
    \li The contents of \e node as boolean value.  False is returned if a node 
    matching \e searchPath cannot be found.
*/
//************************************************************************
Bool getContentsAsBool(const xmlNode &node, const char *searchPath)
{
    const xmlNode *pNode = findNode(node, searchPath);
    if (pNode == NULL)
        return False;

    const std::string &value = getContents(*pNode);
    return (value == "0" || value == "false") ? False : True;
}

}   //namespace

//************************************************************************
//! Decode a BAG_RESPONSIBLE_PARTY from the supplied XML node.
/*!
\param node
    \li The XML node containing the responsible party information.
\param responsibleParty
    \li Modified to contain the responsible party information from \e node.
\parma schemaVersion
    \li The version of the schema stored in \e node.
\return
    \li True if the information was found and decoded properly, False if
        an error occurs.
*/
//************************************************************************
Bool decodeResponsibleParty(const xmlNode &node, BAG_RESPONSIBLE_PARTY * responsibleParty, u16 schemaVersion)
{
    if (schemaVersion == 1)
    {
        //smXML:CI_ResponsibleParty/individualName
        responsibleParty->individualName = getContentsAsString(node, "smXML:CI_ResponsibleParty/individualName");

        //smXML:CI_ResponsibleParty/organisationName
        responsibleParty->organisationName = getContentsAsString(node, "smXML:CI_ResponsibleParty/organisationName");

        //smXML:CI_ResponsibleParty/positionName
        responsibleParty->positionName = getContentsAsString(node, "smXML:CI_ResponsibleParty/positionName");

        //smXML:CI_ResponsibleParty/role
        responsibleParty->role = getContentsAsString(node, "smXML:CI_ResponsibleParty/role");
    }
    else if (schemaVersion == 2)
    {
        //gmd:CI_ResponsibleParty/gmd:individualName
        responsibleParty->individualName = getContentsAsString(node, "gmd:CI_ResponsibleParty/gmd:individualName/gco:CharacterString");

        //gmd:CI_ResponsibleParty/gmd:organisationName
        responsibleParty->organisationName = getContentsAsString(node, "gmd:CI_ResponsibleParty/gmd:organisationName/gco:CharacterString");

        //gmd:CI_ResponsibleParty/gmd:positionName
        responsibleParty->positionName = getContentsAsString(node, "gmd:CI_ResponsibleParty/gmd:positionName/gco:CharacterString");

        //gmd:CI_ResponsibleParty/gmd:role
        responsibleParty->role = getContentsAsString(node, "gmd:CI_ResponsibleParty/gmd:role/gmd:CI_RoleCode");
    }

    return True;
}

//************************************************************************
//! Decode a BAG_LEGAL_CONSTRAINTS from the supplied XML node.
/*!
\param node
    \li The XML node containing the legal constraints information.
\param legalConstraints
    \li Modified to contain the legal constraints information from \e node.
\parma schemaVersion
    \li The version of the schema stored in \e node.
\return
    \li True if the information was found and decoded properly, False if
        an error occurs.
*/
//************************************************************************
Bool decodeLegalConstraints(const xmlNode &node, BAG_LEGAL_CONSTRAINTS * legalConstraints, u16 schemaVersion)
{
    if (schemaVersion == 1)
    {
        //smXML:MD_LegalConstraints/useConstraints
        legalConstraints->useConstraints = getContentsAsString(node, "smXML:MD_LegalConstraints/useConstraints");

        //smXML:MD_LegalConstraints/otherConstraints
        legalConstraints->otherConstraints = getContentsAsString(node, "smXML:MD_LegalConstraints/otherConstraints");
    }
    else if (schemaVersion == 2)
    {
        //gmd:MD_LegalConstraints/gmd:useConstraints
        legalConstraints->useConstraints = getContentsAsString(node, "gmd:MD_LegalConstraints/gmd:useConstraints/gmd:MD_RestrictionCode");

        //gmd:MD_LegalConstraints/gmd:otherConstraints
        legalConstraints->otherConstraints = getContentsAsString(node, "gmd:MD_LegalConstraints/gmd:otherConstraints/gco:CharacterString");
    }

    return True;
}

//************************************************************************
//! Decode a BAG_SECURITY_CONSTRAINTS from the supplied XML node.
/*!
\param node
    \li The XML node containing the security constraints information.
\param securityConstraints
    \li Modified to contain the security constraints information from \e node.
\parma schemaVersion
    \li The version of the schema stored in \e node.
\return
    \li True if the information was found and decoded properly, False if
        an error occurs.
*/
//************************************************************************
Bool decodeSecurityConstraints(const xmlNode &node, BAG_SECURITY_CONSTRAINTS * securityConstraints, u16 schemaVersion)
{
    if (schemaVersion == 1)
    {
        //smXML:MD_SecurityConstraints/classification
        securityConstraints->classification = getContentsAsString(node, "smXML:MD_SecurityConstraints/classification");

        //smXML:MD_SecurityConstraints/userNote
        securityConstraints->userNote = getContentsAsString(node, "smXML:MD_SecurityConstraints/userNote");
    }
    else if (schemaVersion == 2)
    {
        //gmd:MD_SecurityConstraints/gmd:classification
        securityConstraints->classification = getContentsAsString(node, "gmd:MD_SecurityConstraints/gmd:classification/gmd:MD_ClassificationCode");

        //gmd:MD_SecurityConstraints/gmd:userNote
        securityConstraints->userNote = getContentsAsString(node, "gmd:MD_SecurityConstraints/gmd:userNote/gco:CharacterString");
    }

    return True;
}

//************************************************************************
//! Decode a BAG_SOURCE from the supplied XML node.
/*!
\param node
    \li The XML node containing the source information.
\param sourceInfo
    \li Modified to contain the source information from \e node.
\parma schemaVersion
    \li The version of the schema stored in \e node.
\return
    \li True if the information was found and decoded properly, False if
        an error occurs.
*/
//************************************************************************
Bool decodeSourceInfo(const xmlNode &node, BAG_SOURCE * sourceInfo, u16 schemaVersion)
{
    if (schemaVersion == 1)
    {
        //smXML:LI_Source/description
        sourceInfo->description = getContentsAsString(node, "smXML:LI_Source/description");
    }
    else if (schemaVersion == 2)
    {
        //gmd:LI_Source/gmd:description
        sourceInfo->description = getContentsAsString(node, "gmd:LI_Source/gmd:description/gco:CharacterString");

        //gmd:LI_Source/gmd:sourceCitation/gmd:CI_Citation/gmd:title
        sourceInfo->title = getContentsAsString(node, "gmd:LI_Source/gmd:sourceCitation/gmd:CI_Citation/gmd:title/gco:CharacterString");

        //gmd:LI_Source/gmd:sourceCitation/gmd:CI_Citation/gmd:CI_Date/gmd:date
        sourceInfo->date = getContentsAsString(node, "gmd:LI_Source/gmd:sourceCitation/gmd:CI_Citation/gmd:date/gmd:CI_Date/gmd:date/gco:Date");

        //gmd:LI_Source/gmd:sourceCitation/gmd:CI_Citation/gmd:CI_Date/gmd:dateType
        sourceInfo->dateType = getContentsAsString(node, "gmd:LI_Source/gmd:sourceCitation/gmd:CI_Citation/gmd:date/gmd:CI_Date/gmd:dateType/gmd:CI_DateTypeCode");

        //gmd:LI_Source/gmd:sourceCitation/gmd:CI_Citation/gmd:citedResponsibleParty
        const std::vector<const xmlNode*> partyNodes = findNodes(node, "gmd:LI_Source/gmd:sourceCitation/gmd:CI_Citation/gmd:citedResponsibleParty");
        if (!partyNodes.empty())
        {
            sourceInfo->numberOfResponsibleParties = (u32)partyNodes.size();
            sourceInfo->responsibleParties = (BAG_RESPONSIBLE_PARTY*)malloc(sizeof(BAG_RESPONSIBLE_PARTY) * sourceInfo->numberOfResponsibleParties);

            for (u32 i = 0; i < sourceInfo->numberOfResponsibleParties; i++)
            {
                initResponsibleParty(&sourceInfo->responsibleParties[i]);
                if (!decodeResponsibleParty(*partyNodes[i], &sourceInfo->responsibleParties[i], schemaVersion))
                    return False;
            }
        }
    }

    return True;
}

//************************************************************************
//! Decode a BAG_PROCESS_STEP from the supplied XML node.
/*!
\param node
    \li The XML node containing the process information.
\param processStep
    \li Modified to contain the process information from \e node.
\parma schemaVersion
    \li The version of the schema stored in \e node.
\return
    \li True if the information was found and decoded properly, False if
        an error occurs.
*/
//************************************************************************
Bool decodeProcessStep(const xmlNode &node, BAG_PROCESS_STEP * processStep, u16 schemaVersion)
{
    if (schemaVersion == 1)
    {
        //smXML:BAG_ProcessStep/description
        processStep->description = getContentsAsString(node, "smXML:BAG_ProcessStep/description");

        //smXML:BAG_ProcessStep/dateTime
        processStep->dateTime = getContentsAsString(node, "smXML:BAG_ProcessStep/dateTime");

        //smXML:BAG_ProcessStep/trackingId
        processStep->trackingId = getContentsAsString(node, "smXML:BAG_ProcessStep/trackingId");

        //smXML:BAG_ProcessStep/processor
        const std::vector<const xmlNode*> processorNodes = findNodes(node, "smXML:BAG_ProcessStep/processor");
        if (!processorNodes.empty())
        {
            processStep->numberOfProcessors = (u32)processorNodes.size();
            processStep->processors = (BAG_RESPONSIBLE_PARTY*)malloc(sizeof(BAG_RESPONSIBLE_PARTY) * processStep->numberOfProcessors);

            for (u32 i = 0; i < processStep->numberOfProcessors; i++)
            {
                initResponsibleParty(&processStep->processors[i]);
                if (!decodeResponsibleParty(*processorNodes[i], &processStep->processors[i], schemaVersion))
                    return False;
            }
        }

        //Sources are stored at the same level as the process step, so go up a level.
        const std::vector<const xmlNode*> sourceNodes = findNodes(node, "parent::*/source");
        if (!sourceNodes.empty())
        {
            processStep->numberOfSources = (u32)sourceNodes.size();
            processStep->lineageSources = (BAG_SOURCE*)malloc(sizeof(BAG_SOURCE) * processStep->numberOfSources);

            for (u32 i = 0; i < processStep->numberOfSources; i++)
            {
                initSourceInfo(&processStep->lineageSources[i]);
                if (!decodeSourceInfo(*sourceNodes[i], &processStep->lineageSources[i], schemaVersion))
                    return False;
            }
        }
    }
    else if (schemaVersion == 2)
    {
        //bag:BAG_ProcessStep/gmd:description
        processStep->description = getContentsAsString(node, "bag:BAG_ProcessStep/gmd:description/gco:CharacterString");

        //bag:BAG_ProcessStep/gmd:dateTime
        processStep->dateTime = getContentsAsString(node, "bag:BAG_ProcessStep/gmd:dateTime/gco:DateTime");

        //bag:BAG_ProcessStep/bag:trackingId
        processStep->trackingId = getContentsAsString(node, "bag:BAG_ProcessStep/bag:trackingId/gco:CharacterString");

        //bag:BAG_ProcessStep/gmd:processor
        const std::vector<const xmlNode*> processorNodes = findNodes(node, "bag:BAG_ProcessStep/gmd:processor");
        if (!processorNodes.empty())
        {
            processStep->numberOfProcessors = (u32)processorNodes.size();
            processStep->processors = (BAG_RESPONSIBLE_PARTY*)malloc(sizeof(BAG_RESPONSIBLE_PARTY) * processStep->numberOfProcessors);

            for (u32 i = 0; i < processStep->numberOfProcessors; i++)
            {
                initResponsibleParty(&processStep->processors[i]);
                if (!decodeResponsibleParty(*processorNodes[i], &processStep->processors[i], schemaVersion))
                    return False;
            }
        }

        //bag:BAG_ProcessStep/gmd:source
        const std::vector<const xmlNode*> sourceNodes = findNodes(node, "bag:BAG_ProcessStep/gmd:source");
        if (!sourceNodes.empty())
        {
            processStep->numberOfSources = (u32)sourceNodes.size();
            processStep->lineageSources = (BAG_SOURCE*)malloc(sizeof(BAG_SOURCE) * processStep->numberOfSources);

            for (u32 i = 0; i < processStep->numberOfSources; i++)
            {
                initSourceInfo(&processStep->lineageSources[i]);
                if (!decodeSourceInfo(*sourceNodes[i], &processStep->lineageSources[i], schemaVersion))
                    return False;
            }
        }
    }

    return True;
}

//************************************************************************
//! Decode a BAG_DATA_QUALITY from the supplied XML node.
/*!
\param node
    \li The XML node containing the data quality information.
\param dataQualityInfo
    \li Modified to contain the data quality information from \e node.
\parma schemaVersion
    \li The version of the schema stored in \e node.
\return
    \li True if the information was found and decoded properly, False if
        an error occurs.
*/
//************************************************************************
Bool decodeDataQualityInfo(const xmlNode &node, BAG_DATA_QUALITY * dataQualityInfo, u16 schemaVersion)
{
    if (schemaVersion == 1)
    {
        //smXML:DQ_DataQuality/scope/smXML:DQ_Scope/level
        dataQualityInfo->scope = getContentsAsString(node, "smXML:DQ_DataQuality/scope/smXML:DQ_Scope/level");

        //smXML:DQ_DataQuality/lineage/smXML:LI_Lineage/processStep
        const std::vector<const xmlNode*> stepNodes = findNodes(node, "smXML:DQ_DataQuality/lineage/smXML:LI_Lineage/processStep");
        if (!stepNodes.empty())
        {
            dataQualityInfo->numberOfProcessSteps = (u32)stepNodes.size();
            dataQualityInfo->lineageProcessSteps = (BAG_PROCESS_STEP*)malloc(sizeof(BAG_PROCESS_STEP) * dataQualityInfo->numberOfProcessSteps);

            for (u32 i = 0; i < dataQualityInfo->numberOfProcessSteps; i++)
            {
                if (!decodeProcessStep(*stepNodes[i], &dataQualityInfo->lineageProcessSteps[i], schemaVersion))
                    return False;
            }
        }
    }
    else if (schemaVersion == 2)
    {
        //gmd:DQ_DataQuality/gmd:scope/gmd:DQ_Scope/gmd:level
        dataQualityInfo->scope = getContentsAsString(node, "gmd:DQ_DataQuality/gmd:scope/gmd:DQ_Scope/gmd:level/gmd:MD_ScopeCode");

        //gmd:DQ_DataQuality/gmd:lineage/gmd:LI_Lineage/gmd:processStep
        const std::vector<const xmlNode*> stepNodes = findNodes(node, "gmd:DQ_DataQuality/gmd:lineage/gmd:LI_Lineage/gmd:processStep");
        if (!stepNodes.empty())
        {
            dataQualityInfo->numberOfProcessSteps = (u32)stepNodes.size();
            dataQualityInfo->lineageProcessSteps = (BAG_PROCESS_STEP*)malloc(sizeof(BAG_PROCESS_STEP) * dataQualityInfo->numberOfProcessSteps);

            for (u32 i = 0; i < dataQualityInfo->numberOfProcessSteps; i++)
            {
                if (!decodeProcessStep(*stepNodes[i], &dataQualityInfo->lineageProcessSteps[i], schemaVersion))
                    return False;
            }
        }
    }

    return True;
}

//************************************************************************
//! Decode a BAG_SPATIAL_REPRESENTATION from the supplied XML node.
/*!
\param node
    \li The XML node containing the spatial representation information.
\param spatialRepresentationInfo
    \li Modified to contain the spatial representation information from \e node.
\parma schemaVersion
    \li The version of the schema stored in \e node.
\return
    \li True if the information was found and decoded properly, False if
        an error occurs.
*/
//************************************************************************
Bool decodeSpatialRepresentationInfo(const xmlNode &node, BAG_SPATIAL_REPRESENTATION * spatialRepresentationInfo, u16 schemaVersion)
{
    if (schemaVersion == 1)
    {
        //smXML:MD_Georectified/axisDimensionProperties/smXML:MD_Dimension/dimensionSize
        spatialRepresentationInfo->numberOfRows = getContentsAsInt(node, "smXML:MD_Georectified/axisDimensionProperties/smXML:MD_Dimension[dimensionName='row']/dimensionSize");

        //smXML:MD_Georectified/axisDimensionProperties/smXML:MD_Dimension/resolution/smXML:Measure
        spatialRepresentationInfo->rowResolution = getContentsAsFloat(node, "smXML:MD_Georectified/axisDimensionProperties/smXML:MD_Dimension[dimensionName='row']/resolution/smXML:Measure/smXML:value");

        //smXML:MD_Georectified/axisDimensionProperties/smXML:MD_Dimension/dimensionSize
        spatialRepresentationInfo->numberOfColumns = getContentsAsInt(node, "smXML:MD_Georectified/axisDimensionProperties/smXML:MD_Dimension[dimensionName='column']/dimensionSize");

        //smXML:MD_Georectified/axisDimensionProperties/smXML:MD_Dimension/resolution/smXML:Measure
        spatialRepresentationInfo->columnResolution = getContentsAsFloat(node, "smXML:MD_Georectified/axisDimensionProperties/smXML:MD_Dimension[dimensionName='column']/resolution/smXML:Measure/smXML:value");

        //Not stored... so just set to metres.
        spatialRepresentationInfo->resolutionUnit = (u8*)strdup("Metre");

        //smXML:MD_Georectified/cellGeometry
        spatialRepresentationInfo->cellGeometry = getContentsAsString(node, "smXML:MD_Georectified/cellGeometry");

        //smXML:MD_Georectified/transformationParameterAvailability
        spatialRepresentationInfo->transformationParameterAvailability = getContentsAsBool(node, "smXML:MD_Georectified/transformationParameterAvailability");

        //smXML:MD_Georectified/checkPointAvailability
        spatialRepresentationInfo->checkPointAvailability = getContentsAsBool(node, "smXML:MD_Georectified/checkPointAvailability");

        //smXML:MD_Georectified/cornerPoints/gml:Point
        {
            const xmlNode *pNode = findNode(node, "smXML:MD_Georectified/cornerPoints/gml:Point/gml:coordinates");
            if (pNode == NULL)
                return False;

            //Get the encoded corner values.
            const std::string &value = getContents(*pNode);

            //Decode the extents
            sscanf(value.c_str(), "%lf,%lf %lf,%lf", &spatialRepresentationInfo->llCornerX, &spatialRepresentationInfo->llCornerY,
                &spatialRepresentationInfo->urCornerX, &spatialRepresentationInfo->urCornerY);
        }
    }
    else if (schemaVersion == 2)
    {
        //gmd:MD_Georectified/gmd:axisDimensionProperties/gmd:MD_Dimension/gmd:dimensionSize
        spatialRepresentationInfo->numberOfRows = getContentsAsInt(node, "gmd:MD_Georectified/gmd:axisDimensionProperties/gmd:MD_Dimension/gmd:dimensionName/gmd:MD_DimensionNameTypeCode[@codeListValue='row']/parent::*/parent::*/gmd:dimensionSize/gco:Integer");

        //gmd:MD_Georectified/gmd:axisDimensionProperties/gmd:MD_Dimension/gmd:resolution
        spatialRepresentationInfo->rowResolution = getContentsAsFloat(node, "gmd:MD_Georectified/gmd:axisDimensionProperties/gmd:MD_Dimension/gmd:dimensionName/gmd:MD_DimensionNameTypeCode[@codeListValue='row']/parent::*/parent::*/gmd:resolution/gco:Measure");

        //gmd:MD_Georectified/gmd:axisDimensionProperties/gmd:MD_Dimension/gmd:dimensionSize
        spatialRepresentationInfo->numberOfColumns = getContentsAsInt(node, "gmd:MD_Georectified/gmd:axisDimensionProperties/gmd:MD_Dimension/gmd:dimensionName/gmd:MD_DimensionNameTypeCode[@codeListValue='column']/parent::*/parent::*/gmd:dimensionSize/gco:Integer");

        //gmd:MD_Georectified/gmd:axisDimensionProperties/gmd:MD_Dimension/gmd:resolution
        spatialRepresentationInfo->columnResolution = getContentsAsFloat(node, "gmd:MD_Georectified/gmd:axisDimensionProperties/gmd:MD_Dimension/gmd:dimensionName/gmd:MD_DimensionNameTypeCode[@codeListValue='column']/parent::*/parent::*/gmd:resolution/gco:Measure");

        //gmd:MD_Georectified/gmd:axisDimensionProperties/gmd:MD_Dimension/gmd:resolution[@uom]
        spatialRepresentationInfo->resolutionUnit = getPropertyAsString(node, "gmd:MD_Georectified/gmd:axisDimensionProperties/gmd:MD_Dimension/gmd:dimensionName/gmd:MD_DimensionNameTypeCode[@codeListValue='column']/parent::*/parent::*/gmd:resolution/gco:Measure", "uom");

        //gmd:MD_Georectified/gmd:cellGeometry
        spatialRepresentationInfo->cellGeometry = getContentsAsString(node, "gmd:MD_Georectified/gmd:cellGeometry/gmd:MD_CellGeometryCode");

        //gmd:MD_Georectified/gmd:transformationParameterAvailability
        spatialRepresentationInfo->transformationParameterAvailability = getContentsAsBool(node, "gmd:MD_Georectified/gmd:transformationParameterAvailability/gco:Boolean");

        //gmd:MD_Georectified/gmd:checkPointAvailability
        spatialRepresentationInfo->checkPointAvailability = getContentsAsBool(node, "gmd:MD_Georectified/gmd:checkPointAvailability/gco:Boolean");

        //gmd:MD_Georectified/gmd:cornerPoints/gml:Point
        {
            const xmlNode *pNode = findNode(node, "gmd:MD_Georectified/gmd:cornerPoints/gml:Point/gml:coordinates");
            if (pNode == NULL)
                return False;

            //Get the encoded corner values.
            const std::string &value = getContents(*pNode);

            //Decode the extents
            sscanf(value.c_str(), "%lf,%lf %lf,%lf", &spatialRepresentationInfo->llCornerX, &spatialRepresentationInfo->llCornerY,
                &spatialRepresentationInfo->urCornerX, &spatialRepresentationInfo->urCornerY);
        }
    }

    return True;
}

//************************************************************************
//! Decode a BAG_IDENTIFICATION from the supplied XML node.
/*!
\param node
    \li The XML node containing the data identification information.
\param dataIdentificationInfo
    \li Modified to contain the data identification information from \e node.
\parma schemaVersion
    \li The version of the schema stored in \e node.
\return
    \li True if the information was found and decoded properly, False if
        an error occurs.
*/
//************************************************************************
Bool decodeDataIdentificationInfo(const xmlNode &node, BAG_IDENTIFICATION * dataIdentificationInfo, u16 schemaVersion)
{
    if (schemaVersion == 1)
    {
        //smXML:BAG_DataIdentification/citation/smXML:CI_Citation/title
        dataIdentificationInfo->title = getContentsAsString(node, "smXML:BAG_DataIdentification/citation/smXML:CI_Citation/title");

        //smXML:BAG_DataIdentification/citation/smXML:CI_Citation/date/smXML:CI_Date/date
        dataIdentificationInfo->date = getContentsAsString(node, "smXML:BAG_DataIdentification/citation/smXML:CI_Citation/date/smXML:CI_Date/date");

        //smXML:BAG_DataIdentification/citation/smXML:CI_Citation/date/smXML:CI_Date/dateType
        dataIdentificationInfo->dateType = getContentsAsString(node, "smXML:BAG_DataIdentification/citation/smXML:CI_Citation/date/smXML:CI_Date/dateType");

        //smXML:BAG_DataIdentification/citation/smXML:CI_Citation/citedResponsibleParty"
        const std::vector<const xmlNode*> partyNodes = findNodes(node, "smXML:BAG_DataIdentification/citation/smXML:CI_Citation/citedResponsibleParty");
        if (!partyNodes.empty())
        {
            dataIdentificationInfo->numberOfResponsibleParties = (u32)partyNodes.size();
            dataIdentificationInfo->responsibleParties = (BAG_RESPONSIBLE_PARTY*)malloc(sizeof(BAG_RESPONSIBLE_PARTY) * dataIdentificationInfo->numberOfResponsibleParties);

            for (u32 i = 0; i < dataIdentificationInfo->numberOfResponsibleParties; i++)
            {
                initResponsibleParty(&dataIdentificationInfo->responsibleParties[i]);
                if (!decodeResponsibleParty(*partyNodes[i], &dataIdentificationInfo->responsibleParties[i], schemaVersion))
                    return False;
            }
        }

        //smXML:BAG_DataIdentification/abstract
        dataIdentificationInfo->abstractString = getContentsAsString(node, "smXML:BAG_DataIdentification/abstract");

        //smXML:BAG_DataIdentification/status
        dataIdentificationInfo->status = getContentsAsString(node, "smXML:BAG_DataIdentification/status");

        //smXML:BAG_DataIdentification/spatialRepresentationType
        dataIdentificationInfo->spatialRepresentationType = getContentsAsString(node, "smXML:BAG_DataIdentification/spatialRepresentationType");

        //smXML:BAG_DataIdentification/language
        dataIdentificationInfo->language = getContentsAsString(node, "smXML:BAG_DataIdentification/language");

        //Doesn't appear to be set, so always set to utf8
        dataIdentificationInfo->character_set = (u8*)strdup("utf8");

        //smXML:BAG_DataIdentification/topicCategory
        dataIdentificationInfo->topicCategory = getContentsAsString(node, "smXML:BAG_DataIdentification/topicCategory");

        //smXML:BAG_DataIdentification/extent/smXML:EX_Extent/geographicElement/smXML:EX_GeographicBoundingBox/
        dataIdentificationInfo->westBoundingLongitude = getContentsAsFloat(node,
            "smXML:BAG_DataIdentification/extent/smXML:EX_Extent/geographicElement/smXML:EX_GeographicBoundingBox/westBoundLongitude");
        dataIdentificationInfo->eastBoundingLongitude = getContentsAsFloat(node,
            "smXML:BAG_DataIdentification/extent/smXML:EX_Extent/geographicElement/smXML:EX_GeographicBoundingBox/eastBoundLongitude");
        dataIdentificationInfo->southBoundingLatitude = getContentsAsFloat(node,
            "smXML:BAG_DataIdentification/extent/smXML:EX_Extent/geographicElement/smXML:EX_GeographicBoundingBox/southBoundLatitude");
        dataIdentificationInfo->northBoundingLatitude = getContentsAsFloat(node,
            "smXML:BAG_DataIdentification/extent/smXML:EX_Extent/geographicElement/smXML:EX_GeographicBoundingBox/northBoundLatitude");

        //smXML:BAG_DataIdentification/verticalUncertaintyType
        dataIdentificationInfo->verticalUncertaintyType = getContentsAsString(node, "smXML:BAG_DataIdentification/verticalUncertaintyType");

        //smXML:BAG_DataIdentification/depthCorrectionType
        dataIdentificationInfo->depthCorrectionType = getContentsAsString(node, "smXML:BAG_DataIdentification/depthCorrectionType");

        //smXML:BAG_DataIdentification/nodeGroupType
        dataIdentificationInfo->nodeGroupType = getContentsAsString(node, "smXML:BAG_DataIdentification/nodeGroupType");

        //smXML:BAG_DataIdentification/elevationSolutionGroupType
        dataIdentificationInfo->elevationSolutionGroupType = getContentsAsString(node, "smXML:BAG_DataIdentification/elevationSolutionGroupType");
    }
    else if (schemaVersion == 2)
    {
        //bag:BAG_DataIdentification/gmd:citation/gmd:CI_Citation/gmd:title
        dataIdentificationInfo->title = getContentsAsString(node, "bag:BAG_DataIdentification/gmd:citation/gmd:CI_Citation/gmd:title/gco:CharacterString");

        //bag:BAG_DataIdentification/gmd:citation/gmd:CI_Citation/gmd:CI_Date/gmd:date
        dataIdentificationInfo->date = getContentsAsString(node, "bag:BAG_DataIdentification/gmd:citation/gmd:CI_Citation/gmd:CI_Date/gmd:date/gco:Date");

        //bag:BAG_DataIdentification/gmd:citation/gmd:CI_Citation/gmd:CI_Date/gmd:dateType
        dataIdentificationInfo->dateType = getContentsAsString(node, "bag:BAG_DataIdentification/gmd:citation/gmd:CI_Citation/gmd:CI_Date/gmd:dateType/gmd:CI_DateTypeCode");

        //bag:BAG_DataIdentification/gmd:citation/gmd:CI_Citation/gmd:citedResponsibleParty
        const std::vector<const xmlNode*> partyNodes = findNodes(node, "bag:BAG_DataIdentification/gmd:citation/gmd:CI_Citation/gmd:citedResponsibleParty");
        if (!partyNodes.empty())
        {
            dataIdentificationInfo->numberOfResponsibleParties = (u32)partyNodes.size();
            dataIdentificationInfo->responsibleParties = (BAG_RESPONSIBLE_PARTY*)malloc(sizeof(BAG_RESPONSIBLE_PARTY) * dataIdentificationInfo->numberOfResponsibleParties);

            for (u32 i = 0; i < dataIdentificationInfo->numberOfResponsibleParties; i++)
            {
                initResponsibleParty(&dataIdentificationInfo->responsibleParties[i]);
                if (!decodeResponsibleParty(*partyNodes[i], &dataIdentificationInfo->responsibleParties[i], schemaVersion))
                    return False;
            }
        }

        //bag:BAG_DataIdentification/gmd:abstract
        dataIdentificationInfo->abstractString = getContentsAsString(node, "bag:BAG_DataIdentification/gmd:abstract/gco:CharacterString");

        //bag:BAG_DataIdentification/gmd:status
        dataIdentificationInfo->status = getContentsAsString(node, "bag:BAG_DataIdentification/gmd:status/gmd:MD_ProgressCode");

        //bag:BAG_DataIdentification/gmd:spatialRepresentationType
        dataIdentificationInfo->spatialRepresentationType = getContentsAsString(node, "bag:BAG_DataIdentification/gmd:spatialRepresentationType/gmd:MD_SpatialRepresentationTypeCode");

        //bag:BAG_DataIdentification/gmd:language
        dataIdentificationInfo->language = getContentsAsString(node, "bag:BAG_DataIdentification/gmd:language/gmd:LanguageCode");

        //bag:BAG_DataIdentification/gmd:characterSet
        dataIdentificationInfo->character_set = getContentsAsString(node, "bag:BAG_DataIdentification/gmd:characterSet/gmd:MD_CharacterSetCode");

        //bag:BAG_DataIdentification/gmd:topicCategory
        dataIdentificationInfo->topicCategory = getContentsAsString(node, "bag:BAG_DataIdentification/gmd:topicCategory/gmd:MD_TopicCategoryCode");

        //bag:BAG_DataIdentification/gmd:extent/gmd:EX_Extent/gmd:geographicElement/gmd:EX_GeographicBoundingBox/
        dataIdentificationInfo->westBoundingLongitude = getContentsAsFloat(node,
            "bag:BAG_DataIdentification/gmd:extent/gmd:EX_Extent/gmd:geographicElement/gmd:EX_GeographicBoundingBox/gmd:westBoundLongitude/gco:Decimal");
        dataIdentificationInfo->eastBoundingLongitude = getContentsAsFloat(node,
            "bag:BAG_DataIdentification/gmd:extent/gmd:EX_Extent/gmd:geographicElement/gmd:EX_GeographicBoundingBox/gmd:eastBoundLongitude/gco:Decimal");
        dataIdentificationInfo->southBoundingLatitude = getContentsAsFloat(node,
            "bag:BAG_DataIdentification/gmd:extent/gmd:EX_Extent/gmd:geographicElement/gmd:EX_GeographicBoundingBox/gmd:southBoundLatitude/gco:Decimal");
        dataIdentificationInfo->northBoundingLatitude = getContentsAsFloat(node,
            "bag:BAG_DataIdentification/gmd:extent/gmd:EX_Extent/gmd:geographicElement/gmd:EX_GeographicBoundingBox/gmd:northBoundLatitude/gco:Decimal");

        //bag:BAG_DataIdentification/bag:verticalUncertaintyType
        dataIdentificationInfo->verticalUncertaintyType = getContentsAsString(node, "bag:BAG_DataIdentification/bag:verticalUncertaintyType/bag:BAG_VertUncertCode");

        //bag:BAG_DataIdentification/bag:depthCorrectionType
        dataIdentificationInfo->depthCorrectionType = getContentsAsString(node, "bag:BAG_DataIdentification/bag:depthCorrectionType/bag:BAG_DepthCorrectCode");

        //bag:BAG_DataIdentification/bag:nodeGroupType
        dataIdentificationInfo->nodeGroupType = getContentsAsString(node, "bag:BAG_DataIdentification/bag:nodeGroupType/bag:BAG_OptGroupCode");

        //bag:BAG_DataIdentification/bag:elevationSolutionGroupType
        dataIdentificationInfo->elevationSolutionGroupType = getContentsAsString(node, "bag:BAG_DataIdentification/bag:elevationSolutionGroupType/bag:BAG_OptGroupCode");
    }

    return True;
}

//************************************************************************
//! Decode a BAG_REFERENCE_SYSTEM from the supplied XML node.
/*!
\param node
    \li The XML node containing the reference system information.
\param referenceSystemInfo
    \li Modified to contain the reference system information from \e node.
\parma schemaVersion
    \li The version of the schema stored in \e node.
\return
    \li True if the information was found and decoded properly, False if
        an error occurs.
*/
//************************************************************************
Bool decodeReferenceSystemInfo(const xmlNode &node, BAG_REFERENCE_SYSTEM * referenceSystemInfo, u16 schemaVersion)
{
    if (schemaVersion == 1)
    {
        //If I have an ellipsoid, then this must be horizontal.
        u8 *ellipsoid = getContentsAsString(node, "smXML:MD_CRS/ellipsoid/smXML:RS_Identifier/code");
        if (ellipsoid != NULL)
        {
            u8 *projectionId = getContentsAsString(node, "smXML:MD_CRS/projection/smXML:RS_Identifier/code");
            u8 *datumId = getContentsAsString(node, "smXML:MD_CRS/datum/smXML:RS_Identifier/code");

            bagLegacyReferenceSystem v1Def;
            memset(&v1Def, 0, sizeof(bagLegacyReferenceSystem));

            //Store the projection information.
            v1Def.coordSys = bagCoordsys((char*)projectionId);
            free(projectionId);

            //Store the ellipsoid information.
            strncpy((char*)v1Def.geoParameters.ellipsoid, (const char*)ellipsoid, 256);
            free(ellipsoid);

            //Store the datum information.
            v1Def.geoParameters.datum = bagDatumID((char*)datumId);
            free(datumId);

            v1Def.geoParameters.zone = getContentsAsInt(node, "smXML:MD_CRS/projectionParameters/smXML:MD_ProjectionParameters/zone");
            v1Def.geoParameters.std_parallel_1 = getContentsAsFloat(node, "smXML:MD_CRS/projectionParameters/smXML:MD_ProjectionParameters/standardParallel[1]");
            v1Def.geoParameters.std_parallel_2 = getContentsAsFloat(node, "smXML:MD_CRS/projectionParameters/smXML:MD_ProjectionParameters/standardParallel[2]");
            v1Def.geoParameters.central_meridian = getContentsAsFloat(node, "smXML:MD_CRS/projectionParameters/smXML:MD_ProjectionParameters/longitudeOfCentralMeridian");
            v1Def.geoParameters.origin_latitude = getContentsAsFloat(node, "smXML:MD_CRS/projectionParameters/smXML:MD_ProjectionParameters/latitudeOfProjectionOrigin");
            v1Def.geoParameters.false_easting = getContentsAsFloat(node, "smXML:MD_CRS/projectionParameters/smXML:MD_ProjectionParameters/falseEasting");
            v1Def.geoParameters.false_northing = getContentsAsFloat(node, "smXML:MD_CRS/projectionParameters/smXML:MD_ProjectionParameters/falseNorthing");
            double scaleFactAtEq = getContentsAsFloat(node, "smXML:MD_CRS/projectionParameters/smXML:MD_ProjectionParameters/scaleFactorAtEquator");
            //double heightOfPersPoint = getContentsAsFloat(node, "smXML:MD_CRS/projectionParameters/smXML:MD_ProjectionParameters/heightOfProspectivePointAboveSurface");
            v1Def.geoParameters.longitude_of_centre = getContentsAsFloat(node, "smXML:MD_CRS/projectionParameters/smXML:MD_ProjectionParameters/longitudeOfProjectionCenter");
            v1Def.geoParameters.latitude_of_centre = getContentsAsFloat(node, "smXML:MD_CRS/projectionParameters/smXML:MD_ProjectionParameters/latitudeOfProjectionCenter");
            //double scaleAtCenterLine = getContentsAsFloat(node, "smXML:MD_CRS/projectionParameters/smXML:MD_ProjectionParameters/scaleFactorAtCenterLine");
            v1Def.geoParameters.longitude_down_from_pole = getContentsAsFloat(node, "smXML:MD_CRS/projectionParameters/smXML:MD_ProjectionParameters/straightVerticalLongitudeFromPole");
            double scaleAtProjOrigin = getContentsAsFloat(node, "smXML:MD_CRS/projectionParameters/smXML:MD_ProjectionParameters/scaleFactorAtProjectionOrigin");
            
            /* dhf */
            /* scaleFactAtEq - for mercator */
            /* scaleAtCenterLine - for oblique mercator (not supported) */
            /* scaleAtProjOrigin - for polar stereographic & transverse mercator */
            if ( v1Def.coordSys == Mercator )
    	        v1Def.geoParameters.scale_factor = scaleFactAtEq;
            if ( v1Def.coordSys == Transverse_Mercator || v1Def.coordSys == Polar_Stereo )
    	        v1Def.geoParameters.scale_factor = scaleAtProjOrigin;

            char buffer[2048];
            bagError error = bagLegacyToWkt(v1Def, buffer, 2048, NULL, 0);
            if (error)
                return False;

            referenceSystemInfo->definition = (u8*)strdup(buffer);
            referenceSystemInfo->type = (u8*)strdup("WKT");
        }
        //Else it must be vertical.
        else
        {
            u8 *datum = getContentsAsString(node, "smXML:MD_CRS/datum/smXML:RS_Identifier/code");
            
            bagLegacyReferenceSystem system;
            strncpy((char *)system.geoParameters.vertical_datum, (const char*)datum, 256);
            free(datum);

            char buffer[1024];
            bagError error = bagLegacyToWkt(system, NULL, 0, buffer, 1024);
            if (error)
                return False;

            referenceSystemInfo->definition = (u8*)strdup(buffer);
            referenceSystemInfo->type = (u8*)strdup("WKT");
        }
    }
    else if (schemaVersion == 2)
    {
        //gmd:MD_ReferenceSystem/gmd:referenceSystemIdentifier/gmd:RS_Identifier/gmd:code
        referenceSystemInfo->definition = getContentsAsString(node, "gmd:MD_ReferenceSystem/gmd:referenceSystemIdentifier/gmd:RS_Identifier/gmd:code/gco:CharacterString");

        //gmd:MD_ReferenceSystem/gmd:referenceSystemIdentifier/gmd:RS_Identifier/gmd:codeSpace
        referenceSystemInfo->type = getContentsAsString(node, "gmd:MD_ReferenceSystem/gmd:referenceSystemIdentifier/gmd:RS_Identifier/gmd:codeSpace/gco:CharacterString");
    }

    return True;
}

//************************************************************************
//! Validate an XML document against the current BAG metadata schema.
/*!
\param metadataDocument
    \li The document to be validated.
\return
    \li BAG_SUCCESS if the document passes schema validation, a BAG error
        code if an error occurs.
*/
//************************************************************************
bagError validateSchema(xmlDoc &metadataDocument)
{
    //Get the location of the BAG home directory.
    const std::string bagHome = getBagHomeDirectory();

    if (bagHome.empty())
        return BAG_METADTA_NO_HOME;
     
    //Build the full path to the schema location.
    std::string schemaFile(bagHome);
    schemaFile += "/";
    schemaFile += ons_schema_location;

    // make sure the main schema file exists.
    struct stat fStat;
    if (stat(schemaFile.c_str(), &fStat))
        return BAG_METADTA_SCHEMA_FILE_MISSING;

    // Open the schema.
    xmlDoc *pSchemaDoc = xmlParseFile(schemaFile.c_str()); 
    if (pSchemaDoc == NULL)
        return BAG_METADTA_PARSE_FAILED;

    // Parse the schema.
    xmlSchemaParserCtxt *pContext = xmlSchemaNewDocParserCtxt(pSchemaDoc);
    if (pContext == NULL)
    {
        xmlFreeDoc(pSchemaDoc);
        return BAG_METADTA_SCHEMA_SETUP_FAILED;
    }

    // Initialize the schema object.
    xmlSchema *pSchema = xmlSchemaParse(pContext);
    if (pSchema == NULL)
    {
        xmlSchemaFreeParserCtxt(pContext);
        xmlFreeDoc(pSchemaDoc);
        return BAG_METADTA_SCHEMA_SETUP_FAILED;
    }

    // Create the validation object.
    xmlSchemaValidCtxt *pValidationContext = xmlSchemaNewValidCtxt(pSchema);
    if (pValidationContext == NULL)
    {
        xmlSchemaFree(pSchema);
        xmlSchemaFreeParserCtxt(pContext);
        xmlFreeDoc(pSchemaDoc);
        return BAG_METADTA_SCHEMA_VALIDATION_SETUP_FAILED;
    }

    // Validate the document.
    int result = xmlSchemaValidateDoc(pValidationContext, &metadataDocument);

    xmlSchemaFreeValidCtxt(pValidationContext);
    xmlSchemaFree(pSchema);
    xmlSchemaFreeParserCtxt(pContext);
    xmlFreeDoc(pSchemaDoc);

    return (result == 0) ? BAG_SUCCESS : BAG_METADTA_VALIDATE_FAILED;
}

//************************************************************************
//! Import the BAG_METADATA from a version 1 schema.
/*!
    Imports the metadata from a 'version 1' BAG schema.  Version 1 schemas
    were used in BAG versions 1.0 - 1.4.  Version 2 schemas are used in
    BAG version 1.5 and newer.

\param document
    \li The XML document containing the 'version 1' schema.
\param metadata
    \li Modified to contain the BAG metadata information from \e document.
        Input should not be NULL.
\return
    \li BAG_SUCCESS if the information is successfully extracted from \e document,
        an error code otherwise.
*/
//************************************************************************
bagError bagImportMetadataFromXmlV1(xmlDoc &document, BAG_METADATA * metadata)
{
    xmlNode *pRoot = xmlDocGetRootElement(&document);
    if (pRoot == NULL)
        return BAG_METADTA_NOT_INITIALIZED;

    //gmd:language
    metadata->language = getContentsAsString(*pRoot, "/smXML:MD_Metadata/language");

    //gmd:characterSet
    metadata->characterSet = (u8*)strdup("eng");

    //gmd:hierarchyLevel
    metadata->hierarchyLevel = (u8*)strdup("dataset");
    
    //gmd:contact
    {
        const xmlNode *pNode = findNode(*pRoot, "/smXML:MD_Metadata/contact");
        if (pNode == NULL)
        {
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
        }

        if (!decodeResponsibleParty(*pNode, metadata->contact, 1))
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
    }

    //gmd:dateStamp
    metadata->dateStamp = getContentsAsString(*pRoot, "/smXML:MD_Metadata/dateStamp");

    //gmd:metadataStandardName
    metadata->metadataStandardName = getContentsAsString(*pRoot, "/smXML:MD_Metadata/metadataStandardName");

    //gmd:metadataStandardVersion
    metadata->metadataStandardVersion = getContentsAsString(*pRoot, "/smXML:MD_Metadata/metadataStandardVersion");

    //gmd:spatialRepresentationInfo
    {
        const xmlNode *pNode = findNode(*pRoot, "/smXML:MD_Metadata/spatialRepresentationInfo");
        if (pNode == NULL)
        {
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
        }

        if (!decodeSpatialRepresentationInfo(*pNode, metadata->spatialRepresentationInfo, 1))
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
    }

    //gmd:referenceSystemInfo (horizontal)
    {
        const xmlNode *pNode = findNode(*pRoot, "/smXML:MD_Metadata/referenceSystemInfo[1]");
        if (pNode == NULL)
        {
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
        }

        if (!decodeReferenceSystemInfo(*pNode, metadata->horizontalReferenceSystem, 1))
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
    }

    //gmd:referenceSystemInfo (vertical)
    {
        const xmlNode *pNode = findNode(*pRoot, "/smXML:MD_Metadata/referenceSystemInfo[2]");
        if (pNode == NULL)
        {
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
        }

        if (!decodeReferenceSystemInfo(*pNode, metadata->verticalReferenceSystem, 1))
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
    }

    //gmd:identificationInfo
    {
        const xmlNode *pNode = findNode(*pRoot, "/smXML:MD_Metadata/identificationInfo");
        if (pNode == NULL)
        {
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
        }

        if (!decodeDataIdentificationInfo(*pNode, metadata->identificationInfo, 1))
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
    }

    //gmd:dataQualityInfo
    {
        const xmlNode *pNode = findNode(*pRoot, "/smXML:MD_Metadata/dataQualityInfo");
        if (pNode == NULL)
        {
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
        }

        if (!decodeDataQualityInfo(*pNode, metadata->dataQualityInfo, 1))
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
    }

    //gmd:metadataConstraints (legal)
    {
        const xmlNode *pNode = findNode(*pRoot, "/smXML:MD_Metadata/metadataConstraints/smXML:MD_LegalConstraints/parent::*");
        if (pNode == NULL)
        {
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
        }

        if (!decodeLegalConstraints(*pNode, metadata->legalConstraints, 1))
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
    }

    //gmd:metadataConstraints (security)
    {
        const xmlNode *pNode = findNode(*pRoot, "/smXML:MD_Metadata/metadataConstraints/smXML:MD_SecurityConstraints/parent::*");
        if (pNode == NULL)
        {
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
        }

        if (!decodeSecurityConstraints(*pNode, metadata->securityConstraints, 1))
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
    }

    return BAG_SUCCESS;
}

//************************************************************************
//! Import the BAG_METADATA from a version 2 schema.
/*!
    Imports the metadata from a 'version 2' BAG schema.  Version 2 schemas 
    are used in BAG version 1.5 and newer.

\param document
    \li The XML document containing the 'version 2' schema.
\param metadata
    \li Modified to contain the BAG metadata information from \e document.
        Input should not be NULL.
\return
    \li BAG_SUCCESS if the information is successfully extracted from \e document,
        an error code otherwise.
*/
//************************************************************************
bagError bagImportMetadataFromXmlV2(xmlDoc &document, BAG_METADATA * metadata)
{
    xmlNode *pRoot = xmlDocGetRootElement(&document);
    if (pRoot == NULL)
        return BAG_METADTA_NOT_INITIALIZED;

    //gmd:fileIdentifier
    metadata->fileIdentifier = getContentsAsString(*pRoot, "/gmi:MI_Metadata/gmd:fileIdentifier/gco:CharacterString");

    //gmd:language
    metadata->language = getContentsAsString(*pRoot, "/gmi:MI_Metadata/gmd:language/gmd:LanguageCode");

    //gmd:characterSet
    metadata->characterSet = getContentsAsString(*pRoot, "/gmi:MI_Metadata/gmd:characterSet/gmd:MD_CharacterSetCode");

    //gmd:hierarchyLevel
    metadata->hierarchyLevel = getContentsAsString(*pRoot, "/gmi:MI_Metadata/gmd:hierarchyLevel/gmd:MD_ScopeCode");
    
    //gmd:contact
    {
        const xmlNode *pNode = findNode(*pRoot, "/gmi:MI_Metadata/gmd:contact");
        if (pNode == NULL)
        {
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
        }

        if (!decodeResponsibleParty(*pNode, metadata->contact, 2))
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
    }

    //gmd:dateStamp
    metadata->dateStamp = getContentsAsString(*pRoot, "/gmi:MI_Metadata/gmd:dateStamp/gco:Date");

    //gmd:metadataStandardName
    metadata->metadataStandardName = getContentsAsString(*pRoot, "/gmi:MI_Metadata/gmd:metadataStandardName/gco:CharacterString");

    //gmd:metadataStandardVersion
    metadata->metadataStandardVersion = getContentsAsString(*pRoot, "/gmi:MI_Metadata/gmd:metadataStandardVersion/gco:CharacterString");

    //gmd:spatialRepresentationInfo
    {
        const xmlNode *pNode = findNode(*pRoot, "/gmi:MI_Metadata/gmd:spatialRepresentationInfo");
        if (pNode == NULL)
        {
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
        }

        if (!decodeSpatialRepresentationInfo(*pNode, metadata->spatialRepresentationInfo, 2))
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
    }

    //gmd:referenceSystemInfo (horizontal)
    {
        const xmlNode *pNode = findNode(*pRoot, "/gmi:MI_Metadata/gmd:referenceSystemInfo[1]");
        if (pNode == NULL)
        {
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
        }

        if (!decodeReferenceSystemInfo(*pNode, metadata->horizontalReferenceSystem, 2))
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
    }

    //gmd:referenceSystemInfo (vertical)
    {
        const xmlNode *pNode = findNode(*pRoot, "/gmi:MI_Metadata/gmd:referenceSystemInfo[2]");
        if (pNode == NULL)
        {
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
        }

        if (!decodeReferenceSystemInfo(*pNode, metadata->verticalReferenceSystem, 2))
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
    }

    //gmd:identificationInfo
    {
        const xmlNode *pNode = findNode(*pRoot, "/gmi:MI_Metadata/gmd:identificationInfo");
        if (pNode == NULL)
        {
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
        }

        if (!decodeDataIdentificationInfo(*pNode, metadata->identificationInfo, 2))
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
    }

    //gmd:dataQualityInfo
    {
        const xmlNode *pNode = findNode(*pRoot, "/gmi:MI_Metadata/gmd:dataQualityInfo");
        if (pNode == NULL)
        {
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
        }

        if (!decodeDataQualityInfo(*pNode, metadata->dataQualityInfo, 2))
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
    }

    //gmd:metadataConstraints (legal)
    {
        const xmlNode *pNode = findNode(*pRoot, "/gmi:MI_Metadata/gmd:metadataConstraints/gmd:MD_LegalConstraints/parent::*");
        if (pNode == NULL)
        {
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
        }

        if (!decodeLegalConstraints(*pNode, metadata->legalConstraints, 2))
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
    }

    //gmd:metadataConstraints (security)
    {
        const xmlNode *pNode = findNode(*pRoot, "/gmi:MI_Metadata/gmd:metadataConstraints/gmd:MD_SecurityConstraints/parent::*");
        if (pNode == NULL)
        {
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
        }

        if (!decodeSecurityConstraints(*pNode, metadata->securityConstraints, 2))
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
    }

    return BAG_SUCCESS;
}

//************************************************************************
//! Import the BAG_METADATA from an XML document.
/*!
    Imports the BAG metadata information from any supported schema version.

\param document
    \li The XML document containing the 'version 2' schema.
\param metadata
    \li Modified to contain the BAG metadata information from \e document.
        Input should not be NULL.
\param doValidation
    \li True to perform the schema validation (conformance test).  False
        to simply extract the data.
\return
    \li BAG_SUCCESS on success.  A BAG error code is returned if there
        is an error, or if schema validation fails (only if \e doValidation
        is True).
*/
//************************************************************************
bagError bagImportMetadataFromXml(xmlDoc &document, BAG_METADATA * metadata, Bool doValidation)
{
    if (metadata == NULL)
        return BAG_METADTA_INVALID_HANDLE;

    xmlNode *pRoot = xmlDocGetRootElement(&document);
    if (pRoot == NULL)
        return BAG_METADTA_EMPTY_DOCUMENT;

    //Do we want to validate against the xml schema?
    if (doValidation)
    {
        bagError ret = validateSchema(document);
        if (ret)
            return ret;
    }

    //Get the name of the root node.
    const std::string rootName = getNodeName(*pRoot);

    //We will use the root node's name to figure out what version of the schema we are working with.
    const u16 schemaVersion = (rootName == "smXML:MD_Metadata") ? 1 : 2;

    return (schemaVersion == 1) ? bagImportMetadataFromXmlV1(document, metadata) :
        bagImportMetadataFromXmlV2(document, metadata);
}

//************************************************************************
//! Import the BAG_METADATA from an XML document stored in a character buffer.
/*!
    Imports the BAG metadata information from any supported schema version.

\param xmlBuffer
    \li The character buffer containing the XML document to be used for
        import.  Input should not be NULL.
\param bufferSize
    \li The size in bytes of the \e xmlBuffer.
\param metadata
    \li Modified to contain the BAG metadata information from \e xmlBuffer.
        Input should not be NULL.
\param doValidation
    \li True to perform the schema validation (conformance test).  False
        to simply extract the data.
\return
    \li BAG_SUCCESS if the information is successfully extracted from \e xmlBuffer,
        an error code otherwise.  If \e doValidation is True, an error will be returned
        if the validation fails.
*/
//************************************************************************
bagError bagImportMetadataFromXmlBuffer(const u8 *xmlBuffer, u32 bufferSize, BAG_METADATA * metadata, Bool doValidation)
{
    xmlDoc *pDocument = xmlParseMemory((const char *)xmlBuffer, bufferSize);
    if (pDocument == NULL)
        return BAG_METADTA_NOT_INITIALIZED;

    return bagImportMetadataFromXml(*pDocument, metadata, doValidation);
}

//************************************************************************
//! Import the BAG_METADATA from an XML file.
/*!
    Imports the BAG metadata information from any supported schema version.

\param fileName
    \li The fully qualified name of the XML file to be read.
\param metadata
    \li Modified to contain the BAG metadata information from \e fileName.
        Input should not be NULL.
\param doValidation
    \li True to perform the schema validation (conformance test).  False
        to simply extract the data.
\return
    \li BAG_SUCCESS if the information is successfully extracted from \e xmlBuffer,
        an error code otherwise.  If \e doValidation is True, an error will be returned
        if the validation fails.
*/
//************************************************************************
bagError bagImportMetadataFromXmlFile(const u8 *fileName, BAG_METADATA * metadata, Bool doValidation)
{
    xmlDoc *pDocument = xmlParseFile((const char *)fileName); 
    if (pDocument == NULL)
        return BAG_METADTA_NOT_INITIALIZED;

    return bagImportMetadataFromXml(*pDocument, metadata, doValidation);
}
