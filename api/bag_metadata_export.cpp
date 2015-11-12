//************************************************************************
//
//      Open Navigation Surface Working Group, 2013
//
//************************************************************************
#include "bag_metadata.h"

#include <iostream>
#include <sstream>
#include <string>
#include <string.h>

#include <libxml/parser.h>

namespace
{

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

#define XMLCast(__value__) (const xmlChar *)(__value__)

//************************************************************************
//! Convert a string to a double value.
/*!
\param value
    \li The intput string to be converted.
\return
    \li The doulbe value.
*/
//************************************************************************
double toDouble(const char *value)
{
    std::stringstream lineStream;
    lineStream.imbue(std::locale::classic());
    lineStream << value;

    double dblValue = 0.0;
    lineStream >> dblValue;

    return dblValue;
}

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
    const xmlNsPtr pGcoNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("gco"));

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

    //If the content is NULL, then we will just add a nilReason.
    if (content == NULL)
    {
        xmlSetProp(&parentNode, XMLCast("gco:nilReason"), XMLCast("unknown"));
        return NULL;
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
                         const char *url, const char *value, Bool appendValueToUrl = True)
{
    const xmlNsPtr pNamespace = xmlSearchNs(parentNode.doc, &parentNode, EncodedString(*parentNode.doc, codeNameSpace));

    //Create the codeList node.
    xmlNode *pCodeNode = xmlNewChild(&parentNode, pNamespace, EncodedString(*parentNode.doc, codeName), EncodedString(*parentNode.doc, value));

    std::stringstream fullUrlStream;
    fullUrlStream << url;
    
    if (appendValueToUrl == True)
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
    const xmlNsPtr pGcoNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("gco"));

    xmlNode *pDimNode = xmlNewChild(&parentNode, pGmdNamespace, XMLCast("axisDimensionProperties"), NULL);
    pDimNode = xmlNewChild(pDimNode, pGmdNamespace, XMLCast("MD_Dimension"), NULL);

    //dimensionName
    {
        xmlNode *pNode = xmlNewChild(pDimNode, pGmdNamespace, XMLCast("dimensionName"), NULL);
        addCodeListNode(*pNode, "gmd", "MD_DimensionNameTypeCode",
            "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml", name);
    }

    //dimensionSize
    {
        xmlNode *pNode = xmlNewChild(pDimNode, pGmdNamespace, XMLCast("dimensionSize"), NULL);
        addIntegerNode(*pNode, size);
    }

    //resolution
    {
        xmlNode *pNode = xmlNewChild(pDimNode, pGmdNamespace, XMLCast("resolution"), NULL);
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
xmlNode* addBooleanNode(xmlNode &parentNode, Bool value)
{
    const std::string valString((value == True) ? "1" : "0");

    const xmlNsPtr pGcoNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("gco"));

    //Create the Integer node.
    xmlNode *pBoolNode = xmlNewChild(&parentNode, pGcoNamespace, XMLCast("Boolean"), XMLCast(valString.c_str()));
    return pBoolNode;
}

}   //namespace


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
    xmlNodePtr pRoot = xmlNewDocNode(pDocument, NULL, XMLCast(rootName), NULL);
    xmlDocSetRootElement(pDocument, pRoot);

    //Create the root namespace and assign to the root.
    xmlNs *pRootNameSpace = xmlNewNs(pRoot, XMLCast("http://www.isotc211.org/2005/gmi"), XMLCast("gmi"));
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
//! Add the BAG_RESPONSIBLE_PARTY information to the parent XML node.
/*!
\param parentNode
    \li The parent XML node to be modified.
\param responsiblePartyStruct
    \li The structure to be added to \e parentNode.
\return 
    \li True if the structure is added, False if an error occurs.
*/
//************************************************************************
Bool addResponsibleParty(xmlNode &parentNode, const BAG_RESPONSIBLE_PARTY &responsiblePartyStruct)
{
    /* Criteria for this node is that "role must be supplied and at least one of the following fileds must be supplied. */
    if (responsiblePartyStruct.individualName == NULL &&
        responsiblePartyStruct.organisationName == NULL &&
        responsiblePartyStruct.positionName == NULL)
    {
        return False;
    }

    /* If "role" is not populated, don't create the element.  "role" is a required element of the schema. */
    if (responsiblePartyStruct.role == NULL)
    {
        fprintf(stderr, "ERROR: The \"role\" is required in order to create the CI_ResponsibleParty node.\n");
        return False;
    }

    //Find the gmd namespace.
    const xmlNsPtr pGmdNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("gmd"));
                        
    //Create the CI_ResponsibleParty node.
    xmlNode *pPartyNode = xmlNewChild(&parentNode, pGmdNamespace, XMLCast("CI_ResponsibleParty"), NULL);
                
    /* If an individual name has been supplied, Create the individual node and populate it. */
    if (responsiblePartyStruct.individualName != NULL)
    {
        xmlNode *pNode = xmlNewChild(pPartyNode, pGmdNamespace, XMLCast("individualName"), NULL);
        addCharacterNode(*pNode, (char*)responsiblePartyStruct.individualName);
    }

    /* If an organisation name has been supplied, Create the organisation node and populate it. */
    if (responsiblePartyStruct.organisationName != NULL)
    {
        xmlNode *pNode = xmlNewChild(pPartyNode, pGmdNamespace, XMLCast("organisationName"), NULL);
        addCharacterNode(*pNode, (char*)responsiblePartyStruct.organisationName);
    }

    /* If a postiion name has been supplied, Create the position node and populate it. */
    if (responsiblePartyStruct.positionName != NULL)
    {
        xmlNode *pNode = xmlNewChild(pPartyNode, pGmdNamespace, XMLCast("positionName"), NULL);
        addCharacterNode(*pNode, (char*)responsiblePartyStruct.positionName);
    }

    //Create the role node and populate it.
    xmlNode *pNode = xmlNewChild(pPartyNode, pGmdNamespace, XMLCast("role"), NULL);
    addCodeListNode(*pNode, "gmd", "CI_RoleCode",
        "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml", (char*)responsiblePartyStruct.role);

    return True;
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
Bool addCitation(xmlNode &parentNode, const char *title, const char *date, const char *dateType, 
                 const BAG_RESPONSIBLE_PARTY *responsibleParties, u32 numberOfParties)
{
    //CI_citation is optional, so if no title was given just return.
    if (title == NULL)
        return True;

    const xmlNsPtr pGmdNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("gmd"));
                        
    //Create the CI_Citation node.
    xmlNode *pCitationNode = xmlNewChild(&parentNode, pGmdNamespace, XMLCast("CI_Citation"), NULL);

    //Add the title
    {
        //Create the title node.
        xmlNode *pTitleNode = xmlNewChild(pCitationNode, pGmdNamespace, XMLCast("title"), NULL);

        //Set the title value.
        addCharacterNode(*pTitleNode, title);
    }

    //Add the date
    {
        //Create the date nodes.
        xmlNode *pDateNode = xmlNewChild(pCitationNode, pGmdNamespace, XMLCast("date"), NULL);
        pDateNode = xmlNewChild(pDateNode, pGmdNamespace, XMLCast("CI_Date"), NULL);

        //Create the date node.
        xmlNode *pDateNode2 = xmlNewChild(pDateNode, pGmdNamespace, XMLCast("date"), NULL);

        //Set the date value.
        addDateNode(*pDateNode2, date);

        //Create the dateType node.
        xmlNode *pDateTypeNode = xmlNewChild(pDateNode, pGmdNamespace, XMLCast("dateType"), NULL);

        //Set the date type value.
        addCodeListNode(*pDateTypeNode, "gmd", "CI_DateTypeCode",
            "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml", dateType);
    }

    //Add the responsible parties
    {
        for (u32 r = 0; r < numberOfParties; r++)
        {
            //Create the citedResponsibleParty node.
            xmlNode *pPartyNode = xmlNewChild(pCitationNode, pGmdNamespace, XMLCast("citedResponsibleParty"), NULL);

            const Bool ret = addResponsibleParty(*pPartyNode, responsibleParties[r]);
            if (!ret)
            {
                fprintf(stderr, "ERROR: responsibleParties[%d]: At least one of the following fields must be supplied. individualName, organisationName, postionName.\n", r);
                return False;
            }
        }
    }

    return True;
}

