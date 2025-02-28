//************************************************************************
//
//      Open Navigation Surface Working Group, 2013
//
//************************************************************************
#include "bag_metadata_export.h"

#include <cstring>
#include <iostream>
#include <libxml/parser.h>
#include <sstream>
#include <string>


namespace BAG {

namespace {

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
    EncodedString(xmlDoc &doc, const char *string)
    {
        this->m_pEncodedString = xmlEncodeEntitiesReentrant(&doc,
            reinterpret_cast<const xmlChar*>(string));
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
    xmlChar* m_pEncodedString = nullptr;
};

#define XMLCast(value) reinterpret_cast<const xmlChar*>(value)

//************************************************************************
//! Add a CharacterString node to the supplied parent XML node.
/*!
\param parentNode
    \li The parent XML node to be modified.
\param content
    \li The content to be added to \e parentNode.
*/
//************************************************************************
xmlNode* addCharacterNode(xmlNode &parentNode, const char *content)
{
    const auto pGcoNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("gco"));

    //Create the CharacterString node.
    xmlNode *pCharacterNode = xmlNewChild(&parentNode, pGcoNamespace, XMLCast("CharacterString"), EncodedString(*parentNode.doc, content));
    return pCharacterNode;
}

//************************************************************************
//! Add a Date node to the supplied parent XML node.
/*!
\param parentNode
    \li The parent XML node to be modified.
\param content
    \li The content to be added to \e parentNode.
*/
//************************************************************************
xmlNode* addDateNode(xmlNode &parentNode, const char *content)
{
    const xmlNsPtr pGcoNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("gco"));

    //If the content is nullptr, then we will just add a nilReason.
    if (content == nullptr)
    {
        xmlSetProp(&parentNode, XMLCast("gco:nilReason"), XMLCast("unknown"));
        return nullptr;
    }

    //Create the Date node.
    xmlNode *pDateNode = xmlNewChild(&parentNode, pGcoNamespace, XMLCast("Date"), EncodedString(*parentNode.doc, content));
    return pDateNode;
}

//************************************************************************
//! Add a DateTime node to the supplied parent XML node.
/*!
\param parentNode
    \li The parent XML node to be modified.
\param content
    \li The content to be added to \e parentNode.
*/
//************************************************************************
xmlNode* addDateTimeNode(xmlNode &parentNode, const char *content)
{
    const xmlNsPtr pGcoNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("gco"));

    //Create the DateTime node.
    xmlNode *pDateTimeNode = xmlNewChild(&parentNode, pGcoNamespace, XMLCast("DateTime"), EncodedString(*parentNode.doc, content));
    return pDateTimeNode;
}

//************************************************************************
//! Add a Code value to the supplied parent XML node.
/*!
\param parentNode
    \li The parent XML node to be modified.
\param codeNameSpace
    \li The namespace to which the code belongs
\param codeName
    \li The code name.
\param url
    \li The url that manages houses code list.
\param value
    \li The actual value from the code list.
*/
//************************************************************************
xmlNode* addCodeListNode(xmlNode &parentNode, const char *codeNameSpace, const char *codeName,
                         const char *url, const char *value, bool appendValueToUrl = true)
{
    const xmlNsPtr pNamespace = xmlSearchNs(parentNode.doc, &parentNode, EncodedString(*parentNode.doc, codeNameSpace));

    //Create the codeList node.
    xmlNode *pCodeNode = xmlNewChild(&parentNode, pNamespace, EncodedString(*parentNode.doc, codeName), EncodedString(*parentNode.doc, value));

    std::stringstream fullUrlStream;
    fullUrlStream << url;

    if (appendValueToUrl)
        fullUrlStream << "#" << codeName;

    xmlSetProp(pCodeNode, XMLCast("codeList"), EncodedString(*parentNode.doc, fullUrlStream.str().c_str()));
    xmlSetProp(pCodeNode, XMLCast("codeListValue"), EncodedString(*parentNode.doc, value));

    return pCodeNode;
}

//************************************************************************
//! Add a Decimal node to the supplied parent XML node.
/*!
\param parentNode
    \li The parent XML node to be modified.
\param value
    \li The content to be added to \e parentNode.
*/
//************************************************************************
xmlNode* addDecimalNode(xmlNode &parentNode, double value)
{
    std::stringstream lineStream;
    lineStream.imbue(std::locale::classic());
    lineStream << value;

    const xmlNsPtr pGcoNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("gco"));

    //Create the Decimal node.
    xmlNode *pDecimalNode = xmlNewChild(&parentNode, pGcoNamespace, XMLCast("Decimal"), XMLCast(lineStream.str().c_str()));
    return pDecimalNode;
}

//************************************************************************
//! Add a Integer node to the supplied parent XML node.
/*!
\param parentNode
    \li The parent XML node to be modified.
\param value
    \li The content to be added to \e parentNode.
*/
//************************************************************************
xmlNode* addIntegerNode(xmlNode &parentNode, int value)
{
    std::stringstream lineStream;
    lineStream.imbue(std::locale::classic());
    lineStream << value;

    const xmlNsPtr pGcoNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("gco"));

    //Create the Integer node.
    xmlNode *pIntegerNode = xmlNewChild(&parentNode, pGcoNamespace, XMLCast("Integer"), XMLCast(lineStream.str().c_str()));
    return pIntegerNode;
}

