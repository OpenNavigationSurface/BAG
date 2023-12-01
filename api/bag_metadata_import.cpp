//************************************************************************
//
//      Open Navigation Surface Working Group, 2013
//
//************************************************************************
#include "bag_errors.h"
#include "bag_legacy_crs.h"
#include "bag_metadata_import.h"
#include "bag_metadatatypes.h"

#include <cstring>
#include <iostream>
#include <libxml/parser.h>
#include <libxml/xmlschemas.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>


namespace {

//! The location of the BAG home folder.
static std::string bagHomeFolder;

//! The relative path from the BAG home folder, to the Bag schema file.
static std::string ons_schema_location = "ISO19139/bag/bag.xsd";

//! Utility class to convert an encoded XML string.
class EncodedString final
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
    EncodedString(xmlDoc& doc, const char* string) noexcept
    {
        this->m_pEncodedString = xmlEncodeEntitiesReentrant(&doc,
            reinterpret_cast<const xmlChar*>(string));
    }

    EncodedString(const EncodedString&) = delete;
    EncodedString(EncodedString&&) = delete;
    EncodedString& operator=(const EncodedString&) = delete;
    EncodedString& operator=(EncodedString&&) = delete;

    //************************************************************************
    //! Destructor.
    //************************************************************************
    ~EncodedString() noexcept
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
    operator xmlChar*() const noexcept
    {
        return this->m_pEncodedString;
    }

private:
    //! The encoded string.
    xmlChar* m_pEncodedString = nullptr;
};

//************************************************************************
//! Convert a string to a double value.
/*!
\param value
    \li The input string to be converted.
\return
    \li The double value.
*/
//************************************************************************
double toDouble(const std::string& value)
{
    std::stringstream lineStream;
    (void)lineStream.imbue(std::locale::classic());
    lineStream << value;

    double dblValue = 0.0;
    lineStream >> dblValue;

    return dblValue;
}