//************************************************************************
//! Add the BAG_IDENTIFICATION information to the parent XML node.
/*!
\param parentNode
    \li The parent XML node to be modified.
\param identificationInfo
    \li The identification information to be added to \e parentNode
\return 
    \li True if the structure is added, False if an error occurs.
*/
//************************************************************************
Bool addDataIdentification(xmlNode &parentNode, const BAG_IDENTIFICATION &identificationInfo)
{
    /* Check for the required fields. If they are not present, return NULL. */
    if (identificationInfo.abstractString == NULL ||
        identificationInfo.language == NULL ||
        identificationInfo.verticalUncertaintyType == NULL)
    {
	    fprintf(stderr, "ERROR: can not create BAG identificationInfo.  Missing one or more required fields... abstract, language or verticalUncertaintyType. \n");
	    return False;
    }

    //Find the gmd namespace.
    const xmlNsPtr pGmdNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("gmd"));
    const xmlNsPtr pBagNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("bag"));
                        
    //Create the identificationInfo node.
    xmlNode *pIdentInfoNode = xmlNewChild(&parentNode, pGmdNamespace, XMLCast("identificationInfo"), NULL);

    //Create the BAG_DataIdentification node.
    xmlNode *pBagIdentInfoNode = xmlNewChild(pIdentInfoNode, pBagNamespace, XMLCast("BAG_DataIdentification"), NULL);

    //Citation
    {
        //Create the citation node.
        xmlNode *pCitationNode = xmlNewChild(pBagIdentInfoNode, pGmdNamespace, XMLCast("citation"), NULL);

        //Add the citation info.
        const Bool ret = addCitation(*pCitationNode, (const char *)identificationInfo.title, 
            (const char *)identificationInfo.date, 
            (const char *)identificationInfo.dateType,
            identificationInfo.responsibleParties,
            identificationInfo.numberOfResponsibleParties);
        if (!ret)
            return False;
    }

    //Abstract
    {
        //Add the abstract.
        xmlNode *pAbstractNode = xmlNewChild(pBagIdentInfoNode, pGmdNamespace, XMLCast("abstract"), NULL);

        //Set the value.
        addCharacterNode(*pAbstractNode, (const char*)identificationInfo.abstractString);
    }

    //Status (Optional)
    if (identificationInfo.status != NULL)
    {
        //Add the status.
        xmlNode *pStatusNode = xmlNewChild(pBagIdentInfoNode, pGmdNamespace, XMLCast("status"), NULL);

        //Set the value.
        addCodeListNode(*pStatusNode, "gmd", "MD_ProgressCode",
            "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml", (const char*)identificationInfo.status);
    }
     
    //spatialRepresentationType (Optional)
    if (identificationInfo.spatialRepresentationType != NULL)
    {
        //Add the spatialRepresentationType.
        xmlNode *pTypeNode = xmlNewChild(pBagIdentInfoNode, pGmdNamespace, XMLCast("spatialRepresentationType"), NULL);

        //Set the value.
        addCodeListNode(*pTypeNode, "gmd", "MD_SpatialRepresentationTypeCode",
            "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml", (const char*)identificationInfo.spatialRepresentationType);
    }

    //language
    {
        //Add the language.
        xmlNode *pLanguageNode = xmlNewChild(pBagIdentInfoNode, pGmdNamespace, XMLCast("language"), NULL);

        //Set the value.
        addCodeListNode(*pLanguageNode, "gmd", "LanguageCode",
            "http://www.loc.gov/standards/iso639-2/", (const char*)identificationInfo.language, False);
    }

    //characterSet
    {
        //Add the characterSet.
        xmlNode *pCharacterNode = xmlNewChild(pBagIdentInfoNode, pGmdNamespace, XMLCast("characterSet"), NULL);

        //Set the value.
        addCodeListNode(*pCharacterNode, "gmd", "MD_CharacterSetCode",
            "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml", (const char*)identificationInfo.character_set);
    }

    //topicCategory
    {
        //Add the topicCategory.
        xmlNode *pTopicNode = xmlNewChild(pBagIdentInfoNode, pGmdNamespace, XMLCast("topicCategory"), NULL);

        //Create the MD_TopicCategoryCode node.
        xmlNode *pCodeNode = xmlNewChild(pTopicNode, pGmdNamespace, XMLCast("MD_TopicCategoryCode"),
            EncodedString(*parentNode.doc, (const char *)identificationInfo.topicCategory));
    }

    //extent (Optional)
    if ( identificationInfo.westBoundingLongitude != f64(INIT_VALUE) && 
         identificationInfo.eastBoundingLongitude != f64(INIT_VALUE) &&  
         identificationInfo.southBoundingLatitude != f64(INIT_VALUE) &&  
         identificationInfo.northBoundingLatitude != f64(INIT_VALUE) )
    {
        //Add the extent.
        xmlNode *pExtentNode = xmlNewChild(pBagIdentInfoNode, pGmdNamespace, XMLCast("extent"), NULL);

        //Add the EX_Extent
        xmlNode *pExtentNode2 = xmlNewChild(pExtentNode, pGmdNamespace, XMLCast("EX_Extent"), NULL);

        //Add the geographicElement
        xmlNode *pGeoNode = xmlNewChild(pExtentNode2, pGmdNamespace, XMLCast("geographicElement"), NULL);

        //Add the EX_GeographicBoundingBox
        xmlNode *pGeoNode2 = xmlNewChild(pGeoNode, pGmdNamespace, XMLCast("EX_GeographicBoundingBox"), NULL);

        //Add the westBoundLongitude
        xmlNode *pCoordNode = xmlNewChild(pGeoNode2, pGmdNamespace, XMLCast("westBoundLongitude"), NULL);
        addDecimalNode(*pCoordNode, identificationInfo.westBoundingLongitude);

        //Add the eastBoundLongitude
        pCoordNode = xmlNewChild(pGeoNode2, pGmdNamespace, XMLCast("eastBoundLongitude"), NULL);
        addDecimalNode(*pCoordNode, identificationInfo.eastBoundingLongitude);

        //Add the southBoundLatitude
        pCoordNode = xmlNewChild(pGeoNode2, pGmdNamespace, XMLCast("southBoundLatitude"), NULL);
        addDecimalNode(*pCoordNode, identificationInfo.southBoundingLatitude);

        //Add the northBoundLatitude
        pCoordNode = xmlNewChild(pGeoNode2, pGmdNamespace, XMLCast("northBoundLatitude"), NULL);
        addDecimalNode(*pCoordNode, identificationInfo.northBoundingLatitude);
    }

    //verticalUncertaintyType
    {
        //Add the verticalUncertaintyType.
        xmlNode *pVertUncertNode = xmlNewChild(pBagIdentInfoNode, pBagNamespace, XMLCast("verticalUncertaintyType"), NULL);

        //Set the value.
        addCodeListNode(*pVertUncertNode, "bag", "BAG_VertUncertCode",
            "http://www.opennavsurf.org/schema/bag/bagCodelists.xml", (const char*)identificationInfo.verticalUncertaintyType);
    }

    //depthCorrectionType (Optional)
    if (identificationInfo.depthCorrectionType != NULL)
    {
        //Add the depthCorrectionType.
        xmlNode *pDepthCorrNode = xmlNewChild(pBagIdentInfoNode, pBagNamespace, XMLCast("depthCorrectionType"), NULL);

        //Set the value.
        addCodeListNode(*pDepthCorrNode, "bag", "BAG_DepthCorrectCode",
            "http://www.opennavsurf.org/schema/bag/bagCodelists.xml", (const char*)identificationInfo.depthCorrectionType);
    }

    //elevationSolutionGroupType (Optional)
    if (identificationInfo.elevationSolutionGroupType != NULL)
    {
        //Add the depthCorrectionType.
        xmlNode *pElevNode = xmlNewChild(pBagIdentInfoNode, pBagNamespace, XMLCast("elevationSolutionGroupType"), NULL);

        //Set the value.
        addCodeListNode(*pElevNode, "bag", "BAG_OptGroupCode",
            "http://www.opennavsurf.org/schema/bag/bagCodelists.xml", (const char*)identificationInfo.elevationSolutionGroupType);
    }

    //nodeGroupType (Optional)
    if (identificationInfo.nodeGroupType != NULL)
    {
        //Add the depthCorrectionType.
        xmlNode *pNodeGroupNode = xmlNewChild(pBagIdentInfoNode, pBagNamespace, XMLCast("nodeGroupType"), NULL);

        //Set the value.
        addCodeListNode(*pNodeGroupNode, "bag", "BAG_OptGroupCode",
            "http://www.opennavsurf.org/schema/bag/bagCodelists.xml", (const char*)identificationInfo.nodeGroupType);
    }
		
    return True;
}