//************************************************************************
//! Add a Measure node to the supplied parent XML node.
/*!
\param parentNode
    \li The parent XML node to be modified.
\param value
    \li The content to be added to \e parentNode.
*/
//************************************************************************
xmlNode* addMeasureNode(xmlNode &parentNode, const char *uomName, double value)
{
    std::stringstream lineStream;
    lineStream.imbue(std::locale::classic());
    lineStream << value;

    const xmlNsPtr pGcoNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("gco"));

    //Create the Measure node.
    xmlNode *pMeasureNode = xmlNewChild(&parentNode, pGcoNamespace, XMLCast("Measure"), XMLCast(lineStream.str().c_str()));
    xmlSetProp(pMeasureNode, XMLCast("uom"), EncodedString(*parentNode.doc, uomName));
    return pMeasureNode;
}

//************************************************************************
//! Add an MD_Dimension node to the supplied parent XML node.
/*!
\param parentNode
    \li The parent XML node to be modified.
\param name
    \li The name for the MD_DimensionNameTypeCode
\param size
    \li The number of dimensions to be added.
\param resolution
    \li The resolution of the dimension.
\param resolutionUnit
    \li The units of \e resolution.
*/
//************************************************************************
xmlNode* addDimension(xmlNode &parentNode, const char *name, unsigned int size,
                      double resolution, const char *resolutionUnit)
{
    const xmlNsPtr pGmdNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("gmd"));

    xmlNode *pDimNode = xmlNewChild(&parentNode, pGmdNamespace, XMLCast("axisDimensionProperties"), nullptr);
    pDimNode = xmlNewChild(pDimNode, pGmdNamespace, XMLCast("MD_Dimension"), nullptr);

    //dimensionName
    {
        xmlNode *pNode = xmlNewChild(pDimNode, pGmdNamespace, XMLCast("dimensionName"), nullptr);
        addCodeListNode(*pNode, "gmd", "MD_DimensionNameTypeCode",
            "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml", name);
    }

    //dimensionSize
    {
        xmlNode *pNode = xmlNewChild(pDimNode, pGmdNamespace, XMLCast("dimensionSize"), nullptr);
        addIntegerNode(*pNode, size);
    }

    //resolution
    {
        xmlNode *pNode = xmlNewChild(pDimNode, pGmdNamespace, XMLCast("resolution"), nullptr);
        addMeasureNode(*pNode, resolutionUnit, resolution);
    }

    return pDimNode;
}

//************************************************************************
//! Add a Boolean node to the supplied parent XML node.
/*!
\param parentNode
    \li The parent XML node to be modified.
\param value
    \li The content to be added to \e parentNode.
*/
//************************************************************************
xmlNode* addBooleanNode(xmlNode &parentNode, bool value)
{
    const std::string valString(value ? "1" : "0");

    const xmlNsPtr pGcoNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("gco"));

    //Create the Integer node.
    xmlNode *pBoolNode = xmlNewChild(&parentNode, pGcoNamespace, XMLCast("Boolean"), XMLCast(valString.c_str()));
    return pBoolNode;
}

}  // namespace


//************************************************************************
//! Create a new XML document and configure it for the BAG metadata profile.
/*!
\return
    \li The new XML document.
*/
//************************************************************************
xmlDoc* createNewDocument()
{
    const char version[] = "1.0";
    const char rootName[] = "MI_Metadata";
    const char rootNameSpaceUrl[] = "http://www.isotc211.org/2005/gmi";

    //Create the new document.
    xmlDoc *pDocument = xmlNewDoc(XMLCast(version));

    //Create the root node and assign to the document.
    xmlNodePtr pRoot = xmlNewDocNode(pDocument, nullptr, XMLCast(rootName), nullptr);
    xmlDocSetRootElement(pDocument, pRoot);

    //Create the root namespace and assign to the root.
    xmlNs *pRootNameSpace = xmlNewNs(pRoot, XMLCast(rootNameSpaceUrl), XMLCast("gmi"));
    xmlSetNs(pRoot, pRootNameSpace);

    //Add the rest of the required namespaces.
    xmlNewNs(pRoot, XMLCast("http://www.isotc211.org/2005/gmd"), XMLCast("gmd"));
    xmlNewNs(pRoot, XMLCast("http://www.w3.org/2001/XMLSchema-instance"), XMLCast("xsi"));
    xmlNewNs(pRoot, XMLCast("http://www.opengis.net/gml/3.2"), XMLCast("gml"));
    xmlNewNs(pRoot, XMLCast("http://www.isotc211.org/2005/gco"), XMLCast("gco"));
    xmlNewNs(pRoot, XMLCast("http://www.w3.org/1999/xlink"), XMLCast("xlink"));
    xmlNewNs(pRoot, XMLCast("http://www.opennavsurf.org/schema/bag"), XMLCast("bag"));

    return pDocument;
}