//************************************************************************
//! Get the name of a node.
/*!
\param value
    \li The input string to be converted.
\return
    \li The node name.
*/
//************************************************************************
std::string getNodeName(const xmlNode& node)
{
    std::string name;

    // append the namespace prefix
    const xmlNs* nameSpace = node.ns;
    if (nameSpace)
    {
        name = reinterpret_cast<const char*>(nameSpace->prefix);
        name += ':';
    }

    name += reinterpret_cast<const char*>(node.name);

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
std::vector<xmlNode*> findNodes(
    xmlNode& relativeNode,
    const char* searchString)
{
    //Get the root node of the document.
    const xmlNode* pRoot = xmlDocGetRootElement(relativeNode.doc);
    if (!pRoot)
        return {};

    //If the xPath context has not been initialized yet, do it now.
    xmlXPathContext* pContext = xmlXPathNewContext(relativeNode.doc);
    if (!pContext)
        return {};

    pContext->node = &relativeNode;

    //Register any namespaces with the xPath context.
    const xmlNs* xmlNameSpace = pRoot->nsDef;

    while (xmlNameSpace)
    {
        if (xmlNameSpace->prefix)
        {
            const int ret = xmlXPathRegisterNs(pContext, xmlNameSpace->prefix,
                xmlNameSpace->href);
            if (ret != 0)
            {
                //Error
                 xmlXPathFreeContext(pContext);
                 return {};
            }
        }

        xmlNameSpace = xmlNameSpace->next;
    }

    //Encode the specified search string.
    const EncodedString encodedSearch{*relativeNode.doc, searchString};

    //Evaluate the expression.
    xmlXPathObject* pPathObject = xmlXPathEvalExpression(encodedSearch, pContext);
    if (!pPathObject)
    {
        //Error
        xmlXPathFreeContext(pContext);
        return {};
    }

    std::vector<xmlNode*> retList;

    //Add each value that was returned.
    if (pPathObject->nodesetval)
    {
        retList.reserve(pPathObject->nodesetval->nodeNr);

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
xmlNode* findNode(
    xmlNode& relativeNode,
    const char* searchString)
{
    auto retList = findNodes(relativeNode, searchString);
    if (retList.empty())
        return nullptr;

    return retList.front();
}

//************************************************************************
//! Get the named property from an XML node.
/*!
\param current
    \li The node to retrieve the property from.
\param propertyName
    \li The name of the property to be retrieved.
\return
    \li The property matching \e propertyName from \e current.
*/
//************************************************************************
std::string getProperty(
    const xmlNode& current,
    const char* propertyName)
{
     // Retrieve the property.
    xmlChar* temp = xmlGetProp(&current, EncodedString{*current.doc,
        propertyName});
    if (!temp)
        return {};

    const std::string value(reinterpret_cast<char*>(temp));

    // Free the memory allocated by xmlGetProp().
    xmlFree(temp);

    return value;
}

//************************************************************************
//! Get the contents of an XML node.
/*!
\param current
    \li The node to retrieve the contents from.
\return
    \li The contents of \e current.
*/
//************************************************************************
std::string getContents(const xmlNode& current)
{
    std::string contents;

    // Get the children of the current element.
    const xmlNode* text = current.children;

    // Concatenate all the text elements.
    while (text)
    {
        if (text->type == XML_TEXT_NODE && text->content)
            contents += reinterpret_cast<const char*>(text->content);

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
    \li The name of the property to be retrieved from the node matching
        \e searchPath
\return
    \li The specified property as a string value.  NULL is returned if
        a node matching \e searchPath cannot be found.
*/
//************************************************************************
char* getPropertyAsString(
    xmlNode& node,
    const char* searchPath,
    const char* propertyName)
{
    const xmlNode* pNode = findNode(node, searchPath);
    if (!pNode)
        return nullptr;

    const std::string value = getProperty(*pNode, propertyName);

    char* result = new char[value.size() + 1];
    strcpy(result, value.c_str());
    result[value.size()] = '\0';

    return result;

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
std::string getContentsAsString(
    xmlNode& node,
    const char* searchPath)
{
    const xmlNode* pNode = findNode(node, searchPath);
    if (!pNode)
        return {};

    return getContents(*pNode);
}

char* copyString(const char* source)
{
    if (!source || strlen(source) == 0)
        return nullptr;

    char* result = new char[strlen(source) + 1];
    strcpy(result, source);

    return result;
}

char* getContentsAsCharStar(
    xmlNode& node,
    const char* searchPath)
{
    return copyString(getContentsAsString(node, searchPath).c_str());
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
int32_t getContentsAsInt(
    xmlNode& node,
    const char* searchPath)
{
    auto* pNode = findNode(node, searchPath);
    if (!pNode)
        return 0;

    return static_cast<int32_t>(toDouble(getContents(*pNode)));
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
double getContentsAsFloat(
    xmlNode& node,
    const char* searchPath)
{
    const xmlNode* pNode = findNode(node, searchPath);
    if (!pNode)
        return 0.0;

    return toDouble(getContents(*pNode));
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
bool getContentsAsBool(
    xmlNode& node,
    const char* searchPath)
{
    const xmlNode* pNode = findNode(node, searchPath);
    if (!pNode)
        return false;

    const std::string value = getContents(*pNode);

    // Prevent repeated memory allocations.
    static std::string kZero{"0"};
    static std::string kFalse{"false"};

    return (value != kZero && value != kFalse);
}

}  // namespace

namespace BAG {

//************************************************************************
//! Get the current setting for the BAG_HOME directory.
/*!
\return
    \li If the user has called bagSetHomeFolder(), then that
        value is returned, otherwise, getenv("BAG_HOME") is
        returned.
*/
//************************************************************************
std::string bagGetHomeFolder()
{
    //If the BAG home folder has been set, then return it.
    if (!bagHomeFolder.empty())
        return bagHomeFolder;

    //Else we will return the environment variable.
    const char* bagHome = getenv("BAG_HOME");
    if (bagHome)
        return std::string{bagHome};

    return {};
}

//************************************************************************
//! Set the location of the BAG 'home' directory.
/*!
\param homeFolder
    \li The new location for the BAG home directory.
*/
//************************************************************************
void bagSetHomeFolder(const char* homeFolder)
{
    bagHomeFolder = homeFolder;
}

//************************************************************************
//! Decode a BagResponsibleParty from the supplied XML node.
/*!
\param node
    \li The XML node containing the responsible party information.
\param responsibleParty
    \li Modified to contain the responsible party information from \e node.
\param schemaVersion
    \li The version of the schema stored in \e node.
\return
    \li True if the information was found and decoded properly, False if
        an error occurs.
*/
//************************************************************************
bool decodeResponsibleParty(
    xmlNode& node,
    BagResponsibleParty& responsibleParty,
    uint16_t schemaVersion)
{
    if (schemaVersion == 1)
    {
        //smXML:CI_ResponsibleParty/individualName
        responsibleParty.individualName = getContentsAsCharStar(node,
            "smXML:CI_ResponsibleParty/individualName");

        //smXML:CI_ResponsibleParty/organisationName
        responsibleParty.organisationName = getContentsAsCharStar(node,
            "smXML:CI_ResponsibleParty/organisationName");

        //smXML:CI_ResponsibleParty/positionName
        responsibleParty.positionName = getContentsAsCharStar(node,
            "smXML:CI_ResponsibleParty/positionName");

        //smXML:CI_ResponsibleParty/role
        responsibleParty.role = getContentsAsCharStar(node,
            "smXML:CI_ResponsibleParty/role");
    }
    else if (schemaVersion == 2)
    {
        //gmd:CI_ResponsibleParty/gmd:individualName
        responsibleParty.individualName = getContentsAsCharStar(node,
            "gmd:CI_ResponsibleParty/gmd:individualName/gco:CharacterString");

        //gmd:CI_ResponsibleParty/gmd:organisationName
        responsibleParty.organisationName = getContentsAsCharStar(node,
            "gmd:CI_ResponsibleParty/gmd:organisationName/gco:CharacterString");

        //gmd:CI_ResponsibleParty/gmd:positionName
        responsibleParty.positionName = getContentsAsCharStar(node,
            "gmd:CI_ResponsibleParty/gmd:positionName/gco:CharacterString");

        //gmd:CI_ResponsibleParty/gmd:role
        responsibleParty.role = getContentsAsCharStar(node,
            "gmd:CI_ResponsibleParty/gmd:role/gmd:CI_RoleCode");
    }

    return true;
}

//************************************************************************
//! Decode a BagLegalConstraints from the supplied XML node.
/*!
\param node
    \li The XML node containing the legal constraints information.
\param legalConstraints
    \li Modified to contain the legal constraints information from \e node.
\param schemaVersion
    \li The version of the schema stored in \e node.
\return
    \li True if the information was found and decoded properly, False if
        an error occurs.
*/
//************************************************************************
bool decodeLegalConstraints(
    xmlNode& node,
    BagLegalConstraints& legalConstraints,
    uint16_t schemaVersion)
{
    if (schemaVersion == 1)
    {
        //smXML:MD_LegalConstraints/useConstraints
        legalConstraints.useConstraints = getContentsAsCharStar(node,
            "smXML:MD_LegalConstraints/useConstraints");

        //smXML:MD_LegalConstraints/otherConstraints
        legalConstraints.otherConstraints = getContentsAsCharStar(node,
            "smXML:MD_LegalConstraints/otherConstraints");
    }
    else if (schemaVersion == 2)
    {
        //gmd:MD_LegalConstraints/gmd:useConstraints
        legalConstraints.useConstraints = getContentsAsCharStar(node,
            "gmd:MD_LegalConstraints/gmd:useConstraints/gmd:MD_RestrictionCode");

        //gmd:MD_LegalConstraints/gmd:otherConstraints
        legalConstraints.otherConstraints = getContentsAsCharStar(node,
            "gmd:MD_LegalConstraints/gmd:otherConstraints/gco:CharacterString");
    }

    return true;
}

//************************************************************************
//! Decode a BagSecurityConstraints from the supplied XML node.
/*!
\param node
    \li The XML node containing the security constraints information.
\param securityConstraints
    \li Modified to contain the security constraints information from \e node.
\param schemaVersion
    \li The version of the schema stored in \e node.
\return
    \li True if the information was found and decoded properly, False if
        an error occurs.
*/
//************************************************************************
bool decodeSecurityConstraints(
    xmlNode& node,
    BagSecurityConstraints& securityConstraints,
    uint16_t schemaVersion)
{
    if (schemaVersion == 1)
    {
        //smXML:MD_SecurityConstraints/classification
        securityConstraints.classification = getContentsAsCharStar(node,
            "smXML:MD_SecurityConstraints/classification");

        //smXML:MD_SecurityConstraints/userNote
        securityConstraints.userNote = getContentsAsCharStar(node,
            "smXML:MD_SecurityConstraints/userNote");
    }
    else if (schemaVersion == 2)
    {
        //gmd:MD_SecurityConstraints/gmd:classification
        securityConstraints.classification = getContentsAsCharStar(node,
            "gmd:MD_SecurityConstraints/gmd:classification/gmd:MD_ClassificationCode");

        //gmd:MD_SecurityConstraints/gmd:userNote
        securityConstraints.userNote = getContentsAsCharStar(node,
            "gmd:MD_SecurityConstraints/gmd:userNote/gco:CharacterString");
    }

    return true;
}

//************************************************************************
//! Decode a BagSource from the supplied XML node.
/*!
\param node
    \li The XML node containing the source information.
\param sourceInfo
    \li Modified to contain the source information from \e node.
\param schemaVersion
    \li The version of the schema stored in \e node.
\return
    \li True if the information was found and decoded properly, False if
        an error occurs.
*/
//************************************************************************
bool decodeSourceInfo(
    xmlNode& node,
    BagSource& sourceInfo,
    uint16_t schemaVersion)
{
    if (schemaVersion == 1)
    {
        //smXML:LI_Source/description
        sourceInfo.description = getContentsAsCharStar(node,
            "smXML:LI_Source/description");
    }
    else if (schemaVersion == 2)
    {
        //gmd:LI_Source/gmd:description
        sourceInfo.description = getContentsAsCharStar(node,
            "gmd:LI_Source/gmd:description/gco:CharacterString");

        //gmd:LI_Source/gmd:sourceCitation/gmd:CI_Citation/gmd:title
        sourceInfo.title = getContentsAsCharStar(node,
            "gmd:LI_Source/gmd:sourceCitation/gmd:CI_Citation/gmd:title/gco:CharacterString");

        //gmd:LI_Source/gmd:sourceCitation/gmd:CI_Citation/gmd:CI_Date/gmd:date
        sourceInfo.date = getContentsAsCharStar(node,
            "gmd:LI_Source/gmd:sourceCitation/gmd:CI_Citation/gmd:date/gmd:CI_Date/gmd:date/gco:Date");

        //gmd:LI_Source/gmd:sourceCitation/gmd:CI_Citation/gmd:CI_Date/gmd:dateType
        sourceInfo.dateType = getContentsAsCharStar(node,
            "gmd:LI_Source/gmd:sourceCitation/gmd:CI_Citation/gmd:date/gmd:CI_Date/gmd:dateType/gmd:CI_DateTypeCode");

        //gmd:LI_Source/gmd:sourceCitation/gmd:CI_Citation/gmd:citedResponsibleParty
        const auto partyNodes = findNodes(node,
            "gmd:LI_Source/gmd:sourceCitation/gmd:CI_Citation/gmd:citedResponsibleParty");
        if (!partyNodes.empty())
        {
            sourceInfo.numberOfResponsibleParties =
                static_cast<uint32_t>(partyNodes.size());
            sourceInfo.responsibleParties = new BagResponsibleParty[sourceInfo.numberOfResponsibleParties];

            for (uint32_t i = 0; i < sourceInfo.numberOfResponsibleParties; i++)
            {
                initResponsibleParty(sourceInfo.responsibleParties[i]);
                if (!decodeResponsibleParty(*partyNodes[i],
                    sourceInfo.responsibleParties[i], schemaVersion))
                    return false;
            }
        }
    }

    return true;
}

//************************************************************************
//! Decode a BagProcessStep from the supplied XML node.
/*!
\param node
    \li The XML node containing the process information.
\param processStep
    \li Modified to contain the process information from \e node.
\param schemaVersion
    \li The version of the schema stored in \e node.
\return
    \li True if the information was found and decoded properly, False if
        an error occurs.
*/
//************************************************************************
bool decodeProcessStep(
    xmlNode& node,
    BagProcessStep& processStep,
    uint16_t schemaVersion)
{
    if (schemaVersion == 1)
    {
        //smXML:BAG_ProcessStep/description
        processStep.description = getContentsAsCharStar(node,
            "smXML:BAG_ProcessStep/description");

        //smXML:BAG_ProcessStep/dateTime
        processStep.dateTime = getContentsAsCharStar(node,
            "smXML:BAG_ProcessStep/dateTime");

        //smXML:BAG_ProcessStep/trackingId
        processStep.trackingId = getContentsAsCharStar(node,
            "smXML:BAG_ProcessStep/trackingId");

        //smXML:BAG_ProcessStep/processor
        const auto processorNodes = findNodes(node,
            "smXML:BAG_ProcessStep/processor");
        if (!processorNodes.empty())
        {
            processStep.numberOfProcessors =
                static_cast<uint32_t>(processorNodes.size());
            processStep.processors =
                new BagResponsibleParty[processStep.numberOfProcessors];

            for (uint32_t i = 0; i < processStep.numberOfProcessors; i++)
            {
                initResponsibleParty(processStep.processors[i]);
                if (!decodeResponsibleParty(*processorNodes[i],
                    processStep.processors[i], schemaVersion))
                    return false;
            }
        }

        //Sources are stored at the same level as the process step, so go up a level.
        const auto sourceNodes = findNodes(node,
            "parent::*/source");
        if (!sourceNodes.empty())
        {
            processStep.numberOfSources =
                static_cast<uint32_t>(sourceNodes.size());
            processStep.lineageSources =
                new BagSource[processStep.numberOfSources];

            for (uint32_t i = 0; i < processStep.numberOfSources; i++)
            {
                initSourceInfo(processStep.lineageSources[i]);
                if (!decodeSourceInfo(*sourceNodes[i],
                    processStep.lineageSources[i], schemaVersion))
                    return false;
            }
        }
    }
    else if (schemaVersion == 2)
    {
        //bag:BAG_ProcessStep/gmd:description
        processStep.description = getContentsAsCharStar(node,
            "//gmd:description/gco:CharacterString");

        //bag:BAG_ProcessStep/gmd:dateTime
        processStep.dateTime = getContentsAsCharStar(node,
            "//gmd:dateTime/gco:DateTime");

        //bag:BAG_ProcessStep/bag:trackingId
        processStep.trackingId = getContentsAsCharStar(node,
            "//bag:trackingId/gco:CharacterString");

        //bag:BAG_ProcessStep/gmd:processor
        const auto processorNodes = findNodes(node,
            "//gmd:processor");
        if (!processorNodes.empty())
        {
            processStep.numberOfProcessors =
                static_cast<uint32_t>(processorNodes.size());
            processStep.processors =
                new BagResponsibleParty[processStep.numberOfProcessors];

            for (uint32_t i = 0; i < processStep.numberOfProcessors; i++)
            {
                initResponsibleParty(processStep.processors[i]);
                if (!decodeResponsibleParty(*processorNodes[i],
                    processStep.processors[i], schemaVersion))
                    return false;
            }
        }

        //bag:BAG_ProcessStep/gmd:source
        const auto sourceNodes = findNodes(node, "//gmd:source");
        if (!sourceNodes.empty())
        {
            processStep.numberOfSources =
                static_cast<uint32_t>(sourceNodes.size());
            processStep.lineageSources =
                new BagSource[processStep.numberOfSources];

            for (uint32_t i = 0; i < processStep.numberOfSources; i++)
            {
                initSourceInfo(processStep.lineageSources[i]);
                if (!decodeSourceInfo(*sourceNodes[i],
                    processStep.lineageSources[i], schemaVersion))
                    return false;
            }
        }
    }

    return true;
}

//************************************************************************
//! Decode a BagDataQuality from the supplied XML node.
/*!
\param node
    \li The XML node containing the data quality information.
\param dataQualityInfo
    \li Modified to contain the data quality information from \e node.
\param schemaVersion
    \li The version of the schema stored in \e node.
\return
    \li True if the information was found and decoded properly, False if
        an error occurs.
*/
//************************************************************************
bool decodeDataQualityInfo(
    xmlNode& node,
    BagDataQuality& dataQualityInfo,
    uint16_t schemaVersion)
{
    if (schemaVersion == 1)
    {
        //smXML:DQ_DataQuality/scope/smXML:DQ_Scope/level
        dataQualityInfo.scope = getContentsAsCharStar(node,
            "smXML:DQ_DataQuality/scope/smXML:DQ_Scope/level");

        //smXML:DQ_DataQuality/lineage/smXML:LI_Lineage/processStep
        const auto stepNodes = findNodes(node,
            "smXML:DQ_DataQuality/lineage/smXML:LI_Lineage/processStep");
        if (!stepNodes.empty())
        {
            dataQualityInfo.numberOfProcessSteps =
                static_cast<uint32_t>(stepNodes.size());
            dataQualityInfo.lineageProcessSteps =
                new BagProcessStep[dataQualityInfo.numberOfProcessSteps];

            for (uint32_t i = 0; i < dataQualityInfo.numberOfProcessSteps; i++)
            {
                initProcessStep(dataQualityInfo.lineageProcessSteps[i]);
                if (!decodeProcessStep(*stepNodes[i],
                    dataQualityInfo.lineageProcessSteps[i], schemaVersion))
                    return false;
            }
        }
    }
    else if (schemaVersion == 2)
    {
        //gmd:DQ_DataQuality/gmd:scope/gmd:DQ_Scope/gmd:level
        dataQualityInfo.scope = getContentsAsCharStar(node,
            "gmd:DQ_DataQuality/gmd:scope/gmd:DQ_Scope/gmd:level/gmd:MD_ScopeCode");

        //gmd:DQ_DataQuality/gmd:lineage/gmd:LI_Lineage/gmd:processStep
        const auto stepNodes = findNodes(node,
            "gmd:DQ_DataQuality/gmd:lineage/gmd:LI_Lineage/gmd:processStep");
        if (!stepNodes.empty())
        {
            dataQualityInfo.numberOfProcessSteps =
                static_cast<uint32_t>(stepNodes.size());
            dataQualityInfo.lineageProcessSteps =
                new BagProcessStep[dataQualityInfo.numberOfProcessSteps];

            for (uint32_t i = 0; i < dataQualityInfo.numberOfProcessSteps; i++)
            {
                initProcessStep(dataQualityInfo.lineageProcessSteps[i]);
                if (!decodeProcessStep(*stepNodes[i],
                    dataQualityInfo.lineageProcessSteps[i], schemaVersion))
                    return false;
            }
        }
    }

    return true;
}

//************************************************************************
//! Decode a BagSpatialRepresentation from the supplied XML node.
/*!
\param node
    \li The XML node containing the spatial representation information.
\param spatialRepresentationInfo
    \li Modified to contain the spatial representation information from \e node.
\param schemaVersion
    \li The version of the schema stored in \e node.
\return
    \li True if the information was found and decoded properly, False if
        an error occurs.
*/
//************************************************************************
bool decodeSpatialRepresentationInfo(
    xmlNode& node,
    BagSpatialRepresentation& spatialRepresentationInfo,
    uint16_t schemaVersion)
{
    if (schemaVersion == 1)
    {
        //smXML:MD_Georectified/axisDimensionProperties/smXML:MD_Dimension/dimensionSize
        spatialRepresentationInfo.numberOfRows = getContentsAsInt(node,
            "smXML:MD_Georectified/axisDimensionProperties/smXML:MD_Dimension[dimensionName='row']/dimensionSize");

        //smXML:MD_Georectified/axisDimensionProperties/smXML:MD_Dimension/resolution/smXML:Measure
        spatialRepresentationInfo.rowResolution = getContentsAsFloat(node,
            "smXML:MD_Georectified/axisDimensionProperties/smXML:MD_Dimension[dimensionName='row']/resolution/smXML:Measure/smXML:value");

        //smXML:MD_Georectified/axisDimensionProperties/smXML:MD_Dimension/dimensionSize
        spatialRepresentationInfo.numberOfColumns = getContentsAsInt(node,
            "smXML:MD_Georectified/axisDimensionProperties/smXML:MD_Dimension[dimensionName='column']/dimensionSize");

        //smXML:MD_Georectified/axisDimensionProperties/smXML:MD_Dimension/resolution/smXML:Measure
        spatialRepresentationInfo.columnResolution = getContentsAsFloat(node,
            "smXML:MD_Georectified/axisDimensionProperties/smXML:MD_Dimension[dimensionName='column']/resolution/smXML:Measure/smXML:value");

        //smXML:MD_Georectified/cellGeometry
        spatialRepresentationInfo.cellGeometry = getContentsAsCharStar(node,
            "smXML:MD_Georectified/cellGeometry");

        //smXML:MD_Georectified/transformationParameterAvailability
        spatialRepresentationInfo.transformationParameterAvailability =
            getContentsAsBool(node, "smXML:MD_Georectified/transformationParameterAvailability");

        //smXML:MD_Georectified/checkPointAvailability
        spatialRepresentationInfo.checkPointAvailability = getContentsAsBool(
            node, "smXML:MD_Georectified/checkPointAvailability");

        //smXML:MD_Georectified/cornerPoints/gml:Point
        {
            const xmlNode* pNode = findNode(node,
                "smXML:MD_Georectified/cornerPoints/gml:Point/gml:coordinates");
            if (!pNode)
                return false;

            //Get the encoded corner values.
            const std::string value = getContents(*pNode);

            //Decode the extents
            (void)sscanf(value.c_str(), "%lf,%lf %lf,%lf",
                &spatialRepresentationInfo.llCornerX,
                &spatialRepresentationInfo.llCornerY,
                &spatialRepresentationInfo.urCornerX,
                &spatialRepresentationInfo.urCornerY);
        }

        //smXML:MD_Georectified/transformationDimensionDescription
        spatialRepresentationInfo.transformationDimensionDescription =
            getContentsAsCharStar(node,
                "smXML:MD_Georectified/transformationDimensionDescription");

        //smXML:MD_Georectified/transformationDimensionMapping
        spatialRepresentationInfo.transformationDimensionMapping =
            getContentsAsCharStar(node,
                "smXML:MD_Georectified/transformationDimensionMapping");

    }
    else if (schemaVersion == 2)
    {
        //gmd:MD_Georectified/gmd:axisDimensionProperties/gmd:MD_Dimension/gmd:dimensionSize
        spatialRepresentationInfo.numberOfRows = getContentsAsInt(node,
            "gmd:MD_Georectified/gmd:axisDimensionProperties/gmd:MD_Dimension/gmd:dimensionName/gmd:MD_DimensionNameTypeCode[@codeListValue='row']/parent::*/parent::*/gmd:dimensionSize/gco:Integer");

        //gmd:MD_Georectified/gmd:axisDimensionProperties/gmd:MD_Dimension/gmd:resolution
        spatialRepresentationInfo.rowResolution = getContentsAsFloat(node,
            "gmd:MD_Georectified/gmd:axisDimensionProperties/gmd:MD_Dimension/gmd:dimensionName/gmd:MD_DimensionNameTypeCode[@codeListValue='row']/parent::*/parent::*/gmd:resolution/gco:Measure");

        //gmd:MD_Georectified/gmd:axisDimensionProperties/gmd:MD_Dimension/gmd:dimensionSize
        spatialRepresentationInfo.numberOfColumns = getContentsAsInt(node,
            "gmd:MD_Georectified/gmd:axisDimensionProperties/gmd:MD_Dimension/gmd:dimensionName/gmd:MD_DimensionNameTypeCode[@codeListValue='column']/parent::*/parent::*/gmd:dimensionSize/gco:Integer");

        //gmd:MD_Georectified/gmd:axisDimensionProperties/gmd:MD_Dimension/gmd:resolution
        spatialRepresentationInfo.columnResolution = getContentsAsFloat(node,
            "gmd:MD_Georectified/gmd:axisDimensionProperties/gmd:MD_Dimension/gmd:dimensionName/gmd:MD_DimensionNameTypeCode[@codeListValue='column']/parent::*/parent::*/gmd:resolution/gco:Measure");

        //gmd:MD_Georectified/gmd:axisDimensionProperties/gmd:MD_Dimension/gmd:resolution[@uom]
        spatialRepresentationInfo.resolutionUnit = getPropertyAsString(node,
            "gmd:MD_Georectified/gmd:axisDimensionProperties/gmd:MD_Dimension/gmd:dimensionName/gmd:MD_DimensionNameTypeCode[@codeListValue='column']/parent::*/parent::*/gmd:resolution/gco:Measure", "uom");

        //gmd:MD_Georectified/gmd:cellGeometry
        spatialRepresentationInfo.cellGeometry = getContentsAsCharStar(node,
            "gmd:MD_Georectified/gmd:cellGeometry/gmd:MD_CellGeometryCode");

        //gmd:MD_Georectified/gmd:transformationParameterAvailability
        spatialRepresentationInfo.transformationParameterAvailability =
            getContentsAsBool(node, "gmd:MD_Georectified/gmd:transformationParameterAvailability/gco:Boolean");

        //gmd:MD_Georectified/gmd:checkPointAvailability
        spatialRepresentationInfo.checkPointAvailability = getContentsAsBool(
            node, "gmd:MD_Georectified/gmd:checkPointAvailability/gco:Boolean");

        //gmd:MD_Georectified/gmd:cornerPoints/gml:Point
        {
            const xmlNode *pNode = findNode(node,
                "gmd:MD_Georectified/gmd:cornerPoints/gml:Point/gml:coordinates");
            if (!pNode)
                return false;

            //Get the encoded corner values.
            const std::string value = getContents(*pNode);

            //Decode the extents
            (void)sscanf(value.c_str(), "%lf,%lf %lf,%lf",
                &spatialRepresentationInfo.llCornerX,
                &spatialRepresentationInfo.llCornerY,
                &spatialRepresentationInfo.urCornerX,
                &spatialRepresentationInfo.urCornerY);
        }
    }

    return true;
}

//************************************************************************
//! Decode a BagIdentification from the supplied XML node.
/*!
\param node
    \li The XML node containing the data identification information.
\param dataIdentificationInfo
    \li Modified to contain the data identification information from \e node.
\param schemaVersion
    \li The version of the schema stored in \e node.
\return
    \li True if the information was found and decoded properly, False if
        an error occurs.
*/
//************************************************************************
bool decodeDataIdentificationInfo(
    xmlNode& node,
    BagIdentification& dataIdentificationInfo,
    uint16_t schemaVersion)
{
    if (schemaVersion == 1)
    {
        //smXML:BAG_DataIdentification/citation/smXML:CI_Citation/title
        dataIdentificationInfo.title = getContentsAsCharStar(node,
            "smXML:BAG_DataIdentification/citation/smXML:CI_Citation/title");

        //smXML:BAG_DataIdentification/citation/smXML:CI_Citation/date/smXML:CI_Date/date
        dataIdentificationInfo.date = getContentsAsCharStar(node,
            "smXML:BAG_DataIdentification/citation/smXML:CI_Citation/date/smXML:CI_Date/date");

        //smXML:BAG_DataIdentification/citation/smXML:CI_Citation/date/smXML:CI_Date/dateType
        dataIdentificationInfo.dateType = getContentsAsCharStar(node,
            "smXML:BAG_DataIdentification/citation/smXML:CI_Citation/date/smXML:CI_Date/dateType");

        //smXML:BAG_DataIdentification/citation/smXML:CI_Citation/citedResponsibleParty"
        const auto partyNodes = findNodes(node,
            "smXML:BAG_DataIdentification/citation/smXML:CI_Citation/citedResponsibleParty");
        if (!partyNodes.empty())
        {
            dataIdentificationInfo.numberOfResponsibleParties =
                static_cast<uint32_t>(partyNodes.size());
            dataIdentificationInfo.responsibleParties =
                new BagResponsibleParty[dataIdentificationInfo.numberOfResponsibleParties];

            for (uint32_t i = 0; i < dataIdentificationInfo.numberOfResponsibleParties; i++)
            {
                initResponsibleParty(dataIdentificationInfo.responsibleParties[i]);
                if (!decodeResponsibleParty(*partyNodes[i],
                    dataIdentificationInfo.responsibleParties[i], schemaVersion))
                    return false;
            }
        }

        //smXML:BAG_DataIdentification/abstract
        dataIdentificationInfo.abstractString = getContentsAsCharStar(node,
            "smXML:BAG_DataIdentification/abstract");

        //smXML:BAG_DataIdentification/status
        dataIdentificationInfo.status = getContentsAsCharStar(node,
            "smXML:BAG_DataIdentification/status");

        //smXML:BAG_DataIdentification/spatialRepresentationType
        dataIdentificationInfo.spatialRepresentationType = getContentsAsCharStar(
            node, "smXML:BAG_DataIdentification/spatialRepresentationType");

        //smXML:BAG_DataIdentification/language
        dataIdentificationInfo.language = getContentsAsCharStar(node,
            "smXML:BAG_DataIdentification/language");

        //Doesn't appear to be set, so always set to utf8
        dataIdentificationInfo.characterSet = copyString("utf8");

        //smXML:BAG_DataIdentification/topicCategory
        dataIdentificationInfo.topicCategory = getContentsAsCharStar(node,
            "smXML:BAG_DataIdentification/topicCategory");

        //smXML:BAG_DataIdentification/extent/smXML:EX_Extent/geographicElement/smXML:EX_GeographicBoundingBox/
        dataIdentificationInfo.westBoundingLongitude = getContentsAsFloat(node,
            "smXML:BAG_DataIdentification/extent/smXML:EX_Extent/geographicElement/smXML:EX_GeographicBoundingBox/westBoundLongitude");
        dataIdentificationInfo.eastBoundingLongitude = getContentsAsFloat(node,
            "smXML:BAG_DataIdentification/extent/smXML:EX_Extent/geographicElement/smXML:EX_GeographicBoundingBox/eastBoundLongitude");
        dataIdentificationInfo.southBoundingLatitude = getContentsAsFloat(node,
            "smXML:BAG_DataIdentification/extent/smXML:EX_Extent/geographicElement/smXML:EX_GeographicBoundingBox/southBoundLatitude");
        dataIdentificationInfo.northBoundingLatitude = getContentsAsFloat(node,
            "smXML:BAG_DataIdentification/extent/smXML:EX_Extent/geographicElement/smXML:EX_GeographicBoundingBox/northBoundLatitude");

        //smXML:BAG_DataIdentification/verticalUncertaintyType
        dataIdentificationInfo.verticalUncertaintyType = getContentsAsCharStar(
            node, "smXML:BAG_DataIdentification/verticalUncertaintyType");

        //smXML:BAG_DataIdentification/depthCorrectionType
        dataIdentificationInfo.depthCorrectionType = getContentsAsCharStar(node,
            "smXML:BAG_DataIdentification/depthCorrectionType");

        //smXML:BAG_DataIdentification/nodeGroupType
        dataIdentificationInfo.nodeGroupType = getContentsAsCharStar(node,
            "smXML:BAG_DataIdentification/nodeGroupType");

        //smXML:BAG_DataIdentification/elevationSolutionGroupType
        dataIdentificationInfo.elevationSolutionGroupType = getContentsAsCharStar(
            node, "smXML:BAG_DataIdentification/elevationSolutionGroupType");
    }
    else if (schemaVersion == 2)
    {
        //bag:BAG_DataIdentification/gmd:citation/gmd:CI_Citation/gmd:title
        dataIdentificationInfo.title = getContentsAsCharStar(node,
            "bag:BAG_DataIdentification/gmd:citation/gmd:CI_Citation/gmd:title/gco:CharacterString");

        //bag:BAG_DataIdentification/gmd:citation/gmd:CI_Citation/gmd:CI_Date/gmd:date
        dataIdentificationInfo.date = getContentsAsCharStar(node,
            "bag:BAG_DataIdentification/gmd:citation/gmd:CI_Citation/gmd:date/gmd:CI_Date/gmd:date/gco:Date");

        //bag:BAG_DataIdentification/gmd:citation/gmd:CI_Citation/gmd:CI_Date/gmd:dateType
        dataIdentificationInfo.dateType = getContentsAsCharStar(node,
            "bag:BAG_DataIdentification/gmd:citation/gmd:CI_Citation/gmd:date/gmd:CI_Date/gmd:dateType/gmd:CI_DateTypeCode");

        //bag:BAG_DataIdentification/gmd:citation/gmd:CI_Citation/gmd:citedResponsibleParty
        const auto partyNodes = findNodes(node,
            "bag:BAG_DataIdentification/gmd:citation/gmd:CI_Citation/gmd:citedResponsibleParty");
        if (!partyNodes.empty())
        {
            dataIdentificationInfo.numberOfResponsibleParties =
                static_cast<uint32_t>(partyNodes.size());
            dataIdentificationInfo.responsibleParties =
                new BagResponsibleParty[dataIdentificationInfo.numberOfResponsibleParties];

            for (uint32_t i = 0; i < dataIdentificationInfo.numberOfResponsibleParties; i++)
            {
                initResponsibleParty(dataIdentificationInfo.responsibleParties[i]);
                if (!decodeResponsibleParty(*partyNodes[i],
                    dataIdentificationInfo.responsibleParties[i], schemaVersion))
                    return false;
            }
        }

        //bag:BAG_DataIdentification/gmd:abstract
        dataIdentificationInfo.abstractString = getContentsAsCharStar(node,
            "bag:BAG_DataIdentification/gmd:abstract/gco:CharacterString");

        //bag:BAG_DataIdentification/gmd:status
        dataIdentificationInfo.status = getContentsAsCharStar(node,
            "bag:BAG_DataIdentification/gmd:status/gmd:MD_ProgressCode");

        //bag:BAG_DataIdentification/gmd:spatialRepresentationType
        dataIdentificationInfo.spatialRepresentationType = getContentsAsCharStar(
            node, "bag:BAG_DataIdentification/gmd:spatialRepresentationType/gmd:MD_SpatialRepresentationTypeCode");

        //bag:BAG_DataIdentification/gmd:language
        dataIdentificationInfo.language = getContentsAsCharStar(node,
            "bag:BAG_DataIdentification/gmd:language/gmd:LanguageCode");

        //bag:BAG_DataIdentification/gmd:characterSet
        dataIdentificationInfo.characterSet = getContentsAsCharStar(node,
            "bag:BAG_DataIdentification/gmd:characterSet/gmd:MD_CharacterSetCode");

        //bag:BAG_DataIdentification/gmd:topicCategory
        dataIdentificationInfo.topicCategory = getContentsAsCharStar(node,
            "bag:BAG_DataIdentification/gmd:topicCategory/gmd:MD_TopicCategoryCode");

        //bag:BAG_DataIdentification/gmd:extent/gmd:EX_Extent/gmd:geographicElement/gmd:EX_GeographicBoundingBox/
        dataIdentificationInfo.westBoundingLongitude = getContentsAsFloat(node,
            "bag:BAG_DataIdentification/gmd:extent/gmd:EX_Extent/gmd:geographicElement/gmd:EX_GeographicBoundingBox/gmd:westBoundLongitude/gco:Decimal");
        dataIdentificationInfo.eastBoundingLongitude = getContentsAsFloat(node,
            "bag:BAG_DataIdentification/gmd:extent/gmd:EX_Extent/gmd:geographicElement/gmd:EX_GeographicBoundingBox/gmd:eastBoundLongitude/gco:Decimal");
        dataIdentificationInfo.southBoundingLatitude = getContentsAsFloat(node,
            "bag:BAG_DataIdentification/gmd:extent/gmd:EX_Extent/gmd:geographicElement/gmd:EX_GeographicBoundingBox/gmd:southBoundLatitude/gco:Decimal");
        dataIdentificationInfo.northBoundingLatitude = getContentsAsFloat(node,
            "bag:BAG_DataIdentification/gmd:extent/gmd:EX_Extent/gmd:geographicElement/gmd:EX_GeographicBoundingBox/gmd:northBoundLatitude/gco:Decimal");

        //bag:BAG_DataIdentification/bag:verticalUncertaintyType
        dataIdentificationInfo.verticalUncertaintyType = getContentsAsCharStar(
            node, "bag:BAG_DataIdentification/bag:verticalUncertaintyType/bag:BAG_VertUncertCode");

        //bag:BAG_DataIdentification/bag:depthCorrectionType
        dataIdentificationInfo.depthCorrectionType = getContentsAsCharStar(node,
            "bag:BAG_DataIdentification/bag:depthCorrectionType/bag:BAG_DepthCorrectCode");

        //bag:BAG_DataIdentification/bag:nodeGroupType
        dataIdentificationInfo.nodeGroupType = getContentsAsCharStar(node,
            "bag:BAG_DataIdentification/bag:nodeGroupType/bag:BAG_OptGroupCode");

        //bag:BAG_DataIdentification/bag:elevationSolutionGroupType
        dataIdentificationInfo.elevationSolutionGroupType = getContentsAsCharStar(
            node, "bag:BAG_DataIdentification/bag:elevationSolutionGroupType/bag:BAG_OptGroupCode");
    }

    return true;
}

//************************************************************************
//! Decode a BagReferenceSystem from a BagSpatialRepresentation, the narrowly defined as the parsing of an EPSG code
//! from the transformationDimensionDescription tag
/*!
\param spatialRepresentationInfo
    \li Spatial representation information input.
\param referenceSystemInfo
    \li Modified to contain the reference system information from \e spatialRepresentationInfo->transformationDimensionDescription.
\param schemaVersion
    \li The version of the schema stored in \e node.
\return
    \li True if the information was found and decoded properly, False if
        the optional tag is not present in the header or an error occurs.
*/
//************************************************************************
bool decodeReferenceSystemInfoFromSpatial(
    const BagSpatialRepresentation* spatialRepresentationInfo,
    BagReferenceSystem* referenceSystemInfo,
    uint16_t /*schemaVersion*/)
{
    if ( spatialRepresentationInfo && referenceSystemInfo )
    {
        // Is the TransformationDimensionDescription parameter present?
        if ( spatialRepresentationInfo->transformationDimensionDescription )
        {
            // Find the assignment operator
            auto* equal = strchr(
                spatialRepresentationInfo->transformationDimensionDescription, '=');
            if (equal != nullptr)
            {
                ++equal;
                const auto epsg = atoi(equal);

                // Proceed to create a string containing the EPSG authority
                // code, and set the type to "EPSG".
                if (epsg > 0 && strncmp(
                    spatialRepresentationInfo->transformationDimensionDescription, "EPSG", 4) == 0)
                {
                    char buffer[2048];

                    sprintf(buffer, "%d", epsg);
                    referenceSystemInfo->definition = copyString(buffer);
                    referenceSystemInfo->type = copyString("EPSG");

                    return true;
                }
            }
        }
    }

    return false;
}


//************************************************************************
//! Decode a BagReferenceSystem from the supplied XML node.
/*!
\param node
    \li The XML node containing the reference system information.
\param referenceSystemInfo
    \li Modified to contain the reference system information from \e node.
\param schemaVersion
    \li The version of the schema stored in \e node.
\return
    \li True if the information was found and decoded properly, False if
        an error occurs.
*/
//************************************************************************
bool decodeReferenceSystemInfo(
    xmlNode& node,
    BagReferenceSystem& referenceSystemInfo,
    uint16_t schemaVersion)
{
    if (schemaVersion == 1)
    {
        //If I have an ellipsoid, then this must be horizontal.
        auto ellipsoid = getContentsAsString(node, "smXML:MD_CRS/ellipsoid/smXML:RS_Identifier/code");
        if (!ellipsoid.empty())
        {
            auto projectionId = getContentsAsString(node, "smXML:MD_CRS/projection/smXML:RS_Identifier/code");
            auto datumId = getContentsAsString(node, "smXML:MD_CRS/datum/smXML:RS_Identifier/code");

            BagLegacyReferenceSystem v1Def;
            memset(&v1Def, 0, sizeof(BagLegacyReferenceSystem));

            //Store the projection information.
            v1Def.coordSys = bagCoordsys(projectionId.c_str());

            //Store the ellipsoid information.
            strncpy(v1Def.geoParameters.ellipsoid, ellipsoid.c_str(), 256);

            //Store the datum information.
            v1Def.geoParameters.datum = bagDatumID(datumId.c_str());

            v1Def.geoParameters.zone = getContentsAsInt(node,
                "smXML:MD_CRS/projectionParameters/smXML:MD_ProjectionParameters/zone");
            v1Def.geoParameters.std_parallel_1 = getContentsAsFloat(node,
                "smXML:MD_CRS/projectionParameters/smXML:MD_ProjectionParameters/standardParallel[1]");
            v1Def.geoParameters.std_parallel_2 = getContentsAsFloat(node,
                "smXML:MD_CRS/projectionParameters/smXML:MD_ProjectionParameters/standardParallel[2]");
            v1Def.geoParameters.central_meridian = getContentsAsFloat(node,
                "smXML:MD_CRS/projectionParameters/smXML:MD_ProjectionParameters/longitudeOfCentralMeridian");
            v1Def.geoParameters.origin_latitude = getContentsAsFloat(node,
                "smXML:MD_CRS/projectionParameters/smXML:MD_ProjectionParameters/latitudeOfProjectionOrigin");
            v1Def.geoParameters.false_easting = getContentsAsFloat(node,
                "smXML:MD_CRS/projectionParameters/smXML:MD_ProjectionParameters/falseEasting");
            v1Def.geoParameters.false_northing = getContentsAsFloat(node,
                "smXML:MD_CRS/projectionParameters/smXML:MD_ProjectionParameters/falseNorthing");
            const double scaleFactAtEq = getContentsAsFloat(node,
                "smXML:MD_CRS/projectionParameters/smXML:MD_ProjectionParameters/scaleFactorAtEquator");
            v1Def.geoParameters.longitude_of_centre = getContentsAsFloat(node,
                "smXML:MD_CRS/projectionParameters/smXML:MD_ProjectionParameters/longitudeOfProjectionCenter");
            v1Def.geoParameters.latitude_of_centre = getContentsAsFloat(node,
                "smXML:MD_CRS/projectionParameters/smXML:MD_ProjectionParameters/latitudeOfProjectionCenter");
            v1Def.geoParameters.longitude_down_from_pole = getContentsAsFloat(
                node, "smXML:MD_CRS/projectionParameters/smXML:MD_ProjectionParameters/straightVerticalLongitudeFromPole");
            const double scaleAtProjOrigin = getContentsAsFloat(node,
                "smXML:MD_CRS/projectionParameters/smXML:MD_ProjectionParameters/scaleFactorAtProjectionOrigin");

            /* dhf */
            /* scaleFactAtEq - for mercator */
            /* scaleAtCenterLine - for oblique mercator (not supported) */
            /* scaleAtProjOrigin - for polar stereographic & transverse mercator */
            if (v1Def.coordSys == CoordinateType::Mercator)
    	        v1Def.geoParameters.scale_factor = scaleFactAtEq;
            if (v1Def.coordSys == CoordinateType::Transverse_Mercator ||
                v1Def.coordSys == CoordinateType::Polar_Stereo)
    	        v1Def.geoParameters.scale_factor = scaleAtProjOrigin;

            char buffer[2048];
            const BagError error = bagLegacyToWkt(v1Def, buffer, sizeof(buffer),
                nullptr, 0);
            if (error)
                return false;

            referenceSystemInfo.definition = copyString(buffer);
            referenceSystemInfo.type = copyString("WKT");
        }
        //Else it must be vertical.
        else
        {
            auto datum = getContentsAsString(node, "smXML:MD_CRS/datum/smXML:RS_Identifier/code");

            BagLegacyReferenceSystem system;
            strncpy(system.geoParameters.vertical_datum, datum.c_str(), 256);

            char buffer[1024];
            const BagError error = bagLegacyToWkt(system, nullptr, 0, buffer,
                sizeof(buffer));
            if (error)
                return false;

            referenceSystemInfo.definition = copyString(buffer);
            referenceSystemInfo.type = copyString("WKT");
        }
    }
    else if (schemaVersion == 2)
    {
        //gmd:MD_ReferenceSystem/gmd:referenceSystemIdentifier/gmd:RS_Identifier/gmd:code
        referenceSystemInfo.definition = getContentsAsCharStar(node,
            "gmd:MD_ReferenceSystem/gmd:referenceSystemIdentifier/gmd:RS_Identifier/gmd:code/gco:CharacterString");

        //gmd:MD_ReferenceSystem/gmd:referenceSystemIdentifier/gmd:RS_Identifier/gmd:codeSpace
        referenceSystemInfo.type = getContentsAsCharStar(node,
            "gmd:MD_ReferenceSystem/gmd:referenceSystemIdentifier/gmd:RS_Identifier/gmd:codeSpace/gco:CharacterString");
    }

    return true;
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
BagError validateSchema(xmlDoc& metadataDocument)
{
    //Get the location of the BAG home directory.
    const std::string bagHome = bagGetHomeFolder();

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
    if (!pSchemaDoc)
        return BAG_METADTA_PARSE_FAILED;

    // Parse the schema.
    xmlSchemaParserCtxt *pContext = xmlSchemaNewDocParserCtxt(pSchemaDoc);
    if (!pContext)
    {
        xmlFreeDoc(pSchemaDoc);

        return BAG_METADTA_SCHEMA_SETUP_FAILED;
    }

    // Initialize the schema object.
    xmlSchema *pSchema = xmlSchemaParse(pContext);
    if (!pSchema)
    {
        xmlSchemaFreeParserCtxt(pContext);
        xmlFreeDoc(pSchemaDoc);

        return BAG_METADTA_SCHEMA_SETUP_FAILED;
    }

    // Create the validation object.
    xmlSchemaValidCtxt *pValidationContext = xmlSchemaNewValidCtxt(pSchema);
    if (!pValidationContext)
    {
        xmlSchemaFree(pSchema);
        xmlSchemaFreeParserCtxt(pContext);
        xmlFreeDoc(pSchemaDoc);

        return BAG_METADTA_SCHEMA_VALIDATION_SETUP_FAILED;
    }

    // Validate the document.
    const int result = xmlSchemaValidateDoc(pValidationContext, &metadataDocument);

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
BagError bagImportMetadataFromXmlV1(
    const xmlDoc& document,
    BagMetadata& metadata)
{
    xmlNode* pRoot = xmlDocGetRootElement(&document);
    if (!pRoot)
        return BAG_METADTA_NOT_INITIALIZED;

    //gmd:language
    metadata.language = getContentsAsCharStar(*pRoot, "/smXML:MD_Metadata/language");

    //gmd:characterSet
    metadata.characterSet = copyString("eng");

    //gmd:hierarchyLevel
    metadata.hierarchyLevel = copyString("dataset");

    //gmd:contact
    {
        auto* pNode = findNode(*pRoot, "/smXML:MD_Metadata/contact");
        if (!pNode)
            return BAG_METADTA_MISSING_MANDATORY_ITEM;

        if (!decodeResponsibleParty(*pNode, *metadata.contact, 1))
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
    }

    //gmd:dateStamp
    metadata.dateStamp = getContentsAsCharStar(*pRoot,
        "/smXML:MD_Metadata/dateStamp");

    //gmd:metadataStandardName
    metadata.metadataStandardName = getContentsAsCharStar(*pRoot,
        "/smXML:MD_Metadata/metadataStandardName");

    //gmd:metadataStandardVersion
    metadata.metadataStandardVersion = getContentsAsCharStar(*pRoot,
        "/smXML:MD_Metadata/metadataStandardVersion");

    //gmd:spatialRepresentationInfo
    {
        auto* pNode = findNode(*pRoot,
            "/smXML:MD_Metadata/spatialRepresentationInfo");
        if (!pNode)
            return BAG_METADTA_MISSING_MANDATORY_ITEM;

        if (!decodeSpatialRepresentationInfo(*pNode,
            *metadata.spatialRepresentationInfo, 1))
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
    }

    //gmd:referenceSystemInfo (horizontal)
    {
        auto* pNode = findNode(*pRoot,
            "/smXML:MD_Metadata/referenceSystemInfo[1]");
        if (!pNode)
            return BAG_METADTA_MISSING_MANDATORY_ITEM;

        if (!decodeReferenceSystemInfo(*pNode, *metadata.horizontalReferenceSystem, 1))
        {
            // If the reference system could not be identified from this block
            // of code look for an EPSG code in the spatial reference system
            if (!decodeReferenceSystemInfoFromSpatial(
                metadata.spatialRepresentationInfo,
                metadata.horizontalReferenceSystem, 1))
                return BAG_METADTA_MISSING_MANDATORY_ITEM;
        }
    }

    //gmd:referenceSystemInfo (vertical)
    {
        auto* pNode = findNode(*pRoot,
            "/smXML:MD_Metadata/referenceSystemInfo[2]");
        if (!pNode)
        {
            //If we could not find the vertical coordinate system node, then
            //lets look in the other location.
            auto* pNode2 = findNode(*pRoot,
                "/smXML:MD_Metadata/referenceSystemInfo[1]//verticalDatum");
            if (!pNode2)
                return BAG_METADTA_MISSING_MANDATORY_ITEM;

            const auto datum = getContentsAsString(*pNode2,
                "smXML:RS_Identifier/code");
            if (datum.empty())
                return BAG_METADTA_MISSING_MANDATORY_ITEM;

            BagLegacyReferenceSystem system;
            strncpy(system.geoParameters.vertical_datum, datum.c_str(), 256);

            char buffer[1024];
            BagError error = bagLegacyToWkt(system, nullptr, 0, buffer,
                sizeof(buffer));
            if (error)
                return error;

            metadata.verticalReferenceSystem->definition = copyString(buffer);
            metadata.verticalReferenceSystem->type = copyString("WKT");
        }
        else if (!decodeReferenceSystemInfo(*pNode,
            *metadata.verticalReferenceSystem, 1))
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
    }

    //Because the previous metadata profile did not store the unit of measure
    //for the spatial representation element, we will use the spatial reference
    //system to figure it out.
    {
        //Get the WKT horizontal reference system string.
        const std::string horizontalWKT{
            metadata.horizontalReferenceSystem->definition};

        //If we have a projection, then it must be metres (its all we supported in the past)
        if (horizontalWKT.find("PROJCS[") >= 0)
            metadata.spatialRepresentationInfo->resolutionUnit = copyString("Metre");
        //Else it must be geographic
        else
            metadata.spatialRepresentationInfo->resolutionUnit = copyString("Degree");
    }

    //gmd:identificationInfo
    {
        auto* pNode = findNode(*pRoot, "/smXML:MD_Metadata/identificationInfo");
        if (!pNode)
            return BAG_METADTA_MISSING_MANDATORY_ITEM;

        if (!decodeDataIdentificationInfo(*pNode, *metadata.identificationInfo, 1))
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
    }

    //gmd:dataQualityInfo
    {
        auto* pNode = findNode(*pRoot, "/smXML:MD_Metadata/dataQualityInfo");
        if (!pNode)
        {
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
        }

        if (!decodeDataQualityInfo(*pNode, *metadata.dataQualityInfo, 1))
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
    }

    //gmd:metadataConstraints (legal)
    {
        auto* pNode = findNode(*pRoot,
            "/smXML:MD_Metadata/metadataConstraints/smXML:MD_LegalConstraints/parent::*");
        if (!pNode)
            return BAG_METADTA_MISSING_MANDATORY_ITEM;

        if (!decodeLegalConstraints(*pNode, *metadata.legalConstraints, 1))
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
    }

    //gmd:metadataConstraints (security)
    {
        auto* pNode = findNode(*pRoot,
            "/smXML:MD_Metadata/metadataConstraints/smXML:MD_SecurityConstraints/parent::*");
        if (!pNode)
            return BAG_METADTA_MISSING_MANDATORY_ITEM;

        if (!decodeSecurityConstraints(*pNode, *metadata.securityConstraints, 1))
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
BagError bagImportMetadataFromXmlV2(
    const xmlDoc& document,
    BagMetadata& metadata)
{
    auto* pRoot = xmlDocGetRootElement(&document);
    if (!pRoot)
        return BAG_METADTA_NOT_INITIALIZED;

    //gmd:fileIdentifier
    metadata.fileIdentifier = getContentsAsCharStar(*pRoot,
        "/gmi:MI_Metadata/gmd:fileIdentifier/gco:CharacterString");

    //gmd:language
    metadata.language = getContentsAsCharStar(*pRoot,
        "/gmi:MI_Metadata/gmd:language/gmd:LanguageCode");

    //gmd:characterSet
    metadata.characterSet = getContentsAsCharStar(*pRoot,
        "/gmi:MI_Metadata/gmd:characterSet/gmd:MD_CharacterSetCode");

    //gmd:hierarchyLevel
    metadata.hierarchyLevel = getContentsAsCharStar(*pRoot,
        "/gmi:MI_Metadata/gmd:hierarchyLevel/gmd:MD_ScopeCode");

    //gmd:contact
    {
        auto* pNode = findNode(*pRoot, "/gmi:MI_Metadata/gmd:contact");
        if (!pNode)
            return BAG_METADTA_MISSING_MANDATORY_ITEM;

        if (!decodeResponsibleParty(*pNode, *metadata.contact, 2))
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
    }

    //gmd:dateStamp
    metadata.dateStamp = getContentsAsCharStar(*pRoot,
        "/gmi:MI_Metadata/gmd:dateStamp/gco:Date");

    //gmd:metadataStandardName
    metadata.metadataStandardName = getContentsAsCharStar(*pRoot,
        "/gmi:MI_Metadata/gmd:metadataStandardName/gco:CharacterString");

    //gmd:metadataStandardVersion
    metadata.metadataStandardVersion = getContentsAsCharStar(*pRoot,
        "/gmi:MI_Metadata/gmd:metadataStandardVersion/gco:CharacterString");

    //gmd:spatialRepresentationInfo
    {
        auto* pNode = findNode(*pRoot,
            "/gmi:MI_Metadata/gmd:spatialRepresentationInfo/gmd:MD_Georectified/parent::*");
        if (!pNode)
            return BAG_METADTA_MISSING_MANDATORY_ITEM;

        if (!decodeSpatialRepresentationInfo(*pNode, *metadata.spatialRepresentationInfo, 2))
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
    }

    //gmd:referenceSystemInfo (horizontal)
    {
        auto* pNode = findNode(*pRoot, "/gmi:MI_Metadata/gmd:referenceSystemInfo[1]");
        if (!pNode)
            return BAG_METADTA_MISSING_MANDATORY_ITEM;

        if (!decodeReferenceSystemInfo(*pNode, *metadata.horizontalReferenceSystem, 2))
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
    }

    //gmd:referenceSystemInfo (vertical)
    {
        auto* pNode = findNode(*pRoot, "/gmi:MI_Metadata/gmd:referenceSystemInfo[2]");
        if (!pNode)
            return BAG_METADTA_MISSING_MANDATORY_ITEM;

        if (!decodeReferenceSystemInfo(*pNode, *metadata.verticalReferenceSystem, 2))
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
    }

    //gmd:identificationInfo
    {
        auto* pNode = findNode(*pRoot, "/gmi:MI_Metadata/gmd:identificationInfo");
        if (!pNode)
            return BAG_METADTA_MISSING_MANDATORY_ITEM;

        if (!decodeDataIdentificationInfo(*pNode, *metadata.identificationInfo, 2))
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
    }

    //gmd:dataQualityInfo
    {
        auto* pNode = findNode(*pRoot, "/gmi:MI_Metadata/gmd:dataQualityInfo");
        if (!pNode)
            return BAG_METADTA_MISSING_MANDATORY_ITEM;

        if (!decodeDataQualityInfo(*pNode, *metadata.dataQualityInfo, 2))
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
    }

    //gmd:metadataConstraints (legal)
    {
        auto* pNode = findNode(*pRoot, "/gmi:MI_Metadata/gmd:metadataConstraints/gmd:MD_LegalConstraints/parent::*");
        if (!pNode)
            return BAG_METADTA_MISSING_MANDATORY_ITEM;

        if (!decodeLegalConstraints(*pNode, *metadata.legalConstraints, 2))
            return BAG_METADTA_MISSING_MANDATORY_ITEM;
    }

    //gmd:metadataConstraints (security)
    {
        auto* pNode = findNode(*pRoot, "/gmi:MI_Metadata/gmd:metadataConstraints/gmd:MD_SecurityConstraints/parent::*");
        if (!pNode)
            return BAG_METADTA_MISSING_MANDATORY_ITEM;

        if (!decodeSecurityConstraints(*pNode, *metadata.securityConstraints, 2))
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
BagError bagImportMetadataFromXml(
    xmlDoc& document,
    BagMetadata& metadata,
    bool doValidation)
{
    const xmlNode* pRoot = xmlDocGetRootElement(&document);
    if (!pRoot)
        return BAG_METADTA_EMPTY_DOCUMENT;

    if (doValidation)
    {
        const BagError ret = validateSchema(document);
        if (ret)
            return ret;
    }

    //Get the name of the root node.
    const std::string rootName = getNodeName(*pRoot);

    //We will use the root node's name to figure out what version of the schema we are working with.
    const uint16_t schemaVersion = (rootName == "smXML:MD_Metadata") ? 1 : 2;

    return (schemaVersion == 1) ?
        bagImportMetadataFromXmlV1(document, metadata) :
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
BagError bagImportMetadataFromXmlBuffer(
    const char* xmlBuffer,
    int bufferSize,
    BagMetadata& metadata,
    bool doValidation)
{
    xmlDoc* pDocument = xmlParseMemory(xmlBuffer, bufferSize);
    if (!pDocument)
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
BagError bagImportMetadataFromXmlFile(
    const char* fileName,
    BagMetadata& metadata,
    bool doValidation)
{
    xmlDoc* pDocument = xmlParseFile(fileName);
    if (!pDocument)
        return BAG_METADTA_NOT_INITIALIZED;

    return bagImportMetadataFromXml(*pDocument, metadata, doValidation);
}

}  // namespace BAG

