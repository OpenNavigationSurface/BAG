
#include "bag_xml_meta.hpp"

#include "ons_xml_error_handler.h"
#include "ons_xml.h"

#include "bag.h"

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



bool XMLinitialized=false;
DOMImplementation* impl;





// ---------------------------------------------------------------------------
//  This is a simple class that lets us do easy (though not terribly efficient)
//  trancoding of char* data to XMLCh data.
// ---------------------------------------------------------------------------
class XStr
{
public :

    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    XStr(const char* const toTranscode)
    {
        // Call the private transcoding method
        fUnicodeForm = XMLString::transcode(toTranscode);
    }

    ~XStr()
    {
        XMLString::release(&fUnicodeForm);
    }


    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    const XMLCh* unicodeForm() const
    {
        return fUnicodeForm;
    }

private :
    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fUnicodeForm
    //      This is the Unicode XMLCh format of the string.
    // -----------------------------------------------------------------------
    XMLCh*   fUnicodeForm;
};

#define X(str) XStr(str).unicodeForm()

/*********************************************************************************
 * Purpose: Implementation of methods declared in bag_xml_meta.h 
 *
 *
 * History: 6/2008 - E. Warner - Initially created.
 *          7/2008 - E. Warner - Added "set" methods.  
 *                               Modified to accomodate new data structures.
 *
 *          12/2008 - E. Warner - Altered get & set methods for cornerPoints to 
 *                                Allocate 88 bytes instead of 72.  this was for UTM coordinates.  
 *          1/2009 - E. Warner -  Altered method that writes XML string to set UTF-8 encoding. 
 *                                Added InitResponsibleParty method.  
 *                                General comment cleanup.
 *
 *          5/2009 - E. Warner - Added methods to free memory that was allocated 
 *                               during reading of XML into data structures.
 *	
 *			6/2009 - M. Russell - Modified the length of the depthCorrectionType in the IDENTIFICATION_INFO
 *							structure from 22 to 32 to adjust for the types agreed on from the
 *							ONSWG 2009-06-12 teleconference
 ********************************************************************************/



/* Methods specific to the the BAG metadata schema. */
DOMElement * createCI_CitationNode(char title[100], char date[20], char dateType[20], RESPONSIBLE_PARTY responsibleParties[MAX_CI_RESPONSIBLE_PARTIES], DOMDocument* doc, signed short * status);
DOMElement * createCI_ResponsiblePartyNode(RESPONSIBLE_PARTY responsiblePartyStruct, DOMDocument * doc);
DOMElement * createMD_LegalConstraintsNode(LEGAL_CONSTRAINTS  legalConstraints, DOMDocument * doc);
DOMElement * createMD_SecurityConstraintsNode(SECURITY_CONSTRAINTS  securityConstraints, DOMDocument * doc);
DOMElement * createDQ_DataQualityNode(DATA_QUALITY_INFO  dataQuality, DOMDocument * doc);
DOMElement * createMD_GeorectifiedNode(SPATIAL_REPRESENTATION_INFO  spatialRepresentationInfo, DOMDocument * doc);
DOMElement * createMD_CRSVertNode(bagLegacyReferenceSystem SystemInfo, DOMDocument * doc);
DOMElement * createMD_CRSHorizNode(bagLegacyReferenceSystem SystemInfo, DOMDocument * doc);
DOMElement * createBAG_DataIdentificationNode(IDENTIFICATION_INFO identificationInfo, DOMDocument * doc);
DOMElement * createSourceNode(SOURCE_INFO  source, DOMDocument * doc);
DOMElement * createHierarchyLevelNode(DATA_QUALITY_INFO  dataQuality, DOMDocument * doc);
DOMElement * createCharacterSetNode(IDENTIFICATION_INFO identificationInfo, DOMDocument * doc);




/* General DOM tree methods. */
DOMElement * createElement(char * elementName, char * elementValue, DOMDocument * doc);
DOMNode * FindNode(DOMDocument * doc, char *tag, char *attName, char *attValue);
DOMNode * FindNode(DOMElement * searchElement, char *tag, char *attName, char *attValue);
u16 GetNodeValue(DOMNode * someNode, char * val, signed short valLength);
bagError SetAttribute(DOMElement * someElement, char *attName, char *attValue);


/* General Xerces methods. */
u16 IsXMLInitialized(void);
u16 TerminateXML();