//************************************************************************
//! Add the BAG_SECURITY_CONSTRAINTS information to the parent XML node.
/*!
\param parentNode
    \li The parent XML node to be modified.
\param securityConstraints
    \li The security constraint information to be added to \e parentNode
\return 
    \li True if the structure is added, False if an error occurs.
*/
//************************************************************************
Bool addSecurityConstraints(xmlNode &parentNode, const BAG_SECURITY_CONSTRAINTS &securityConstraints)
{
    /* If either the classification or the distribution statement is not supplied, the node should not be created.*/
    if (securityConstraints.classification == NULL ||
        securityConstraints.userNote == NULL)
    {
        fprintf(stderr, "ERROR: creating security constraints. Classification and Distribution statement must be supplied!.\n");
        return False;
    }

    const xmlNsPtr pGmdNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("gmd"));
                        
    //Create the metadataConstraints node.
    xmlNode *pConstraintNode = xmlNewChild(&parentNode, pGmdNamespace, XMLCast("metadataConstraints"), NULL);

    //Create the MD_SecurityConstraints node.
    xmlNode *pConstraintNode2 = xmlNewChild(pConstraintNode, pGmdNamespace, XMLCast("MD_SecurityConstraints"), NULL);
    
    //classification
    {
        //Create the title node.
        xmlNode *pNode = xmlNewChild(pConstraintNode2, pGmdNamespace, XMLCast("classification"), NULL);
        addCodeListNode(*pNode, "gmd", "MD_ClassificationCode",
            "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml", (char*)securityConstraints.classification);
    }

    //userNote
    {
        //Create the title node.
        xmlNode *pNode = xmlNewChild(pConstraintNode2, pGmdNamespace, XMLCast("userNote"), NULL);

        //Set the title value.
        addCharacterNode(*pNode, (char*)securityConstraints.userNote);
    }

    return True;
}