//************************************************************************
//! Add the BagResponsibleParty information to the parent XML node.
/*!
\param parentNode
    \li The parent XML node to be modified.
\param responsiblePartyStruct
    \li The structure to be added to \e parentNode.
\return
    \li True if the structure is added, False if an error occurs.
*/
//************************************************************************
bool addResponsibleParty(xmlNode &parentNode, const BagResponsibleParty &responsiblePartyStruct)
{
    /* Criteria for this node is that "role must be supplied and at least one of the following fileds must be supplied. */
    if (responsiblePartyStruct.individualName == nullptr &&
        responsiblePartyStruct.organisationName == nullptr &&
        responsiblePartyStruct.positionName == nullptr)
    {
        return false;
    }

    /* If "role" is not populated, don't create the element.  "role" is a required element of the schema. */
    if (responsiblePartyStruct.role == nullptr)
    {
        fprintf(stderr, "ERROR: The \"role\" is required in order to create the CI_ResponsibleParty node.\n");
        return false;
    }

    //Find the gmd namespace.
    const xmlNsPtr pGmdNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("gmd"));

    //Create the CI_ResponsibleParty node.
    xmlNode *pPartyNode = xmlNewChild(&parentNode, pGmdNamespace, XMLCast("CI_ResponsibleParty"), nullptr);

    /* If an individual name has been supplied, Create the individual node and populate it. */
    if (responsiblePartyStruct.individualName != nullptr)
    {
        xmlNode *pNode = xmlNewChild(pPartyNode, pGmdNamespace, XMLCast("individualName"), nullptr);
        addCharacterNode(*pNode, (char*)responsiblePartyStruct.individualName);
    }

    /* If an organisation name has been supplied, Create the organisation node and populate it. */
    if (responsiblePartyStruct.organisationName != nullptr)
    {
        xmlNode *pNode = xmlNewChild(pPartyNode, pGmdNamespace, XMLCast("organisationName"), nullptr);
        addCharacterNode(*pNode, (char*)responsiblePartyStruct.organisationName);
    }

    /* If a postiion name has been supplied, Create the position node and populate it. */
    if (responsiblePartyStruct.positionName != nullptr)
    {
        xmlNode *pNode = xmlNewChild(pPartyNode, pGmdNamespace, XMLCast("positionName"), nullptr);
        addCharacterNode(*pNode, (char*)responsiblePartyStruct.positionName);
    }

    //Create the role node and populate it.
    xmlNode *pNode = xmlNewChild(pPartyNode, pGmdNamespace, XMLCast("role"), nullptr);
    addCodeListNode(*pNode, "gmd", "CI_RoleCode",
        "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml", (char*)responsiblePartyStruct.role);

    return true;
}

//************************************************************************
//! Add the citation information to the parent XML node.
/*!
\param parentNode
    \li The parent XML node to be modified.
\param title
    \li The title of the person for the citation.
\param date
    \li The date of the citation.
\param dateType
    \li The type of date in \e date.
\param responsibleParties
    \li The array of responsible parties.
\param numberOfParties
    \li The number of elements in \e responsibleParties.
\return
    \li True if the structure is added, False if an error occurs.
*/
//************************************************************************
bool addCitation(xmlNode &parentNode, const char *title, const char *date, const char *dateType,
                 const BagResponsibleParty *responsibleParties, uint32_t numberOfParties)
{
    //CI_citation is optional, so if no title was given just return.
    if (!title)
        return true;

    const xmlNsPtr pGmdNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("gmd"));

    //Create the CI_Citation node.
    xmlNode *pCitationNode = xmlNewChild(&parentNode, pGmdNamespace, XMLCast("CI_Citation"), nullptr);

    //Add the title
    {
        //Create the title node.
        xmlNode *pTitleNode = xmlNewChild(pCitationNode, pGmdNamespace, XMLCast("title"), nullptr);

        //Set the title value.
        addCharacterNode(*pTitleNode, title);
    }

    //Add the date
    {
        //Create the date nodes.
        xmlNode *pDateNode = xmlNewChild(pCitationNode, pGmdNamespace, XMLCast("date"), nullptr);
        pDateNode = xmlNewChild(pDateNode, pGmdNamespace, XMLCast("CI_Date"), nullptr);

        //Create the date node.
        xmlNode *pDateNode2 = xmlNewChild(pDateNode, pGmdNamespace, XMLCast("date"), nullptr);

        //Set the date value.
        addDateNode(*pDateNode2, date);

        //Create the dateType node.
        xmlNode *pDateTypeNode = xmlNewChild(pDateNode, pGmdNamespace, XMLCast("dateType"), nullptr);

        //Set the date type value.
        addCodeListNode(*pDateTypeNode, "gmd", "CI_DateTypeCode",
            "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml", dateType);
    }

    //Add the responsible parties
    {
        for (uint32_t r = 0; r < numberOfParties; r++)
        {
            //Create the citedResponsibleParty node.
            xmlNode *pPartyNode = xmlNewChild(pCitationNode, pGmdNamespace, XMLCast("citedResponsibleParty"), nullptr);

            const bool ret = addResponsibleParty(*pPartyNode, responsibleParties[r]);
            if (!ret)
            {
                fprintf(stderr, "ERROR: responsibleParties[%d]: At least one of the following fields must be supplied. individualName, organisationName, postionName.\n", r);
                return false;
            }
        }
    }

    return true;
}