//*****************************************************************************
/*!
\ Function: CreateXmlMetadataString

\brief  Converts BAG metadata structures to a well-formed, valid BAG XML metadata string.

\param  identificationInfo 
	\li		populated structure whose contents will be used to populate "IDENTIFICATION_INFO" node in XML metdata string.
\param  legalConstraints 
	\li		populated structure whose contents will be used to populate "LEGAL_CONSTRAINTS" node in XML metdata string.
\param  securityConstraints 
	\li		populated structure whose contents will be used to populate "SECURITY_CONSTRAINTS" node in XML metdata string.
\param  dataQuality 
	\li		populated structure whose contents will be used to populate "DATA_QUALITY_INFO" node in XML metdata string.
\param  spatialRepresentationInfo 
	\li		populated structure whose contents will be used to populate "SPATIAL_REPRESENTATION_INFO" node in XML metdata string.
\param  SystemInfo 
	\li		populated structure whose contents will be used to populate "bagLegacyReferenceSystem" node in XML metdata string.
\param  contact 
	\li		populated structure whose contents will be used to populate "RESPONSIBLE_PARTY" node in XML metdata string.
\param  creationDate 
	\li		populated string  whose contents will be used to populate "dateStamp" in XML metdata string.
\param  file_identifier 
	\li		populated string  whose contents will be used to populate "fileIdentifier" in XML metdata string.
\param  xmlString 
	\li		address of the XML string to populate.

\return
	\li u16 = Length of XML string created (if successful).  non-zero if the function fails.

*/
//*****************************************************************************
bagError  CreateXmlMetadataString( IDENTIFICATION_INFO identificationInfo, 
	LEGAL_CONSTRAINTS legalConstraints, SECURITY_CONSTRAINTS securityConstraints,  
	DATA_QUALITY_INFO dataQuality, SPATIAL_REPRESENTATION_INFO spatialRepresentationInfo, 
	bagLegacyReferenceSystem SystemInfo,  RESPONSIBLE_PARTY contact, 
	char * creationDate, char * file_identifier, unsigned char ** xmlString)
{

    signed short xmlStringLength = 0; 
    DOMDocument* doc = NULL;
  
    /* Initialize XERCES. */
    if (!XMLinitialized)
    {
        try
        {
            XMLPlatformUtils::Initialize();
        }
        catch(const XMLException& toCatch)
        {
            char *pMsg = XMLString::transcode(toCatch.getMessage());
  
            XMLString::release(&pMsg);
            return -1;
        }

        /* Get an Implementation with Factory Setttings for both the Load and Save Methods .*/
        static const XMLCh gLS[] = {chLatin_L, chLatin_S, chNull };
        impl = DOMImplementationRegistry::getDOMImplementation(gLS);
        if (impl)
        {
            XMLinitialized=true;
        }

    }


    if (impl != NULL)
    {

        try
        {

            doc = impl->createDocument(
                                       X("http://www.isotc211.org/2005/gmi"),                     // root element namespace URI.
                                       X("gmi:MI_Metadata"),         // root element name
                                                0);                    // document type object (DTD).
     

            //1.5.2009 - E. Warner - Set encoding to UTF-8
            XMLCh *temp = XMLString::transcode("UTF-8");
            doc->setEncoding(temp);
            XMLString::release(&temp);

            DOMElement* rootElem = doc->getDocumentElement();
    
            SetAttribute(rootElem,"xmlns:gmd", "http://www.isotc211.org/2005/gmd");
			SetAttribute(rootElem,"xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
			SetAttribute(rootElem, "xmlns:gml", "http://www.opengis.net/gml/3.2");
			SetAttribute(rootElem,"xmlns:gco", "http://www.isotc211.org/2005/gco");
            SetAttribute(rootElem, "xmlns:xlink", "http://www.w3.org/1999/xlink");
			SetAttribute(rootElem, "xmlns:bag", "http://www.opennavsurf.org/schema/bag");
			SetAttribute(rootElem, "xsi:schemaLocation", "http://www.opennavsurf.org/schema/bag http://www.opennavsurf.org/schema/bag/bag.xsd");
          
            DOMElement * bagDataIdentificationElem;
            bagDataIdentificationElem = createBAG_DataIdentificationNode(identificationInfo, doc);

            if(bagDataIdentificationElem != NULL)
            {
                DOMElement*  identificationInfoElem = doc->createElement(X("gmd:identificationInfo"));
                identificationInfoElem->appendChild(bagDataIdentificationElem);
                rootElem->appendChild(identificationInfoElem);
            }
            /* Otherwise don't make the document. Required by ISO 19139. */
            else
            {
                throw (signed short) -11;
            }

			 /* If security constraints are not specified, do not create document. */
            /* Required navigation specific.                                      */
            DOMElement * securityConstraintsElem;
            securityConstraintsElem = createMD_SecurityConstraintsNode(securityConstraints, doc);
           
			if(securityConstraintsElem != NULL)
            {
				DOMElement*  securityInfoElem = doc->createElement(X("gmd:metadataConstraints"));
                securityInfoElem->appendChild(securityConstraintsElem);
                rootElem->appendChild(securityInfoElem);
			}
			else
            {
                throw (signed short) -10;
			}
	   
          
           /* Append a metadataContraints node to the metadata node. Required navigation specific for BAG.*/
	        DOMElement * legalConstraintsElem;
            legalConstraintsElem = createMD_LegalConstraintsNode(legalConstraints, doc);

			if(legalConstraintsElem != NULL)
            {

                DOMElement*  metadataConstraintsElem = doc->createElement(X("gmd:metadataConstraints"));
                metadataConstraintsElem->appendChild(legalConstraintsElem);
                rootElem->appendChild(metadataConstraintsElem);
            }
            else
            {

                throw (signed short) -12;
            }
       
	 
            /* Append a dataQualityInfo node to the metadata node.Required navigation specific for BAG. */
            DOMElement * dqDataQualityElem;
            dqDataQualityElem = createDQ_DataQualityNode(dataQuality, doc);
          

            if(dqDataQualityElem != NULL)
            {

                DOMElement*  dataQualityElem = doc->createElement(X("gmd:dataQualityInfo"));
                dataQualityElem->appendChild(dqDataQualityElem);
                rootElem->appendChild(dataQualityElem);
            }
            else
            {
                throw (signed short) -13;
            }

			 /* Append a dataQualityInfo node to the metadata node.Required navigation specific for BAG. */
            DOMElement * HierarchyLevelElem;
            HierarchyLevelElem = createHierarchyLevelNode(dataQuality, doc);
          

            if(HierarchyLevelElem != NULL)
            {

                DOMElement* HierLevelElem = doc->createElement(X("gmd:hierarchyLevel"));
                HierLevelElem->appendChild(HierarchyLevelElem);
                rootElem->appendChild(HierLevelElem);
            }
            else
            {
                throw (signed short) -13;
            }

			 /* Append a dataQualityInfo node to the metadata node.Required navigation specific for BAG. */
            DOMElement * CharacterSetElem;
            CharacterSetElem = createCharacterSetNode(identificationInfo, doc);
          

            if(CharacterSetElem != NULL)
            {

                DOMElement* CharSetElem = doc->createElement(X("gmd:characterSet"));
                CharSetElem->appendChild(CharacterSetElem);
                rootElem->appendChild(CharSetElem);
            }
            else
            {
                throw (signed short) -13;
            }

            /* Append a spatialRepresentationInfo node to the metadata node. Required navigation specific for BAG */
            DOMElement * georectifiedElem;
            georectifiedElem = createMD_GeorectifiedNode(spatialRepresentationInfo, doc);
          

            if(georectifiedElem != NULL)
            {
                DOMElement*  spatialRepresentationElem = doc->createElement(X("gmd:spatialRepresentationInfo"));
                spatialRepresentationElem->appendChild(georectifiedElem);
                rootElem->appendChild(spatialRepresentationElem);
            }
            else
            {
                
                throw (signed short) -14;
            }

	

            /* Append a  referenceSystemInfo node to the metadata node. Required navigation specific for BAG.*/
            DOMElement * crsElem;
            crsElem = createMD_CRSVertNode(SystemInfo, doc);
           

            if(crsElem != NULL)
            {
                DOMElement*  referenceSystemElem = doc->createElement(X("gmd:referenceSystemInfo"));
                referenceSystemElem->appendChild(crsElem);
                rootElem->appendChild(referenceSystemElem);
            }
            else
            {

                throw (signed short) -15;

            }

	 

            /* Append another referenceSystemInfo node to the metadata node for horizontal data */
            DOMElement * crsElem2;
            crsElem2 = createMD_CRSHorizNode(SystemInfo, doc);



            if(crsElem2 != NULL)
            {
           
                DOMElement*  referenceSystemElem2 = doc->createElement(X("gmd:referenceSystemInfo"));
                referenceSystemElem2->appendChild(crsElem2);
                rootElem->appendChild(referenceSystemElem2);

            }
            else
            {

                throw (signed short) -15;
            }

			
  
			DOMElement*  languageElem = doc->createElement(X("gmd:language"));
            rootElem->appendChild(languageElem);

            /* Append the creation date node to the metadata node. Required per ISO 19139.*/
			DOMElement*  languageValueElem = createElement("gmd:LanguageCode", "eng", doc);
			languageElem->appendChild(languageValueElem);

			SetAttribute(languageValueElem,"codeList", "http://www.loc.gov/standards/iso639-2/");
			SetAttribute(languageValueElem,"codeListValue", (char*)identificationInfo.language);


			/* Append the contact node to the metadata node. Required per ISO 19139. */
			DOMElement * responsiblePartyElem;
            responsiblePartyElem = createCI_ResponsiblePartyNode(contact, doc);
        
            if(responsiblePartyElem != NULL)
            {
                DOMElement*  contactElem = doc->createElement(X("gmd:contact"));
                contactElem->appendChild(responsiblePartyElem);
                rootElem->appendChild(contactElem);
            }
            else
            {
                throw (signed short) -17;
            }
	 

			DOMElement*  dateStampElem = doc->createElement(X("gmd:dateStamp"));
            rootElem->appendChild(dateStampElem);

  
            /* Append the creation date node to the metadata node. Required per ISO 19139.*/
			DOMElement*  dateStampValueElem = createElement("gco:Date", creationDate, doc);
			dateStampElem->appendChild(dateStampValueElem);

			
			DOMElement*  FileIdentifierElem = doc->createElement(X("gmd:fileIdentifier"));
            rootElem->appendChild(FileIdentifierElem);

  
            /* Append the creation date node to the metadata node. Required per ISO 19139.*/
			DOMElement*  FileIdentStrElem = createElement("gco:CharacterString", file_identifier, doc);
			FileIdentifierElem->appendChild(FileIdentStrElem);
			           
  
			DOMElement*  metadataStandardNameElem = doc->createElement(X("gmd:metadataStandardName"));
            rootElem->appendChild(metadataStandardNameElem);

            /* Append the metadataStandardName node to the metadata node. Required navigation specific.*/
            DOMElement*  metadataStandardNameValueElem = createElement("gco:CharacterString", "ISO 19115", doc);
            metadataStandardNameElem->appendChild(metadataStandardNameValueElem);
           

			DOMElement*  metadataStandardVersionElem = doc->createElement(X("gmd:metadataStandardVersion"));
            rootElem->appendChild(metadataStandardVersionElem);

            /* Append the metadataStandardName node to the metadata node. Required navigation specific */
            DOMElement*  metadataStandardVersionValueElem = createElement("gco:CharacterString", "2003/Cor.1:2006", doc);
		    metadataStandardVersionElem->appendChild(metadataStandardVersionValueElem); 
          

            DOMWriter *theSerializer = ((DOMImplementationLS*)impl)->createDOMWriter();
		  
 
            /*1.5.2009 - E. Warner - Set encoding to UTF-8. */
            /* Write the document to a string.  Done this way because must use writeNode in order for setEncoding method to work.*/
            MemBufFormatTarget *myFormTarget;
            myFormTarget = new MemBufFormatTarget();
            bool wroteBuffer = theSerializer->writeNode(myFormTarget,*doc);

            if(wroteBuffer)
            {

                xmlStringLength=myFormTarget->getLen();

                *xmlString = (unsigned char *)calloc(xmlStringLength, sizeof(unsigned char));

                *xmlString = (unsigned char *)myFormTarget->getRawBuffer();
            }
            else
            {
                throw (char *)"Could not write buffer. ";

            }

          
        }
        catch (const DOMException& e)
        {
            fprintf(stderr, "ERROR: DOMException code is: %d\n", e.code);
            xmlStringLength = -2;
        }
        catch (const OutOfMemoryException&)
        {

            fprintf(stderr, "ERROR: OutOfMemoryException\n");
            xmlStringLength = -3;
        }

        catch (int intException)
        {          
            
            fprintf(stderr,"ERROR: An exception occurred creating the XML metadata document.  Exception message is: %d\n", intException );
            xmlStringLength = intException;

        }
        catch (...)
        {

            fprintf(stderr, "ERROR: An exception occurred creating the XML metadata document.\n");
            xmlStringLength = -20;
        }

    } 
    else
    {

        fprintf(stderr, "ERROR: Requested implementation is not supported\n");
        xmlStringLength = -4;
    }



    doc->release();
    TerminateXML();

    return xmlStringLength;

       
}






//*****************************************************************************
/*!
\ Function: IsXMLInitialized

\brief  This function is used to determine if the Xerces XML engine is intialized or not. 


\return
	\li Status indicating the status of the XML initialization.
              1 - Yes, the Xerces XML engine is initialized. 
              0 - No, the Xerces XML engine is not initialized.

*/
//*****************************************************************************
u16 IsXMLInitialized(void)
{

    if (XMLinitialized)
    {
        return 1;
    }
    else
    {
        return 0; 
    }
}






//*****************************************************************************
/*!
\ Function: TerminateXML

\brief  This function shutsdown the Xerces XML engine releasing all resources back to the system.


\return
	\li 1 if the function is successful, non-zero if Xerces was not initialized, so no need to stop.

*/
//*****************************************************************************
u16 TerminateXML()
{

    // Stop XERCES
    if (!XMLinitialized)
    {
        return -1;
    }
    else
    {
        XMLPlatformUtils::Terminate();
        XMLinitialized = false;
        return 1;
    }
}





//*****************************************************************************
/*!
\ Function: createCI_ResponsiblePartyNode

\brief  This function creates and populates a DOMElement named gmd:CI_ResponsibleParty.

\param  responsiblePartyStruct 
	\li		Node from which to get value.
\param  doc 
	\li		String in which to load  node value

\return
	\li Returns a pointer to the DOMElement created. Returns a NULL  pointer if the "role" field is empty or if any errors were encountered.

*/
//*****************************************************************************
DOMElement * createCI_ResponsiblePartyNode(RESPONSIBLE_PARTY responsiblePartyStruct, DOMDocument * doc)
{


    DOMElement * responsiblePartyElem = NULL;

 
    try
    {

        /* Criteria for this node is that "role must be supplied and at least one of the following fileds must be supplied. */
        signed short nodeCriteria = strcmp((char*)responsiblePartyStruct.individualName, "") + 
                                strcmp((char*)responsiblePartyStruct.organisationName, "") + 
		                        strcmp((char*)responsiblePartyStruct.positionName, "");

            
        /* One of the above fields must be supplied per the schema documentation. */
        if(nodeCriteria < 1)
        {
            return responsiblePartyElem;
        }


        /* If "role" is not populated, don't create the element.  "role" is a required element of the schema. */
        if(strcmp((char*)responsiblePartyStruct.role, "") == 0)
        {
            fprintf(stderr, "ERROR: The \"role\" is required in order to create the CI_ResponsibleParty node.\n");
            return responsiblePartyElem;
        }

        responsiblePartyElem = doc->createElement(X("gmd:CI_ResponsibleParty"));


                
        /* If an individual name has been supplied, Create the individual node and populate it. */
        if(strcmp((char*)responsiblePartyStruct.individualName, "") != 0)
        {
			DOMElement * IndivNameElem = doc->createElement(X("gmd:individualName"));
			responsiblePartyElem->appendChild(IndivNameElem);

            DOMElement *  individualElem = createElement("gco:CharacterString",(char*)responsiblePartyStruct.individualName, doc );
            IndivNameElem->appendChild(individualElem);
        }


        /* If an organisation name has been supplied, Create the organisation node and populate it. */
        if(strcmp((char*)responsiblePartyStruct.organisationName, "") != 0)
        {
			DOMElement * OrganisationNameElem = doc->createElement(X("gmd:organisationName"));

            DOMElement *  organizationElem = createElement("gco:CharacterString",(char*)responsiblePartyStruct.organisationName, doc );
            OrganisationNameElem->appendChild(organizationElem);
        }



        /* If a postiion name has been supplied, Create the position node and populate it. */
        if(strcmp((char*)responsiblePartyStruct.positionName, "") != 0)
        {
			DOMElement * PositionNameElem = doc->createElement(X("gmd:positionName"));

            DOMElement *  positionElem = createElement("gco:CharacterString",(char*)responsiblePartyStruct.positionName, doc );
            PositionNameElem->appendChild(positionElem);
        }


        /* Create the role node and populate it. Return NULL if unable to create the required "role" node.*/
      	DOMElement* roleNameElem = doc->createElement(X("gmd:role"));
		responsiblePartyElem->appendChild(roleNameElem);

        DOMElement *  roleElem = createElement("gmd:CI_RoleCode",(char*)responsiblePartyStruct.role, doc );
        roleNameElem->appendChild(roleElem);

       
	
		SetAttribute(roleElem,"codeList", "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#CI_RoleCode");
		SetAttribute(roleElem,"codeListValue", (char*)responsiblePartyStruct.role);
			
    }
    catch(...)
    { 

        fprintf(stderr, "ERROR: An exception happened during the creation of the CI_ResponsibleParty node.\n");
        responsiblePartyElem->release();
        responsiblePartyElem = NULL;
    }


    return responsiblePartyElem;


}





//*****************************************************************************
/*!
\ Function: createMD_LegalConstraintsNode

\brief  This function creates and populates a DOMElement named gmd:MD_LegalConstraints.

\param  legalConstraints 
	\li		Node from which to get value.
\param  doc 
	\li		String in which to load  node value

\return
	\li Returns a pointer to the DOMElement created. Returns NULL is the MD_LegalConstraints node could not be created.

*/
//*****************************************************************************
DOMElement * createMD_LegalConstraintsNode(LEGAL_CONSTRAINTS  legalConstraints, DOMDocument * doc)
{


    DOMElement * legalConstraintsElem = NULL;


    try{

        legalConstraintsElem = doc->createElement(X("gmd:MD_LegalConstraints"));
         
        /* Create the useContraints node and populate it. */
        if(strcmp((char*)legalConstraints.useConstraints, "") != 0)
        {
			DOMElement * useConstraintsElem = doc->createElement(X("gmd:useConstraints"));
			legalConstraintsElem->appendChild(useConstraintsElem);

            DOMElement *  useConstraintsValueElem = createElement("gmd:MD_RestrictionCode",(char*)legalConstraints.useConstraints, doc);
            useConstraintsElem->appendChild(useConstraintsValueElem);

			SetAttribute(useConstraintsValueElem,"codeList", "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#MD_RestrictionCode");
			SetAttribute(useConstraintsValueElem,"codeListValue", (char*)legalConstraints.useConstraints);
			
        }


        /* Create the otherConstraints node and populate it. */
        if(strcmp((char*)legalConstraints.otherConstraints, "") != 0)
        {

			DOMElement * otherConstraintsElem = doc->createElement(X("gmd:otherConstraints"));
			legalConstraintsElem->appendChild(otherConstraintsElem);

            DOMElement *  otherConstraintsValueElem = createElement("gco:CharacterString",(char*)legalConstraints.otherConstraints, doc);
            otherConstraintsElem->appendChild(otherConstraintsValueElem);
        }

    }
    catch(...)
    { 
        fprintf(stderr, "ERROR: Error creating legal constraints.\n");
        legalConstraintsElem->release();
        legalConstraintsElem = NULL;
    }


    return legalConstraintsElem;


}



//*****************************************************************************
/*!
\ Function: createMD_SecurityConstraintsNode

\brief  This function gets the value of a given DOMNode.

\param  securityConstraints 
	\li		populated structure from which to make DOMElement.
\param  doc 
	\li		DOMdocument that should be used to create the DOMElement.

\return
	\li Returns a pointer to the DOMElement created. Returns NULL if an exception was encountered.
           Returns NULL if classification or userNote not supplied.  These are required navigation specific.
           userNote holds the distribution Statement for the data.

*/
//*****************************************************************************
DOMElement * createMD_SecurityConstraintsNode(SECURITY_CONSTRAINTS  securityConstraints, DOMDocument * doc)
{


    DOMElement * securityConstraintsElem = NULL;


    /* If either the classification or the distribution statement is not supplied, the node should not be created.*/
    if(strcmp((char*)securityConstraints.classification, "\0") == 0 || strcmp((char*)securityConstraints.userNote, "\0") == 0)
    {

        fprintf(stderr, "ERROR: creating security constraints. Classification and Distribution statement must be supplied!.\n");
        return securityConstraintsElem;
    }

    try{

        securityConstraintsElem = doc->createElement(X("gmd:MD_SecurityConstraints"));

		DOMElement *classElem = doc->createElement(X("gmd:classification"));
		securityConstraintsElem->appendChild(classElem);
     
        /* Create the classification node an populate it. */
        DOMElement *  classificationValueElem = createElement("gmd:MD_ClassificationCode",(char*)securityConstraints.classification, doc );


		DOMElement *userNoteElem = doc->createElement(X("gmd:userNote"));
		securityConstraintsElem->appendChild(userNoteElem);

        /* Create the otherConstraints node an populate it. */
        DOMElement *  userNoteValueElem = createElement("gco:CharacterString",(char*)securityConstraints.userNote, doc);
 
        /* If either the classification or the userNote(distribution statement) could not be created, return NULL. */
        if(classificationValueElem != NULL && userNoteValueElem != NULL)
        {
            classElem->appendChild(classificationValueElem);
            userNoteElem->appendChild(userNoteValueElem);

			
			SetAttribute(classificationValueElem,"codeList", "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#MD_ClassificationCode");
			SetAttribute(classificationValueElem,"codeListValue", (char*)securityConstraints.classification);
		
        }
        else
        {        
            fprintf(stderr, "ERROR: creating security constraints. Could not create classification or userNote nodes..\n");
            securityConstraintsElem = NULL;
        }
            

    }
    catch(...)
    { 
        fprintf(stderr, "ERROR: Error creating security constraints.\n");
        securityConstraintsElem->release();
        securityConstraintsElem = NULL;
    }


    return securityConstraintsElem;


}


//*****************************************************************************
/*!
\ Function: createDQ_DataQualityNode

\brief  This function creates and populates a DOMElement named smXML:DQ_DataQuality.

\param  dataQuality 
	\li		populated structure from which to make DOMElement.
\param  doc 
	\li		DOMdocument that should be used to create the DOMElement.

\return
	\li Returns a pointer to the DOMElement created. Returns NULL if an exceptions were encountered.

*/
//*****************************************************************************
DOMElement * createDQ_DataQualityNode(DATA_QUALITY_INFO  dataQuality, DOMDocument * doc)
{


    DOMElement * dataQualityElem = NULL;
    int i = 0;


    try
    {

		dataQualityElem = doc->createElement(X("gmd:DQ_DataQuality"));

                
        /* Create the scope node. */
        DOMElement *  scopeElem = doc->createElement(X("gmd:scope"));
        dataQualityElem->appendChild(scopeElem);


        DOMElement *  dqScopeElem = doc->createElement(X("gmd:DQ_Scope"));
        scopeElem->appendChild(dqScopeElem);

		DOMElement *  levelElem = doc->createElement(X("gmd:level"));
        dqScopeElem->appendChild(levelElem);



        DOMElement *  scopeCodeElem = createElement("gmd:MD_ScopeCode", (char*)dataQuality.scope , doc);
		levelElem->appendChild(scopeCodeElem);

       
	
		SetAttribute(scopeCodeElem,"codeList", "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#MD_ScopeCode");
		SetAttribute(scopeCodeElem,"codeListValue", (char*)dataQuality.scope);


        /* Create the lineage node. */
        DOMElement *  lineageElem = doc->createElement(X("gmd:lineage"));
		dataQualityElem->appendChild(lineageElem);



        /* Create the smXML:LI_Lineage. */
        DOMElement *  liLineageElem = doc->createElement(X("gmd:LI_Lineage"));
		lineageElem->appendChild(liLineageElem);

		DOMElement *  processStepElem = doc->createElement(X("gmd:processStep"));
		liLineageElem->appendChild(processStepElem);
		
		DOMElement *  BAG_processStepElem = doc->createElement(X("bag:BAG_ProcessStep"));
		processStepElem->appendChild(BAG_processStepElem);

       
		 u16 numberOfSuccessfulSources = 0;
     
        /* For each source given, add a "source" node. */
        for( i = 0; i < dataQuality.numberOfSources; i++)
        {
 
            DOMElement * sourceElem = createSourceNode(dataQuality.lineageSources[i], doc);

            if(sourceElem != NULL)
            {
                BAG_processStepElem->appendChild(sourceElem);
                numberOfSuccessfulSources++;
            }
            /* Otherwise and error was encountered making the source, return NULL. */
            else
            {

                fprintf(stderr, "ERROR: Could not create source node.  Check source number %d.\n", i);
                dataQualityElem->release();
                return NULL;
            }
      
        }
	  

        signed short numberOfSuccessfulProcessSteps = 0;
     
        /* For each proceessStep given, add a "processStep" node. */
        for( i = 0; i < dataQuality.numberOfProcessSteps; i++ )
        {

     		DOMElement *DescriptElem = doc->createElement(X("gmd:description"));
    
			/* Create the description node. Required per ISO schema. */
			DOMElement *  descriptionElem = createElement("gco:CharacterString", (char*)dataQuality.lineageProcessSteps[i].description,  doc );
			DescriptElem->appendChild(descriptionElem);

			DOMElement *  DateTimeNameElem = doc->createElement(X("gmd:dateTime"));
			BAG_processStepElem->appendChild(DateTimeNameElem);

			/* Create the dateTime node. */
			DOMElement *  dateTimeElem = createElement("gco:DateTime", (char*)dataQuality.lineageProcessSteps[i].dateTime, doc );
			DateTimeNameElem->appendChild(dateTimeElem);

			 /* For each proceesing party listed, add it. */
			/* role is a required element, so use it as the criteria. */
			int j = 0;
			while( j < MAX_CI_RESPONSIBLE_PARTIES  && strcmp((char*)dataQuality.lineageProcessSteps[i].processors[j].role, "") != 0 )
			{

				DOMElement * processorElem = doc->createElement(X("gmd:processor"));
				DOMElement * responsiblePartyElem = createCI_ResponsiblePartyNode(dataQuality.lineageProcessSteps[i].processors[j], doc);

				if(responsiblePartyElem != NULL)
				{
					processorElem->appendChild(responsiblePartyElem);
					BAG_processStepElem->appendChild(processorElem);
				}
				/* If an error was encountered, stop making the node. */
				else
				{
					fprintf(stderr, "Error creating processStep node. Problem creating processor %d. \n", i );
					fprintf(stderr, "Ensure that at least one of the following exist: individualName, organisationName, positionName. \n");
					processStepElem->release();
					return NULL;
				}

				j++;
			}


			DOMElement *  TrackingIDNameElem = doc->createElement(X("bag:trackingId"));
			BAG_processStepElem->appendChild(TrackingIDNameElem);

			/* Create the trackingId node.  Required per ISO schema. */
			DOMElement *  trackingIdElem = createElement("gco:CharacterString", (char*)dataQuality.lineageProcessSteps[i].trackingId, doc );
			TrackingIDNameElem->appendChild(trackingIdElem);


            if(processStepElem != NULL)
            {
                BAG_processStepElem->appendChild(DescriptElem);
				
                numberOfSuccessfulProcessSteps++;
            }
            
	   
        }

		

	  
        /* Requires at least 1 source or process step. */
        if( numberOfSuccessfulSources + numberOfSuccessfulProcessSteps < 1)
        {

            dataQualityElem->release();
            dataQualityElem = NULL;
            return dataQualityElem;
        }

    }
    catch(...)
    { 

        fprintf(stderr, "ERROR: Error creating data quality.\n");
        dataQualityElem->release();
        dataQualityElem = NULL;

    }

 
    return dataQualityElem;


}






//*****************************************************************************
/*!
\ Function: createSourceNode

\brief  This function creates and populates a DOMElement named "source".

\param  source 
	\li		populated structure from which to make DOMElement.
\param  doc 
	\li		DOMdocument that should be used to create the DOMElement.

\return
	\li Returns a pointer to the DOMElement created. Returns NULL if an exceptions were encountered.

*/
//*****************************************************************************
DOMElement * createSourceNode(SOURCE_INFO  source, DOMDocument * doc)
{


    DOMElement * sourceElem = NULL;
   
   
    try
    {

        if(strcmp((char*)source.description, "") == 0)
        {

            fprintf(stderr, "ERROR: source description not supplied.\n");
            return sourceElem;
        }

  
        sourceElem = doc->createElement(X("gmd:source"));
                
        /* Create the smXML:LI_Source node. */
        DOMElement *  liSourceElem = doc->createElement(X("gmd:LI_Source"));
        sourceElem->appendChild(liSourceElem);

		DOMElement * DescriptNameElem = doc->createElement(X("gmd:description"));
        liSourceElem->appendChild(DescriptNameElem);

        DOMElement *  descriptionElem = createElement("gco:CharacterString", (char*)source.description, doc );
        DescriptNameElem->appendChild(descriptionElem);



        /* Attempt to create CI_Citation node. */
        signed short status = 0;
        DOMElement *  ciCitationElem =  createCI_CitationNode((char*)source.title, 
                                                              (char*)source.date, 
                                                              (char*)source.dateType,
                                                              source.responsibleParties, doc, &status);
  
        /* If element was successfully created, add it and all of its optional parent nodes. */
        if(ciCitationElem != NULL)
        {

            /* Create the optional sourceCitation node. */
            DOMElement *  sourceCitationElem = doc->createElement(X("gmd:sourceCitation"));
            sourceCitationElem->appendChild(ciCitationElem);

            liSourceElem->appendChild(sourceCitationElem);

        }
        /* If an error was encountered creating the CI_Citation node, stop making the current element. */
        else if(ciCitationElem == NULL && status == -1)
        {

            /* Release any resources allocated to the  bagIdentificationElem. */
            fprintf(stderr, "RELEASING SOURCE\n");
            sourceElem->release();
            sourceElem = NULL;
            return sourceElem;
        }
        /* Otherwise, the citationElem is NULL because it was determined by the supplied XML that the element is not desired. */
        else if(ciCitationElem == NULL && status == 1)
        {
        }


    }
    catch(...)
    { 
        fprintf(stderr, "ERROR: Error creating source node.\n");
        sourceElem->release();
        sourceElem = NULL;
    }


    return sourceElem;


}




//*****************************************************************************
/*!
\ Function: createMD_GeorectifiedNode

\brief  This function creates and populates a DOMElement named smXML:MD_Georectified.

\param  spatialRepresentationInfo 
	\li		populated structure from which to make DOMElement.
\param  doc 
	\li		DOMdocument that should be used to create the DOMElement.

\return
	\li Returns a pointer to the DOMElement created.  Returns NULL if an exceptions were encountered.

*/
//*****************************************************************************
DOMElement * createMD_GeorectifiedNode(SPATIAL_REPRESENTATION_INFO  spatialRepresentationInfo, DOMDocument * doc)
{


    DOMElement * geoRectifiedElem = NULL;


  
    try
    {

        /* Check for required elements. If do not exist, return NULL*/

        /* Must have at least two dimensions. */
        /* Must have specified cellGeometry, transformationParameterAvailability,and checkPointAvailability */ 
        /* If any of the four corner points equal the INIT_VALUE, this indicates the points have not been populated by the user. */

        if( spatialRepresentationInfo.numberOfDimensions <= 1)
        {
            fprintf(stderr, "ERROR: spatialRepresentationInfo.numberOfDimensions must be >= 2\n");
            return geoRectifiedElem;
        }

 

        if(strcmp( (char*)spatialRepresentationInfo.cellGeometry, "") == 0 ||
		   strcmp( (char*)spatialRepresentationInfo.transformationParameterAvailability, "") == 0 ||
		   strcmp( (char*)spatialRepresentationInfo.checkPointAvailability, "") == 0 )
        {

            fprintf(stderr, "ERROR: spatialRepresentationInfo.cellGeometry, transformationParameterAvailability,checkPointAvailability must be supplied\n");
            return geoRectifiedElem;
        }



        if(spatialRepresentationInfo.llCornerX == (f64)INIT_VALUE ||  
           spatialRepresentationInfo.llCornerY == (f64)INIT_VALUE ||   
           spatialRepresentationInfo.urCornerX == (f64)INIT_VALUE ||   
           spatialRepresentationInfo.urCornerY == (f64)(INIT_VALUE))

        {
 
            fprintf(stderr, "ERROR: All four spatialRepresentationInfo corner points must be supplied. \n"); 
            return geoRectifiedElem;

        }

        /* For the number of dimensions speicifed, ensure that there are dimensionNames, sizes and resolutions. */
        for(signed short i=0; i< spatialRepresentationInfo.numberOfDimensions; i++)
        {

            if((strcmp((char*)spatialRepresentationInfo.dimensionName[i], "") == 0 )|| 
                spatialRepresentationInfo.dimensionSize == 0 || 
                spatialRepresentationInfo.resolutionValue == 0 )
            {
			 
                fprintf(stderr, "ERROR: numberOfDimensions does not match the the number of dimensionNames, sizes and resolutions. \n"); 
                return geoRectifiedElem;
            }

        }

		geoRectifiedElem = doc->createElement(X("gmd:MD_Georectified"));

	 
        /* Create the numberOfDimensions node an populate it. */
        char dimAsString[2]; /* assume no more than 1 digits in integer specifiying number of dimensions. */
        sprintf(dimAsString, "%d", spatialRepresentationInfo.numberOfDimensions);

		DOMElement *  numberOfDimensionsElem = doc->createElement(X("gmd:numberOfDimensions"));
            geoRectifiedElem->appendChild(numberOfDimensionsElem);

        
		DOMElement *  datatypeElem = createElement("gco:Integer", dimAsString, doc);
        numberOfDimensionsElem->appendChild(datatypeElem);
   

     
         
        /* For each dimension, create an axisDimensionProperties node. */
        for (int d = 0; d < spatialRepresentationInfo.numberOfDimensions; d++ )
        {


            /* Create the axisDimensionProperties node and populate it. */
            DOMElement *  axisDimensionPropertiesElem = doc->createElement(X("gmd:axisDimensionProperties"));
            geoRectifiedElem->appendChild(axisDimensionPropertiesElem);


            DOMElement *  mdDimensionElem = doc->createElement(X("gmd:MD_Dimension"));
            axisDimensionPropertiesElem->appendChild(mdDimensionElem);

			/* Create dimensionName node. */
            DOMElement *  dimensionNameElem = doc->createElement(X("gmd:dimensionName"));
            mdDimensionElem->appendChild(dimensionNameElem);
			
			DOMElement *  typeCodeElem = createElement("gmd:MD_DimensionNameTypeCode", (char*)spatialRepresentationInfo.dimensionName[d], doc);
			 dimensionNameElem->appendChild(typeCodeElem);

			SetAttribute(typeCodeElem,"codeList", "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#MD_DimensionNameTypeCode");
			SetAttribute(typeCodeElem,"codeListValue", (char*)spatialRepresentationInfo.dimensionName[d]);
			
   
			/* Create dimensionName node. */
            DOMElement *  dimensionSizeElem = doc->createElement(X("gmd:dimensionSize"));
            mdDimensionElem->appendChild(dimensionSizeElem);


            /* Create dimensionSize node. */
            char numberAsString[21]; /* assume no more than 5 digits in integer specifiying dimension size. */
            sprintf(numberAsString, "%d", spatialRepresentationInfo.dimensionSize[d]);
            DOMElement *  dimensionIntElem = createElement("gco:Integer", numberAsString, doc);
            dimensionSizeElem->appendChild(dimensionIntElem);


            /* Create reolution node. */
            DOMElement *  resolutionElem = doc->createElement(X("gmd:resolution"));
            mdDimensionElem->appendChild(resolutionElem);


           
			/* Create spatialValue node. */
            strcpy(numberAsString, "");
            sprintf(numberAsString, "%.15lf", spatialRepresentationInfo.resolutionValue[d]);
           
			DOMElement *  measureElem = createElement("gco:Measure", numberAsString, doc);
			 resolutionElem->appendChild(measureElem);

			SetAttribute(measureElem,"uom", "metres");
			
       
        }
           

    
        /* Create the cell geometry node and its text value. Use "point" as default */
        if(strcmp((char*)spatialRepresentationInfo.cellGeometry, "") == 0)
        {

            strcpy((char*)spatialRepresentationInfo.cellGeometry,"point" );
        }

		 /* Create reolution node. */
         DOMElement *  cellGeometryElem = doc->createElement(X("gmd:cellGeometry"));
         geoRectifiedElem->appendChild(cellGeometryElem);

        DOMElement *  cellGeometryCodeElem = createElement("gmd:MD_CellGeometryCode",(char*)spatialRepresentationInfo.cellGeometry, doc);
        cellGeometryElem->appendChild(cellGeometryCodeElem);
     
		SetAttribute(cellGeometryCodeElem,"codeList", "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#MD_CellGeometryCode");
		SetAttribute(cellGeometryCodeElem,"codeListValue",(char*)spatialRepresentationInfo.cellGeometry	);

        /* Create the transformationParameterAvailability node and its text value.  Use "false" as default. */
        if(strcmp((char*)spatialRepresentationInfo.transformationParameterAvailability, "") == 0)
        {
            strcpy((char*)spatialRepresentationInfo.transformationParameterAvailability,"false");
        }

		DOMElement *  transParamAvailElem = doc->createElement(X("gmd:transformationParameterAvailability"));
         geoRectifiedElem->appendChild(transParamAvailElem);

        DOMElement *  transParamAvailValElem = createElement("gco:Boolean",(char*)spatialRepresentationInfo.transformationParameterAvailability, doc);
        transParamAvailElem->appendChild(transParamAvailValElem);

		
        /* Create the checkPointAvailability node and its text value.  Use "0" as default */
        if(strcmp((char*)spatialRepresentationInfo.checkPointAvailability, "") == 0)
        {
            strcpy((char*)spatialRepresentationInfo.checkPointAvailability,"0" );
        }

		DOMElement *  checkPtAvailElem = doc->createElement(X("gmd:checkPointAvailability"));
         geoRectifiedElem->appendChild(checkPtAvailElem);

        DOMElement *  checkPtAvailValElem = createElement("gco:Boolean",(char*)spatialRepresentationInfo.checkPointAvailability, doc);
        checkPtAvailElem->appendChild(checkPtAvailValElem);

     
       
        /* Create the corner points node. */
        DOMElement *  cornerPointsElem = doc->createElement(X("gmd:cornerPoints"));
        geoRectifiedElem->appendChild(cornerPointsElem);


        DOMElement *  gmlPointElem = doc->createElement(X("gml:Point"));
        cornerPointsElem->appendChild(gmlPointElem);


		char idStr[10];
		strcpy(idStr, "id1");
		SetAttribute(gmlPointElem,"gml:id",(char*)idStr	);

      
			

        /* Create the gml:coordinates node. */
        DOMElement *  gmlCoordinatesElem = doc->createElement(X("gml:coordinates"));
        gmlCoordinatesElem->setAttribute(XMLString::transcode("decimal"),XMLString::transcode("."));
        gmlCoordinatesElem->setAttribute(XMLString::transcode("cs"),XMLString::transcode("," ));
        gmlCoordinatesElem->setAttribute(XMLString::transcode("ts"),XMLString::transcode(" " ));
        gmlPointElem->appendChild(gmlCoordinatesElem);

 
        /* Create string that contains corner points. */
        char * pointsString = (char *) calloc(88, sizeof(char));
        sprintf(pointsString, "%.12lf,%.12lf %.12lf,%.12lf", spatialRepresentationInfo.llCornerX, spatialRepresentationInfo.llCornerY, spatialRepresentationInfo.urCornerX, spatialRepresentationInfo.urCornerY);
   

        DOMText *   gmlCoordinatesText = doc->createTextNode(X(pointsString));
        gmlCoordinatesElem->appendChild(gmlCoordinatesText);



        free(pointsString);

		char * orientString = (char *) calloc(32, sizeof(char));

		strcpy(orientString, "center");

		DOMElement *  pixelOrientElem = doc->createElement(X("gmd:pointInPixel"));
		geoRectifiedElem->appendChild(pixelOrientElem);

		DOMElement *  MD_PixelOrientationCodeElem = createElement("gmd:MD_PixelOrientationCode",(char*)orientString, doc);
		pixelOrientElem->appendChild(MD_PixelOrientationCodeElem);
     
		
       

    }
    catch(...)
    { 
        fprintf(stderr, "ERROR: Error creating georectified element.\n");
        geoRectifiedElem->release();
        geoRectifiedElem = NULL;
    }


    return geoRectifiedElem;


}


//*****************************************************************************
/*!
\ Function: createMD_CRSVertNode

\brief  This function gets the value of a given DOMNode.

\param  system 
	\li		populated structure from which to create DOMElement.
\param  doc 
	\li		DOMdocument that should be used to create the DOMElement.

\return
	\li Returns a pointer to the DOMElement created. Returns NULL if an exceptions were encountered.

*/
//*****************************************************************************
DOMElement * createMD_CRSVertNode(bagLegacyReferenceSystem system, DOMDocument * doc)
{


    DOMElement * crsElem = NULL;
    char doubleAsString[30]; /* Used to represent text values of various lats and  lons. */
    char intAsString[12];    /* Used to represent text values of various integers. */



    try{


        /* Check for required fields. If not present, return NULL. */
        if(strcmp((char*)system.geoParameters.ellipsoid, "")  == 0  )

        {

            fprintf(stderr, "ERROR: Unable to create MD_CRS node.  Missing one of the required fields.\n");
            fprintf(stderr, "Check projection, ellipsoid and horizontalDatum codes. All three are required as referenceSystemIdentifier is not yet supported.\n");
            return crsElem;

        }

	

		 char *hBuffer;
		 u32 hBuffer_size,vBuffer_size;
		 char *vBuffer;

		 hBuffer= (char*)malloc(sizeof(char) * REF_SYS_MAX_LENGTH);
		 vBuffer= (char*)malloc(sizeof(char) * REF_SYS_MAX_LENGTH);

		 hBuffer_size = REF_SYS_MAX_LENGTH;
		 vBuffer_size = REF_SYS_MAX_LENGTH;


		bagLegacyToWkt( system, NULL, 0, vBuffer, vBuffer_size);



        /* Create the vertical MD_CRS node. */
        crsElem = doc->createElement(X("gmd:MD_ReferenceSystem"));
		
                /* Create the RS_Identifier node. */
        DOMElement *  rsElem1 = doc->createElement(X("gmd:referenceSystemIdentifier"));
        crsElem->appendChild(rsElem1);
		
        /* Create the RS_Identifier node. */
        DOMElement *  rsElem2 = doc->createElement(X("gmd:RS_Identifier"));
        rsElem1->appendChild(rsElem2);

		/* Create the RS_Identifier node. */
        DOMElement *  rsElem3 = doc->createElement(X("gmd:code"));
        rsElem2->appendChild(rsElem3);

        /* Create the ellipsoid node and its text value. */
        DOMElement *  codeElem2 = createElement("gco:CharacterString", vBuffer, doc);
        rsElem3->appendChild(codeElem2);
     
        /* Create the RS_Identifier node. */
        DOMElement *  rsElem4 = doc->createElement(X("gmd:codeSpace"));
        rsElem2->appendChild(rsElem4);

  		char wktString[30];
        strcpy(wktString,"WKT");

        /* Create the datum node. */
        DOMElement *  codeElem3 = createElement("gco:CharacterString",wktString, doc);
        rsElem4->appendChild(codeElem3);




    }
    catch(...)
    { 
        fprintf(stderr, "ERROR: Error creating MD_CRS.\n");
        crsElem->release();
        crsElem = NULL;
    }

    return crsElem;


}



//*****************************************************************************
/*!
\ Function: createMD_CRSHorizNode

\brief  This function creates and populates a DOMElement named gmd:MD_CRS.

\param  system 
	\li		populated structure from which to create DOMElement.
\param  doc 
	\li		DOMdocument that should be used to create the DOMElement.

\return
	\li Returns a pointer to the DOMElement created. Returns NULL if an exceptions were encountered.

*/
//*****************************************************************************
DOMElement * createMD_CRSHorizNode(bagLegacyReferenceSystem system, DOMDocument * doc)
{


    DOMElement * crsElem = NULL;
    char doubleAsString[30]; /* Used to represent text values of various lats and  lons. */
    char intAsString[12];    /* Used to represent text values of various integers. */



    try{


        /* Check for required fields. If not present, return NULL. */
        if(strcmp((char*)system.geoParameters.ellipsoid, "")  == 0  )

        {

            fprintf(stderr, "ERROR: Unable to create MD_CRS node.  Missing one of the required fields.\n");
            fprintf(stderr, "Check projection, ellipsoid and horizontalDatum codes. All three are required as referenceSystemIdentifier is not yet supported.\n");
            return crsElem;

        }

	

		 char *hBuffer;
		 u32 hBuffer_size,vBuffer_size;
		 char *vBuffer;

		 hBuffer= (char*)malloc(sizeof(char) * REF_SYS_MAX_LENGTH);
		 vBuffer= (char*)malloc(sizeof(char) * REF_SYS_MAX_LENGTH);

		 hBuffer_size = REF_SYS_MAX_LENGTH;
		 vBuffer_size = REF_SYS_MAX_LENGTH;

		bagLegacyToWkt( system, hBuffer, hBuffer_size, NULL, 0);



        /* Create the horizontal MD_CRS node. */
        crsElem = doc->createElement(X("gmd:MD_ReferenceSystem"));
		
                /* Create the RS_Identifier node. */
        DOMElement *  rsElem1 = doc->createElement(X("gmd:referenceSystemIdentifier"));
        crsElem->appendChild(rsElem1);
		
        /* Create the RS_Identifier node. */
        DOMElement *  rsElem2 = doc->createElement(X("gmd:RS_Identifier"));
        rsElem1->appendChild(rsElem2);

		/* Create the RS_Identifier node. */
        DOMElement *  rsElem3 = doc->createElement(X("gmd:code"));
        rsElem2->appendChild(rsElem3);

        /* Create the ellipsoid node and its text value. */
        DOMElement *  codeElem2 = createElement("gco:CharacterString", hBuffer, doc);
        rsElem3->appendChild(codeElem2);
     
        /* Create the RS_Identifier node. */
        DOMElement *  rsElem4 = doc->createElement(X("gmd:codeSpace"));
        rsElem2->appendChild(rsElem4);

  		char wktString[30];
        strcpy(wktString,"WKT");

        /* Create the datum node. */
        DOMElement *  codeElem3 = createElement("gco:CharacterString",wktString, doc);
        rsElem4->appendChild(codeElem3);




    }
    catch(...)
    { 
        fprintf(stderr, "ERROR: Error creating MD_CRS.\n");
        crsElem->release();
        crsElem = NULL;
    }

    return crsElem;


}




//*****************************************************************************
/*!
\ Function: createBAG_DataIdentificationNode

\brief  This function creates and populates a DOMElement named BAG_DataIdentification.

\param  identificationInfo 
	\li		populated structure from which to create DOMElement.
\param  doc 
	\li		DOMdocument that should be used to create the DOMElement.

\return
	\li Returns a pointer to the DOMElement created.  Returns NULL if an exceptions were encountered.

*/
//*****************************************************************************
DOMElement * createBAG_DataIdentificationNode(IDENTIFICATION_INFO identificationInfo, DOMDocument * doc)
{


    DOMElement * bagIdentificationElem = NULL;
    char doubleAsString[30];

    /* Check for the required fields. If they are not present, return NULL. */
    if(strcmp((char*)identificationInfo.abstract, "") == 0 ||
       strcmp((char*)identificationInfo.language, "") == 0 || 
       strcmp((char*)identificationInfo.verticalUncertaintyType, "") == 0 )
    {
	    fprintf(stderr, "ERROR: can not create BAG identificationInfo.  Missing one or more required fields... abstract, language or verticalUncertaintyType. \n");
	    return bagIdentificationElem;
    }

 
    try{


        /* Create the BAG_DataIdentification node. */
        bagIdentificationElem = doc->createElement(X("bag:BAG_DataIdentification"));
  
    

        /* Create the citation node. */
        DOMElement *  citationElem = doc->createElement(X("gmd:citation"));
        bagIdentificationElem->appendChild(citationElem);


        signed short status = 0;
        DOMElement *  ciCitationElem =  createCI_CitationNode((char*)identificationInfo.title, 
                                                              (char*)identificationInfo.date, 
                                                              (char*)identificationInfo.dateType,
                                                              identificationInfo.responsibleParties, doc, &status);
  
        /* If element was successfully created, add it. */
        if(ciCitationElem != NULL)
        {
            citationElem->appendChild(ciCitationElem);
        }
        /* If an error was encountered creating the CI_Citation node, stop making the current element. */
        else if(ciCitationElem == NULL && status == -1)
        {
            /* Release any resources allocated to the  bagIdentificationElem. */
            bagIdentificationElem->release();
            bagIdentificationElem = NULL;
            return bagIdentificationElem;
		}
        /* Otherwise, the citationElem is NULL because it was determined by the supplied XML that the element is not desired. */
        else if(ciCitationElem == NULL && status == 1)
        {
        }

		DOMElement*  abstractElem = doc->createElement(X("gmd:abstract"));
        bagIdentificationElem->appendChild(abstractElem);

        /* Append the creation date node to the metadata node. Required per ISO 19139.*/
		DOMElement*  abstractValueElem = createElement("gco:CharacterString", (char*)identificationInfo.abstract, doc);
		abstractElem->appendChild(abstractValueElem);
 
      

        /* Create the optional status node and its text value. */
        if(strcmp((char*)identificationInfo.status, "") != 0)
        {
			DOMElement*  statusElem = doc->createElement(X("gmd:status"));
			bagIdentificationElem->appendChild(statusElem);

            DOMElement *  statusValueElem = createElement("gmd:MD_ProgressCode",(char*)identificationInfo.status, doc);
            statusElem->appendChild(statusValueElem);

			SetAttribute(statusValueElem,"codeList", "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#MD_ProgressCode");
			SetAttribute(statusValueElem,"codeListValue", (char*)identificationInfo.status);
        }

        /* Create the spatial representaion type node and its text value. */
        if(strcmp((char*)identificationInfo.spatialRepresentationType, "") != 0)
        {
			DOMElement*  spatialRepElem = doc->createElement(X("gmd:spatialRepresentationType"));
			bagIdentificationElem->appendChild(spatialRepElem);

            DOMElement *  spatialRepValueElem = createElement("gmd:MD_SpatialRepresentationTypeCode",(char*)identificationInfo.spatialRepresentationType, doc);
            spatialRepElem->appendChild(spatialRepValueElem);

			SetAttribute(spatialRepValueElem,"codeList", "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#MD_SpatialRepresentationTypeCode");
			SetAttribute(spatialRepValueElem,"codeListValue", (char*)identificationInfo.spatialRepresentationType);


        }
     

        DOMElement*  languageElem = doc->createElement(X("gmd:language"));
        bagIdentificationElem->appendChild(languageElem);

        /* Append the creation date node to the metadata node. Required per ISO 19139.*/
		DOMElement*  languageValueElem = createElement("gmd:LanguageCode", (char*)identificationInfo.language, doc);
		languageElem->appendChild(languageValueElem);

		SetAttribute(languageValueElem,"codeList", "http://www.loc.gov/standards/iso639-2/");
		SetAttribute(languageValueElem,"codeListValue", (char*)identificationInfo.language);


		DOMElement*  characterSetElem = doc->createElement(X("gmd:characterSet"));
        bagIdentificationElem->appendChild(characterSetElem);

        /* Append the creation date node to the metadata node. Required per ISO 19139.*/
		DOMElement*  characterSetValueElem = createElement("gmd:MD_CharacterSetCode", (char*)identificationInfo.character_set, doc);
		characterSetElem->appendChild(characterSetValueElem);

		SetAttribute(characterSetValueElem,"codeList", "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#MD_CharacterSetCode");
		SetAttribute(characterSetValueElem,"codeListValue", (char*)identificationInfo.character_set);

		DOMElement*  topicElem = doc->createElement(X("gmd:topicCategory"));
        bagIdentificationElem->appendChild(topicElem);

        /* Append the creation date node to the metadata node. Required per ISO 19139.*/
		DOMElement*  topicValueElem = createElement("gmd:MD_TopicCategoryCode", (char*)identificationInfo.topicCategory, doc);
		topicElem->appendChild(topicValueElem);


        /* If any lats & lons are set to INIT_VALUE, the bounding box has not been initialized and the optional "extent" node should not be made.*/
        if(identificationInfo.westBoundingLongitude != f64(INIT_VALUE) && 
           identificationInfo.eastBoundingLongitude != f64(INIT_VALUE) &&  
           identificationInfo.southBoundingLatitude != f64(INIT_VALUE) &&  
           identificationInfo.northBoundingLatitude != f64(INIT_VALUE) )
        {

            /* Create the extent node. */
            DOMElement *  extentElem = doc->createElement(X("gmd:extent"));
            bagIdentificationElem->appendChild(extentElem);

            /* Create the EX_Extent node. */
            DOMElement *  exExtentElem = doc->createElement(X("gmd:EX_Extent"));
            extentElem->appendChild(exExtentElem);

            /* Create the geographic node. */
            DOMElement *  geographicElementElem = doc->createElement(X("gmd:geographicElement"));
            exExtentElem->appendChild(geographicElementElem);

            /* Create EX_GeographicBoundingBox node. */
            DOMElement *  exGeographicBoundingBoxElem = doc->createElement(X("gmd:EX_GeographicBoundingBox"));
            geographicElementElem->appendChild(exGeographicBoundingBoxElem);

			DOMElement *  westBoundLongitudeElem = doc->createElement(X("gmd:westBoundLongitude"));
            exGeographicBoundingBoxElem->appendChild(westBoundLongitudeElem);
  
            sprintf(doubleAsString,"%.7f", identificationInfo.westBoundingLongitude);
            DOMElement * westBoundLongitudeValueElem = createElement("gco:Decimal", doubleAsString, doc);
            westBoundLongitudeElem->appendChild(westBoundLongitudeValueElem);

			DOMElement *  eastBoundLongitudeElem = doc->createElement(X("gmd:eastBoundLongitude"));
            exGeographicBoundingBoxElem->appendChild(eastBoundLongitudeElem);
  
            sprintf(doubleAsString,"%.7f", identificationInfo.eastBoundingLongitude);
            DOMElement * eastBoundLongitudeValueElem = createElement("gco:Decimal", doubleAsString, doc);
            eastBoundLongitudeElem->appendChild(eastBoundLongitudeValueElem);

			DOMElement *  southBoundLongitudeElem = doc->createElement(X("gmd:southBoundLatitude"));
            exGeographicBoundingBoxElem->appendChild(southBoundLongitudeElem);
 
            sprintf(doubleAsString,"%.7f", identificationInfo.southBoundingLatitude);
            DOMElement * southBoundLatitudeValueElem = createElement("gco:Decimal",doubleAsString, doc);
            southBoundLongitudeElem->appendChild(southBoundLatitudeValueElem);

			DOMElement *  northBoundLongitudeElem = doc->createElement(X("gmd:northBoundLatitude"));
            exGeographicBoundingBoxElem->appendChild(northBoundLongitudeElem);

            sprintf(doubleAsString,"%.7f", identificationInfo.northBoundingLatitude);
            DOMElement * northBoundLatitudeValueElem = createElement("gco:Decimal",doubleAsString, doc);
            northBoundLongitudeElem->appendChild(northBoundLatitudeValueElem);
        }
		
  
        /* Create the vertical uncertainty node its text value. */

		DOMElement *  verticalUncertaintyTypeElem = doc->createElement(X("bag:verticalUncertaintyType"));
        bagIdentificationElem->appendChild(verticalUncertaintyTypeElem);

           
        DOMElement * verticalUncertaintyTypeValueElem = createElement("bag:BAG_VertUncertCode",(char*)identificationInfo.verticalUncertaintyType, doc);
        verticalUncertaintyTypeElem->appendChild(verticalUncertaintyTypeValueElem);
        
		SetAttribute(verticalUncertaintyTypeValueElem,"codeList", "http://www.opennavsurf.org/schema/bag/bagCodelists.xml#BAG_VertUncertCode");
		SetAttribute(verticalUncertaintyTypeValueElem,"codeListValue", (char*)identificationInfo.verticalUncertaintyType);

		

        /* Create the optional depthCorrectionType node its text value. */
        if(strcmp((char*)identificationInfo.depthCorrectionType, "") != 0)
        {
			DOMElement *  depthCorrectionTypeElem = doc->createElement(X("bag:depthCorrectionType"));
			bagIdentificationElem->appendChild(depthCorrectionTypeElem);

           
			DOMElement * depthCorrectionTypeValueElem = createElement("bag:BAG_DepthCorrectCode",(char*)identificationInfo.depthCorrectionType, doc);
			depthCorrectionTypeElem->appendChild(depthCorrectionTypeValueElem);
        
			SetAttribute(depthCorrectionTypeValueElem,"codeList", "http://www.opennavsurf.org/schema/bag/bagCodelists.xml#BAG_DepthCorrectCode");
			SetAttribute(depthCorrectionTypeValueElem,"codeListValue", (char*)identificationInfo.depthCorrectionType);

		}

		 /* Create the optional depthCorrectionType node its text value. */
        if(strcmp((char*)identificationInfo.elevationSolutionGroupType, "") != 0)
        {
			DOMElement *  ElevSoluGroupTypeElem = doc->createElement(X("bag:elevationSolutionGroupType"));
			bagIdentificationElem->appendChild(ElevSoluGroupTypeElem);

           
			DOMElement * ElevSoluGroupTypeValueElem = createElement("bag:BAG_OptGroupCode",(char*)identificationInfo.elevationSolutionGroupType, doc);
			ElevSoluGroupTypeElem->appendChild(ElevSoluGroupTypeValueElem);
        
			SetAttribute(ElevSoluGroupTypeValueElem,"codeList", "http://www.opennavsurf.org/schema/bag/bagCodelists.xml#BAG_OptGroupCode");
			SetAttribute(ElevSoluGroupTypeValueElem,"codeListValue", (char*)identificationInfo.elevationSolutionGroupType);

		}

		 /* Create the optional depthCorrectionType node its text value. */
        if(strcmp((char*)identificationInfo.nodeGroupType, "") != 0)
        {
			DOMElement *  NodeGroupTypeElem = doc->createElement(X("bag:nodeGroupType"));
			bagIdentificationElem->appendChild(NodeGroupTypeElem);

           
			DOMElement * NodeGroupTypeValueElem = createElement("bag:BAG_OptGroupCode",(char*)identificationInfo.nodeGroupType, doc);
			NodeGroupTypeElem->appendChild(NodeGroupTypeValueElem);
        
			SetAttribute(NodeGroupTypeValueElem,"codeList", "http://www.opennavsurf.org/schema/bag/bagCodelists.xml#BAG_OptGroupCode");
			SetAttribute(NodeGroupTypeValueElem,"codeListValue", (char*)identificationInfo.nodeGroupType);

		}

    }
    catch(...)
    { 
        fprintf(stderr, "ERROR: Error creating smXML:BAG_DataIdentification.\n");

        /* Release any rewources allocated to the  bagIdentificationElem. */
        bagIdentificationElem->release();
        bagIdentificationElem = NULL;
    }
  
    return bagIdentificationElem;

}






//*****************************************************************************
/*!
\ Function: createCI_CitationNode

\brief  This function creates and populates a DOMElement named CI_Citation.

\param  title 
	\li		title of the citation
\param  date 
	\li		date of the citation
\param  dateType 
	\li		type of citation
\param  responsibleParties 
	\li		array of parties responsible for the citation.
\param  doc 
	\li		DOMdocument that should be used to create the DOMElement.
\param  status 
	\li		used to indicate the reason for returnign a NULL element.
            1 = element is NULL and this is due to the user not wanting the element. 
            -1 = element is NULL and this is due to the user not supplying enough information or an error is encountered. 
\return
	\li Returns a pointer to the DOMElement created.  Returns NULL if an exceptions were encountered 
           or XML supplied indicates that caller does not want this optional element to be created.

*/
//*****************************************************************************
DOMElement * createCI_CitationNode(char title[100], char date[20], char dateType[20], RESPONSIBLE_PARTY responsibleParties[MAX_CI_RESPONSIBLE_PARTIES], DOMDocument* doc, signed short * status)
{

    DOMElement *  ciCitationElem = NULL;

	try
    {

        /* CI_citation element is optional.  If caller supplied title, then caller wants a citation element. */
        if(strcmp(title, "") == 0  )
        {

            //fprintf(stderr, "INFO: Caller does not want CI_Citation node.\n");
            *status = 1;
            return ciCitationElem;
        }


        ciCitationElem = doc->createElement(X("gmd:CI_Citation"));


        /* Create the title node and its text value. */
		DOMElement * titleElem = doc->createElement(X("gmd:title"));
		ciCitationElem->appendChild(titleElem);
        
		DOMElement * titleValueElem = createElement("gco:CharacterString", title, doc);
        titleElem->appendChild(titleValueElem);


        /* Create the date node. */
        DOMElement *  dateElem = doc->createElement(X("gmd:date"));
        ciCitationElem->appendChild(dateElem);


        /* Must have both the date and the dateType in order to make the optional CI_Date node. */
        signed short numberOfElements = 0;
        if(strcmp(date, "") != 0){numberOfElements++;}
    	if(strcmp(dateType, "") != 0){numberOfElements++;}
	
	

        /* Create the CI date node. */
        DOMElement *  ciDateElem = doc->createElement(X("gmd:CI_Date"));
        dateElem->appendChild(ciDateElem);

		DOMElement *  DateElem1 = doc->createElement(X("gmd:date"));
        ciDateElem->appendChild(DateElem1);


        /* Create the date node and its text value. */
		if(strcmp(date,"") == 0)
		{
			DOMElement *  gcodateElem = createElement("gco:Date", " ", doc);
			DateElem1->appendChild(gcodateElem);

			char reason[10];
			strcpy(reason, "unknown");
			SetAttribute(gcodateElem,"gco:nilReason", reason);

		}
		else
		{
			DOMElement *  gcodateElem = createElement("gco:Date", date, doc);
			DateElem1->appendChild(gcodateElem);
		}

			
        /* Create the dateType node and its text value. */
		DOMElement *  DateTypeElem1 = doc->createElement(X("gmd:dateType"));
        ciDateElem->appendChild(DateTypeElem1);

        DOMElement *  codeTypeElem = createElement("gmd:CI_DateTypeCode", dateType, doc);
        DateTypeElem1->appendChild(codeTypeElem);
	   
		SetAttribute(codeTypeElem,"codeList", "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#CI_DateTypeCode");
		SetAttribute(codeTypeElem,"codeListValue", dateType);
     
    
        /* While there are responsible parties to add, add them. "role" is a required item, so it is used as criteria. */
        int r = 0;
        while(strcmp((char*)responsibleParties[r].role, "")   != 0  && r < MAX_CI_RESPONSIBLE_PARTIES)
        {



            DOMElement *  CI_ResponsiblePartyElem = createCI_ResponsiblePartyNode(responsibleParties[r],doc);


            /* CI_ResponsiblePartyElem should be NULL if one of the following fields is not supplied:                      */ 
            /* individualName, organisationName, postionName.                                                              */
            /* This criteria is not enforced by XML document validation, but is stated in the notes of the ISO schema.     */
            if(CI_ResponsiblePartyElem == NULL)
            {

                fprintf(stderr, "ERROR: responsibleParties[%d]: At least one of the following fields must be supplied. individualName, organisationName, postionName.\n", r);
			 
                ciCitationElem->release();
                *status = -1;
                ciCitationElem = NULL;
                return ciCitationElem;
               
            }

            /* Create the citedResponsibleParty node. & append the CI_Responsible_Party */
            DOMElement *  citedResponsiblePartyElem = doc->createElement(X("gmd:citedResponsibleParty"));
            ciCitationElem->appendChild(citedResponsiblePartyElem);
            citedResponsiblePartyElem->appendChild(CI_ResponsiblePartyElem);
            r++;
        }

    }// end else citation title was supplied.

    catch(...)
    {
        fprintf(stderr, "ERROR: an exception was encountered while attempting to create the CI_Citation node.\n ");
        ciCitationElem->release();
        ciCitationElem = NULL;
        *status = -1;
        return ciCitationElem;

    }

    return ciCitationElem;
}


//*****************************************************************************
/*!
\ Function: createCharacterSetNode

\brief  This function creates and populates a DOMElement named gmd:characterSety.

\param  ident_info 
	\li		populated structure from which to make DOMElement.
\param  doc 
	\li		DOMdocument that should be used to create the DOMElement.

\return
	\li Returns a pointer to the DOMElement created. Returns NULL if an exceptions were encountered.

*/
//*****************************************************************************
DOMElement * createCharacterSetNode(IDENTIFICATION_INFO  ident_info, DOMDocument * doc)
{


    DOMElement * CharacterSetElem = NULL;
    int i = 0;


    try
    {

        CharacterSetElem = doc->createElement(X("gmd:characterSet"));

                
		DOMElement *  CharacterSetCodeElem = createElement("gmd:MD_CharacterSetCode", (char*)ident_info.character_set , doc);
		 CharacterSetElem->appendChild(CharacterSetCodeElem);

       
	
		SetAttribute(CharacterSetCodeElem,"codeList", "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#MD_CharacterSetCode");
		SetAttribute(CharacterSetCodeElem,"codeListValue", (char*)ident_info.character_set);
	}
	catch(...)
    { 

        fprintf(stderr, "ERROR: Error creating character set tag.\n");
        CharacterSetElem->release();
        CharacterSetElem = NULL;

    }
  
    return CharacterSetElem;


}

	

//*****************************************************************************
/*!
\ Function: createHierarchyLevelNode

\brief  This function creates and populates a DOMElement named gmd:characterSety.

\param  dataQuality 
	\li		populated structure from which to make DOMElement.
\param  doc 
	\li		DOMdocument that should be used to create the DOMElement.

\return
	\li Returns a pointer to the DOMElement created. Returns NULL if an exceptions were encountered.

*/
//*****************************************************************************
DOMElement * createHierarchyLevelNode(DATA_QUALITY_INFO  dataQuality, DOMDocument * doc)
{


    DOMElement * HierarchyLevelElem = NULL;
    int i = 0;


    try
    {

        HierarchyLevelElem = doc->createElement(X("gmd:hierarchyLevel"));

                
		DOMElement *  MD_ScopeCodeElem = createElement("gmd:MD_ScopeCode", (char*)dataQuality.scope , doc);
		 HierarchyLevelElem->appendChild(MD_ScopeCodeElem);

       
	
		SetAttribute(MD_ScopeCodeElem,"codeList", "http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#MD_ScopeCode");
		SetAttribute(MD_ScopeCodeElem,"codeListValue", (char*)dataQuality.scope);
	}
	catch(...)
    { 

        fprintf(stderr, "ERROR: Error creating data quality.\n");
        HierarchyLevelElem->release();
        HierarchyLevelElem = NULL;

    }
  
    return HierarchyLevelElem;


}






//*****************************************************************************
/*!
\ Function: createElement

\brief  This function creates an element and its text value.

\param  elementName 
	\li		name of element to create.
\param  elementValue 
	\li		string value that should assigned to the newly created element.
\param  doc 
	\li		the DOMDocument that should be used to create the element.
\return
	\li Returns a pointer to specified DOMElement.  
        Returns a pointer to NULL if not able to create element and set value.

*/
//*****************************************************************************
DOMElement * createElement(char * elementName, char * elementValue, DOMDocument * doc){


    DOMElement *  genericElement = NULL;
    

    try
    {

        /* Create the element. */
        genericElement = doc->createElement(X(elementName));

        /* Set the text value of the element. */
        if(strcmp(elementValue, "") != 0)
        {
            DOMText *  genericElementText = doc->createTextNode(X(elementValue));
            genericElement->appendChild(genericElementText);
        }


    }
    catch (const DOMException& toCatch) 
    {
        char * message = XMLString::transcode(toCatch.msg);
        fprintf(stderr,"Exception message is: %s\n", message);
        XMLString::release(&message);
        genericElement = NULL;
      
    }
    catch (...)
    {
        fprintf(stderr,"ERROR: an exception occurred while creating an element.\n");
        genericElement = NULL;
    }

    return genericElement;
}


  

//*****************************************************************************
/*!
\ Function: SetAttribute

\brief  This function sets the value of an attribute of the given DOMElement.  
           If the attribute has already been set then the value is changed.
\param  someElement 
	\li		element on which to set attribute.
\param  attName 
	\li		the name of the attribute on which the value should be set.
\param  attValue 
	\li		the value of the attribute.
\return
	\li 1 if the function is successful, non-zero if the function fails.

*/
//*****************************************************************************
bagError SetAttribute(DOMElement * someElement, char *attName, char *attValue)
{


   try 
   {

       /* Set the attribute */
       someElement->setAttribute(XMLString::transcode(attName),XMLString::transcode(attValue));

   }
   catch (const DOMException& toCatch) 
   {

       char * message = XMLString::transcode(toCatch.msg);
       fprintf(stderr, "Exception message is: \n %s\n", message);
       XMLString::release(&message);
       return -2;

   }
   catch (...)
   {
       fprintf(stderr, "ERROR: an exception was encountered while setting the value of an attribute.\n");
       return -3;
   }

   /* Got to this point,  so must be successful */
   return 1;
 
}



//*****************************************************************************
/*!
\ Function: FindNode

\brief  This function finds a node in the node tree defined by the following criteria:
           tag = a valid tag name, attName = NULL, attValue = NULL. 
                 The first element with the tag in the node tree is made the current node.
           tag = a valid tag name, attName = A valide Attribute Name, attValue = a value.
                 The first element with the tag in the node tree that has the corresponding 
                 attribute information is made current.

           Returns the first node with the tag in the node tree that has the corresponding 
           attribute information.
           If no element meeting the specified criteria is found, then a NULL DOMElement is returned.
           Else the DOMElement meeting the criteria is returned.

\param  doc 
	\li		document in which to find node.
\param  tag 
	\li		The tag name of the node to find.
\param  attName 
	\li		the name of the attribute the check for.
\param  attValue 
	\li		attValue - the value of the attribute to check for.
\return
	\li DOMNode specified, NULL if DOM node can not be found or if an exception is encountered.

*/
//*****************************************************************************
DOMNode * FindNode(DOMDocument * doc, char *tag, char *attName, char *attValue)
{


    bool found = false;
    DOMElement *theElement = NULL;


    try{

        /* Create a list of all elements with the tag name */
        DOMNodeList* mylist = doc->getElementsByTagName(XMLString::transcode(tag));
 

        /* If no nodes of this tag name exist then return NULL node. */
        if (mylist->getLength() < 1){

            return theElement;
        }


        /* If the attName and attValue not specified, then return the first node found with the specified tag name. */
        if (attName == NULL || attValue == NULL)
        {
            return (DOMNode *)mylist->item(0);
  
        }


        /* Traverse the list looking for the attName and Value */
        /* If found, then break from loop and return the current node. */
        for (XMLSize_t i=0; i<mylist->getLength();i++)
        {

            theElement = (DOMElement *)mylist->item(i);

            if (theElement->hasAttribute(XMLString::transcode(attName)))
            {
                if (XMLString::compareString(XMLString::transcode(attValue), theElement->getAttribute(XMLString::transcode(attName))) ==0)
                {
                    found = true;
                    break;
         
                }
            }
        }
    }
    catch (...)
    {

        fprintf(stderr,"ERROR: an exception occurred while finding a child node in a given DOMDocument.\n");
        theElement = NULL;
    }


    return theElement;


}



//*****************************************************************************
/*!
\ Function: FindNode

\brief  This function finds a node in the node tree defined by the following criteria:
           tag = a valid tag name, attName = NULL, attValue = NULL. 
                 The first element with the tag in the node tree is made the current node.
           tag = a valid tag name, attName = A valide Attribute Name, attValue = a value.
                 The first element with the tag in the node tree that has the corresponding 
                 attribute information is made current.

           If no element meeting the specified criteria is found, then a NULL DOMElement is returned.
           Else the DOMElement meeting the criteria is returned.

\param  searchElement 
	\li		DOMElement on which to find a node.
\param  tag 
	\li		The tag name of the node to find.
\param  attName 
	\li		the name of the attribute the check for.
\param  attValue 
	\li		attValue - the value of the attribute to check for.
\return
	\li DOMNode specified, NULL if DOM node can not be found or if an exception is encountered.

*/
//*****************************************************************************
DOMNode * FindNode(DOMElement * searchElement, char *tag, char *attName, char *attValue)
{

    bool found = false;
    DOMElement *theElement = NULL;


    try
    {

        /* Create a list of all elements with the tag name */
 
        DOMNodeList* mylist = searchElement->getElementsByTagName(XMLString::transcode(tag));
 

        /* If none exist the return null node */
        if (mylist->getLength() < 1)
        {

            return theElement;
        }


        /* If the attName and attValue not specified, then the first one found becomes the current node */
        if (attName == NULL || attValue == NULL)
        {
            return (DOMNode *)mylist->item(0);
  
        }


        /* Traverse the list looking for the attName and Value */
        /* If found then make that node the current and return 1 (Success) */
        for (XMLSize_t i=0; i<mylist->getLength(); i++)
        {
            theElement = (DOMElement *)mylist->item(i);

            if (theElement->hasAttribute(XMLString::transcode(attName)))
            {

                if (XMLString::compareString(XMLString::transcode(attValue), theElement->getAttribute(XMLString::transcode(attName))) == 0)
                {
                    found = true;
                    break;
         
                }
            }
        }


    }
    catch (...)
    {
        fprintf(stderr,"ERROR: an exception occurred while finding a child node of a given DOMNode .\n");
        theElement = NULL;

    }

    return theElement;


}


//*****************************************************************************
/*!
\ Function: GetNodeValue

\brief  This function gets the value of a given DOMNode.

\param  someNode 
	\li		Node from which to get value.
\param  val 
	\li		String in which to load  node value
\param  valLength 
	\li		maxiumum allowable length of the value to be retrieved from the given node.
\return
	\li 1 if the function is successful, non-zero if the function fails.

*/
//*****************************************************************************
u16 GetNodeValue(DOMNode * someNode, char * val, signed short valLength)
{

    const XMLCh *xVal;


    if (someNode->getNodeType() != DOMNode::ELEMENT_NODE)
    {

        return -1;
    }



    /* Get the value */
    try
    {

        xVal = someNode->getTextContent();

    }
    catch (DOMException& toCatch)
    {

        char * message = XMLString::transcode(toCatch.msg);
        fprintf(stderr, "Exception message is: \n %s\n", message);
        XMLString::release(&message);
        return -1;

    }   
    catch (...)
    {
        fprintf(stderr, "ERROR: an exception was encountered while getting the value of a node.\n");
        return -1;
    }

   
    strncpy(val, XMLString::transcode(xVal), valLength );
 

    return 1;

}







//*****************************************************************************
/*!
\ Function: FreeDataQualityInfo

\brief  Frees a DATA_QUALITY_INFO structure.

\param  dataQuality 
	\li		pointer to structure to be freed.
\return
	\li 1 if the function is successful, non-zero if the function fails.

*/
//*****************************************************************************
signed short  FreeDataQualityInfo(DATA_QUALITY_INFO * dataQuality )
{

    signed short status = 1;

    try{
		
		if(dataQuality != NULL)
		{
			free(dataQuality->lineageSources);
			free(dataQuality->lineageProcessSteps);
			free(dataQuality);
		}
	    else 
			throw (signed short) -1;
    }               
    catch(signed short intException)
    {
		fprintf(stderr,"ERROR: Exception when attempting to FREE data structure. Exception message is: Null pointer\n");
        status = -1;
    }

    return status;
}

    