//************************************************************************
//! Add the BAG_LEGAL_CONSTRAINTS information to the parent XML node.
/*!
\param parentNode
    \li The parent XML node to be modified.
\param legalConstraints
    \li The legal constraint information to be added to \e parentNode
\return 
    \li True if the structure is added, False if an error occurs.
*/
//************************************************************************
Bool addLegalConstraints(xmlNode &parentNode, const BAG_LEGAL_CONSTRAINTS &legalConstraints)
{
    const xmlNsPtr pGmdNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("gmd"));
                        
    //Create the metadataConstraints node.
    xmlNode *pConstraintNode = xmlNewChild(&parentNode, pGmdNamespace, XMLCast("metadataConstraints"), NULL);

    //Create the MD_LegalConstraints node.
    xmlNode *pConstraintNode2 = xmlNewChild(pConstraintNode, pGmdNamespace, XMLCast("MD_LegalConstraints"), NULL);

    //useConstraints
    {
        //Create the useConstraints node.
        xmlNode *pNode = xmlNewChild(pConstraintNode2, pGmdNamespace, XMLCast("useConstraints"), NULL);
        addCodeListNode(*pNode, "gmd", "MD_RestrictionCode",
            "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml", (char*)legalConstraints.useConstraints);
    }

    //otherConstraints
    {
        //Create the title node.
        xmlNode *pNode = xmlNewChild(pConstraintNode2, pGmdNamespace, XMLCast("otherConstraints"), NULL);

        //Set the title value.
        addCharacterNode(*pNode, (char*)legalConstraints.otherConstraints);
    }

    return True;
}