//************************************************************************
//! Add the BagIdentification information to the parent XML node.
/*!
\param parentNode
    \li The parent XML node to be modified.
\param identificationInfo
    \li The identification information to be added to \e parentNode
\return
    \li True if the structure is added, False if an error occurs.
*/
//************************************************************************
bool addDataIdentification(xmlNode &parentNode, const BagIdentification &identificationInfo)
{
    /* Check for the required fields. If they are not present, return nullptr. */
    if (identificationInfo.abstractString == nullptr ||
        identificationInfo.language == nullptr ||
        identificationInfo.verticalUncertaintyType == nullptr)
    {
	    fprintf(stderr, "ERROR: can not create BAG identificationInfo.  Missing one or more required fields... abstract, language or verticalUncertaintyType. \n");
	    return false;
    }

    //Find the gmd namespace.
    const xmlNsPtr pGmdNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("gmd"));
    const xmlNsPtr pBagNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("bag"));

    //Create the identificationInfo node.
    xmlNode *pIdentInfoNode = xmlNewChild(&parentNode, pGmdNamespace, XMLCast("identificationInfo"), nullptr);

    //Create the BAG_DataIdentification node.
    xmlNode *pBagIdentInfoNode = xmlNewChild(pIdentInfoNode, pBagNamespace, XMLCast("BAG_DataIdentification"), nullptr);

    //Citation
    {
        //Create the citation node.
        xmlNode *pCitationNode = xmlNewChild(pBagIdentInfoNode, pGmdNamespace, XMLCast("citation"), nullptr);

        //Add the citation info.
        const bool ret = addCitation(*pCitationNode, identificationInfo.title,
            identificationInfo.date,
            identificationInfo.dateType,
            identificationInfo.responsibleParties,
            identificationInfo.numberOfResponsibleParties);
        if (!ret)
            return false;
    }

    //Abstract
    {
        //Add the abstract.
        xmlNode *pAbstractNode = xmlNewChild(pBagIdentInfoNode, pGmdNamespace, XMLCast("abstract"), nullptr);

        //Set the value.
        addCharacterNode(*pAbstractNode, identificationInfo.abstractString);
    }

    //Status (Optional)
    if (identificationInfo.status != nullptr)
    {
        //Add the status.
        xmlNode *pStatusNode = xmlNewChild(pBagIdentInfoNode, pGmdNamespace, XMLCast("status"), nullptr);

        //Set the value.
        addCodeListNode(*pStatusNode, "gmd", "MD_ProgressCode",
            "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml", identificationInfo.status);
    }

    //spatialRepresentationType (Optional)
    if (identificationInfo.spatialRepresentationType != nullptr)
    {
        //Add the spatialRepresentationType.
        xmlNode *pTypeNode = xmlNewChild(pBagIdentInfoNode, pGmdNamespace, XMLCast("spatialRepresentationType"), nullptr);

        //Set the value.
        addCodeListNode(*pTypeNode, "gmd", "MD_SpatialRepresentationTypeCode",
            "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml", identificationInfo.spatialRepresentationType);
    }

    //language
    {
        //Add the language.
        xmlNode *pLanguageNode = xmlNewChild(pBagIdentInfoNode, pGmdNamespace, XMLCast("language"), nullptr);

        //Set the value.
        addCodeListNode(*pLanguageNode, "gmd", "LanguageCode",
            "http://www.loc.gov/standards/iso639-2/", identificationInfo.language, false);
    }

    //characterSet
    {
        //Add the characterSet.
        xmlNode *pCharacterNode = xmlNewChild(pBagIdentInfoNode, pGmdNamespace, XMLCast("characterSet"), nullptr);

        //Set the value.
        addCodeListNode(*pCharacterNode, "gmd", "MD_CharacterSetCode",
            "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml", identificationInfo.characterSet);
    }

    //topicCategory
    {
        //Add the topicCategory.
        xmlNode *pTopicNode = xmlNewChild(pBagIdentInfoNode, pGmdNamespace, XMLCast("topicCategory"), nullptr);

        //Create the MD_TopicCategoryCode node.
        xmlNode *pCodeNode = xmlNewChild(pTopicNode, pGmdNamespace, XMLCast("MD_TopicCategoryCode"),
            EncodedString(*parentNode.doc, identificationInfo.topicCategory));

        //Set the value.
        addCodeListNode(*pCodeNode, "gmd", "MD_TopicCategoryCode",
            "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml", identificationInfo.topicCategory);
    }

    //extent (Optional)
    if ( identificationInfo.westBoundingLongitude != double(INIT_VALUE) &&
         identificationInfo.eastBoundingLongitude != double(INIT_VALUE) &&
         identificationInfo.southBoundingLatitude != double(INIT_VALUE) &&
         identificationInfo.northBoundingLatitude != double(INIT_VALUE) )
    {
        //Add the extent.
        xmlNode *pExtentNode = xmlNewChild(pBagIdentInfoNode, pGmdNamespace, XMLCast("extent"), nullptr);

        //Add the EX_Extent
        xmlNode *pExtentNode2 = xmlNewChild(pExtentNode, pGmdNamespace, XMLCast("EX_Extent"), nullptr);

        //Add the geographicElement
        xmlNode *pGeoNode = xmlNewChild(pExtentNode2, pGmdNamespace, XMLCast("geographicElement"), nullptr);

        //Add the EX_GeographicBoundingBox
        xmlNode *pGeoNode2 = xmlNewChild(pGeoNode, pGmdNamespace, XMLCast("EX_GeographicBoundingBox"), nullptr);

        //Add the westBoundLongitude
        xmlNode *pCoordNode = xmlNewChild(pGeoNode2, pGmdNamespace, XMLCast("westBoundLongitude"), nullptr);
        addDecimalNode(*pCoordNode, identificationInfo.westBoundingLongitude);

        //Add the eastBoundLongitude
        pCoordNode = xmlNewChild(pGeoNode2, pGmdNamespace, XMLCast("eastBoundLongitude"), nullptr);
        addDecimalNode(*pCoordNode, identificationInfo.eastBoundingLongitude);

        //Add the southBoundLatitude
        pCoordNode = xmlNewChild(pGeoNode2, pGmdNamespace, XMLCast("southBoundLatitude"), nullptr);
        addDecimalNode(*pCoordNode, identificationInfo.southBoundingLatitude);

        //Add the northBoundLatitude
        pCoordNode = xmlNewChild(pGeoNode2, pGmdNamespace, XMLCast("northBoundLatitude"), nullptr);
        addDecimalNode(*pCoordNode, identificationInfo.northBoundingLatitude);
    }

    //verticalUncertaintyType
    {
        //Add the verticalUncertaintyType.
        xmlNode *pVertUncertNode = xmlNewChild(pBagIdentInfoNode, pBagNamespace, XMLCast("verticalUncertaintyType"), nullptr);

        //Set the value.
        addCodeListNode(*pVertUncertNode, "bag", "BAG_VertUncertCode",
            "http://www.opennavsurf.org/schema/bag/bagCodelists.xml", (const char*)identificationInfo.verticalUncertaintyType);
    }

    //depthCorrectionType (Optional)
    if (identificationInfo.depthCorrectionType != nullptr)
    {
        //Add the depthCorrectionType.
        xmlNode *pDepthCorrNode = xmlNewChild(pBagIdentInfoNode, pBagNamespace, XMLCast("depthCorrectionType"), nullptr);

        //Set the value.
        addCodeListNode(*pDepthCorrNode, "bag", "BAG_DepthCorrectCode",
            "http://www.opennavsurf.org/schema/bag/bagCodelists.xml", (const char*)identificationInfo.depthCorrectionType);
    }

    //elevationSolutionGroupType (Optional)
    if (identificationInfo.elevationSolutionGroupType != nullptr)
    {
        //Add the depthCorrectionType.
        xmlNode *pElevNode = xmlNewChild(pBagIdentInfoNode, pBagNamespace, XMLCast("elevationSolutionGroupType"), nullptr);

        //Set the value.
        addCodeListNode(*pElevNode, "bag", "BAG_OptGroupCode",
            "http://www.opennavsurf.org/schema/bag/bagCodelists.xml", (const char*)identificationInfo.elevationSolutionGroupType);
    }

    //nodeGroupType (Optional)
    if (identificationInfo.nodeGroupType != nullptr)
    {
        //Add the depthCorrectionType.
        xmlNode *pNodeGroupNode = xmlNewChild(pBagIdentInfoNode, pBagNamespace, XMLCast("nodeGroupType"), nullptr);

        //Set the value.
        addCodeListNode(*pNodeGroupNode, "bag", "BAG_OptGroupCode",
            "http://www.opennavsurf.org/schema/bag/bagCodelists.xml", (const char*)identificationInfo.nodeGroupType);
    }

    return true;
}