//************************************************************************
//! Add the BAG_SOURCE information to the parent XML node.
/*!
\param parentNode
    \li The parent XML node to be modified.
\param source
    \li The process source information to be added to \e parentNode
\return 
    \li True if the structure is added, False if an error occurs.
*/
//************************************************************************
Bool addProcessSource(xmlNode &parentNode, const BAG_SOURCE &source)
{
    const xmlNsPtr pGmdNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("gmd"));

    //The description is required.
    if (source.description == NULL)
    {
        fprintf(stderr, "ERROR: source description not supplied.\n");
        return False;
    }

    //Create the source node.
    xmlNode *pSourceNode = xmlNewChild(&parentNode, pGmdNamespace, XMLCast("source"), NULL);

    //Create the LI_Source node.
    xmlNode *pSourceNode2 = xmlNewChild(pSourceNode, pGmdNamespace, XMLCast("LI_Source"), NULL);

    //description
    {
        //Create the description node.
        xmlNode *pNode = xmlNewChild(pSourceNode2, pGmdNamespace, XMLCast("description"), NULL);

        //Set the title value.
        addCharacterNode(*pNode, (char*)source.description);
    }

    //sourceCitation
    {
        //Create the sourceCitation node.
        xmlNode *pNode = xmlNewChild(pSourceNode2, pGmdNamespace, XMLCast("sourceCitation"), NULL);

        const Bool ret = addCitation(*pNode, (char*)source.title, (char*)source.date, (char*)source.dateType,
            source.responsibleParties, source.numberOfResponsibleParties);
        if (!ret)
            return False;
    }

    return True;
}

//************************************************************************
//! Add the BAG_PROCESS_STEP information to the parent XML node.
/*!
\param parentNode
    \li The parent XML node to be modified.
\param processInfo
    \li The process step information to be added to \e parentNode
\return 
    \li True if the structure is added, False if an error occurs.
*/
//************************************************************************
Bool addProcessStep(xmlNode &parentNode, const BAG_PROCESS_STEP &processInfo)
{
    const xmlNsPtr pGmdNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("gmd"));
    const xmlNsPtr pBagNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("bag"));

    xmlNode *pProcessStepNode = xmlNewChild(&parentNode, pGmdNamespace, XMLCast("processStep"), NULL);
    pProcessStepNode = xmlNewChild(pProcessStepNode, pBagNamespace, XMLCast("BAG_ProcessStep"), NULL);

    //description
    {
        //Create the description node.
        xmlNode *pNode = xmlNewChild(pProcessStepNode, pGmdNamespace, XMLCast("description"), NULL);
        addCharacterNode(*pNode, (char*)processInfo.description);
    }

    //dateTime
    {
        //Create the dateTime node.
        xmlNode *pNode = xmlNewChild(pProcessStepNode, pGmdNamespace, XMLCast("dateTime"), NULL);
        addDateTimeNode(*pNode, (char*)processInfo.dateTime);
    }

    //processor
    for (u32 i = 0; i < processInfo.numberOfProcessors; i++)
    {
        //Create the processor node.
        xmlNode *pNode = xmlNewChild(pProcessStepNode, pGmdNamespace, XMLCast("processor"), NULL);
        addResponsibleParty(*pNode, processInfo.processors[i]);
    }

    //source
    for (u32 i = 0; i < processInfo.numberOfSources; i++)
    {
        const Bool ret = addProcessSource(*pProcessStepNode, processInfo.lineageSources[i]);
        if (!ret)
            return False;
    }

    //trackingId
    {
        //Create the trackingId node.
        xmlNode *pNode = xmlNewChild(pProcessStepNode, pBagNamespace, XMLCast("trackingId"), NULL);
        addCharacterNode(*pNode, (char*)processInfo.trackingId);
    }

    return True;
}

//************************************************************************
//! Add the BAG_DATA_QUALITY information to the parent XML node.
/*!
\param parentNode
    \li The parent XML node to be modified.
\param dataQuality
    \li The data quality information to be added to \e parentNode
\return 
    \li True if the structure is added, False if an error occurs.
*/
//************************************************************************
Bool addDataQuality(xmlNode &parentNode, const BAG_DATA_QUALITY &dataQuality)
{
    const xmlNsPtr pGmdNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("gmd"));

    //Create the dataQualityInfo node.
    xmlNode *pQualityNode = xmlNewChild(&parentNode, pGmdNamespace, XMLCast("dataQualityInfo"), NULL);

    //Create the DQ_DataQuality node.
    xmlNode *pQualityNode2 = xmlNewChild(pQualityNode, pGmdNamespace, XMLCast("DQ_DataQuality"), NULL);

    //scope
    {
        xmlNode *pNode = xmlNewChild(pQualityNode2, pGmdNamespace, XMLCast("scope"), NULL);
        pNode = xmlNewChild(pNode, pGmdNamespace, XMLCast("DQ_Scope"), NULL);
        pNode = xmlNewChild(pNode, pGmdNamespace, XMLCast("level"), NULL);
        addCodeListNode(*pNode, "gmd", "MD_ScopeCode",
            "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml", (char*)dataQuality.scope);
    }

    //lineage
    {
        xmlNode *pLineageNode = xmlNewChild(pQualityNode2, pGmdNamespace, XMLCast("lineage"), NULL);
        pLineageNode = xmlNewChild(pLineageNode, pGmdNamespace, XMLCast("LI_Lineage"), NULL);

        //Add each process step.
        for (u32 i = 0; i < dataQuality.numberOfProcessSteps; i++)
        {
            const Bool ret = addProcessStep(*pLineageNode, dataQuality.lineageProcessSteps[i]);
            if (!ret)
                return False;
        }
    }

    return True;
}

//************************************************************************
//! Add the BAG_SPATIAL_REPRESENTATION information to the parent XML node.
/*!
\param parentNode
    \li The parent XML node to be modified.
\param spatialRepresentationInfo
    \li The spatial representation information to be added to \e parentNode
\return 
    \li True if the structure is added, False if an error occurs.
*/
//************************************************************************
Bool addSpatialRepresentation(xmlNode &parentNode, const BAG_SPATIAL_REPRESENTATION &spatialRepresentationInfo)
{
    /* Check for required elements. If do not exist, return NULL*/

    /* Must have specified cellGeometry, transformationParameterAvailability,and checkPointAvailability */ 
    /* If any of the four corner points equal the INIT_VALUE, this indicates the points have not been populated by the user. */

    if (spatialRepresentationInfo.cellGeometry == NULL)
    {
        fprintf(stderr, "ERROR: spatialRepresentationInfo.cellGeometry, transformationParameterAvailability,checkPointAvailability must be supplied\n");
        return False;
    }

    if (spatialRepresentationInfo.llCornerX == (f64)INIT_VALUE ||  
        spatialRepresentationInfo.llCornerY == (f64)INIT_VALUE ||   
        spatialRepresentationInfo.urCornerX == (f64)INIT_VALUE ||   
        spatialRepresentationInfo.urCornerY == (f64)(INIT_VALUE))
    {

        fprintf(stderr, "ERROR: All four spatialRepresentationInfo corner points must be supplied. \n"); 
        return False;

    }

    const xmlNsPtr pGmdNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("gmd"));
    const xmlNsPtr pGmlNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("gml"));

    //Create the spatialRepresentationInfo node.
    xmlNode *pSpatialRepNode = xmlNewChild(&parentNode, pGmdNamespace, XMLCast("spatialRepresentationInfo"), NULL);

    //Create the MD_Georectified node.
    xmlNode *pGeoNode = xmlNewChild(pSpatialRepNode, pGmdNamespace, XMLCast("MD_Georectified"), NULL);

    //numberOfDimensions
    {
        xmlNode *pNode = xmlNewChild(pGeoNode, pGmdNamespace, XMLCast("numberOfDimensions"), NULL);
        addIntegerNode(*pNode, 2);
    }

    //axisDimensionProperties
    addDimension(*pGeoNode, "row", spatialRepresentationInfo.numberOfRows, spatialRepresentationInfo.rowResolution,
        (const char *)spatialRepresentationInfo.resolutionUnit);
    addDimension(*pGeoNode, "column", spatialRepresentationInfo.numberOfColumns, spatialRepresentationInfo.columnResolution,
        (const char *)spatialRepresentationInfo.resolutionUnit);

    //cellGeometry
    {
        xmlNode *pNode = xmlNewChild(pGeoNode, pGmdNamespace, XMLCast("cellGeometry"), NULL);
        addCodeListNode(*pNode, "gmd", "MD_CellGeometryCode",
            "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml", (char*)spatialRepresentationInfo.cellGeometry);
    }

    //transformationParameterAvailability
    {
        xmlNode *pNode = xmlNewChild(pGeoNode, pGmdNamespace, XMLCast("transformationParameterAvailability"), NULL);
        addBooleanNode(*pNode, spatialRepresentationInfo.transformationParameterAvailability);
    }

    //checkPointAvailability
    {
        xmlNode *pNode = xmlNewChild(pGeoNode, pGmdNamespace, XMLCast("checkPointAvailability"), NULL);
        addBooleanNode(*pNode, spatialRepresentationInfo.checkPointAvailability);
    }

    //cornerPoints
    {
        xmlNode *pCornerNode = xmlNewChild(pGeoNode, pGmdNamespace, XMLCast("cornerPoints"), NULL);

        xmlNode *pPointNode = xmlNewChild(pCornerNode, pGmlNamespace, XMLCast("Point"), NULL);
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
        xmlNode *pNode = xmlNewChild(pGeoNode, pGmdNamespace, XMLCast("pointInPixel"), NULL);
        xmlNewChild(pNode, pGmdNamespace, XMLCast("MD_PixelOrientationCode"), XMLCast("center"));
    }

    return True;
}