//************************************************************************
//! Add the BagSecurityConstraints information to the parent XML node.
/*!
\param parentNode
    \li The parent XML node to be modified.
\param securityConstraints
    \li The security constraint information to be added to \e parentNode
\return
    \li True if the structure is added, False if an error occurs.
*/
//************************************************************************
bool addSecurityConstraints(xmlNode &parentNode, const BagSecurityConstraints &securityConstraints)
{
    /* If either the classification or the distribution statement is not supplied, the node should not be created.*/
    if (securityConstraints.classification == nullptr ||
        securityConstraints.userNote == nullptr)
    {
        fprintf(stderr, "ERROR: creating security constraints. Classification and Distribution statement must be supplied!.\n");
        return false;
    }

    const xmlNsPtr pGmdNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("gmd"));

    //Create the metadataConstraints node.
    xmlNode *pConstraintNode = xmlNewChild(&parentNode, pGmdNamespace, XMLCast("metadataConstraints"), nullptr);

    //Create the MD_SecurityConstraints node.
    xmlNode *pConstraintNode2 = xmlNewChild(pConstraintNode, pGmdNamespace, XMLCast("MD_SecurityConstraints"), nullptr);

    //classification
    {
        //Create the title node.
        xmlNode *pNode = xmlNewChild(pConstraintNode2, pGmdNamespace, XMLCast("classification"), nullptr);
        addCodeListNode(*pNode, "gmd", "MD_ClassificationCode",
            "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml", (char*)securityConstraints.classification);
    }

    //userNote
    {
        //Create the title node.
        xmlNode *pNode = xmlNewChild(pConstraintNode2, pGmdNamespace, XMLCast("userNote"), nullptr);

        //Set the title value.
        addCharacterNode(*pNode, (char*)securityConstraints.userNote);
    }

    return true;
}

//************************************************************************
//! Add the BagLegalConstraints information to the parent XML node.
/*!
\param parentNode
    \li The parent XML node to be modified.
\param legalConstraints
    \li The legal constraint information to be added to \e parentNode
\return
    \li True if the structure is added, False if an error occurs.
*/
//************************************************************************
bool addLegalConstraints(xmlNode &parentNode, const BagLegalConstraints &legalConstraints)
{
    const xmlNsPtr pGmdNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("gmd"));

    //Create the metadataConstraints node.
    xmlNode *pConstraintNode = xmlNewChild(&parentNode, pGmdNamespace, XMLCast("metadataConstraints"), nullptr);

    //Create the MD_LegalConstraints node.
    xmlNode *pConstraintNode2 = xmlNewChild(pConstraintNode, pGmdNamespace, XMLCast("MD_LegalConstraints"), nullptr);

    //useConstraints
    {
        //Create the useConstraints node.
        xmlNode *pNode = xmlNewChild(pConstraintNode2, pGmdNamespace, XMLCast("useConstraints"), nullptr);
        addCodeListNode(*pNode, "gmd", "MD_RestrictionCode",
            "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml", (char*)legalConstraints.useConstraints);
    }

    //otherConstraints
    {
        //Create the title node.
        xmlNode *pNode = xmlNewChild(pConstraintNode2, pGmdNamespace, XMLCast("otherConstraints"), nullptr);

        //Set the title value.
        addCharacterNode(*pNode, (char*)legalConstraints.otherConstraints);
    }

    return true;
}