//************************************************************************
//! Add the BAG_REFERENCE_SYSTEM information to the parent XML node.
/*!
\param parentNode
    \li The parent XML node to be modified.
\param system
    \li The reference system information to be added to \e parentNode
\return 
    \li True if the structure is added, False if an error occurs.
*/
//************************************************************************
Bool addReferenceSystem(xmlNode &parentNode, const BAG_REFERENCE_SYSTEM &system)
{
    const xmlNsPtr pGmdNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("gmd"));
    const xmlNsPtr pGcoNamespace = xmlSearchNs(parentNode.doc, &parentNode, XMLCast("gco"));

    xmlNode *pNode = xmlNewChild(&parentNode, pGmdNamespace, XMLCast("referenceSystemInfo"), NULL);
    pNode = xmlNewChild(pNode, pGmdNamespace, XMLCast("MD_ReferenceSystem"), NULL);
    pNode = xmlNewChild(pNode, pGmdNamespace, XMLCast("referenceSystemIdentifier"), NULL);
    pNode = xmlNewChild(pNode, pGmdNamespace, XMLCast("RS_Identifier"), NULL);

    xmlNode *pCodeNode = xmlNewChild(pNode, pGmdNamespace, XMLCast("code"), NULL);
    addCharacterNode(*pCodeNode, (const char *)system.definition);

    xmlNode *pCodeNameNode = xmlNewChild(pNode, pGmdNamespace, XMLCast("codeSpace"), NULL);
    addCharacterNode(*pCodeNameNode, (const char *)system.type);

    return True;
}