//************************************************************************
//! Add the BagSource information to the parent XML node.
/*!
\param parentNode
    \li The parent XML node to be modified.
\param source
    \li The process source information to be added to \e parentNode
\return
    \li True if the structure is added, False if an error occurs.
*/
//************************************************************************
bool addProcessSource(xmlNode &parentNode, const BagSource &source)
{
    const xmlNsPtr pGmdNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("gmd"));

    //The description is required.
    if (source.description == nullptr)
    {
        fprintf(stderr, "ERROR: source description not supplied.\n");
        return false;
    }

    //Create the source node.
    xmlNode *pSourceNode = xmlNewChild(&parentNode, pGmdNamespace, XMLCast("source"), nullptr);

    //Create the LI_Source node.
    xmlNode *pSourceNode2 = xmlNewChild(pSourceNode, pGmdNamespace, XMLCast("LI_Source"), nullptr);

    //description
    {
        //Create the description node.
        xmlNode *pNode = xmlNewChild(pSourceNode2, pGmdNamespace, XMLCast("description"), nullptr);

        //Set the title value.
        addCharacterNode(*pNode, (char*)source.description);
    }

    //sourceCitation
    {
        //Create the sourceCitation node.
        xmlNode *pNode = xmlNewChild(pSourceNode2, pGmdNamespace, XMLCast("sourceCitation"), nullptr);

        const bool ret = addCitation(*pNode, (char*)source.title, (char*)source.date, (char*)source.dateType,
            source.responsibleParties, source.numberOfResponsibleParties);
        if (!ret)
            return false;
    }

    return true;
}

//************************************************************************
//! Add the BagProcessStep information to the parent XML node.
/*!
\param parentNode
    \li The parent XML node to be modified.
\param processInfo
    \li The process step information to be added to \e parentNode
\return
    \li True if the structure is added, False if an error occurs.
*/
//************************************************************************
bool addProcessStep(xmlNode &parentNode, const BagProcessStep &processInfo)
{
    const xmlNsPtr pGmdNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("gmd"));
    const xmlNsPtr pBagNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("bag"));

    xmlNode *pProcessStepNode = xmlNewChild(&parentNode, pGmdNamespace, XMLCast("processStep"), nullptr);
    pProcessStepNode = xmlNewChild(pProcessStepNode, pBagNamespace, XMLCast("BAG_ProcessStep"), nullptr);

    //description
    {
        //Create the description node.
        xmlNode *pNode = xmlNewChild(pProcessStepNode, pGmdNamespace, XMLCast("description"), nullptr);
        addCharacterNode(*pNode, (char*)processInfo.description);
    }

    //dateTime
    {
        //Create the dateTime node.
        xmlNode *pNode = xmlNewChild(pProcessStepNode, pGmdNamespace, XMLCast("dateTime"), nullptr);
        addDateTimeNode(*pNode, (char*)processInfo.dateTime);
    }

    //processor
    for (uint32_t i = 0; i < processInfo.numberOfProcessors; i++)
    {
        //Create the processor node.
        xmlNode *pNode = xmlNewChild(pProcessStepNode, pGmdNamespace, XMLCast("processor"), nullptr);
        addResponsibleParty(*pNode, processInfo.processors[i]);
    }

    //source
    for (uint32_t i = 0; i < processInfo.numberOfSources; i++)
    {
        const bool ret = addProcessSource(*pProcessStepNode, processInfo.lineageSources[i]);
        if (!ret)
            return false;
    }

    //trackingId
    {
        //Create the trackingId node.
        xmlNode *pNode = xmlNewChild(pProcessStepNode, pBagNamespace, XMLCast("trackingId"), nullptr);
        addCharacterNode(*pNode, (char*)processInfo.trackingId);
    }

    return true;
}

//************************************************************************
//! Add the BagDataQuality information to the parent XML node.
/*!
\param parentNode
    \li The parent XML node to be modified.
\param dataQuality
    \li The data quality information to be added to \e parentNode
\return
    \li True if the structure is added, False if an error occurs.
*/
//************************************************************************
bool addDataQuality(xmlNode &parentNode, const BagDataQuality &dataQuality)
{
    const xmlNsPtr pGmdNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("gmd"));

    //Create the dataQualityInfo node.
    xmlNode *pQualityNode = xmlNewChild(&parentNode, pGmdNamespace, XMLCast("dataQualityInfo"), nullptr);

    //Create the DQ_DataQuality node.
    xmlNode *pQualityNode2 = xmlNewChild(pQualityNode, pGmdNamespace, XMLCast("DQ_DataQuality"), nullptr);

    //scope
    {
        xmlNode *pNode = xmlNewChild(pQualityNode2, pGmdNamespace, XMLCast("scope"), nullptr);
        pNode = xmlNewChild(pNode, pGmdNamespace, XMLCast("DQ_Scope"), nullptr);
        pNode = xmlNewChild(pNode, pGmdNamespace, XMLCast("level"), nullptr);
        addCodeListNode(*pNode, "gmd", "MD_ScopeCode",
            "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml", (char*)dataQuality.scope);
    }

    //lineage
    {
        xmlNode *pLineageNode = xmlNewChild(pQualityNode2, pGmdNamespace, XMLCast("lineage"), nullptr);
        pLineageNode = xmlNewChild(pLineageNode, pGmdNamespace, XMLCast("LI_Lineage"), nullptr);

        //Add each process step.
        for (uint32_t i = 0; i < dataQuality.numberOfProcessSteps; i++)
        {
            const bool ret = addProcessStep(*pLineageNode, dataQuality.lineageProcessSteps[i]);
            if (!ret)
                return false;
        }
    }

    return true;
}

//************************************************************************
//! Add the BagSpatialRepresentation information to the parent XML node.
/*!
\param parentNode
    \li The parent XML node to be modified.
\param spatialRepresentationInfo
    \li The spatial representation information to be added to \e parentNode
\return
    \li True if the structure is added, False if an error occurs.
*/
//************************************************************************
bool addSpatialRepresentation(xmlNode &parentNode, const BagSpatialRepresentation &spatialRepresentationInfo)
{
    /* Check for required elements. If do not exist, return nullptr*/

    /* Must have specified cellGeometry, transformationParameterAvailability,and checkPointAvailability */
    /* If any of the four corner points equal the INIT_VALUE, this indicates the points have not been populated by the user. */

    if (spatialRepresentationInfo.cellGeometry == nullptr)
    {
        fprintf(stderr, "ERROR: spatialRepresentationInfo.cellGeometry, transformationParameterAvailability,checkPointAvailability must be supplied\n");
        return false;
    }

    if (spatialRepresentationInfo.llCornerX == (double)INIT_VALUE ||
        spatialRepresentationInfo.llCornerY == (double)INIT_VALUE ||
        spatialRepresentationInfo.urCornerX == (double)INIT_VALUE ||
        spatialRepresentationInfo.urCornerY == (double)(INIT_VALUE))
    {

        fprintf(stderr, "ERROR: All four spatialRepresentationInfo corner points must be supplied. \n");
        return false;

    }

    const xmlNsPtr pGmdNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("gmd"));
    const xmlNsPtr pGmlNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("gml"));

    //Create the spatialRepresentationInfo node.
    xmlNode *pSpatialRepNode = xmlNewChild(&parentNode, pGmdNamespace, XMLCast("spatialRepresentationInfo"), nullptr);

    //Create the MD_Georectified node.
    xmlNode *pGeoNode = xmlNewChild(pSpatialRepNode, pGmdNamespace, XMLCast("MD_Georectified"), nullptr);

    //numberOfDimensions
    {
        xmlNode *pNode = xmlNewChild(pGeoNode, pGmdNamespace, XMLCast("numberOfDimensions"), nullptr);
        addIntegerNode(*pNode, 2);
    }

    //axisDimensionProperties
    addDimension(*pGeoNode, "row", spatialRepresentationInfo.numberOfRows, spatialRepresentationInfo.rowResolution,
        spatialRepresentationInfo.resolutionUnit);
    addDimension(*pGeoNode, "column", spatialRepresentationInfo.numberOfColumns, spatialRepresentationInfo.columnResolution,
        spatialRepresentationInfo.resolutionUnit);

    //cellGeometry
    {
        xmlNode *pNode = xmlNewChild(pGeoNode, pGmdNamespace, XMLCast("cellGeometry"), nullptr);
        addCodeListNode(*pNode, "gmd", "MD_CellGeometryCode",
            "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml", (char*)spatialRepresentationInfo.cellGeometry);
    }

    //transformationParameterAvailability
    {
        xmlNode *pNode = xmlNewChild(pGeoNode, pGmdNamespace, XMLCast("transformationParameterAvailability"), nullptr);
        addBooleanNode(*pNode, spatialRepresentationInfo.transformationParameterAvailability);
    }

    //checkPointAvailability
    {
        xmlNode *pNode = xmlNewChild(pGeoNode, pGmdNamespace, XMLCast("checkPointAvailability"), nullptr);
        addBooleanNode(*pNode, spatialRepresentationInfo.checkPointAvailability);
    }

    //cornerPoints
    {
        xmlNode *pCornerNode = xmlNewChild(pGeoNode, pGmdNamespace, XMLCast("cornerPoints"), nullptr);

        xmlNode *pPointNode = xmlNewChild(pCornerNode, pGmlNamespace, XMLCast("Point"), nullptr);
        xmlSetProp(pPointNode, XMLCast("gml:id"), XMLCast("id1"));

        char pointsString[88];
        sprintf(pointsString, "%.12lf,%.12lf %.12lf,%.12lf", spatialRepresentationInfo.llCornerX, spatialRepresentationInfo.llCornerY, spatialRepresentationInfo.urCornerX, spatialRepresentationInfo.urCornerY);

        xmlNode *pCoordNode = xmlNewChild(pPointNode, pGmlNamespace, XMLCast("coordinates"), EncodedString(*parentNode.doc, pointsString));
        xmlSetProp(pCoordNode, XMLCast("decimal"), XMLCast("."));
        xmlSetProp(pCoordNode, XMLCast("cs"), XMLCast(","));
        xmlSetProp(pCoordNode, XMLCast("ts"), XMLCast(" "));
    }

    //pointInPixel
    {
        xmlNode *pNode = xmlNewChild(pGeoNode, pGmdNamespace, XMLCast("pointInPixel"), nullptr);
        xmlNewChild(pNode, pGmdNamespace, XMLCast("MD_PixelOrientationCode"), XMLCast("center"));
    }

    return true;
}

//************************************************************************
//! Add the BagReferenceSystem information to the parent XML node.
/*!
\param parentNode
    \li The parent XML node to be modified.
\param system
    \li The reference system information to be added to \e parentNode
\return
    \li True if the structure is added, False if an error occurs.
*/
//************************************************************************
bool addReferenceSystem(xmlNode &parentNode, const BagReferenceSystem &system)
{
    const xmlNsPtr pGmdNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("gmd"));
    //const xmlNsPtr pGcoNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("gco"));

    xmlNode *pNode = xmlNewChild(&parentNode, pGmdNamespace, XMLCast("referenceSystemInfo"), nullptr);
    pNode = xmlNewChild(pNode, pGmdNamespace, XMLCast("MD_ReferenceSystem"), nullptr);
    pNode = xmlNewChild(pNode, pGmdNamespace, XMLCast("referenceSystemIdentifier"), nullptr);
    pNode = xmlNewChild(pNode, pGmdNamespace, XMLCast("RS_Identifier"), nullptr);

    xmlNode *pCodeNode = xmlNewChild(pNode, pGmdNamespace, XMLCast("code"), nullptr);
    addCharacterNode(*pCodeNode, system.definition);

    xmlNode *pCodeNameNode = xmlNewChild(pNode, pGmdNamespace, XMLCast("codeSpace"), nullptr);
    addCharacterNode(*pCodeNameNode, system.type);

    return true;
}