//************************************************************************
//! Export the BAG_METADATA structure to a character buffer.
/*!
\param metadata
    \li The metadata information to be exported.
\param xmlString
    \li Updated to contain the XML buffer generated from \e metadata.  User
        is responsible for freeing the memory.
\return 
    \li The number of bytes allocated to \e xmlString.  0 is returned if 
        an error occurs creating the buffer.
*/
//************************************************************************
u32 bagExportMetadataToXmlBuffer(BAG_METADATA *metadata, u8** xmlString)
{
    if (metadata == NULL)
        return 0;

    //Create a new xml document.
    xmlDoc *pDocument = createNewDocument();

    //Get the document's root node.
    xmlNode *pRoot = xmlDocGetRootElement(pDocument);

    //Add the fileIdentifier
    {
        const xmlNsPtr pGmdNamespace = xmlSearchNs(pDocument, pRoot, XMLCast("gmd"));

        //Create the fileIdentifier node.
        xmlNode *pNode = xmlNewChild(pRoot, pGmdNamespace, XMLCast("fileIdentifier"), NULL);
        addCharacterNode(*pNode, (const char *)metadata->fileIdentifier);
    }

    //Add the language
    {
        const xmlNsPtr pGmdNamespace = xmlSearchNs(pDocument, pRoot, XMLCast("gmd"));

        //Create the language node.
        xmlNode *pNode = xmlNewChild(pRoot, pGmdNamespace, XMLCast("language"), NULL);
        addCodeListNode(*pNode, "gmd", "LanguageCode",
            "http://www.loc.gov/standards/iso639-2/", (const char *)metadata->language, False);
    }

    //Add the characterSet
    {
        const xmlNsPtr pGmdNamespace = xmlSearchNs(pDocument, pRoot, XMLCast("gmd"));

        //Create the characterSet node.
        xmlNode *pNode = xmlNewChild(pRoot, pGmdNamespace, XMLCast("characterSet"), NULL);
        addCodeListNode(*pNode, "gmd", "MD_CharacterSetCode",
            "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml", (char*)metadata->characterSet);
    }

    //Add the hierarchyLevel.
    {
        const xmlNsPtr pGmdNamespace = xmlSearchNs(pDocument, pRoot, XMLCast("gmd"));

        //Create the hierarchyLevel node.
        xmlNode *pNode = xmlNewChild(pRoot, pGmdNamespace, XMLCast("hierarchyLevel"), NULL);
        addCodeListNode(*pNode, "gmd", "MD_ScopeCode",
            "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml", (char*)metadata->hierarchyLevel);
    }

    //Add the contact
    {
         const xmlNsPtr pGmdNamespace = xmlSearchNs(pDocument, pRoot, XMLCast("gmd"));

        //Create the characterSet node.
        xmlNode *pNode = xmlNewChild(pRoot, pGmdNamespace, XMLCast("contact"), NULL);
        if (!addResponsibleParty(*pNode, *metadata->contact))
        {
            //Error

            xmlFreeDoc(pDocument);
            return 0;
        }
    }

    //Add the dateStamp
    {
        const xmlNsPtr pGmdNamespace = xmlSearchNs(pDocument, pRoot, XMLCast("gmd"));

        //Create the hierarchyLevel node.
        xmlNode *pNode = xmlNewChild(pRoot, pGmdNamespace, XMLCast("dateStamp"), NULL);
        addDateNode(*pNode, (const char *)metadata->dateStamp);
    }

    //Add the metadataStandardName
    {
        const xmlNsPtr pGmdNamespace = xmlSearchNs(pDocument, pRoot, XMLCast("gmd"));

        //Create the metadataStandardName node.
        xmlNode *pNode = xmlNewChild(pRoot, pGmdNamespace, XMLCast("metadataStandardName"), NULL);
        addCharacterNode(*pNode, (const char *)metadata->metadataStandardName);
    }

    //Add the metadataStandardVersion
    {
        const xmlNsPtr pGmdNamespace = xmlSearchNs(pDocument, pRoot, XMLCast("gmd"));

        //Create the metadataStandardVersion node.
        xmlNode *pNode = xmlNewChild(pRoot, pGmdNamespace, XMLCast("metadataStandardVersion"), NULL);
        addCharacterNode(*pNode, (const char *)metadata->metadataStandardVersion);
    }

    //Add the spatialRepresentationInfo
    Bool ret = addSpatialRepresentation(*pRoot, *metadata->spatialRepresentationInfo);
    if (!ret)
    {
        //Error

        xmlFreeDoc(pDocument);
        return 0;
    }

    //Add the horizontal referenceSystemInfo
    ret = addReferenceSystem(*pRoot, *metadata->horizontalReferenceSystem);
    if (!ret)
    {
        //Error

        xmlFreeDoc(pDocument);
        return 0;
    }

    //Add the vertical referenceSystemInfo
    ret = addReferenceSystem(*pRoot, *metadata->verticalReferenceSystem);
    if (!ret)
    {
        //Error

        xmlFreeDoc(pDocument);
        return 0;
    }

    //Add the data identification information.
    ret = addDataIdentification(*pRoot, *metadata->identificationInfo);
    if (!ret)
    {
        //Error

        xmlFreeDoc(pDocument);
        return 0;
    }

    //Add the data quality information.
    ret = addDataQuality(*pRoot, *metadata->dataQualityInfo);
    if (!ret)
    {
        //Error

        xmlFreeDoc(pDocument);
        return 0;
    }

    //Add the legal constraint information.
    ret = addLegalConstraints(*pRoot, *metadata->legalConstraints);
    if (!ret)
    {
        //Error

        xmlFreeDoc(pDocument);
        return 0;
    }

    //Add the security constraint information.
    ret = addSecurityConstraints(*pRoot, *metadata->securityConstraints);
    if (!ret)
    {
        //Error

        xmlFreeDoc(pDocument);
        return 0;
    }
            
    //Export to the buffer.
    int bufferSize = 0;
    xmlChar *pBuffer = NULL;
    xmlDocDumpMemoryEnc(pDocument, &pBuffer, &bufferSize, "UTF-8");
    xmlFreeDoc(pDocument);

    //If nothing was exported, then just return.
    if (bufferSize == 0)
        return 0;

    //Copy the buffer to our output string and add a null terminator.
    *xmlString = (u8*)realloc(*xmlString, bufferSize + 1);
    memcpy(*xmlString, pBuffer, bufferSize);
    (*xmlString)[bufferSize] = 0;
    
    xmlFree(pBuffer);
    return bufferSize;
}