//************************************************************************
//! Export the Metadata to a string.
/*!
\param metadata
    \li The metadata information to be exported.
\return
    \li The metadata as XML in a string.
*/
//************************************************************************
std::string exportMetadataToXML(
    const BagMetadata& metadata)
{
    //Create a new xml document.
    xmlDoc *pDocument = createNewDocument();

    //Get the document's root node.
    xmlNode *pRoot = xmlDocGetRootElement(pDocument);

    //Add the fileIdentifier
    {
        const xmlNsPtr pGmdNamespace = xmlSearchNs(pDocument, pRoot, XMLCast("gmd"));

        //Create the fileIdentifier node.
        xmlNode *pNode = xmlNewChild(pRoot, pGmdNamespace, XMLCast("fileIdentifier"), nullptr);
        addCharacterNode(*pNode, metadata.fileIdentifier);
    }

    //Add the language
    {
        const xmlNsPtr pGmdNamespace = xmlSearchNs(pDocument, pRoot, XMLCast("gmd"));

        //Create the language node.
        xmlNode *pNode = xmlNewChild(pRoot, pGmdNamespace, XMLCast("language"), nullptr);
        addCodeListNode(*pNode, "gmd", "LanguageCode",
            "http://www.loc.gov/standards/iso639-2/", metadata.language, false);
    }

    //Add the characterSet
    {
        const xmlNsPtr pGmdNamespace = xmlSearchNs(pDocument, pRoot, XMLCast("gmd"));

        //Create the characterSet node.
        xmlNode *pNode = xmlNewChild(pRoot, pGmdNamespace, XMLCast("characterSet"), nullptr);
        addCodeListNode(*pNode, "gmd", "MD_CharacterSetCode",
            "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml",
            metadata.characterSet);
    }

    //Add the hierarchyLevel.
    {
        const xmlNsPtr pGmdNamespace = xmlSearchNs(pDocument, pRoot, XMLCast("gmd"));

        //Create the hierarchyLevel node.
        xmlNode *pNode = xmlNewChild(pRoot, pGmdNamespace, XMLCast("hierarchyLevel"), nullptr);
        addCodeListNode(*pNode, "gmd", "MD_ScopeCode",
            "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml",
            metadata.hierarchyLevel);
    }

    //Add the contact
    {
         const xmlNsPtr pGmdNamespace = xmlSearchNs(pDocument, pRoot, XMLCast("gmd"));

        //Create the characterSet node.
        xmlNode *pNode = xmlNewChild(pRoot, pGmdNamespace, XMLCast("contact"), nullptr);
        if (!addResponsibleParty(*pNode, *metadata.contact))
        {
            xmlFreeDoc(pDocument);
            return {};
        }
    }

    //Add the dateStamp
    {
        const xmlNsPtr pGmdNamespace = xmlSearchNs(pDocument, pRoot, XMLCast("gmd"));

        //Create the hierarchyLevel node.
        xmlNode *pNode = xmlNewChild(pRoot, pGmdNamespace, XMLCast("dateStamp"), nullptr);
        addDateNode(*pNode, metadata.dateStamp);
    }

    //Add the metadataStandardName
    {
        const xmlNsPtr pGmdNamespace = xmlSearchNs(pDocument, pRoot, XMLCast("gmd"));

        //Create the metadataStandardName node.
        xmlNode *pNode = xmlNewChild(pRoot, pGmdNamespace, XMLCast("metadataStandardName"), nullptr);
        addCharacterNode(*pNode, metadata.metadataStandardName);
    }

    //Add the metadataStandardVersion
    {
        const xmlNsPtr pGmdNamespace = xmlSearchNs(pDocument, pRoot, XMLCast("gmd"));

        //Create the metadataStandardVersion node.
        xmlNode *pNode = xmlNewChild(pRoot, pGmdNamespace, XMLCast("metadataStandardVersion"), nullptr);
        addCharacterNode(*pNode, metadata.metadataStandardVersion);
    }

    //Add the spatialRepresentationInfo
    if (!addSpatialRepresentation(*pRoot, *metadata.spatialRepresentationInfo))
    {
        xmlFreeDoc(pDocument);
        return {};
    }

    //Add the horizontal referenceSystemInfo
    if (!addReferenceSystem(*pRoot, *metadata.horizontalReferenceSystem))
    {
        xmlFreeDoc(pDocument);
        return {};
    }

    //Add the vertical referenceSystemInfo
    if (!addReferenceSystem(*pRoot, *metadata.verticalReferenceSystem))
    {
        xmlFreeDoc(pDocument);
        return {};
    }

    //Add the data identification information.
    if (!addDataIdentification(*pRoot, *metadata.identificationInfo))
    {
        xmlFreeDoc(pDocument);
        return {};
    }

    //Add the data quality information.
    if (!addDataQuality(*pRoot, *metadata.dataQualityInfo))
    {
        xmlFreeDoc(pDocument);
        return {};
    }

    //Add the legal constraint information.
    if (!addLegalConstraints(*pRoot, *metadata.legalConstraints))
    {
        xmlFreeDoc(pDocument);
        return {};
    }

    //Add the security constraint information.
    if (!addSecurityConstraints(*pRoot, *metadata.securityConstraints))
    {
        xmlFreeDoc(pDocument);
        return {};
    }

    //Export to the buffer.
    int bufferSize = 0;
    xmlChar *pBuffer = nullptr;
    xmlDocDumpMemoryEnc(pDocument, &pBuffer, &bufferSize, "UTF-8");
    xmlFreeDoc(pDocument);

    //If nothing was exported, then just return.
    if (bufferSize == 0)
        return {};

    //Copy the buffer to our output string and add a null terminator.
    std::string result{reinterpret_cast<char*>(pBuffer), static_cast<size_t>(bufferSize)};

    xmlFree(pBuffer);
    return result;
}

}  // namespace BAG

