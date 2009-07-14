
#include "bag_xml_meta.hpp"



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
DOMElement * createCI_CitationNode(NV_CHAR title[100], NV_CHAR date[20], NV_CHAR dateType[20], RESPONSIBLE_PARTY responsibleParties[MAX_CI_RESPONSIBLE_PARTIES], DOMDocument* doc, NV_INT16 * status);
DOMElement * createCI_ResponsiblePartyNode(RESPONSIBLE_PARTY responsiblePartyStruct, DOMDocument * doc);
DOMElement * createMD_LegalConstraintsNode(MD_LEGAL_CONSTRAINTS  legalConstraints, DOMDocument * doc);
DOMElement * createMD_SecurityConstraintsNode(MD_SECURITY_CONSTRAINTS  securityConstraints, DOMDocument * doc);
DOMElement * createDQ_DataQualityNode(DATA_QUALITY_INFO  dataQuality, DOMDocument * doc);
DOMElement * createMD_GeorectifiedNode(SPATIAL_REPRESENTATION_INFO  spatialRepresentationInfo, DOMDocument * doc);
DOMElement * createMD_CRSNode(REFERENCE_SYSTEM_INFO referenceSystemInfo, DOMDocument * doc);
DOMElement * createMD_CRSNode(char verticalDatum[100], DOMDocument * doc);
DOMElement * createBAG_DataIdentificationNode(IDENTIFICATION_INFO identificationInfo, DOMDocument * doc);
DOMElement * createSourceNode(SOURCE_INFO  source, DOMDocument * doc);
DOMElement * createProcessStepNode(PROCESS_STEP_INFO  processStep, DOMDocument * doc);



/* General DOM tree methods. */
DOMElement * createElement(NV_CHAR * elementName, NV_CHAR * elementValue, DOMDocument * doc);
DOMNode * FindNode(DOMDocument * doc, NV_CHAR *tag, NV_CHAR *attName, NV_CHAR *attValue);
DOMNode * FindNode(DOMElement * searchElement, NV_CHAR *tag, NV_CHAR *attName, NV_CHAR *attValue);
NV_INT16 GetNodeValue(DOMNode * someNode, NV_CHAR * val, NV_INT16 valLength);
NV_INT16 SetAttribute(DOMElement * someElement, NV_CHAR *attName, NV_CHAR *attValue);


/* General Xerces methods. */
NV_INT16 IsXMLInitialized(void);
NV_INT16 TerminateXML();




/******************************************************************************************************
*
* Function: GetContact
*
* Purpose:  Populates a RESPONSIBLE_PARTY structure with "contact" information retreived from a BAG metadata XML string.
*
* Inputs:   xmlString - XML string from which to retrieve contact information. 
*           contact   - pointer to structure to load with converted XML.
*
* Outputs:  Error messages to stderr.
*
* Returns:  Status 1 = Successful.  
*                 -1 = Error initializing Xerces.
*                 -3 = No element named "contact" exists.
*                -20 = An exception occurred while trying to retrieve the contact from the given XML string.
*
* Assumptions: Assumes that only one "contact" node exists in the XML.
*
*******************************************************************************************************/

NV_INT16  GetContact( NV_U_CHAR * xmlString, RESPONSIBLE_PARTY * contact )
{

    NV_INT16 errorCode = 0;
    NV_CHAR *  gMemBufId = "metadata";
    DOMDocument * xmlDoc = NULL;
 

    DOMBuilder *myParser;    /* This will parse the given XML string into a DOM document. */
       


    /* Initialize XERCES. */
    if (!XMLinitialized)
    {
        try
        {
           XMLPlatformUtils::Initialize();
        }
        catch(const XMLException& toCatch)
        {

            NV_CHAR *pMsg = XMLString::transcode(toCatch.getMessage());
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
 
    try
    {

        /* Init the parser. */
        myParser = ((DOMImplementationLS*)impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS,NULL);


        /* Set up memory input source (XML string) to be parsed.*/
        //MemBufInputSource* memBufIS = new MemBufInputSource   (  (const XMLByte*)xmlString , strlen(xmlString), gMemBufId, false);
        /* 6.18.2009 - E. Warner - Using NV_U_CHAR for consistency. */
        MemBufInputSource* memBufIS = new MemBufInputSource   (  (const XMLByte*)xmlString , strlen((NV_CHAR *)xmlString), gMemBufId, false);

        Wrapper4InputSource wrap(memBufIS,false);   
        xmlDoc = myParser->parse(wrap);

        memBufIS-> ~MemBufInputSource();



       DOMNode * contactNode = FindNode(xmlDoc, "contact", NULL, NULL);

       if (contactNode != NULL)
       {
  
            /*Get a list of the contact subnodes. */

            DOMNodeList* contactSubNodesList = ((DOMElement *)contactNode)->getElementsByTagName(X("*"));
            XMLSize_t length = contactSubNodesList->getLength();

            DOMNode *tempNode;
            XMLSize_t j=0;


            NV_CHAR * nodeName = (NV_CHAR *)calloc(100, sizeof(NV_CHAR)); 
		    NV_CHAR nodeValue[100];
     
            for (j = 0; j < length; j++)
            {
    
                tempNode = (DOMNode *)contactSubNodesList->item(j);
			

                nodeName = XMLString::transcode(tempNode->getNodeName());

      
                GetNodeValue(tempNode, nodeValue, 99);
                //fprintf(stderr, "--%s, %s--\n", nodeName, nodeValue);
    
   
                if(strcmp(nodeName, "individualName") == 0)
                {
    
                   strncpy(contact->individualName, nodeValue, 99);

                }
                else if(strcmp(nodeName, "organisationName") == 0)
                {
        
                    strncpy(contact->organisationName, nodeValue, 99);

                }
                else if(strcmp(nodeName, "positionName") == 0)
                {

                    strncpy(contact->positionName, nodeValue, 99);

                }
                else if(strcmp(nodeName, "phoneNumber") == 0)
                {
        
                    strncpy(contact->phoneNumber, nodeValue, 16);

                }
                else if(strcmp(nodeName, "role") == 0)
                {
  
                    strncpy(contact->role, nodeValue, 99);
                }

    
            }//end for

            /* Free the transcoded memory. */
            XMLString::release(&nodeName);
            free(nodeName);
            contactSubNodesList->~DOMNodeList();

        }//end if

        /* Otherwise contact node was not found. */
        else
        {

            throw (NV_INT16) -3;
        }
   
    }
    catch (int intException)
    {          
            
        fprintf(stderr,"ERROR: An exception occurred while attempting to get the contact.  Exception message is: %d\n", intException );
        errorCode = intException;

    }
    catch(...)
    {

        fprintf(stderr,"ERROR: An exception occurred while attemoting to get the contact.\n");
        errorCode =  -20;
    }

    /* Release resources. */
    xmlDoc->release();
    TerminateXML();


    return errorCode;


}






/******************************************************************************************************
*
* Function:    GetContactList
*
* Purpose:     Populates a list of RESPONSIBLE_PARTY structures with contact information retreived from a BAG metadata XML string.
*
* Inputs:      xmlString   - a well-formed, valid XML string containing BAG metadata
*              contactList - address of a pointer that points to RESPONSIBLE_PARTY structures to populate with contacts listed in given XML string.
*
* Assumptions: Assumes that given XML string is populated and well formed.
*
* Outputs:     Error messages to stderr.
*
* Returns:     NV_INT16 = number of contacts that were put in the contactList. 
*                    -1 = Error initializing Xerces.
*                    -3 = No elements named "contact" exist.
*                   -20 = A exception occurred while trying to retrieve the list of contacts from the given XML string.
*
* Note:        contactList is calloc'd in this method and must be freed by the programmer calling this method.
*
*******************************************************************************************************/

NV_INT16  GetContactList( NV_U_CHAR * xmlString, RESPONSIBLE_PARTY ** contactList )
{
 
    NV_INT16 errorCode = 0;
    XMLSize_t numberOfContacts = 0;
    NV_CHAR *  gMemBufId = "metadata";
 

    DOMBuilder *myParser;    /* This will parse the given XML string into a DOM document. */
    DOMDocument * xmlDoc = NULL;


    /* Initialize XERCES. */
    if (!XMLinitialized)
    {
        try
        {
            XMLPlatformUtils::Initialize();
        }
        catch(const XMLException& toCatch)
        {
            NV_CHAR *pMsg = XMLString::transcode(toCatch.getMessage());
            fprintf(stderr,"ERROR: %s.\n" , pMsg);
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

    try
    {

        /* Init the parser. */
        myParser = ((DOMImplementationLS*)impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS,NULL);


        /* Set up memory input source (XML string) to be parsed.*/
        //MemBufInputSource* memBufIS = new MemBufInputSource   (  (const XMLByte*)xmlString , strlen(xmlString), gMemBufId, false);
        /* 6.18.2009 - E. Warner - Using NV_U_CHAR for consistency. */
        MemBufInputSource* memBufIS = new MemBufInputSource   (  (const XMLByte*)xmlString , strlen((NV_CHAR *)xmlString), gMemBufId, false);

        Wrapper4InputSource wrap(memBufIS,false);   
        xmlDoc = myParser->parse(wrap);
        memBufIS-> ~MemBufInputSource();

 
        /* Determine number of contacts in given XML string. */
        DOMNodeList * contactNodeList = xmlDoc->getElementsByTagName(X("contact"));
        numberOfContacts = contactNodeList->getLength();


        /* Return an error code if no "contact" nodes were found. */
        if(numberOfContacts <= 0)
        {

            throw (NV_INT16) -3;
        }


        /* Loop through all of the contacts listed in the XML string and populate a RESPONSIBLE_PARTY structure for each contact. */
        DOMNode * contactNode = NULL;
        *contactList = (RESPONSIBLE_PARTY *)calloc(numberOfContacts, sizeof(RESPONSIBLE_PARTY));



        DOMNodeList * contactSubNodesList = NULL;
        XMLSize_t subNodesLength = 0;

        DOMNode * contactSubNode = NULL;
        NV_CHAR * nodeName = (NV_CHAR *)calloc(100, sizeof(NV_CHAR)); 
        NV_CHAR nodeValue[100];


        for (XMLSize_t c = 0; c < numberOfContacts; c++)
        {

            /* Get the contact node from the list of contacts. */
            contactNode = contactNodeList->item(c);
  
            /* Get a list of this contact's subnodes. */
            contactSubNodesList = ((DOMElement *)contactNode)->getElementsByTagName(X("*"));
            subNodesLength = contactSubNodesList->getLength();

   
            /* For each subnode, populate the corresponding field in the data structure. */
            for (XMLSize_t j = 0; j < subNodesLength; j++)
            {
    

                contactSubNode = (DOMNode *)contactSubNodesList->item(j);

                nodeName = XMLString::transcode(contactSubNode->getNodeName());
        
                GetNodeValue(contactSubNode, nodeValue, 99);
    

       
                if(strcmp(nodeName, "individualName") == 0)
                {
	       
                    strncpy((*contactList)[c].individualName, nodeValue, 99);

                }
                else if(strcmp(nodeName, "organisationName") == 0)
                {
        
                    strncpy((*contactList)[c].organisationName, nodeValue, 99);

                }
                else if(strcmp(nodeName, "positionName") == 0)
                {
        
                    strncpy((*contactList)[c].positionName, nodeValue, 99);
                }

                else if(strcmp(nodeName, "phoneNumber") == 0)
                {
    
                    strncpy((*contactList)[c].phoneNumber, nodeValue, 16);
                }

                else if(strcmp(nodeName, "role") == 0)
                {
	     
                    strncpy((*contactList)[c].role, nodeValue, 99);
                    
                }

           }//end for each contact subnode

        }//end for

        /* Free the transcoded memory. */
        XMLString::release(&nodeName);
        free(nodeName);

        errorCode = numberOfContacts;

    }
    catch (int intException)
    {          

        fprintf(stderr,"ERROR: An exception occurred while attempting to get list of contacts.  Exception message is: %d\n", intException );
        errorCode = intException;

    }
    catch(...)
    {

        fprintf(stderr,"ERROR: An exception occurred while attempting to get list of contacts from XML string.\n");
        errorCode =   -20;
    }



    /* Release resources. */
    xmlDoc->release();
    TerminateXML();


    return errorCode;


}






/******************************************************************************************************
*
* Function: GetLegalConstraints
*
* Purpose:  Populates a MD_LEGAL_CONSTRAINTS structure with smXML:MD_LegalConstraints information retreived from a BAG metadata XML string.
*
* Inputs:   xmlString - XML string to parse for information
*           legalConstraints - pointer to MD_LEGAL_CONSTRAINTS structure to load with retrieved XML.
*
* Outputs:  Error messages to stderr.
*
* Returns:  Status 1 = Successful.   
*                 -1 = Error initializing Xerces.
*                 -3 = node smXML:MD_LegalConstraints does not exist.
*                -20 = An exception occurred while attempting to retrieve the legal constraints from the given XML string.
*             
* Assumptions: Assumes that only one MD_LegalConstraints node exists.
*
*******************************************************************************************************/

NV_INT16  GetLegalConstraints( NV_U_CHAR * xmlString, MD_LEGAL_CONSTRAINTS * legalConstraints )
{

    NV_INT16 errorCode = 1;  /* Assume method will be  successful. */
    NV_CHAR *  gMemBufId = "metadata";
 

    DOMBuilder *myParser;    /* This will parse the given XML string into a DOM document. */
    DOMDocument* xmlDoc = NULL;




    /* Initialize XERCES. */
    if (!XMLinitialized)
    {
        try
        {
            XMLPlatformUtils::Initialize();
        }
        catch(const XMLException& toCatch)
        {
            NV_CHAR *pMsg = XMLString::transcode(toCatch.getMessage());
            fprintf(stderr,"ERROR: %s.\n" , pMsg);
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
 

    try
    {
     
        /* Init the parser. */
        myParser = ((DOMImplementationLS*)impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS,NULL);

       

        /* Set up memory input source (XML string) to be parsed.*/
        //MemBufInputSource* memBufIS = new MemBufInputSource   (  (const XMLByte*)xmlString , strlen(xmlString), gMemBufId, false);
        /* 6.18.2009 - E. Warner - Using NV_U_CHAR for consistency. */
        MemBufInputSource* memBufIS = new MemBufInputSource   (  (const XMLByte*)xmlString , strlen((NV_CHAR *)xmlString), gMemBufId, false);

        Wrapper4InputSource wrap(memBufIS,false);   
        xmlDoc = myParser->parse(wrap);
        memBufIS-> ~MemBufInputSource();


        /* Get a list of nodes in the given XML string. (should only be 1 at this time, but leave this way for future expansion.) */
        DOMNodeList* legalConstraintNodeList = xmlDoc->getElementsByTagName(XMLString::transcode("smXML:MD_LegalConstraints"));


      
 
        /* If no nodes exist, then return none found */
        if (legalConstraintNodeList->getLength() == 0 )
        {

            throw (NV_INT16) -3;
        }

  

        /* Go through nodes under the found node (smXML:MD_LegalConstraints).  Find nodes that correspond to structures and populate them. */
        DOMNode * legalConstraintsNode = NULL;
        DOMNode * tempNode = NULL;
        NV_CHAR nodeValue[100];

        for (XMLSize_t i=0; i < legalConstraintNodeList->getLength();i++)
        {
  
		    
            legalConstraintsNode = legalConstraintNodeList->item(i);

            tempNode = FindNode((DOMElement *)legalConstraintsNode,"useConstraints",  NULL, NULL);

            if(tempNode != NULL)
            {
                if(GetNodeValue(tempNode, nodeValue, 39)==1)
                {
                    strncpy(legalConstraints->useConstraints, nodeValue, 39);
                }

            }

            tempNode = FindNode((DOMElement *)legalConstraintsNode,"otherConstraints",  NULL, NULL);

            if(tempNode != NULL)
            {
                if(GetNodeValue(tempNode, nodeValue, 99)==1)
                {
         
                    strncpy(legalConstraints->otherConstraints, nodeValue, 99);
                }

            }
  
        }/* end for loop */


    }
    catch (int intException)
    {          
            
        fprintf(stderr,"ERROR: An exception occurred while attemoting to get the legal constraints.  Exception message is: %d\n", intException );
        errorCode = intException;

    }
    catch(...)
    {

        fprintf(stderr,"ERROR: An exception occurred while attemoting to get the legal constraints.\n");
        errorCode =  -20;
    }

    /* Release the resources. */
    xmlDoc->release();
    TerminateXML();

    return errorCode;


}


/******************************************************************************************************
*
* Function: GetSecurityConstraint
*
* Purpose:  Populates a MD_SECURITY_CONSTRAINTS structure with smXML:MD_SecurityConstraints information retreived from a BAG metadata XML string.
*
* Inputs:   xmlString - XML string to convert
*          securityConstraints -  pointer to MD_SECURITY_CONSTRAINTS structure to load with converted XML.
*
* Outputs:  Error messages to stderr.
*
* Returns:  Status  1 = Successful.
*                  -1 = Error initializing Xerces.
*                  -3 = Node smXML:MD_SecurityConstraints does not exist.
*                 -20 = An exception occurred while attempting to retrieve security constraints from the given XML string.
*             
* Assumptions: Assumes that only one MD_SecurityConstraints node exists.
*
*******************************************************************************************************/

NV_INT16  GetSecurityConstraints( NV_U_CHAR * xmlString, MD_SECURITY_CONSTRAINTS * securityConstraints )
{

    NV_INT16 errorCode = 1;
    NV_CHAR * gMemBufId = "metadata";
 

    DOMBuilder * myParser;    /* This will parse the given XML string into a DOM document. */
    DOMDocument * xmlDoc = NULL;

    DOMNode * securityConstraintsNode = NULL;
    DOMNode * tempNode = NULL;


    /* Initialize XERCES. */
    if (!XMLinitialized)
    {
        try
        {
            XMLPlatformUtils::Initialize();
        }
        catch(const XMLException& toCatch)
        {

            NV_CHAR *pMsg = XMLString::transcode(toCatch.getMessage());
            fprintf(stderr,"ERROR: %s.\n" , pMsg);
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
 
    
    try
    {

        /* Init the parser. */
        myParser = ((DOMImplementation*)impl)->createDOMBuilder(DOMImplementation::MODE_SYNCHRONOUS,NULL);




        /* Set up memory input source (XML string) to be parsed.*/
        // MemBufInputSource* memBufIS = new MemBufInputSource   (  (const XMLByte*)xmlString , strlen(xmlString), gMemBufId, false);
        /* 6.18.2009 - E. Warner - Using NV_U_CHAR for consistency. */
        MemBufInputSource* memBufIS = new MemBufInputSource   (  (const XMLByte*)xmlString , strlen((NV_CHAR *)xmlString), gMemBufId, false);

        Wrapper4InputSource wrap(memBufIS,false);   
        xmlDoc = myParser->parse(wrap);
        memBufIS-> ~MemBufInputSource();


        /* Get a list of MD_SecurityConstraints nodes in the given XML string.      */
        /* (should only be 1 at this time, but leave this way for future expansion.) */
        DOMNodeList* securityConstraintNodeList = xmlDoc->getElementsByTagName(XMLString::transcode("smXML:MD_SecurityConstraints"));

        //fprintf(stderr,"Number of MD_SecurityConstraints nodes found: %d \n",myList->getLength() );


 
        /* If no nodes exist, then return none found */
        if (securityConstraintNodeList->getLength() < 1)
        {

            throw (NV_INT16) -3;
        }



        /* Go through nodes under the found node (smXML:MD_SecurityConstraints).  Find nodes that correspond to structures and populate them. */
        //DOMNode * securityConstraintsNode = NULL;
        //DOMNode * tempNode = NULL;
        NV_CHAR nodeValue[3999];

        for (XMLSize_t i=0; i < securityConstraintNodeList->getLength();i++)
        {
  
            securityConstraintsNode = securityConstraintNodeList->item(i);

            tempNode = FindNode((DOMElement *)securityConstraintsNode,"classification",  NULL, NULL);

            if(tempNode != NULL)
            {

                if(GetNodeValue(tempNode, nodeValue, 39)==1)
                {
                    strncpy(securityConstraints->classification, nodeValue, 39);
                }

            }

            tempNode = FindNode((DOMElement *)securityConstraintsNode,"userNote",  NULL, NULL);

            if(tempNode != NULL)
            {
                if(GetNodeValue(tempNode, nodeValue, 3999)==1)
                {
         
                    strncpy(securityConstraints->userNote, nodeValue, 3999);
                }
            }
        }



    }
    catch (int intException)
    {          
            
        fprintf(stderr,"ERROR: An exception occurred while attempting to get security constraints.  Exception message is: %d\n", intException );
        errorCode = intException;

    }
    catch(...)
    {
        fprintf(stderr,"ERROR: An exception occurred while attempting to get security constraints.\n");
        errorCode =  -20;
    }


    /* Release resources. */
    xmlDoc->release();
    TerminateXML();

    return errorCode;


}

/******************************************************************************************************
*
* Function: GetDateStamp
*
* Purpose:  Populates a string with dateStamp information retreived from a BAG metadata XML string.
* 
* Inputs:   xmlString - XML string from which to obtain language metadata.
*           dateString -  character pointer to load with dateStamp information. 
*
* Outputs:  Error messages to stderr.
*
* Returns:  NV_INT16  = Length of the dateString tha is being returned.
*                  -1 = Error initializing Xerces.
*                  -3 = dateStamp node does not exist in the given XML string.
*                 -20 = An exception occurred while trying to retrieve the dateStamp from the given XML string.
*             
* Note: dateString memory is allocated in this method and must be freed by the programmer.
*
*******************************************************************************************************/

NV_INT16  GetDateStamp( NV_U_CHAR * xmlString, NV_CHAR ** dateString )
{

    NV_INT16  dateStampSize = 0;
    NV_CHAR *  gMemBufId = "metadata";
 

    DOMBuilder *myParser;    /* This will parse the given XML string into a DOM document. */
    DOMDocument * xmlDoc = NULL;



    /* Initialize XERCES. */
    if (!XMLinitialized)
    {
	  

        try
        {
            XMLPlatformUtils::Initialize();
        }
        catch(const XMLException& toCatch)
        {
            NV_CHAR *pMsg = XMLString::transcode(toCatch.getMessage());
            fprintf(stderr,"ERROR: %s.\n" , pMsg);
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
 
 
    try
    {

        /* Init the parser. */
        myParser = ((DOMImplementation*)impl)->createDOMBuilder(DOMImplementation::MODE_SYNCHRONOUS,NULL);



        /* Set up memory input source (XML string) to be parsed.*/
        //MemBufInputSource* memBufIS = new MemBufInputSource   (  (const XMLByte*)xmlString , strlen(xmlString), gMemBufId, false);
        /* 6.18.2009 - E. Warner - Using NV_U_CHAR for consistency. */
        MemBufInputSource* memBufIS = new MemBufInputSource   (  (const XMLByte*)xmlString , strlen((NV_CHAR *)xmlString), gMemBufId, false);

        Wrapper4InputSource wrap(memBufIS,false);   
        xmlDoc = myParser->parse(wrap);
        memBufIS-> ~MemBufInputSource();


        /* Get a list of nodes in the given XML string. (should only be 1.)*/
        DOMNode* dateStampNode = FindNode(xmlDoc, "dateStamp", NULL, NULL);


 
        /* If no dateStamp node exist, return error */
        if (dateStampNode == NULL)
        {

            throw (NV_INT16) -3;
        }



        /* Determine the size of the dateSamp to be retreived. */
        dateStampSize = strlen( XMLString::transcode(((DOMElement*)dateStampNode)->getTextContent()) );

      

        /* Allocate temporary storage for the dateStamp value.
           Use nodeValue as temporary holder of transcoded dateStamp value. 
           If dateStamp value is loaded directly into dateString, then programmer calling this method would have to call 
           XERCES method XMLString::release to release the dataString trancoded memory.     
        */
        NV_CHAR * nodeValue = (NV_CHAR *)calloc(dateStampSize + 1, sizeof(NV_CHAR));
        nodeValue = XMLString::transcode(((DOMElement*)dateStampNode)->getTextContent()) ;
 
        *dateString = (NV_CHAR *)calloc(dateStampSize + 1, sizeof(NV_CHAR));
        strncpy(*dateString, nodeValue, dateStampSize);
  


        /* Free the resources including transcoded memory. */
        XMLString::release(&nodeValue);
        free(nodeValue);

        dateStampNode->~DOMNode();


    }
    catch (int intException)
    {          
            
        fprintf(stderr,"ERROR: An exception occured while trying to get dateStamp.  Exception message is: %d\n", intException );
        dateStampSize = intException;

    }
    catch(...)
    {
        fprintf(stderr, "ERROR: An exception occured while trying to get dateStamp.\n");
        dateStampSize =  -20;
    }

    /* Release resources. */
    xmlDoc->release();
    TerminateXML();

    return dateStampSize;


}





/******************************************************************************************************
*
* Function: GetDataQualityInfo
*
* Purpose:  Populates a DATA_QUALITY_INFO structure with dataQualityInfo information retreived from a BAG metadata XML string.
*
* Inputs:   xmlString - XML string to convert
*           dataQuality - pointer to DATA_QUALITY_INFO structure to load with retreived information.
*
* Outputs:  Error messages to stderr.
*
* Returns:  Status  1 = Successful.
*                  -1 = Error initializing Xerces.
*                  -3 = Node smXML:DQ_DataQuality does not exist.
*                 -20 = An exception occurred while attempting to retrieve dataQualityInfo from the given XML string.
*             
* Notes:  We assume that there is only one "dataQuality" node although the ISO schema allows for an unbounded number.
*         
*         After calling this method and when the the program has completed using the dataQualityInfo, the programmer must 
*         call freeDataQualityInfo to release the memory that this function dynamically allocates.
*
*******************************************************************************************************/

NV_INT16  GetDataQualityInfo( NV_U_CHAR * xmlString, DATA_QUALITY_INFO * dataQuality )
{

    NV_INT16 errorCode = 1;
    NV_CHAR *  gMemBufId = "metadata";
 

    DOMBuilder *myParser;    /* This will parse the given XML string into a DOM document. */
    DOMDocument * xmlDoc = NULL;



    /* Initialize XERCES. */
    if (!XMLinitialized)
    {
        try
        {
            XMLPlatformUtils::Initialize();
        }
        catch(const XMLException& toCatch)
        {
            NV_CHAR *pMsg = XMLString::transcode(toCatch.getMessage());
            fprintf(stderr,"ERROR: %s.\n" , pMsg);
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

       
    try
    {

        /* Init the parser. */
        myParser = ((DOMImplementationLS*)impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS,NULL);


        /* Set up memory input source (XML string) to be parsed.*/
        //MemBufInputSource* memBufIS = new MemBufInputSource   (  (const XMLByte*)xmlString , strlen((NV_CHAR *)xmlString), gMemBufId, false);
        /* 6.18.2009 - E. Warner - Using NV_U_CHAR for consistency. */
        MemBufInputSource* memBufIS = new MemBufInputSource   (  (const XMLByte*)xmlString , strlen((NV_CHAR *)xmlString), gMemBufId, false);

        Wrapper4InputSource wrap(memBufIS,false);   
        xmlDoc = myParser->parse(wrap);
        memBufIS-> ~MemBufInputSource();
  
	

  
        /* We assume that there is one and  only one smXML:DQ_DataQuality node although */
        /* the schema allows an unbounded number of dataQualityInfo nodes.              */

        DOMElement * dataQualityElem = (DOMElement *) FindNode(xmlDoc, "smXML:DQ_DataQuality", NULL, NULL);
        if(dataQualityElem == NULL)
        {

            throw (NV_INT16) -3;
        }



        /* Get the smXML:DQ_Scope node */
        DOMElement * scopeElem = (DOMElement *) FindNode(dataQualityElem, "smXML:DQ_Scope", NULL, NULL);

   

        /* Get the "level" node. There should be only one "level" node child under "smXML:DQ_Scope" node */
        DOMElement * levelElem = (DOMElement *) FindNode(scopeElem, "level", NULL, NULL);

 

        /* Populate the level field. */
        NV_CHAR * nodeValue = (NV_CHAR *)calloc(200, sizeof(NV_CHAR));

	 

        nodeValue = XMLString::transcode(levelElem->getTextContent());

        strncpy(dataQuality->scope, nodeValue, 99);

	 

        /* Get lineage information. */
        DOMNode * lineageNode = FindNode(xmlDoc, "smXML:LI_Lineage", NULL, NULL);

        /* Can't just getElementsByTagname for "source" nodes under lineage because source nodes also appear in processStep. */
        DOMNodeList* lineageChildNodeList = lineageNode->getChildNodes();
        XMLSize_t lineageChildListLength = lineageChildNodeList->getLength();
        //fprintf(stderr, "smXML:LI_Lineage children: %d\n", lineageChildListLength);


        /* Go through each smXML:LI_Lineage child and populate the appropriate strcutures. */
        DOMNode * lineageSubNode = NULL;
        NV_CHAR * lineageSubNodeName =NULL;



        DOMNode * tempNode = NULL;
        NV_CHAR * tempNodeValue = (NV_CHAR *) calloc(200, sizeof(NV_CHAR));
        DOMNode * tempSubNode = NULL;
        NV_CHAR * tempSubNodeValue = (NV_CHAR *) calloc(200, sizeof(NV_CHAR));

	 

        /* Prepare for sources and process steps to be populated. */
        XMLSize_t sourceCounter = 0;
        XMLSize_t processStepCounter = 0;
        dataQuality->lineageSources = (SOURCE_INFO *)calloc(1, sizeof(SOURCE_INFO));
        dataQuality->lineageProcessSteps = (PROCESS_STEP_INFO  *)calloc(1, sizeof(PROCESS_STEP_INFO));

        XMLSize_t j = 0;



        /* For each child node of smXML:LI_Lineage, examine it and populate either a "source" a "processStep" structure. */
        for(XMLSize_t i = 0; i < lineageChildListLength; i++)
        {
         
            lineageSubNode = lineageChildNodeList->item(i);


            /* Get the node name. */
            lineageSubNodeName = XMLString::transcode(lineageSubNode->getNodeName());
            //fprintf(stderr, "%s\n",lineageSubNodeName );



            /* If this is a source node, get the source information. */
            if(strcmp(lineageSubNodeName, "source") == 0)
            {


                //fprintf(stderr, "source: realloc %d\n", sourceCounter + 1 );
                dataQuality->lineageSources = (SOURCE_INFO *)realloc(dataQuality->lineageSources, (sourceCounter + 1) * sizeof(SOURCE_INFO));

                InitSourceInfo(&dataQuality->lineageSources[sourceCounter]);

                /* Get the description for this source. */
                tempNode =  FindNode((DOMElement *)lineageSubNode, "description", NULL, NULL);

	
                if(tempNode != NULL)
                {

                    GetNodeValue(tempNode, tempNodeValue, 199);
                    strncpy(dataQuality->lineageSources[sourceCounter].description, tempNodeValue,199);
                }


                /* Get the citation for this source. */
                tempNode =  FindNode((DOMElement *)lineageSubNode, "sourceCitation", NULL, NULL);

                if(tempNode != NULL)
                {

                    /* Find the title of the citation. */
                    tempSubNode =  FindNode((DOMElement *)tempNode, "title", NULL, NULL);

                    if(tempSubNode != NULL)
                    {
                        GetNodeValue(tempSubNode, tempSubNodeValue, 99);
                        strncpy(dataQuality->lineageSources[sourceCounter].title, tempSubNodeValue, 99);
                    }


                    /* Find the date and dateType of the citation. */
                    tempSubNode =  FindNode((DOMElement *)tempNode, "smXML:CI_Date", NULL, NULL);
                    if(tempSubNode != NULL)
                    {

                        DOMNode * dateNode =  FindNode((DOMElement *)tempSubNode, "date", NULL, NULL);
                        NV_CHAR * dateNodeValue = (NV_CHAR *)calloc(20, sizeof(NV_CHAR));

                        if(dateNode != NULL)
		        {
			    GetNodeValue(dateNode, dateNodeValue, 19);
                            strncpy(dataQuality->lineageSources[sourceCounter].date, dateNodeValue, 19);
		        }


                        dateNode =  FindNode((DOMElement *)tempSubNode, "dateType", NULL, NULL);
                
                        if(dateNode != NULL)
		        {
			    GetNodeValue(dateNode, dateNodeValue, 19);
                            strncpy(dataQuality->lineageSources[sourceCounter].dateType, dateNodeValue, 19);
	    	        }

                        free(dateNodeValue);


		        /* To be implemented.  Get a list of cited parties */


                     }//end if this is smXML:CI_Date

                 

                }//end if this is the sourceCitation node

                sourceCounter++;
	  
            }//end if this is a source node.

    
            /* If this is a processStep node, get the processStep information. */
            else if(strcmp(lineageSubNodeName, "processStep") == 0)
            {

                /* Get the BAG_Process Step node. */
                DOMNode * bagProcessStepNode = FindNode((DOMElement *)lineageSubNode, "smXML:BAG_ProcessStep", NULL, NULL);

                if(bagProcessStepNode != NULL)
                {

                    //fprintf(stderr, "process step: relloc %d\n", processStepCounter + 1 );
                    dataQuality->lineageProcessSteps = (PROCESS_STEP_INFO *)realloc(dataQuality->lineageProcessSteps, (processStepCounter + 1) * sizeof(PROCESS_STEP_INFO));
	    	    
	  
                    tempNode =  FindNode((DOMElement *)bagProcessStepNode, "description", NULL, NULL);
                    if(tempNode != NULL)
	            {
                        GetNodeValue(tempNode, tempNodeValue, 99);
                        strncpy(dataQuality->lineageProcessSteps[processStepCounter].description, tempNodeValue, 99);
                    }

                    tempNode =  FindNode((DOMElement *)bagProcessStepNode, "dateTime", NULL, NULL);
                    if(tempNode != NULL)
	            {
		        GetNodeValue(tempNode, tempNodeValue, 29);
                        strncpy(dataQuality->lineageProcessSteps[processStepCounter].dateTime, tempNodeValue, 29);
	            }

                    tempNode =  FindNode((DOMElement *)bagProcessStepNode, "trackingId", NULL, NULL);
                    if(tempNode != NULL)
	            {
		        GetNodeValue(tempNode, tempNodeValue, 4);
                        strncpy(dataQuality->lineageProcessSteps[processStepCounter].trackingId, tempNodeValue, 4);
	            }
	  

                    /* Get a list of processors for this process step. */
                    DOMNodeList * processorsNodeList = ((DOMElement *)bagProcessStepNode)->getElementsByTagName(XMLString::transcode("processor"));

                    XMLSize_t processorListLength = processorsNodeList->getLength();
                    //fprintf(stderr, "processors: %d\n", processorListLength);

                    DOMElement * processorElem = NULL;
                    j = 0;

                    /* Populate all processors for this process step. */	    
                    while( j < MAX_CI_RESPONSIBLE_PARTIES  && j < processorListLength )
                    { 
                
	                processorElem = (DOMElement *)processorsNodeList->item(0);

                        if(processorElem == NULL)
                        {
		            fprintf(stderr, "processor elem is null\n");
                        }
                
                        tempNode = FindNode(processorElem, "individualName", NULL, NULL);

                        if(tempNode != NULL)
	                {
			    GetNodeValue(tempNode, tempNodeValue, 99);
                            strncpy(dataQuality->lineageProcessSteps[processStepCounter].processors[j].individualName, tempNodeValue, 99);
			    //fprintf(stderr, "processor: %s\n",tempNodeValue );
	                }

                        tempNode = FindNode(processorElem, "organisationName", NULL, NULL);

                        if(tempNode != NULL)
	                {
			    GetNodeValue(tempNode, tempNodeValue, 99);
                            strncpy(dataQuality->lineageProcessSteps[processStepCounter].processors[j].organisationName, tempNodeValue, 99);
	                }

                        tempNode = FindNode(processorElem, "positionName", NULL, NULL);

                        if(tempNode != NULL)
	                {
			    GetNodeValue(tempNode, tempNodeValue, 99);
                            strncpy(dataQuality->lineageProcessSteps[processStepCounter].processors[j].positionName, tempNodeValue, 99);
	                }
		
                        tempNode = FindNode(processorElem, "role", NULL, NULL);

                        if(tempNode != NULL)
                        {

                            GetNodeValue(tempNode, tempNodeValue, 99);
                            strncpy(dataQuality->lineageProcessSteps[processStepCounter].processors[j].role, tempNodeValue, 99);
                        }

                        j++;

	            }//end while there are processors 

                    processorsNodeList->~DOMNodeList();

	        }//end if bag processStep = ! NULL


	        processStepCounter++;


            }//end  else if node is a processStep node

        }//end for each child node in the lineage node

        /* Record the number of sources and the number of processStep found. */
        dataQuality->numberOfSources = sourceCounter;
        dataQuality->numberOfProcessSteps = processStepCounter;
        

        /* Release resources including transcoded memory. */
        XMLString::release(&lineageSubNodeName);
        free(tempNodeValue);
        free(tempSubNodeValue);


    }
    catch (int intException)
    {          
        fprintf(stderr,"ERROR: Exception occured while trying to get dataQualityInfo.  Exception message is: %d\n", intException );
        errorCode = intException;

    }
    catch(...)
    {
        fprintf(stderr, "ERROR: Exception occured while trying to get dataQualityInfo.\n");
        errorCode =  -20;
    }

    /* Release resources. */
    xmlDoc->release(); 
    TerminateXML();
    return errorCode;


}









/******************************************************************************************************
*
* Function: GetReferenceSystemInfo
*
* Purpose:  Populates a REFERENCE_SYSTEM_INFO structure with referenceSystemInfo information retrieved from a BAG metadata XML string.
*
* Inputs:   xmlString - XML string to conver
*           referenceSystemInfo - pointer to REFERENCE_SYSTEM_INFO structure to load with retrieved information.
*
* Outputs:  Error messages to stderr.
*
* Returns:  Status  1 = Successful.  
*                  -1 = Error initializing Xerces.
*                  -3 = Node smXML:MD_CRS does not exist.
*                 -20 = An exception occurred while retreiving reference system info from the given XML string.
*             
* Assumptions: Assumes that one and only one "referenceSystemInfo" node exists although the schema allows for zero to an unbounded number.
*
*******************************************************************************************************/

NV_INT16  GetReferenceSystemInfo( NV_U_CHAR * xmlString, REFERENCE_SYSTEM_INFO * referenceSystemInfo)
{

    NV_INT16 errorCode = 1;
    NV_CHAR *  gMemBufId = "metadata";
 

    DOMBuilder *myParser;    /* This will parse the given XML string into a DOM document. */
    DOMDocument * xmlDoc = NULL;


    /* Initialize XERCES. */
    if (!XMLinitialized)
    {
        try
        {
            XMLPlatformUtils::Initialize();
        }
        catch(const XMLException& toCatch)
        {
            NV_CHAR *pMsg = XMLString::transcode(toCatch.getMessage());
            fprintf(stderr,"ERROR: %s.\n" , pMsg);
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
 
      
    try
    {

        /* Init the parser. */
        myParser = ((DOMImplementationLS*)impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS,NULL);


        /* Set up memory input source (XML string) to be parsed.*/
        //MemBufInputSource* memBufIS = new MemBufInputSource   (  (const XMLByte*)xmlString , strlen(xmlString), gMemBufId, false);
        /* 6.18.2009 - E. Warner - Using NV_U_CHAR for consistency. */
        MemBufInputSource* memBufIS = new MemBufInputSource   (  (const XMLByte*)xmlString , strlen((NV_CHAR *)xmlString), gMemBufId, false);

        Wrapper4InputSource wrap(memBufIS,false);   
        xmlDoc = myParser->parse(wrap);
        memBufIS-> ~MemBufInputSource();


    
        /* There should be two reference system info nodes. The second one contains only the vertical datum code.                  */
        /* Beneath each reference system info node, there is an MD_CRS node which contains the metadata in which we are interested. */
        /* MD_CRS does not appear in any node other than the reference system info node.  So it is okay to call the MD_CRS node directly. */
  
        DOMNodeList* nodeList = xmlDoc->getElementsByTagName(XMLString::transcode("smXML:MD_CRS"));

  
 
        /* If no nodes exist, then return none found */
        if (nodeList->getLength() < 1)
        {

            throw (NV_INT16) -3;
        }




        /* Iterate through list of MD_CRS elements and populate corresponding structures. */
        DOMElement * referenceElem;


        for (XMLSize_t mdCrsCounter=0; mdCrsCounter<nodeList->getLength();mdCrsCounter++)
        {

            referenceElem = (DOMElement *)nodeList->item(mdCrsCounter);

            NV_CHAR * nodeName = (NV_CHAR *)calloc(100, sizeof(NV_CHAR)); 
            NV_CHAR * nodeValue = (NV_CHAR *)calloc(100, sizeof(NV_CHAR));

            nodeName = XMLString::transcode(referenceElem->getTagName());
        
       

            /* Iterate through sub nodes in each MD_CRS node. */
     
            DOMNodeList* referenceElemSubNodesList = referenceElem->getChildNodes();


            for (XMLSize_t j=0; j < referenceElemSubNodesList->getLength();j++)
            {

        
                DOMElement * referenceElemSubElem = (DOMElement *)referenceElemSubNodesList->item(j);
                NV_CHAR * referenceElemSubNodeName  = (NV_CHAR *)calloc(100, sizeof(NV_CHAR)); 
  

                referenceElemSubNodeName = XMLString::transcode(referenceElemSubElem->getTagName());
        

                if((strcmp(referenceElemSubNodeName, "projectionParameters")) == 0)
                {
          
                    /* Get all subnodes of projectionParameters. */
                    DOMNodeList * projSubNodeList = referenceElemSubElem->getElementsByTagName(X("*"));
 


                    /* Iterate through list of sub nodes and populate corresponding structures. */
                    DOMElement * projSubElem;

                    for (XMLSize_t projSubNodeCounter=0; projSubNodeCounter < projSubNodeList->getLength();projSubNodeCounter++)
                    {

                         projSubElem = (DOMElement *)projSubNodeList->item(projSubNodeCounter);

                         NV_CHAR * projSubNodeName = (NV_CHAR *)calloc(100,sizeof(NV_CHAR)); 
                         NV_CHAR * projSubNodeValue = (NV_CHAR *)calloc(100,sizeof(NV_CHAR));
                         projSubNodeName = XMLString::transcode(projSubElem->getTagName());

         

                         if((strcmp(projSubNodeName, "zone")) == 0)
                         {
                             projSubNodeValue = XMLString::transcode(projSubElem->getTextContent());
                             referenceSystemInfo->zone= atoi(projSubNodeValue);
                         }

                         else if((strcmp(projSubNodeName, "standardParallel")) == 0)
                         {
                             projSubNodeValue = XMLString::transcode(projSubElem->getTextContent());
                             referenceSystemInfo->standardParallel= atof(projSubNodeValue);
                         }
                         else if((strcmp(projSubNodeName, "longitudeOfCentralMeridian")) == 0)
                         {
                             projSubNodeValue = XMLString::transcode(projSubElem->getTextContent());
                             referenceSystemInfo->longitudeOfCentralMeridian= atof(projSubNodeValue);
                         }
                         else if((strcmp(projSubNodeName, "latitudeOfProjectionOrigin")) == 0)
                         {
                             projSubNodeValue = XMLString::transcode(projSubElem->getTextContent());
                             referenceSystemInfo->latitudeOfProjectionOrigin= atof(projSubNodeValue);
                         }
                         else if((strcmp(projSubNodeName, "falseEasting")) == 0)
                         {
                             projSubNodeValue = XMLString::transcode(projSubElem->getTextContent());
                             referenceSystemInfo->falseEasting= atof(projSubNodeValue);
                         }
                         else if((strcmp(projSubNodeName, "falseNorthing")) == 0)
                         {
                             projSubNodeValue = XMLString::transcode(projSubElem->getTextContent());
                             referenceSystemInfo->falseNorthing= atof(projSubNodeValue);
                         }
                         else if((strcmp(projSubNodeName, "scaleFactorAtEquator")) == 0)
                         {
                             projSubNodeValue = XMLString::transcode(projSubElem->getTextContent());
                             referenceSystemInfo->scaleFactorAtEquator= atof(projSubNodeValue);
                         }
                         else if((strcmp(projSubNodeName, "heightOfProspectivePointAboveSurface")) == 0)
                         {
                             projSubNodeValue = XMLString::transcode(projSubElem->getTextContent());
                             referenceSystemInfo->heightOfProspectivePointAboveSurface= atof(projSubNodeValue);
                         }
                         else if((strcmp(projSubNodeName, "longitudeOfProjectionCenter")) == 0)
                         {
                             projSubNodeValue = XMLString::transcode(projSubElem->getTextContent());
                             referenceSystemInfo->longitudeOfProjectionCenter= atof(projSubNodeValue);
                         }
                         else if((strcmp(projSubNodeName, "latitudeOfProjectionCenter")) == 0)
                         {
                             projSubNodeValue = XMLString::transcode(projSubElem->getTextContent());
                             referenceSystemInfo->latitudeOfProjectionCenter= atof(projSubNodeValue);
                         }
                         else if((strcmp(projSubNodeName, "scaleFactorAtCenterLine")) == 0)
                         {
                             projSubNodeValue = XMLString::transcode(projSubElem->getTextContent());
                             referenceSystemInfo->scaleFactorAtCenterLine= atof(projSubNodeValue);
                         }
                         else if((strcmp(projSubNodeName, "straightVerticalLongitudeFromPole")) == 0)
                         {
                             projSubNodeValue = XMLString::transcode(projSubElem->getTextContent());
                             referenceSystemInfo->straightVerticalLongitudeFromPole= atof(projSubNodeValue);
                         }
                         else if((strcmp(projSubNodeName, "scaleFactorAtProjectionOrigin")) == 0)
                         {
                             projSubNodeValue = XMLString::transcode(projSubElem->getTextContent());
                             referenceSystemInfo->scaleFactorAtProjectionOrigin= atof(projSubNodeValue);
                         }

                         /* Free the transcoded memory every time we go through this loop. */
                         XMLString::release(&projSubNodeName);
                         XMLString::release(&projSubNodeValue);
                         free(projSubNodeName);
                         free(projSubNodeValue);

   

                     }//end for projection SubNodeList

                 }//end if nodename = projectionParameters



                 else if((strcmp(referenceElemSubNodeName, "projection")) == 0)
                 { 
          

                     /* Get all subnodes of projectionParameters. */
                     DOMNodeList * projSubNodeList = referenceElemSubElem->getElementsByTagName(X("*"));
 


                     /* Iterate through list of sub nodes and populate corresponding structures. */
                     DOMElement * projSubElem;

                     for (XMLSize_t projSubNodeCounter=0; projSubNodeCounter < projSubNodeList->getLength();projSubNodeCounter++)
                     {

                          projSubElem = (DOMElement *)projSubNodeList->item(projSubNodeCounter);

                          NV_CHAR * projSubNodeName = (NV_CHAR *)calloc(100, sizeof(NV_CHAR)); 
                          NV_CHAR * projSubNodeValue = (NV_CHAR *)calloc(100, sizeof(NV_CHAR));
                          projSubNodeName = XMLString::transcode(projSubElem->getTagName());

     

                          if((strcmp(projSubNodeName, "code")) == 0)
                          {
                              projSubNodeValue = XMLString::transcode(projSubElem->getTextContent());
                              strncpy(referenceSystemInfo->projection, projSubNodeValue, 99);
                          }

      
      

                         /* Free the transcoded memory every time we go through this loop. */
                         XMLString::release(&projSubNodeName);
                         XMLString::release(&projSubNodeValue);
                         free(projSubNodeName);
                         free(projSubNodeValue);
                     }
                 }



                 else if((strcmp(referenceElemSubNodeName, "ellipsoidParameters")) == 0)
                 {

      
                     /* Get all subnodes of projectionParameters. */
                     DOMNodeList * ellipseSubNodeList = referenceElemSubElem->getElementsByTagName(X("*"));


                     /* Iterate through list of sub nodes and populate corresponding structures. */
                     DOMElement * ellipseSubElem;

                     for (XMLSize_t ellipseSubNodeCounter=0; ellipseSubNodeCounter < ellipseSubNodeList->getLength();ellipseSubNodeCounter++)
                     {

                         ellipseSubElem = (DOMElement *)ellipseSubNodeList->item(ellipseSubNodeCounter);

                         NV_CHAR * ellipseSubNodeName = (NV_CHAR *)calloc(100,sizeof(NV_CHAR)); 
                         //NV_CHAR * ellipseSubNodeValue = (NV_CHAR *)calloc(100, sizeof(NV_CHAR));
                         NV_CHAR ellipseSubNodeValue[100];
                         ellipseSubNodeName = XMLString::transcode(ellipseSubElem->getTagName());
      


                         /* Populate the semiMajorAxis field. */
                         if((strcmp(ellipseSubNodeName, "semiMajorAxis")) == 0)
                         {

                             GetNodeValue(ellipseSubElem,ellipseSubNodeValue,99 );
                             //ellipseSubNodeValue = XMLString::transcode(ellipseSubElem->getTextContent());
                             referenceSystemInfo->semiMajorAxis= atof(ellipseSubNodeValue);

                         }

                         /* Free the transcoded memory every time we go through this loop. */
                         XMLString::release(&ellipseSubNodeName);
                         free(ellipseSubNodeName);
                  

                     }//end for each ellipsoid subnode


                 }//end else if ellipsoid parameters


                 else if((strcmp(referenceElemSubNodeName, "ellipsoid")) == 0)
                 {

                     /* Get all subnodes of projectionParameters. */
                     DOMNodeList * ellipseSubNodeList = referenceElemSubElem->getElementsByTagName(X("*"));


                     /* Iterate through list of sub nodes and populate corresponding structures. */
                     DOMElement * ellipseSubElem;

                     for (XMLSize_t ellipseSubNodeCounter=0; ellipseSubNodeCounter < ellipseSubNodeList->getLength();ellipseSubNodeCounter++)
                     {

                         ellipseSubElem = (DOMElement *)ellipseSubNodeList->item(ellipseSubNodeCounter);

                         NV_CHAR * ellipseSubNodeName = (NV_CHAR *)calloc(100, sizeof(NV_CHAR)); 
                         NV_CHAR * ellipseSubNodeValue = (NV_CHAR *)calloc(100, sizeof(NV_CHAR));
                         ellipseSubNodeName = XMLString::transcode(ellipseSubElem->getTagName());
     

                         /* Populate the ellipsoid field. */
                         if((strcmp(ellipseSubNodeName, "code")) == 0)
                         {

                             ellipseSubNodeValue = XMLString::transcode(ellipseSubElem->getTextContent());
                             strncpy(referenceSystemInfo->ellipsoid, ellipseSubNodeValue, 99);

                         }


                         /* Free the transcoded memory every time we go through this loop. */
                         XMLString::release(&ellipseSubNodeName);
                         XMLString::release(&ellipseSubNodeValue);
                         free(ellipseSubNodeName);
                         free(ellipseSubNodeValue);
                     }


                 }
                 else if((strcmp(referenceElemSubNodeName, "datum")) == 0)
                 {


                     /* Get all subnodes of projectionParameters. */
                     DOMNodeList * datumSubNodeList = referenceElemSubElem->getElementsByTagName(X("*"));


                     /* Iterate through list of sub nodes and populate corresponding structures. */
                     DOMElement * datumSubElem;

                     for (XMLSize_t m=0; m < datumSubNodeList->getLength();m++)
                     {

                         datumSubElem = (DOMElement *)datumSubNodeList->item(m);

                         NV_CHAR * datumSubNodeName = (NV_CHAR *)calloc(100, sizeof(NV_CHAR)); 
                         NV_CHAR * datumSubNodeValue = (NV_CHAR *)calloc(100, sizeof(NV_CHAR));
                         datumSubNodeName = XMLString::transcode(datumSubElem->getTagName());
     

                         /* Populate the horizontal and vertical datum field. */
                         if((strcmp(datumSubNodeName, "code")) == 0)
                         {

                             datumSubNodeValue = XMLString::transcode(datumSubElem->getTextContent());
 
                     
                             /* If this is the first referenceSystemInfo object, populate the horizontal datum. */
                             if(mdCrsCounter == 0)
                             { 

                                 strncpy(referenceSystemInfo->horizontalDatum, datumSubNodeValue, 99);
                             }
                             else
                             {
                                 strncpy(referenceSystemInfo->verticalDatum, datumSubNodeValue, 99);
                             }

                         }

                         /* Free the transcoded memory every time we go through this loop. */
                         XMLString::release(&datumSubNodeName);
                         XMLString::release(&datumSubNodeValue);
                         free(datumSubNodeName);
                         free(datumSubNodeValue);

                     }//end for item in datumSubNodeList;

                     datumSubNodeList->~DOMNodeList();
                 } 
			 

                 /* Free the transcoded memory every time we go through this loop. */
                 XMLString::release(&nodeName);
                 XMLString::release(&nodeValue);
                 free(nodeName);
                 free(nodeValue);

             } //end for all sub nodes in  referenceElem SubNodesList

        }//end for each MD_CRS element

   
 

    }
    catch (int intException)
    {          
            
        fprintf(stderr,"ERROR: Exception while attempting to set reference system info.  Exception message is: %d\n", intException );
        errorCode = intException;

    }
    catch(...)
    {
        fprintf(stderr, "ERROR: Exception occured while trying to get referenceSystemInfo.\n");
        errorCode =  -20;
    }

    /* Release resources. */
    xmlDoc->release(); 
    TerminateXML();
       
    return errorCode;


}



/******************************************************************************************************
*
* Function: GetLanguage
*
* Purpose:  Populates a string with language information retreived from a BAG metadata XML string.
*
* Inputs:   xmlString - XML string from which to obtain language metadata.
*           language -  character pointer to load with language.
*
* Outputs:  Error messages to stderr.
*
* Returns:  NV_INT16 = Size of language string retrieved. 
*                  -1 = Error initializing Xerces.
*                 -20 = An exception occurred while attempting to get the language value from the given XML string.
*
* Note:     Memory for "language" parameter is allocated in this method and must be freed after the method call by the programmer.
*
*******************************************************************************************************/

NV_INT16  GetLanguage( NV_U_CHAR * xmlString, NV_CHAR ** language )
{

    NV_INT16  languageSize = 0;
    NV_CHAR *  gMemBufId = "metadata";
 

    DOMBuilder *myParser;    /* This will parse the given XML string into a DOM document. */
    DOMDocument * xmlDoc = NULL;

    /* Used in locating and obtaining the value of the language node.*/
    DOMNode * metadataNode = NULL;
    DOMNode * languageNode = NULL;
    NV_CHAR  * nodeName = (NV_CHAR *)calloc(100, sizeof(NV_CHAR));
    NV_CHAR * nodeValue = NULL;


    /* Initialize XERCES. */
    if (!XMLinitialized)
    {
      
   
        try
        {
            XMLPlatformUtils::Initialize();
        }
        catch(const XMLException& toCatch)
        {
            NV_CHAR *pMsg = XMLString::transcode(toCatch.getMessage());
            fprintf(stderr,"ERROR: %s.\n" , pMsg);
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
 
 
    try
    {

        /* Init the parser. */
        myParser = ((DOMImplementation*)impl)->createDOMBuilder(DOMImplementation::MODE_SYNCHRONOUS,NULL);


        /* Set up memory input source (XML string) to be parsed.*/
        //MemBufInputSource* memBufIS = new MemBufInputSource   (  (const XMLByte*)xmlString , strlen(xmlString), gMemBufId, false);
        /* 6.18.2009 - E. Warner - Using NV_U_CHAR for consistency. */
        MemBufInputSource* memBufIS = new MemBufInputSource   (  (const XMLByte*)xmlString , strlen((NV_CHAR *)xmlString), gMemBufId, false);

        Wrapper4InputSource wrap(memBufIS,false);   
        xmlDoc = myParser->parse(wrap);

        memBufIS-> ~MemBufInputSource();


        /* Find the "smXML:MD_Metadata node within the XML document  string.        */
        /* (The "language" tag is in more than one node!  Must parse to exact place.*/
        /* Find node is not guaranteed to find the uppermost language node.         */
        metadataNode =  FindNode(xmlDoc,"smXML:MD_Metadata", NULL, NULL);



        /* Find language node within the "smXML:MD_Metadata" node. */
        DOMNodeList * childNodesList = metadataNode->getChildNodes();
        XMLSize_t length = childNodesList->getLength();


          

        for(XMLSize_t i = 0; i < length; i++)
        {

            languageNode = childNodesList->item(i);
            nodeName =  XMLString::transcode(languageNode->getNodeName());

            if(strcmp(nodeName, "language") == 0)
            {
  

                /* Determine the size of the language value to be retreived. */
                languageSize = strlen(     XMLString::transcode(((DOMElement*)languageNode)->getTextContent())  );


                nodeValue = (NV_CHAR *) calloc(languageSize + 1, sizeof(NV_CHAR));
                *language = (NV_CHAR *) calloc(languageSize + 1, sizeof(NV_CHAR));
  

                /* Get the language node value. */
                GetNodeValue(languageNode, nodeValue, languageSize);
                strncpy(*language, nodeValue,languageSize);


                break;
            }
        }


    }
    catch (int intException)
    {          
            
        fprintf(stderr,"ERROR: An exception occured while trying to get language.  Exception message is: %d\n", intException );
        languageSize = intException;

    }
    catch(...)
    {
        fprintf(stderr, "ERROR: An exception occured while trying to get language.\n");
        languageSize = -20;
    }



    /* Release resources. */
    free(nodeValue);
    languageNode->~DOMNode();
    metadataNode->~DOMNode();
    xmlDoc->release();
    TerminateXML();


    return languageSize;


}


/******************************************************************************************************
*
* Function: GetDataIdentification
*
* Purpose:  Populates an IDENTIFICATION_INFO structure with dataIdentificationInfo information retreived from a BAG metadata XML string.
*
* Inputs:   xmlString - XML string to convert
*           dataIdentificationInfo - pointer to IDENTIFICATION_INFO structure to load with retrieved information..
*
* Outputs:  Error messages to stderr.
*
* Returns:  Status  1 = Successful.   
*                  -1 = Error initializing Xerces.
*                  -3 = Node  smXML:BAG_DataIdentification does not exist.
*                 -20 = An exception occurred while attempting to retrieve the dataIdentification from the given XML string.
*             
* Assumptions: Assumes that only one identificationInfo node exists although the schema allows for an unbounded number.
*
*******************************************************************************************************/
NV_INT16  GetDataIdentification(NV_U_CHAR * xmlString, IDENTIFICATION_INFO * dataIdentificationInfo)
{

    NV_INT16 errorCode = 1;  /* Assume success. */
    NV_CHAR *  gMemBufId = "metadata";
 

    DOMBuilder *myParser;    /* This will parse the given XML string into a DOM document. */
    DOMDocument * xmlDoc = NULL;

  

    /* Initialize XERCES. */
    if (!XMLinitialized)
    {
         
        try
        {
           
            XMLPlatformUtils::Initialize();
        }
        catch(const XMLException& toCatch)
        {
            NV_CHAR *pMsg = XMLString::transcode(toCatch.getMessage());
            fprintf(stderr,"ERROR: %s.\n" , pMsg);
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
   
 

    try
    {

        /* Init the parser. */
        myParser = ((DOMImplementation*)impl)->createDOMBuilder(DOMImplementation::MODE_SYNCHRONOUS,NULL);


        /* Set up memory input source (XML string) to be parsed.*/
        //MemBufInputSource* memBufIS = new MemBufInputSource   (  (const XMLByte*)xmlString , strlen(xmlString), gMemBufId, false);
        /* 6.18.2009 - E. Warner - Using NV_U_CHAR for consistency. */
        MemBufInputSource* memBufIS = new MemBufInputSource   (  (const XMLByte*)xmlString , strlen((NV_CHAR *)xmlString), gMemBufId, false);


        Wrapper4InputSource wrap(memBufIS,false);   
        xmlDoc = myParser->parse(wrap);

        memBufIS-> ~MemBufInputSource();



        /* Locate node for  smXML:BAG_DataIdentification.                             */
        /* There may be more than one of these nodes according to the metdata schema, */
        /* but Navigation Specific requirements limit this to only one.               */
        DOMNode * bagIdentificationNode = FindNode(xmlDoc, "smXML:BAG_DataIdentification", NULL, NULL);

        if(bagIdentificationNode != NULL)
        {

            /* Get list of all subnodes and load into corresponding data structure. */
            DOMNodeList * bagIdentificationSubNodeList =  bagIdentificationNode->getChildNodes();

            XMLSize_t length = bagIdentificationSubNodeList->getLength();
	   

           
            NV_CHAR  * nodeName = (NV_CHAR *)calloc(100, sizeof(NV_CHAR));
            NV_CHAR  nodeValue[100];   /* Must be able to hold longest value that might be encountered. See IDENTIFICATION_INFO structure. */
            DOMElement * identificationElem;

	  

            for (XMLSize_t counter = 0; counter < length; counter++)
            {
	       
                strcpy(nodeValue, "");
	      
                identificationElem = (DOMElement*)bagIdentificationSubNodeList->item(counter);
	      
                nodeName=  XMLString::transcode(identificationElem->getTagName());
                //fprintf(stderr,"nodeName: %s\n", nodeName );
              

                if((strcmp(nodeName, "abstract")) == 0)
                {  

                    NV_CHAR  abstractValue[8000];

                    if(GetNodeValue(identificationElem, abstractValue, 7999 ) == 1)
                    {
                        strncpy(dataIdentificationInfo->abstract, abstractValue, 7999);
                    }
                }
                else if((strcmp(nodeName, "purpose")) == 0)
                {

                    if(GetNodeValue(identificationElem, nodeValue, 99) == 1)
                    {
                        strncpy(dataIdentificationInfo->purpose, nodeValue, 99);
                    }
                }

                /* Per the schema, there may be an unbounded number of status nodes, but we limit it to one. */
                /* If more than one exists, the value in the dataIdentificationInfo will be overwritten each time a status node is found. */
                else if((strcmp(nodeName, "status")) == 0)
                {
                    if(GetNodeValue(identificationElem, nodeValue, 99 ) == 1)
                    {

                        strncpy(dataIdentificationInfo->status, nodeValue, 99);

                    }
                }

                else if((strcmp(nodeName, "verticalUncertaintyType")) == 0)
                {

                    if(GetNodeValue(identificationElem, nodeValue, 39 ) == 1)
                    {
                        strncpy(dataIdentificationInfo->verticalUncertaintyType, nodeValue, 39);
			 
                    }
                }
                else if((strcmp(nodeName, "depthCorrectionType")) == 0)
                {

                    if(GetNodeValue(identificationElem, nodeValue, 31) == 1)
                    {

                        strncpy(dataIdentificationInfo->depthCorrectionType, nodeValue,31);

                    }			
                }

                else if((strcmp(nodeName, "spatialRepresentationType")) == 0)
                {

                    if(GetNodeValue(identificationElem, nodeValue, 99) == 1)
                    {

                        strncpy(dataIdentificationInfo->spatialRepresentationType, nodeValue,99);

                    }
                }
                else if((strcmp(nodeName, "language")) == 0)
                {

                    if(GetNodeValue(identificationElem, nodeValue, 99) == 1)
                    {

                        strncpy(dataIdentificationInfo->language, nodeValue, 99);

                    }
                }

                else if((strcmp(nodeName, "topicCategory")) == 0)
                {

                    if(GetNodeValue(identificationElem, nodeValue, 99) == 1)
                    {

                        strncpy(dataIdentificationInfo->topicCategory, nodeValue, 99);

                    }
                }
 
                else if((strcmp(nodeName, "citation")) == 0)
                {

                    NV_CHAR  * subNodeName;
                    NV_CHAR  subNodeValue[100]; /* Must be as long as the longest value for a field. */
                    DOMElement * subIdElem;
                    NV_INT16 responsiblePartyCounter = 0;



                    DOMNodeList * subNodesList = identificationElem->getElementsByTagName(X("*"));


                    /* Note that the schema does not require the 'citation" node to have any children. */
                    for (XMLSize_t c = 0; c < subNodesList->getLength(); c++)
                    {
       

                        subIdElem = (DOMElement*)subNodesList->item(c);

                        subNodeName =  XMLString::transcode(subIdElem->getTagName());
                        //fprintf(stderr, "found node: %s", subNodeName); 

  
                        if((strcmp(subNodeName, "title")) == 0)
                        {
                            if(GetNodeValue(subIdElem, subNodeValue, 99) == 1)
                            {

                                strncpy(dataIdentificationInfo->title, subNodeValue, 99);
                            }
                        }

                        else if((strcmp(subNodeName, "dateType")) == 0)
                        {
          
                            if(GetNodeValue(subIdElem, subNodeValue, 19) == 1)
                            {

                                strncpy(dataIdentificationInfo->dateType, subNodeValue,19);
                            }
                        }


                        else if((strcmp(subNodeName, "date")) == 0)
                        {

                            /* If parent node is smXML:CI_Date, then this is the correct "date" node from which to get a value. */
                            DOMNode * dateParentNode = subIdElem->getParentNode();
                            NV_CHAR * parentNodeName = XMLString::transcode(dateParentNode->getNodeName());
                           
                            if(strcmp(parentNodeName, "smXML:CI_Date") == 0)
                            {
                                if(GetNodeValue(subIdElem, subNodeValue, 19) == 1)
                                {
                            
                                    strncpy(dataIdentificationInfo->date, subNodeValue, 19);
                                }

                            } 
      
                        }
                        else if((strcmp(subNodeName, "citedResponsibleParty")) == 0)
                        {

                            DOMNodeList * respPartySubNodesList = subIdElem->getElementsByTagName(X("*"));

                            DOMElement  *  tempNode = NULL;
                            NV_CHAR  * tempNodeName;
                            NV_CHAR tempNodeValue[100];  /* Must be as long longest value in RESPONSIBLE_PARTY structure. */


                            for(XMLSize_t r = 0; r < respPartySubNodesList->getLength(); r++)
                            {

                                tempNode = (DOMElement *)respPartySubNodesList->item(r);
                                tempNodeName =  XMLString::transcode(tempNode->getTagName());

                       
                                if(strcmp(tempNodeName, "individualName") == 0)
                                {
			          
                                    if(GetNodeValue(tempNode, tempNodeValue, 99) == 1)
                                    {
				      
                                        strncpy(dataIdentificationInfo->responsibleParties[responsiblePartyCounter].individualName, tempNodeValue, 99);
                                    }

                                }

                                else if(strcmp(tempNodeName, "positionName") == 0)
                                {
 
                                    if(GetNodeValue(tempNode, tempNodeValue, 99) == 1)
                                    {

                                        strncpy(dataIdentificationInfo->responsibleParties[responsiblePartyCounter].positionName, tempNodeValue, 99);
                                    }

                                }
			       

                                else if(strcmp(tempNodeName, "organisationName") == 0)
                                {
 
                                    if(GetNodeValue(tempNode, tempNodeValue, 99) == 1)
                                    {

                                        strncpy(dataIdentificationInfo->responsibleParties[responsiblePartyCounter].organisationName, tempNodeValue, 99);
                                    }

                                }

                                else if(strcmp(tempNodeName, "contactInfo") == 0)
                                {

                                    DOMNode * voiceNode = FindNode(tempNode, "voice", NULL, NULL);

                                    if(voiceNode != NULL)
                                    {

                                        if(GetNodeValue(voiceNode, tempNodeValue, 16) == 1)
                                        {

                                            strncpy(dataIdentificationInfo->responsibleParties[responsiblePartyCounter].phoneNumber, tempNodeValue, 16);
                                        }
                                    }

                                }
                                else if(strcmp(tempNodeName, "role") == 0)
                                {
 
                                    if(GetNodeValue(tempNode, tempNodeValue, 99) == 1)
                                    {
  
                                        strncpy(dataIdentificationInfo->responsibleParties[responsiblePartyCounter].role, tempNodeValue, 99);
                                    }

                                }


                            }//end for respPartySubNodesList

                            responsiblePartyCounter++;

                        }/* end if node = citedResponsibleParty */


                        /* Free the transcode memory. */
                        XMLString::release(&subNodeName);
             
  
                    }//end for each subnode 

                    if(subNodesList->getLength() > 0) free(subNodeName);
		     
                }

                else if((strcmp(nodeName, "extent")) == 0)
                {
 
                    NV_CHAR  * subNodeName;
                    NV_CHAR  subNodeValue[50];
                    DOMElement * subExtentElem;

                    DOMNodeList * subExtentList = identificationElem->getElementsByTagName(X("*"));



                    for (XMLSize_t c = 0; c < subExtentList->getLength(); c++)
                    {
       
                        subExtentElem = (DOMElement*)subExtentList->item(c);
 
                        subNodeName =  XMLString::transcode(subExtentElem->getTagName());
    
                        //fprintf(stderr, "found node: %s", subNodeName); 

                        if((strcmp(subNodeName, "westBoundLongitude")) == 0)
                        {

                            if(GetNodeValue(subExtentElem, subNodeValue, 50) == 1)
                            {

                                dataIdentificationInfo->westBoundingLongitude = atof(subNodeValue);
                            }
                        }

                        else if((strcmp(subNodeName, "eastBoundLongitude")) == 0)
                        {
 
                            if(GetNodeValue(subExtentElem, subNodeValue, 50) == 1)
                            {

                                dataIdentificationInfo->eastBoundingLongitude = atof(subNodeValue);
              
                            }
                        }

                        else if((strcmp(subNodeName, "northBoundLatitude")) == 0)
                        {

                            if(GetNodeValue(subExtentElem, subNodeValue, 50) == 1)
                            {

                                dataIdentificationInfo->northBoundingLatitude = atof(subNodeValue);
        
                            }
                        }

                        else if((strcmp(subNodeName, "southBoundLatitude")) == 0)
                        {

                            if(GetNodeValue(subExtentElem, subNodeValue, 50) == 1)
                            {

                                dataIdentificationInfo->southBoundingLatitude = atof(subNodeValue);
        
                            }
                        }

                        /* Free the transcode memory. */
                        XMLString::release(&subNodeName);
                   
                    }//end for

              
                    free(subNodeName);

                }

            }/* end for each identification sub node*/

	

            /* Free the transcode memory. */
            XMLString::release(&nodeName);
            free(nodeName);

        }/* end if BAG dataIdentifcation  node was found */


        /* Otherwise the node was not found. */
        else
        {

            throw (NV_INT16) -3;
        }
 
      
    }
    catch (int intException)
    {          
            
        fprintf(stderr,"ERROR: An exception occured while trying to get BAG dataIdentification.  Exception message is: %d\n", intException );
        errorCode = intException;

    }
    catch(...)
    {
        fprintf(stderr, "ERROR: An exception occured while trying to get BAG dataIdentification.\n");
        errorCode =  -20;
    }

 
    /* Release resources. */
    xmlDoc->release();
    TerminateXML();

    return errorCode;


}





/******************************************************************************************************
*
* Function: GetSpatialRepresentationInfo
*
* Purpose:  Populates a SPATIAL_INFO structure with spatialRepresentationInfo information retreived from a BAG metadata XML string.
*
* Inputs:   xmlString - XML string to convert
*           spatialRepresentationInfo - pointer to SPATIAL_REPRESENTATION_INFO structure to load with retrieved information.
*
* Outputs:  Error messages to stderr.
*
* Returns:  Status  1 = Successful.
*                  -1 = Error initializing Xerces.
*                  -3 = Node smXML:MD_Georectified does not exist.
*                  -9 = There is a problem with the metadata supplied.  
*                       Specifically the number of axisDimensionProperties does not match the number specified by the numberOfDimensions. 
*                 -20 = An exception occurred while trying to retrieve the spatial representation info from the given XML string.
*
* Assumptions: Assumes that one and only one 'spatialRepresentationInfo" node exists although the schema allows for zero to an unbounded number.
*
*******************************************************************************************************/

NV_INT16  GetSpatialRepresentationInfo(NV_U_CHAR * xmlString, SPATIAL_REPRESENTATION_INFO * spatialRepresentationInfo)
{



    NV_INT16 errorCode = 1         /* Assume success. */;
    NV_CHAR *  gMemBufId = "metadata";
 

    DOMBuilder *myParser;    /* This will parse the given XML string into a DOM document. */
    DOMDocument * xmlDoc = NULL;


    /* Initialize XERCES. */
    if (!XMLinitialized)
    {
        try
        {
            XMLPlatformUtils::Initialize();
        }
        catch(const XMLException& toCatch)
        {
            NV_CHAR *pMsg = XMLString::transcode(toCatch.getMessage());
            fprintf(stderr,"ERROR: %s.\n" , pMsg);
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
 
    try
    {

        /* Init the parser. */
        myParser = ((DOMImplementationLS*)impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS,NULL);



        /* Set up memory input source (XML string) to be parsed.*/
        //MemBufInputSource* memBufIS = new MemBufInputSource   (  (const XMLByte*)xmlString , strlen(xmlString), gMemBufId, false);
        /* 6.18.2009 - E. Warner - Using NV_U_CHAR for consistency. */
        MemBufInputSource* memBufIS = new MemBufInputSource   (  (const XMLByte*)xmlString , strlen((NV_CHAR *)xmlString), gMemBufId, false);

        Wrapper4InputSource wrap(memBufIS,false);   
        xmlDoc = myParser->parse(wrap);
        memBufIS-> ~MemBufInputSource();


  
        /* Beneath each spatialRepresentationInfo node, there is an MD_Georectified node which contains the metadata in which we are interested. */
        /* MD_Georectified does not appear in any node other than the spatialRepresentationInfo node.                                            */
        /* So it is okay to find the MD_Georectified node directly.                                                                              */
        /* Beneath the  MD_Georectified node there should be up to three  axisDimensionProperties nodes. One for row and one for column and depth or z.*/
        DOMNodeList* nodeList = xmlDoc->getElementsByTagName(XMLString::transcode("smXML:MD_Georectified"));


 
        /* If no nodes exist, then return none found */
        if (nodeList->getLength() < 1)
        {

            throw (NV_INT16) -3;
        }




        /* Iterate through list of MD_Georectified elements and populate corresponding structures. */
        DOMElement * geoRectElem;


        for (XMLSize_t geoRectCounter=0; geoRectCounter<nodeList->getLength();geoRectCounter++)
        {

            geoRectElem = (DOMElement *)nodeList->item(geoRectCounter);
    
            NV_CHAR * nodeName = (NV_CHAR *)calloc(100, sizeof(NV_CHAR)); 
       
            nodeName = XMLString::transcode(geoRectElem->getTagName());
        
       

            /* Iterate through sub nodes in each MD_Georectified node. */
            DOMNodeList* geoRectElemSubNodesList = geoRectElem->getChildNodes();


            for (XMLSize_t j=0; j < geoRectElemSubNodesList->getLength();j++)
            {

        
                DOMElement * geoRectElemSubElem = (DOMElement *)geoRectElemSubNodesList->item(j);
                NV_CHAR * geoRectElemSubNodeName  = (NV_CHAR *)calloc(100, sizeof(NV_CHAR)); 
                NV_CHAR * geoRectElemSubNodeValue  = (NV_CHAR *)calloc(100, sizeof(NV_CHAR)); 

                geoRectElemSubNodeName = XMLString::transcode(geoRectElemSubElem->getTagName());
    
                if((strcmp(geoRectElemSubNodeName, "cellGeometry")) == 0)
                {

                    geoRectElemSubNodeValue = XMLString::transcode(geoRectElemSubElem->getTextContent());
                    strncpy(spatialRepresentationInfo->cellGeometry, geoRectElemSubNodeValue, 9);


                }
                else if((strcmp(geoRectElemSubNodeName, "transformationParameterAvailability")) == 0)
                {
                    geoRectElemSubNodeValue = XMLString::transcode(geoRectElemSubElem->getTextContent());
                    strncpy(spatialRepresentationInfo->transformationParameterAvailability,geoRectElemSubNodeValue, 5);

                }
                else if((strcmp(geoRectElemSubNodeName, "checkPointAvailability")) == 0)
                {
                    geoRectElemSubNodeValue = XMLString::transcode(geoRectElemSubElem->getTextContent());
                    strncpy(spatialRepresentationInfo->checkPointAvailability, geoRectElemSubNodeValue, 1);

                }



                /* Determine number of dimensions. */
                else if((strcmp(geoRectElemSubNodeName, "numberOfDimensions")) == 0)
                {

                    geoRectElemSubNodeValue = XMLString::transcode(geoRectElemSubElem->getTextContent());
                    spatialRepresentationInfo->numberOfDimensions= atoi(geoRectElemSubNodeValue);


                    /* Get all the axisDimensionProperties sub nodes that there are to get in the current MD_Georectified node. */
                    DOMNodeList * axisDimensionPropertiesNodes = geoRectElem->getElementsByTagName(XMLString::transcode("axisDimensionProperties"));

                    /* There should be an axisDimensionProperties node for each dimension. */
                    if(axisDimensionPropertiesNodes->getLength() !=  (NV_U_INT16)spatialRepresentationInfo->numberOfDimensions)
                    {

                        throw (NV_INT16)  -9; /* One or more of the dimensions is missing a metadata description. */
                    }

                     DOMElement * axisDimElem;
       



                    /* For each axisDimensionProperties node, get the subnodes dimension name and size. */
                    for(XMLSize_t axisDimNodeCounter=0; axisDimNodeCounter < axisDimensionPropertiesNodes->getLength(); axisDimNodeCounter++)
                    {
         

                        /* Get the  axisDimensionProperties node. */
                        axisDimElem = (DOMElement *)axisDimensionPropertiesNodes->item(axisDimNodeCounter);

                        DOMNodeList * axisDimensionSubNodesList = axisDimElem->getElementsByTagName(X("*"));

                        DOMElement * axisDimSubElem;
                        NV_CHAR * axisDimSubNodeName = (NV_CHAR *)calloc(50, sizeof(NV_CHAR));
                        NV_CHAR * axisDimSubNodeValue= (NV_CHAR *)calloc(21, sizeof(NV_CHAR)); /* For 15 fractional digits in the resolution value. */
     

                        for(XMLSize_t axisDimSubNodeCounter=0; axisDimSubNodeCounter < axisDimensionSubNodesList->getLength();axisDimSubNodeCounter++)
                        {

                           axisDimSubElem = (DOMElement *)axisDimensionSubNodesList->item(axisDimSubNodeCounter);
                           axisDimSubNodeName= XMLString::transcode(axisDimSubElem->getTagName());

                           //fprintf(stderr,"found axis dim sub node: %s\n",axisDimSubNodeName);

                           /* Populate the dimension name field. */
                           if((strcmp(axisDimSubNodeName, "dimensionName")) == 0)
                           {
                               axisDimSubNodeValue = XMLString::transcode(axisDimSubElem->getTextContent());
                               strncpy(spatialRepresentationInfo->dimensionName[axisDimNodeCounter], axisDimSubNodeValue, 19);

                            }

      
                            /* Populate the dimension size field. */
                            else if ((strcmp(axisDimSubNodeName, "dimensionSize")) == 0)
                            {

                                axisDimSubNodeValue = XMLString::transcode( axisDimSubElem->getTextContent());
                                spatialRepresentationInfo->dimensionSize[axisDimNodeCounter] = atoi(axisDimSubNodeValue);
    
                            }


                            /* Populate the the resolution value size field. */
                            else if ((strcmp(axisDimSubNodeName, "smXML:value")) == 0)
                            {

                                axisDimSubNodeValue = XMLString::transcode( axisDimSubElem->getTextContent());
                                spatialRepresentationInfo->resolutionValue[axisDimNodeCounter] = atof(axisDimSubNodeValue);
 
                            }

                        
                        }//end for each axisDimensionProperties sub node

                         /* Release resources. */
                        XMLString::release(&axisDimSubNodeName);
                        XMLString::release(&axisDimSubNodeValue); 

                    }//end for each axisDimensionProperties

                    
                }//end if geoRectElemSubNodeName contains "numberOfDimensions

                else if((strcmp(geoRectElemSubNodeName, "cornerPoints")) == 0)
                {

        
                    /* Get the gml:coordinates node.  There should only be one!. */
                    DOMNodeList * gmlCoordinatesNodesList = geoRectElemSubElem->getElementsByTagName(X("gml:coordinates"));
                    DOMElement * coordinatesElem =  (DOMElement *)gmlCoordinatesNodesList->item(0);


                    NV_CHAR * decimalIndicator = XMLString::transcode(coordinatesElem->getAttribute(X("decimal")));
                    NV_CHAR * csSeperator =  XMLString::transcode(coordinatesElem->getAttribute(X("cs")));
                    NV_CHAR * tsSeperator =  XMLString::transcode(coordinatesElem->getAttribute(X("ts")));


                    NV_CHAR * coordinateString= (NV_CHAR *)calloc(88, sizeof(NV_CHAR));
                    coordinateString = XMLString::transcode(coordinatesElem->getTextContent());
           
                    //fprintf(stderr, "corner pts 4. coordinate string: %s\n",coordinateString );

                    /* Tokenize the string by the ts to get two points. */
                    NV_CHAR * lowerLeftCorner = strtok(coordinateString,tsSeperator);
                    NV_CHAR * upperRightCorner = strtok(NULL,tsSeperator);
                    //fprintf(stderr, "corner pts 5.  Lower left corner: %s\n", lowerLeftCorner);



                    /* Tokenize each of the 2 corner by the cs value. */
                    NV_CHAR * lowerLeftLon = strtok(lowerLeftCorner,csSeperator);
                    NV_CHAR * lowerLeftLat = strtok(NULL,csSeperator);
                    //fprintf(stderr, "corner pts 6: lowerLeftLat:%s ,  lowerleft Lon: %s\n", lowerLeftLat,lowerLeftLon );

                    NV_CHAR * upperRightLon = strtok(upperRightCorner,csSeperator);
                    NV_CHAR * upperRightLat = strtok(NULL,csSeperator);
 

                     /* If the decimal indicator is a decimal point, get the gml points using atof. */
                    if(strcmp(decimalIndicator, ".") == 0)
                    {

                         spatialRepresentationInfo->llCornerX = atof(lowerLeftLon);
                         spatialRepresentationInfo->llCornerY = atof(lowerLeftLat);
                         spatialRepresentationInfo->urCornerX = atof(upperRightLon);
                         spatialRepresentationInfo->urCornerY = atof(upperRightLat);
                    }
                    /* Otherwise, more tokenizing or string parsing is required. TBD*/


                    /* Release resources. */
                    XMLString::release(&decimalIndicator);
                    XMLString::release(&csSeperator);
                    XMLString::release(&tsSeperator);
                    XMLString::release(&coordinateString);

                    free(decimalIndicator);
                    free(csSeperator);
                    free(tsSeperator);
                    free(coordinateString);    
                    
                }//end corner points
                    
            }//end for each georectified sub node

            XMLString::release(&nodeName);
            free(nodeName);
             
        }//end for each MD_Georectified node

        nodeList->~DOMNodeList();


    }
    catch (int intException)
    {          
            
	    fprintf(stderr,"ERROR: Exception while attempting to set reference system info.  Exception message is: %d\n", intException );
        errorCode = intException;

    }
    catch(...)
    {

        fprintf(stderr, "ERROR: Exception occurred while trying to get spatialRepresentationInfo.\n");
        errorCode =  -20;

    }

    /* Release resources. */
    xmlDoc->release(); 
    TerminateXML();

    return errorCode;
}








/******************************************************************************************************
*
* Function: GetMetadataStandardName
*
* Purpose:  Populates a string with metadataStandardName information retreived from a BAG metadata XML string.
*
* Inputs:   xmlString - XML string from which to obtain metadata standard name.
*           metdataStandardName -  character pointer to load with metadata standard name.
*
* Outputs:  Error messages to stderr.
*
* Returns:  +NV_INT16 = Size of metadata standard name string retrieved. 
*                  -1 = Error initializing Xerces.
*                  -20 = An exception occurred  while attempting to retreive metadataStandardNmae from the given XML string.
*
* Note:     Memory for "metdataStandardName" parameter is allocated in this method and must be freed after the method call by the programmer.
*
*******************************************************************************************************/

NV_INT16  GetMetadataStandardName( NV_U_CHAR * xmlString, NV_CHAR ** metadataStandardName)
{

    NV_INT16  nameSize = 0;
    NV_CHAR *  gMemBufId = "metadata";
 

    DOMBuilder *myParser;    /* This will parse the given XML string into a DOM document. */
    DOMDocument * xmlDoc = NULL;



    /* Initialize XERCES. */
    if (!XMLinitialized)
    {
   
        try
        {
            XMLPlatformUtils::Initialize();
        }
        catch(const XMLException& toCatch)
        {
            NV_CHAR *pMsg = XMLString::transcode(toCatch.getMessage());
            fprintf(stderr,"ERROR: %s.\n" , pMsg);
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
 
 
    try
    {

        /* Init the parser. */
        myParser = ((DOMImplementation*)impl)->createDOMBuilder(DOMImplementation::MODE_SYNCHRONOUS,NULL);


        /* Set up memory input source (XML string) to be parsed.*/
        //MemBufInputSource* memBufIS = new MemBufInputSource   (  (const XMLByte*)xmlString , strlen(xmlString), gMemBufId, false);
        /* 6.18.2009 - E. Warner - Using NV_U_CHAR for consistency. */
        MemBufInputSource* memBufIS = new MemBufInputSource   (  (const XMLByte*)xmlString , strlen((NV_CHAR *)xmlString), gMemBufId, false);

        Wrapper4InputSource wrap(memBufIS,false);   
        xmlDoc = myParser->parse(wrap);

        memBufIS-> ~MemBufInputSource();


        /* Find the "smXML:MD_Metadata node within the XML document  string..*/

        DOMNode * metadataNode =  FindNode(xmlDoc,"smXML:MD_Metadata", NULL, NULL);
     

        /* Find metadataStandardName node within the "smXML:MD_Metadata" node. */
        DOMNode * nameNode =  FindNode((DOMElement *)metadataNode,"metadataStandardName", NULL, NULL);


  

        /* Determine the size of the metadataStandardName value to be retreived. */
        nameSize = strlen(     XMLString::transcode(((DOMElement*)nameNode)->getTextContent())                               );



        NV_CHAR * nodeValue = (NV_CHAR *) calloc(nameSize + 1, sizeof(NV_CHAR));
        *metadataStandardName = (NV_CHAR *) calloc(nameSize + 1, sizeof(NV_CHAR));
  

        /* Get the language node value. */
        GetNodeValue(nameNode, nodeValue, nameSize);
        strncpy(*metadataStandardName, nodeValue,nameSize);


        /* Release resources. */
        free(nodeValue);
        nameNode->~DOMNode();
        metadataNode->~DOMNode();


    }
    catch (int intException)
    {          
            
        fprintf(stderr,"ERROR: Exception while attempting to set reference system info.  Exception message is: %d\n", intException );
        nameSize = intException;

    }
    catch(...)
    {
        fprintf(stderr, "ERROR: Exception occured while trying to get metadata standard name.\n");
        nameSize =  -20;
    }


    /* Release resources. */
    xmlDoc->release(); 
    TerminateXML();

    return nameSize;


}






/******************************************************************************************************
*
* Function: GetMetadataStandardVersion
*
* Purpose:  Populates a string with metadataStandardVersion information retreived from a BAG metadata XML string.
*
* Inputs:   xmlString - XML string from which to obtain  metadata standard version.
*           metadataStandardVersion -  character pointer to load with metadata standard version.
*
* Outputs:  Error messages to stderr.
*
* Returns:  NV_INT16 = Size of metadata standard version string retrieved. 
*                 -1 = Error initializing Xerces.
*                -20 = An exception occurred while attempting to retreive metadataStandardVersion from the given XML string.
*
* Note:     Memory for "metdataStandardVersion" parameter is allocated in this method and must be freed after the method call by the programmer.
*
*******************************************************************************************************/

NV_INT16  GetMetadataStandardVersion( NV_U_CHAR * xmlString, NV_CHAR ** metadataStandardVersion)
{

    NV_INT16   versionSize = 0;
    NV_CHAR *  gMemBufId = "metadata";
 

    DOMBuilder *myParser;    /* This will parse the given XML string into a DOM document. */
    DOMDocument * xmlDoc = NULL;



    /* Initialize XERCES. */
    if (!XMLinitialized)
    {
      
   
        try
        {
            XMLPlatformUtils::Initialize();
        }
        catch(const XMLException& toCatch)
        {
            NV_CHAR *pMsg = XMLString::transcode(toCatch.getMessage());
            fprintf(stderr,"ERROR: %s.\n" , pMsg);
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
 
 
    try
    {

        /* Init the parser. */
        myParser = ((DOMImplementation*)impl)->createDOMBuilder(DOMImplementation::MODE_SYNCHRONOUS,NULL);


        /* Set up memory input source (XML string) to be parsed.*/
        //MemBufInputSource* memBufIS = new MemBufInputSource   (  (const XMLByte*)xmlString , strlen(xmlString), gMemBufId, false);
        /* 6.18.2009 - E. Warner - Using NV_U_CHAR for consistency. */
        MemBufInputSource* memBufIS = new MemBufInputSource   (  (const XMLByte*)xmlString , strlen((NV_CHAR *)xmlString), gMemBufId, false);

        Wrapper4InputSource wrap(memBufIS,false);   
        xmlDoc = myParser->parse(wrap);

        memBufIS-> ~MemBufInputSource();


        /* Find the "smXML:MD_Metadata node within the XML document  string..*/

        DOMNode * metadataNode =  FindNode(xmlDoc,"smXML:MD_Metadata", NULL, NULL);
     

        /* Find metadataStandardVersion node within the "smXML:MD_Metadata" node. */
        DOMNode * versionNode =  FindNode((DOMElement *)metadataNode,"metadataStandardVersion", NULL, NULL);


  

        /* Determine the size of the metadata standard version  value to be retreived. */
        versionSize = strlen(     XMLString::transcode(((DOMElement*)versionNode)->getTextContent()) );



        NV_CHAR * nodeValue = (NV_CHAR *) calloc(versionSize + 1, sizeof(NV_CHAR));
        *metadataStandardVersion = (NV_CHAR *) calloc(versionSize + 1, sizeof(NV_CHAR));
  

        /* Get the language node value. */
        GetNodeValue(versionNode, nodeValue, versionSize);
        strncpy(*metadataStandardVersion, nodeValue,versionSize);


        /* Release resources. */
        free(nodeValue);
        versionNode->~DOMNode();
        metadataNode->~DOMNode();


    }
    catch (int intException)
    {          
            
        fprintf(stderr,"ERROR: Exception occured while trying to get metadata standard version.  Exception message is: %d\n", intException );
        versionSize = intException;

    }
    catch(...)
    {
        fprintf(stderr, "ERROR: Exception occured while trying to get metadata standard version.\n");
        versionSize =  -20;
    }

    /*Release resources. */
    xmlDoc->release();
    TerminateXML();

    return versionSize;


}


/******************************************************************************************************
*
* Function: GetAllStructures
*
* Purpose:  Populates IDENTIFICATION_INFO,MD_LEGAL_CONSTRAINTS,MD_SECURITY_CONSTRAINTS,DATA_QUALITY_INFO,SPATIAL_REPRESENTATION_INFO, REFERENCE_SYSTEM_INFO and RESPONSIBLE_PARTY strucutres with corresponding information retreived from a BAG metaadata XML string
*
* Inputs:   xmlString - XML string to convert
*           identificationInfo - pointer to IDENTIFICATION_INFO structure to load with retrieved information.
*           legalConstraints - pointer to MD_LEGAL_CONSTRAINTS structure to load with retrieved information.
*           securityConstraints - pointer to MD_SECURITY_CONSTRAINTS structure to load with retrieved information.
*           dataQuality - 
*           spatialRepresentationInfo - pointer to SPATIAL_REPRESENTATION_INFO structure to load with retrieved information.
*           referenceSystemInfo - pointer to REFERENCE_SYSTEM_INFO structure to load with retrieved information.
*           contact - pointer to RESPONSIBLE_PARTY structure to load with retrieved information.
*
*
* Outputs:  Error messages to stderr.
*
* Returns:  Status 1 = Successful.  
*                 -1 = Not successful 
*
* Notes:
*         After calling this method and when the the program has completed using the dataQualityInfo, the programmer must 
*         call freeDataQualityInfo to release the memory that was dynamically allocated for the dataQualityInfo structure.
*             
*
*******************************************************************************************************/

NV_INT16  GetAllStructures( NV_U_CHAR * xmlString,IDENTIFICATION_INFO * identificationInfo, MD_LEGAL_CONSTRAINTS * legalConstraints, MD_SECURITY_CONSTRAINTS * securityConstraints,  DATA_QUALITY_INFO * dataQuality, SPATIAL_REPRESENTATION_INFO * spatialRepresentationInfo, REFERENCE_SYSTEM_INFO * referenceSystemInfo, RESPONSIBLE_PARTY * contact )
{

    NV_INT16 status = 1; /* Assume success. */
  
    try
    {

        fprintf(stderr,"Getting data identification.\n");
        GetDataIdentification(xmlString, identificationInfo);

        fprintf(stderr,"Getting legal constraints.\n");
        GetLegalConstraints(xmlString, legalConstraints);
 
        fprintf(stderr,"Getting security constraints.\n");
        GetSecurityConstraints(xmlString, securityConstraints);
 
        fprintf(stderr,"Getting data quality.\n");
        GetDataQualityInfo((NV_U_CHAR *)xmlString, dataQuality );
 
        fprintf(stderr,"Getting reference system info.\n");
        GetReferenceSystemInfo(xmlString, referenceSystemInfo);

        fprintf(stderr,"Getting spatial representation info.\n");
        GetSpatialRepresentationInfo(xmlString, spatialRepresentationInfo);
 
        fprintf(stderr,"Getting contact.\n");
        GetContact(xmlString, contact);

    }

    catch(...)
    {

        fprintf(stderr,"ERROR: Getting all structures.\n");
        status = -1;
        
    }
 
  
    return status;

}





/******************************************************************************************************
*
* Function:  createXmlMetadataString

*
* Purpose:   Converts BAG metadata structures to a well-formed, valid BAG XML metadata string.
*
* Inputs:    identificationInfo - populated structure whose contents will be used to populate "identificationInfo" node in XML metdata string.
*            legalConstraints - populated structure whose contents will be used to populate "MD_LegalConstraints" node in XML metdata string.
*            securityConstraints - populated structure whose contents will be used to populate "MD_SecurityConstraints" node in XML metdata string.
*            dataQuality - populated structure whose contents will be used to populate "dataQualityInfo" node in XML metdata string.
*            spatialRepresentationInfo - populated structure whose contents will be used to populate "spatialRepresentationInfo" node in XML metdata string.
*            referenceSystemInfo - populated structure whose contents will be used to populate "referenceSystemInfo" node in XML metdata string.
*            contact - populated structure whose contents will be used to populate "contact" nod in XML metdata string.
*            creationDate - populated string  whose contents will be used to populate "dateStamp" in XML metdata string.
*            xmlString - address of the XML string to populate.

* Outputs:   Error messages/status messages to stderr.
*
* Returns:   NV_INT32 = Length of XML string created (if successful). 
*                  -1 = Error initializing Xerces.
*                  -2 = OutOfMemoryException
*                  -3 = A DOMException occurred.
*                  -4 = DOM implementation is not supported.
*                 -10 = Document could not be made because security constraints were not specified. (Navigation specific)
*                 -11 = Document could not be made because identificationInfo node could not be created.(Required by ISO 19139)
*                 -12 = Document could not be made because contact node could not be created.(Required by ISO 19139)
*                 -13 = Document could not be made because dataQualityInfo could not be created. (Required Navigation specific)
*                 -14 = Document could not be made because spatialReferenceInfo could not be created. (Required Navigation specific)
*                 -15 = Document could not be made because referenceSystemInfo could not be created. (Required Navigation specific)
*                 -16 = Document could not be made because language node could not be created. (Required Navigation specific)
*                 -17 = Document could not be made because contact node could not be created. (Required by ISO 19139)
*                 -18 = Document could not be made because dateStamp node could not be created. (Required Navigation specific)
*                 -19 = Document could not be made because metadataStandardName or Version could not be created. (Navigation specific)
*                 -20 = An exception occurred while attempting to create XML document.
*
* Assumptions: Assumes one "dataIdentificationInfo" node, one "spatialRepresentationInfoNode", one "referenceSystemInfo" node, one "contact" node.
*              Assumes one metadataConstraints node for LegalConstraints and one metadataConstraints node for SecurityConstraints.
*              Assumes a creationDate is supplied.    
*
* Notes:       Creates nodes "language" as en(English),  "metadataStandardName" as ISO 19139 and "metadataStandardVersion" as  1.1.0.  XML encoding attribute is set as  UTF-8.  
*              Memory for xmlString is allocated within this method and must freed by the calling program after the method call.         
*
*******************************************************************************************************/

NV_INT32  CreateXmlMetadataString( IDENTIFICATION_INFO identificationInfo, MD_LEGAL_CONSTRAINTS legalConstraints, MD_SECURITY_CONSTRAINTS securityConstraints,  DATA_QUALITY_INFO dataQuality, SPATIAL_REPRESENTATION_INFO spatialRepresentationInfo, REFERENCE_SYSTEM_INFO referenceSystemInfo,  RESPONSIBLE_PARTY contact, NV_CHAR * creationDate, NV_U_CHAR ** xmlString)
{

    NV_INT16 xmlStringLength = 0; 
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
            NV_CHAR *pMsg = XMLString::transcode(toCatch.getMessage());
  
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
                                       X("http://metadata.dgiwg.org/smXML"),                     // root element namespace URI.
                                       X("smXML:MD_Metadata"),         // root element name
                                                0);                    // document type object (DTD).
     

            //1.5.2009 - E. Warner - Set encoding to UTF-8
            XMLCh *temp = XMLString::transcode("UTF-8");
            doc->setEncoding(temp);
            XMLString::release(&temp);



            /* If security constraints are not specified, do not create document. */
            /* Required navigation specific.                                      */
            DOMElement * securityConstraintsElem;
            securityConstraintsElem = createMD_SecurityConstraintsNode(securityConstraints, doc);
            if(securityConstraintsElem == NULL)
            {

                throw (NV_INT16) -10;
	    }




            DOMElement* rootElem = doc->getDocumentElement();
    


            /* Append a smXML:MD_Metadata  node to the metadata node. */
            //DOMElement*  metadataElem = doc->createElement(X("smXML:MD_Metadata"));

     
            //SetAttribute(rootElem,"xmlns:smXML","http://metadata.dgiwg.org/smXML");
            SetAttribute(rootElem,"xmlns:scXML", "http://metadata.dgiwg.org/smXML");
            SetAttribute(rootElem, "xmlns:xlink", "http://www.w3.org/1999/xlink");
            SetAttribute(rootElem,"xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
            SetAttribute(rootElem, "xmlns:gml", "http://www.opengis.net/gml");




            /* Append a identificationInfo node to the smXML:MD_Metadata node. */
            //  fprintf(stderr, "Creating identificationInfo\n");


            DOMElement * bagDataIdentificationElem;
            bagDataIdentificationElem = createBAG_DataIdentificationNode(identificationInfo, doc);

            if(bagDataIdentificationElem != NULL)
            {

                DOMElement*  identificationInfoElem = doc->createElement(X("identificationInfo"));
                identificationInfoElem->appendChild(bagDataIdentificationElem);
                rootElem->appendChild(identificationInfoElem);

            }
            /* Otherwise don't make the document. Required by ISO 19139. */
            else
            {

                throw (NV_INT16) -11;
                
            }
	   
           

            /* Append a metadataContraints node to the metadata node. Required navigation specific for BAG.*/
            //  fprintf(stderr, "Creating MD_LegalConstraints\n");


            DOMElement * legalConstraintsElem;
            legalConstraintsElem = createMD_LegalConstraintsNode(legalConstraints, doc);


            if(legalConstraintsElem != NULL)
            {

                DOMElement*  metadataConstraintsElem = doc->createElement(X("metadataConstraints"));
                metadataConstraintsElem->appendChild(legalConstraintsElem);
                rootElem->appendChild(metadataConstraintsElem);
            }
            else
            {

                throw (NV_INT16) -12;
            }
 
	   	  

            /* Append a second metadataContraints node to the metadata node. this one contains security metadata.*/
            // fprintf(stderr, "Creating MD_SecurityConstraints\n");

            DOMElement*  metadataConstraintsElem2 = doc->createElement(X("metadataConstraints"));
            metadataConstraintsElem2->appendChild(securityConstraintsElem);
            rootElem->appendChild(metadataConstraintsElem2);

   
       
	 
            /* Append a dataQualityInfo node to the metadata node.Required navigation specific for BAG. */
            //  fprintf(stderr, "Creating dataQualityInfo\n");


            DOMElement * dqDataQualityElem;
            dqDataQualityElem = createDQ_DataQualityNode(dataQuality, doc);
          

            if(dqDataQualityElem != NULL)
            {

                DOMElement*  dataQualityElem = doc->createElement(X("dataQualityInfo"));
                dataQualityElem->appendChild(dqDataQualityElem);
                rootElem->appendChild(dataQualityElem);
            }
            else
            {
                throw (NV_INT16) -13;
            }

	 

            /* Append a spatialRepresentationInfo node to the metadata node. Required navigation specific for BAG */
            //  fprintf(stderr, "Creating spatialRepresentationInfo\n");

  
            DOMElement * georectifiedElem;
            georectifiedElem = createMD_GeorectifiedNode(spatialRepresentationInfo, doc);
          

            if(georectifiedElem != NULL)
            {
                DOMElement*  spatialRepresentationElem = doc->createElement(X("spatialRepresentationInfo"));
                spatialRepresentationElem->appendChild(georectifiedElem);
                rootElem->appendChild(spatialRepresentationElem);
            }
            else
            {
                
                throw (NV_INT16) -14;
            }

	

            /* Append a  referenceSystemInfo node to the metadata node. Required navigation specific for BAG.*/
            // fprintf(stderr, "Creating referenceSystemInfo\n");


            DOMElement * crsElem;
            crsElem = createMD_CRSNode(referenceSystemInfo, doc);
           

            if(crsElem != NULL)
            {
                DOMElement*  referenceSystemElem = doc->createElement(X("referenceSystemInfo"));
                referenceSystemElem->appendChild(crsElem);
                rootElem->appendChild(referenceSystemElem);

            }
            else
            {

                throw (NV_INT16) -15;

            }

	 

            /* Append another referenceSystemInfo node to the metadata node for vertical datum. */
            //  fprintf(stderr, "Creating referenceSystemInfo vertical datum\n");

        
            DOMElement * crsElem2;
            crsElem2 = createMD_CRSNode(referenceSystemInfo.verticalDatum, doc);



            if(crsElem2 != NULL)
            {
           
                DOMElement*  referenceSystemElem2 = doc->createElement(X("referenceSystemInfo"));
                referenceSystemElem2->appendChild(crsElem2);
                rootElem->appendChild(referenceSystemElem2);

            }
            else
            {

                throw (NV_INT16) -15;
            }


            /* Append the creation date node to the metadata node. Required per ISO 19139.*/
            //  fprintf(stderr, "Creating language.\n");

            DOMElement*  languageElem = createElement("language", "en", doc);


            if(languageElem != NULL)
            {
                rootElem->appendChild(languageElem);
            }
            else
            {

                throw (NV_INT16) -16;
            }
	 
  

	

            /* Append the contact node to the metadata node. Required per ISO 19139. */
            //  fprintf(stderr, "Creating contact.\n");


            DOMElement * responsiblePartyElem;
            responsiblePartyElem = createCI_ResponsiblePartyNode(contact, doc);
        
            if(responsiblePartyElem != NULL)
            {
                DOMElement*  contactElem = doc->createElement(X("contact"));
                contactElem->appendChild(responsiblePartyElem);
                rootElem->appendChild(contactElem);
            }
            else
            {

                throw (NV_INT16) -17;
            }
	 


  
            /* Append the creation date node to the metadata node. Required per ISO 19139.*/
            //fprintf(stderr, "Creating dateStamp\n");

            DOMElement*  dateStampElem = createElement("dateStamp", creationDate, doc);


            if(dateStampElem != NULL)
            {
                rootElem->appendChild(dateStampElem);
            }
            else
            {

                throw (NV_INT16) -18;
            }
	 
  


            /* Append the metadataStandardName node to the metadata node. Required navigation specific.*/
            DOMElement*  metadataStandardNameElem = createElement("metadataStandardName", "ISO 19139", doc);


            if(metadataStandardNameElem != NULL)
            {
                rootElem->appendChild(metadataStandardNameElem);
            }
            else
            {

                throw (NV_INT16) -19;
            }


            /* Append the metadataStandardName node to the metadata node. Required navigation specific */
            DOMElement*  metadataStandardVersionElem = createElement("metadataStandardVersion", "1.1.0", doc);
  

            if(metadataStandardVersionElem != NULL)
            {
                 rootElem->appendChild(metadataStandardVersionElem); 
            }
            else
            {

                throw (NV_INT16) -19;
            }




		 
            DOMWriter *theSerializer = ((DOMImplementationLS*)impl)->createDOMWriter();
		  
 
            /*1.5.2009 - E. Warner - Set encoding to UTF-8. */
            /* Write the document to a string.  Done this way because must use writeNode in order for setEncoding method to work.*/
            MemBufFormatTarget *myFormTarget;
            myFormTarget = new MemBufFormatTarget();
            bool wroteBuffer = theSerializer->writeNode(myFormTarget,*doc);

            if(wroteBuffer)
            {

                xmlStringLength=myFormTarget->getLen();

                //xmlStringLength = XMLString::stringLen(myFormTarget->getRawBuffer()));

                *xmlString = (NV_U_CHAR *)calloc(xmlStringLength, sizeof(NV_U_CHAR));

                *xmlString = (NV_U_CHAR *)myFormTarget->getRawBuffer();
            }
            else
            {
                throw (NV_CHAR *)"Could not write buffer. ";

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

    }  // (impl != NULL)
    else
    {

        fprintf(stderr, "ERROR: Requested implementation is not supported\n");
        xmlStringLength = -4;
    }




    /* Release resources. */
    //XMLString::release(&xmlBody);

    doc->release();
    TerminateXML();

    return xmlStringLength;

       
}










/******************************************************************************************************
*
* Function: PrintIdentificationInfo
*
* Purpose:  Prints to stderr the IDENTIFICATION_INFO structure.
*             
*
*******************************************************************************************************/
void PrintIdentificationInfo(IDENTIFICATION_INFO identificationInfo){

    NV_INT32 i;

    fprintf(stderr,"\nIDENTIFICATION INFO:\n");

    fprintf(stderr,"citation title: %s\n", identificationInfo.title);
    fprintf(stderr,"citation date: %s\n", identificationInfo.date);
    fprintf(stderr,"citation dateType: %s\n",identificationInfo.dateType);
  
    i = 0;

    while(i < MAX_CI_RESPONSIBLE_PARTIES && strcmp(identificationInfo.responsibleParties[i].individualName, "") != 0)
    {

        fprintf(stderr,"\nResponsible Party %d:\n", i + 1);


        fprintf(stderr,"individualName: %s\n", identificationInfo.responsibleParties[i].individualName);
        fprintf(stderr,"positionName: %s\n", identificationInfo.responsibleParties[i].positionName);
        fprintf(stderr,"organisationName: %s\n", identificationInfo.responsibleParties[i].organisationName);
        fprintf(stderr,"role: %s\n", identificationInfo.responsibleParties[i].role);

        i++;
    }

    fprintf(stderr,"\n");


    fprintf(stderr,"abstract: %s\n", identificationInfo.abstract);
    fprintf(stderr,"purpose: %s\n", identificationInfo.purpose);
    fprintf(stderr,"status: %s\n", identificationInfo.status);
    fprintf(stderr,"spatialRepresentationType: %s\n", identificationInfo.spatialRepresentationType);
    fprintf(stderr,"language: %s\n", identificationInfo.language);
    fprintf(stderr,"topicCategory: %s\n", identificationInfo.topicCategory);

    fprintf(stderr,"westBoundingLongitude: %lf\n",  identificationInfo.westBoundingLongitude);
    fprintf(stderr,"eastBoundingLongitude: %lf\n",  identificationInfo.eastBoundingLongitude);
    fprintf(stderr,"southBoundingLatitude: %lf\n",  identificationInfo.southBoundingLatitude);
    fprintf(stderr,"northBoundingLatitude: %lf\n",  identificationInfo.northBoundingLatitude);
 
    fprintf(stderr,"verticalUncertaintyType: %s\n", identificationInfo.verticalUncertaintyType);
    fprintf(stderr,"depthCorrectionType: %s\n",   identificationInfo.depthCorrectionType); 
	
 
}



/******************************************************************************************************
*
* Function: PrintLegalConstraint
*
* Purpose:  Prints to stderr the MD_LEGAL_CONSTRAINT structure.
*             
*
*******************************************************************************************************/
void PrintLegalConstraints(MD_LEGAL_CONSTRAINTS legalConstraints){

    fprintf(stderr,"\nLEGAL CONSTRAINTS:\n");
    fprintf(stderr,"useConstraint: %s\n", legalConstraints.useConstraints);
    fprintf(stderr,"otherConstraint: %s\n", legalConstraints.otherConstraints);
}



/******************************************************************************************************
*
* Function: PrintSecurityConstraint
*
* Purpose:  Prints to stderr the MD_SECURITY_CONSTRAINT structure.
*             
*
*******************************************************************************************************/
void PrintSecurityConstraints(MD_SECURITY_CONSTRAINTS securityConstraints)  {

    fprintf(stderr,"\nSECURITY CONSTRAINTS:\n");
    fprintf(stderr,"classification: %s\n", securityConstraints.classification);
    fprintf(stderr,"userNote: %s\n", securityConstraints.userNote);
}



/******************************************************************************************************
*
* Function: PrintDataQuality
*
* Purpose:  Prints to stderr the DATA_QUALITY_INFO structure.
*             
*
*******************************************************************************************************/
void PrintDataQuality(DATA_QUALITY_INFO dataQuality){


    NV_INT16 i = 0;
    NV_INT16 j = 0;



    fprintf(stderr,"\nDATA QUALITY INFO:\n");
    fprintf(stderr,"scope: %s\n", dataQuality.scope);

  

    /* Print the sources. */
    for(i = 0; i < dataQuality.numberOfSources; i++)
    { 
      
        fprintf(stderr, "\nLineage source %d:\n", i) ;
        fprintf(stderr,"description: %s\n", dataQuality.lineageSources[i].description);
    	fprintf(stderr,"citation title: %s\n", dataQuality.lineageSources[i].title);
        fprintf(stderr,"citation date: %s\n", dataQuality.lineageSources[i].date);
        fprintf(stderr,"citation dateType: %s\n", dataQuality.lineageSources[i].dateType);

        j =0;

        /* Print the cited parties for each source. */
        while(j < MAX_CI_RESPONSIBLE_PARTIES && strcmp(dataQuality.lineageSources[i].responsibleParties[j].role, "") != 0 )
        {
            fprintf(stderr,"citation responsible party %d:\n", j);
            PrintContact(dataQuality.lineageSources[i].responsibleParties[j]);
            j++;

        }

    }



    /* Print the process steps. */
    i = 0;
  
    for(i = 0; i < dataQuality.numberOfProcessSteps; i++ )
    {


        fprintf(stderr, "\nLineage process step %d:\n", i) ;
        fprintf(stderr, "description: %s\n", dataQuality.lineageProcessSteps[i].description);
        fprintf(stderr, "dateTime: %s\n", dataQuality.lineageProcessSteps[i].dateTime);
        fprintf(stderr, "trackingId: %s\n", dataQuality.lineageProcessSteps[i].trackingId);

        j = 0;

        /* Print the cited parties for each source. */
        while(j < MAX_CI_RESPONSIBLE_PARTIES && strcmp(dataQuality.lineageProcessSteps[i].processors[j].role, "") != 0 )
        {

            fprintf(stderr,"\nProcessor %d:\n", j);
            PrintContact(dataQuality.lineageProcessSteps[i].processors[j]);
            j++;

        }
    }


}




/******************************************************************************************************
*
* Function: PrintSpatialRepresentationInfo
*
* Purpose:  Prints to stderr the SPATIAL_REPRESENTATION_INFO structure.
*             
*
*******************************************************************************************************/
void PrintSpatialRepresentationInfo(SPATIAL_REPRESENTATION_INFO spatialRepresentationInfo){

    NV_INT16 i;

    fprintf(stderr,"\nSPATIAL REPRESENTATION INFO:\n");
    fprintf(stderr,"Number of dimensions: %d\n", spatialRepresentationInfo.numberOfDimensions);

    for(i = 0; i < spatialRepresentationInfo.numberOfDimensions; i++ )
    {

        fprintf(stderr,"dimension name: %s\t dimension size: %d\n", spatialRepresentationInfo.dimensionName[i], spatialRepresentationInfo.dimensionSize[i]);

        fprintf(stderr,"resolution: %.15f\n", spatialRepresentationInfo.resolutionValue[i]);

  
    }

    fprintf(stderr,"\n");

    fprintf(stderr,"cellGeometry:%s \n", spatialRepresentationInfo.cellGeometry);
    fprintf(stderr,"transformationParameterAvailability: %s\n",  spatialRepresentationInfo.transformationParameterAvailability);
    fprintf(stderr,"checkPointAvailability: %s\n", spatialRepresentationInfo.checkPointAvailability);

    fprintf(stderr,"lower left longitude: %.12lf\n", spatialRepresentationInfo.llCornerX);
    fprintf(stderr,"lower left latitude: %.12lf\n", spatialRepresentationInfo.llCornerY);
    fprintf(stderr,"upper right longitude: %.12lf\n", spatialRepresentationInfo.urCornerX);
    fprintf(stderr,"upper right latitude: %.12lf\n", spatialRepresentationInfo.urCornerY);

}




/******************************************************************************************************
*
* Function: PrintReferenceSystemInfo
*
* Purpose:  Prints to stderr the REFERENCE_SYSTEM_INFO structure.
*             
*
*******************************************************************************************************/
void PrintReferenceSystemInfo(REFERENCE_SYSTEM_INFO referenceSystemInfo){

    fprintf(stderr,"\nREFERENCE SYSTEM INFO:\n");
    fprintf(stderr,"projection: %s\n", referenceSystemInfo.projection);
    fprintf(stderr,"ellipsoid: %s\n", referenceSystemInfo.ellipsoid);
    fprintf(stderr,"horizontalDatum: %s\n", referenceSystemInfo.horizontalDatum);
    fprintf(stderr,"zone: %d\n", referenceSystemInfo.zone);
    fprintf(stderr,"standard parallel: %lf\n", referenceSystemInfo.standardParallel);
    fprintf(stderr,"longitudeOfCentralMeridian: %lf\n", referenceSystemInfo.longitudeOfCentralMeridian);
    fprintf(stderr,"latitudeOfProjectionOrigin: %lf\n", referenceSystemInfo.latitudeOfProjectionOrigin);
    fprintf(stderr,"falseEasting: %.12lf\n", referenceSystemInfo.falseEasting);
    fprintf(stderr,"falseNorthing: %.12lf\n", referenceSystemInfo.falseNorthing);
    fprintf(stderr,"scaleFactorAtEquator: %lf\n", referenceSystemInfo.scaleFactorAtEquator);
    fprintf(stderr,"heightOfProspectivePointAboveSurface: %lf\n", referenceSystemInfo.heightOfProspectivePointAboveSurface);
    fprintf(stderr,"longitudeOfProjectionCenter: %lf\n", referenceSystemInfo.longitudeOfProjectionCenter);
    fprintf(stderr,"latitudeOfProjectionCenter: %lf\n", referenceSystemInfo.latitudeOfProjectionCenter);
    fprintf(stderr,"scaleFactorAtCenterLine: %lf\n", referenceSystemInfo.scaleFactorAtCenterLine);
    fprintf(stderr,"straightVerticalLongitudeFromPole: %lf\n", referenceSystemInfo.straightVerticalLongitudeFromPole);
    fprintf(stderr,"scaleFactorAtProjectionOrigin: %lf\n", referenceSystemInfo.scaleFactorAtProjectionOrigin);
    fprintf(stderr,"semiMajorAxis: %lf\n", referenceSystemInfo.semiMajorAxis);
    fprintf(stderr,"verticalDatum: %s\n", referenceSystemInfo.verticalDatum);

}



/******************************************************************************************************
*
* Function: PrintContact
*
* Purpose:  Prints to stderr the RESPONSIBLE_PARTY structure.
*             
*
*******************************************************************************************************/
void PrintContact(RESPONSIBLE_PARTY contact){

    fprintf(stderr,"\nCONTACT:\n");
    fprintf(stderr,"Individual: %s\n", contact.individualName);
    fprintf(stderr,"organisationName: %s\n", contact.organisationName);
    fprintf(stderr,"positionName: %s\n", contact.positionName);
    fprintf(stderr,"role: %s\n", contact.role);
}









/* *****************************   BELOW ARE LOCALLY CALLED METHODS.which need clean up & documentation****************************** */




/******************************************************************************************************
*
* Function: IsXMLInitialized
*
* Purpose: This function is used to determine if the Xerces XML engine is intialized or not. 
*
* Inputs: None
*
* Outputs: None
*
* Returns: Status indicating the status of the XML initialization.
*              1 - Yes, the Xerces XML engine is initialized. 
*              0 - No, the Xerces XML engine is not initialized.
*
*******************************************************************************************************/
NV_INT16 IsXMLInitialized(void)
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





/******************************************************************************************************
*
* Function: TerminateXML
*
* Purpose: This function shutsdown the Xerces XML engine releasing all resources back to the system.
*
* Inputs: None
*
* Outputs: None 
*
* Returns: Status 1 = success
*                -1 = Xerces was not initialized, so no need to stop.
*
*******************************************************************************************************/

NV_INT16 TerminateXML()
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





/************************************************************************************************
*
* Function: createCI_ResponsiblePartyNode
*
* Purpose:  This function creates and populates a DOMElement named smXML:CI_ResponsibleParty.
*
* Inputs:   responsiblePartyStruct - populated structure from which to make DOMElement.
*           doc - DOMDocument that should be used to create the DOMElement.
*
* Outputs:  Error messages to stderr.
*
* Returns:  Returns a pointer to the DOMElement created. Returns a NULL  pointer if the "role" field is empty or if any errors were encountered.
*
* Notes:    The "role" element is required by the schema.
*
************************************************************************************************/
DOMElement * createCI_ResponsiblePartyNode(RESPONSIBLE_PARTY responsiblePartyStruct, DOMDocument * doc)
{


    DOMElement * responsiblePartyElem = NULL;

 
    


    try
    {

        /* Criteria for this node is that "role must be supplied and at least one of the following fileds must be supplied. */
        NV_INT16 nodeCriteria = strcmp(responsiblePartyStruct.individualName, "") + 
                                strcmp(responsiblePartyStruct.organisationName, "") + 
		                        strcmp(responsiblePartyStruct.positionName, "");

            
        /* One of the above fields must be supplied per the schema documentation. */
        if(nodeCriteria < 1)
        {
            return responsiblePartyElem;
        }


        /* If "role" is not populated, don't create the element.  "role" is a required element of the schema. */
        if(strcmp(responsiblePartyStruct.role, "") == 0)
        {
            fprintf(stderr, "ERROR: The \"role\" is required in order to create the CI_ResponsibleParty node.\n");
            return responsiblePartyElem;
        }

        responsiblePartyElem = doc->createElement(X("smXML:CI_ResponsibleParty"));


                
        /* If an individual name has been supplied, Create the individual node and populate it. */
        if(strcmp(responsiblePartyStruct.individualName, "") != 0)
        {

            DOMElement *  individualElem = createElement("individualName",responsiblePartyStruct.individualName, doc );
            responsiblePartyElem->appendChild(individualElem);
        }


        /* If an organisation name has been supplied, Create the organisation node and populate it. */
        if(strcmp(responsiblePartyStruct.organisationName, "") != 0)
        {

            DOMElement *  organizationElem = createElement("organisationName",responsiblePartyStruct.organisationName, doc );
            responsiblePartyElem->appendChild(organizationElem);
        }



        /* If a postiion name has been supplied, Create the position node and populate it. */
        if(strcmp(responsiblePartyStruct.positionName, "") != 0)
        {

            DOMElement *  positionElem = createElement("positionName",responsiblePartyStruct.positionName, doc );
            responsiblePartyElem->appendChild(positionElem);
        }


        /* If a phone number has been supplied, Create the phone number node and populate it. */
        if(strcmp(responsiblePartyStruct.phoneNumber, "") != 0)
        {

            DOMElement *  contactInfoElem = doc->createElement(X("contactInfo"));
            DOMElement *  ciContactElem = doc->createElement(X("smXML:CI_Contact"));
            DOMElement *  phoneElem = doc->createElement(X("phone"));
            DOMElement *  ciTelephoneElem = doc->createElement(X("smXML:CI_Telephone"));
		
            DOMElement *  voiceElem = createElement("voice",responsiblePartyStruct.phoneNumber, doc );

            ciTelephoneElem->appendChild(voiceElem);
            phoneElem->appendChild(ciTelephoneElem);
            ciContactElem->appendChild(phoneElem);
            contactInfoElem->appendChild(ciContactElem);
            responsiblePartyElem->appendChild(contactInfoElem);
        }



        /* Create the role node and populate it. Return NULL if unable to create the required "role" node.*/
        DOMElement*  roleElem = createElement("role",responsiblePartyStruct.role, doc );

        if(roleElem != NULL)
        {
            responsiblePartyElem->appendChild(roleElem);
        }
        else
        {
            responsiblePartyElem = NULL;
        }
	

    }
    catch(...)
    { 

        fprintf(stderr, "ERROR: An exception happened during the creation of the CI_ResponsibleParty node.\n");
        responsiblePartyElem->release();
        responsiblePartyElem = NULL;
    }


    return responsiblePartyElem;


}






/************************************************************************************************
*
* Function: createMD_LegalConstraintsNode
*
* Purpose:  This function creates and populates a DOMElement named smXML:MD_LegalConstraints.
*
* Inputs:   legalConstraints - populated structure from which to make DOMElement.
*           doc - DOMdocument that should be used to create the DOMElement.
*
* Outputs:  Error messages to stderr.
*
* Returns:  Returns a pointer to the DOMElement created. Returns NULL is the MD_LegalConstraints node could not be created.
*
************************************************************************************************/

DOMElement * createMD_LegalConstraintsNode(MD_LEGAL_CONSTRAINTS  legalConstraints, DOMDocument * doc)
{


    DOMElement * legalConstraintsElem = NULL;


    try{

        legalConstraintsElem = doc->createElement(X("smXML:MD_LegalConstraints"));


                
        /* Create the useContraints node and populate it. */
        if(strcmp(legalConstraints.useConstraints, "") != 0)
        {
            DOMElement *  useConstraintsElem = createElement("useConstraints",legalConstraints.useConstraints, doc);
            legalConstraintsElem->appendChild(useConstraintsElem);
        }



        /* Create the otherConstraints node and populate it. */
        if(strcmp(legalConstraints.otherConstraints, "") != 0)
        {
            DOMElement *  otherConstraintsElem = createElement("otherConstraints",legalConstraints.otherConstraints, doc);
            legalConstraintsElem->appendChild(otherConstraintsElem);
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



/************************************************************************************************
*
* Function: createMD_SecurityConstraintsNode
*
* Purpose:  This function creates and populates a DOMElement named smXML:MD_SecurityConstraints.
*
* Inputs:   securityConstraints - populated structure from which to make DOMElement.
*           doc - DOMdocument that should be used to create the DOMElement.
*
* Outputs:  Error messages to stderr.
*
* Returns:  Returns a pointer to the DOMElement created. Returns NULL if an exception was encountered.
*           Returns NULL if classification or userNote not supplied.  These are required navigation specific.
*           userNote holds the distribution Statement for the data.
*
************************************************************************************************/
DOMElement * createMD_SecurityConstraintsNode(MD_SECURITY_CONSTRAINTS  securityConstraints, DOMDocument * doc)
{


    DOMElement * securityConstraintsElem = NULL;


    /* If either the classification or the distribution statement is not supplied, the node should not be created.*/
    if(strcmp(securityConstraints.classification, "\0") == 0 || strcmp(securityConstraints.userNote, "\0") == 0)
    {

        fprintf(stderr, "ERROR: creating security constraints. Classification and Distribution statement must be supplied!.\n");
        return securityConstraintsElem;
    }

    try{

        securityConstraintsElem = doc->createElement(X("smXML:MD_SecurityConstraints"));

     
        /* Create the classification node an populate it. */
        DOMElement *  classificationElem = createElement("classification",securityConstraints.classification, doc );


        /* Create the otherConstraints node an populate it. */
        DOMElement *  userNoteElem = createElement("userNote",securityConstraints.userNote, doc);
       


        /* If either the classification or the userNote(distribution statement) could not be created, return NULL. */
        if(classificationElem != NULL && userNoteElem != NULL)
        {
            securityConstraintsElem->appendChild(classificationElem);
            securityConstraintsElem->appendChild(userNoteElem);
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


/************************************************************************************************
*
* Function: createDQ_DataQualityNode
*
* Purpose:  This function creates and populates a DOMElement named smXML:DQ_DataQuality.
*
* Inputs:   dataQuality - populated structure from which to make DOMElement.
*           doc - DOMdocument that should be used to create the DOMElement.
*
* Outputs:  Error messages to stderr.
*
* Returns:  Returns a pointer to the DOMElement created. Returns NULL if an exceptions were encountered.
*
************************************************************************************************/

DOMElement * createDQ_DataQualityNode(DATA_QUALITY_INFO  dataQuality, DOMDocument * doc)
{


    DOMElement * dataQualityElem = NULL;
    NV_INT32 i = 0;


    try
    {

  

        dataQualityElem = doc->createElement(X("smXML:DQ_DataQuality"));

                
        /* Create the scope node. */
        DOMElement *  scopeElem = doc->createElement(X("scope"));
        dataQualityElem->appendChild(scopeElem);


        DOMElement *  dqScopeElem = doc->createElement(X("smXML:DQ_Scope"));
        scopeElem->appendChild(dqScopeElem);



        DOMElement *  levelElem = createElement("level", dataQuality.scope , doc);

       
        if(levelElem != NULL)
        {

            dqScopeElem->appendChild(levelElem);
			
        }
        else
        {
            fprintf(stderr, "level node could not be created.  Required per ISO 19139 schema.\n");
            dataQualityElem->release();
            return  NULL;
        }



	  

        /* Per ISO schema, lineage is required if level == dataset. Navigation will always set level to 'dataset' */

        /* Create the lineage node. */
        DOMElement *  lineageElem = doc->createElement(X("lineage"));



        /* Create the smXML:LI_Lineage. */
        DOMElement *  liLineageElem = doc->createElement(X("smXML:LI_Lineage"));


        /* If both nodes were successfully created and there are sources or process steps, then create the lineage nodes. */
        if(lineageElem != NULL && liLineageElem != NULL  && (dataQuality.numberOfSources + dataQuality.numberOfProcessSteps != 0) )
        {
            dataQualityElem->appendChild(lineageElem);
            lineageElem->appendChild(liLineageElem);
        }

        /* Otherwise return NULL as lineage is required when level is set to 'dataset' */
        else
        {
            fprintf(stderr, "ERROR: Could not create lineage nodes.  Make sure you have specified sources or processSteps.\n");
            return NULL;
        }
 


        NV_INT16 numberOfSuccessfulSources = 0;
        //fprintf(stderr, "NUMBER OF SOURCES: %d.\n",dataQuality.numberOfSources );

        /* For each source given, add a "source" node. */
        for( i = 0; i < dataQuality.numberOfSources; i++)
        {

		   
            DOMElement * sourceElem = createSourceNode(dataQuality.lineageSources[i], doc);

            if(sourceElem != NULL)
            {
                liLineageElem->appendChild(sourceElem);
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
	  

        NV_INT16 numberOfSuccessfulProcessSteps = 0;
        //fprintf(stderr, "NUMBER OF PROCESS_STEPS: %d.\n",dataQuality.numberOfProcessSteps );

        /* For each proceessStep given, add a "processStep" node. */
        for( i = 0; i < dataQuality.numberOfProcessSteps; i++ )
        {

            DOMElement * processStepElem = createProcessStepNode(dataQuality.lineageProcessSteps[i], doc);

            if(processStepElem != NULL)
            {
                liLineageElem->appendChild(processStepElem);
                numberOfSuccessfulProcessSteps++;
            }
            /* Otherwise and error was encountered making the source, return NULL. */
            else
            {

                fprintf(stderr, "ERROR: Could not create processStep node.  Check processStep number %d.\n", i);
                dataQualityElem->release();
                return NULL;
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




/************************************************************************************************
*
* Function: createSourceNode
*
* Purpose:  This function creates and populates a DOMElement named "source".
*
* Inputs:   source - populated structure from which to make DOMElement.
*           doc - DOMdocument that should be used to create the DOMElement.
*
* Outputs:  Error messages to stderr.
*
* Returns:  Returns a pointer to the DOMElement created. Returns NULL if an exceptions were encountered.
*
************************************************************************************************/

DOMElement * createSourceNode(SOURCE_INFO  source, DOMDocument * doc)
{


    DOMElement * sourceElem = NULL;
   
   
    try
    {

        if(strcmp(source.description, "") == 0)
        {

            fprintf(stderr, "ERROR: source description not supplied.\n");
            return sourceElem;
        }

  
        sourceElem = doc->createElement(X("source"));
                
        /* Create the smXML:LI_Source node. */
        DOMElement *  liSourceElem = doc->createElement(X("smXML:LI_Source"));
        sourceElem->appendChild(liSourceElem);

        DOMElement *  descriptionElem = createElement("description", source.description, doc );
        liSourceElem->appendChild(descriptionElem);



        /* Attempt to create CI_Citation node. */
        NV_INT16 status = 0;
        DOMElement *  ciCitationElem =  createCI_CitationNode(source.title, 
                                                              source.date, 
                                                              source.dateType,
                                                              source.responsibleParties, doc, &status);
  
        /* If element was successfully created, add it and all of its optional parent nodes. */
        if(ciCitationElem != NULL)
        {

            /* Create the optional sourceCitation node. */
            DOMElement *  sourceCitationElem = doc->createElement(X("sourceCitation"));
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






/************************************************************************************************
*
* Function: createProcessStepNode
*
* Purpose:  This function creates and populates a DOMElement named "processStep".
*
* Inputs:   source - populated structure from which to make DOMElement.
*           doc - DOMdocument that should be used to create the DOMElement.
*
* Outputs:  Error messages to stderr.
*
* Returns:  Returns a pointer to the DOMElement created.  Returns NULL if an exceptions were encountered.
*
************************************************************************************************/

DOMElement * createProcessStepNode(PROCESS_STEP_INFO  processStep, DOMDocument * doc)
{


    DOMElement * processStepElem = NULL;


    /* Check for the required elements in processStep. */
    if(strcmp(processStep.description, "") == 0 ||strcmp(processStep.trackingId, "") == 0 )
    {
        fprintf(stderr, "ERROR: Could not create processStep node. description and trackingId must be specified.\n");
        return NULL;
    }

   
    try
    {

        processStepElem = doc->createElement(X("processStep"));

                
        /* Create the smXML:BAG_ProcessStep node. */
        DOMElement *  bagProcessStepElem = doc->createElement(X("smXML:BAG_ProcessStep"));
        processStepElem->appendChild(bagProcessStepElem);


        /* Create the description node. Required per ISO schema. */
        DOMElement *  descriptionElem = createElement("description", processStep.description,  doc );
        if(descriptionElem != NULL)
        {
            bagProcessStepElem->appendChild(descriptionElem);
        }
        else
        {
            fprintf(stderr, "Error creating processStep node. Problem creating description.\n");
            processStepElem->release();
            return NULL;
        }


        /* Create the dateTime node. */
        DOMElement *  dateTimeElem = createElement("dateTime", processStep.dateTime, doc );
        bagProcessStepElem->appendChild(dateTimeElem);



        /* For each proceesing party listed, add it. */
        /* role is a required element, so use it as the criteria. */
        NV_INT32 i = 0;
        while( i < MAX_CI_RESPONSIBLE_PARTIES  && strcmp(processStep.processors[i].role, "") != 0 )
        {

            DOMElement * processorElem = doc->createElement(X("processor"));
            DOMElement * responsiblePartyElem = createCI_ResponsiblePartyNode(processStep.processors[i], doc);

            if(responsiblePartyElem != NULL)
            {
                processorElem->appendChild(responsiblePartyElem);
                bagProcessStepElem->appendChild(processorElem);
            }
            /* If an error was encountered, stop making the node. */
            else
            {
                fprintf(stderr, "Error creating processStep node. Problem creating processor %d. \n", i );
                fprintf(stderr, "Ensure that at least one of the following exist: individualName, organisationName, positionName. \n");
                processStepElem->release();
                return NULL;
            }

            i++;
        }



        /* Create the trackingId node.  Required per ISO schema. */
        DOMElement *  trackingIdElem = createElement("trackingId", processStep.trackingId, doc );
        if(trackingIdElem != NULL)
        {
            bagProcessStepElem->appendChild(trackingIdElem);
        }
        else
        {        
            fprintf(stderr, "Error creating processStep node. Problem creating trackingId.\n");
            processStepElem->release();
            return NULL;
        }



    }
    catch(...)
    { 
        fprintf(stderr, "ERROR: Error creating processStep node.\n");
        processStepElem->release();
        processStepElem = NULL;
    }


    return processStepElem;


}






/************************************************************************************************
*
* Function: createMD_GeorectifiedNode
*
* Purpose:  This function creates and populates a DOMElement named smXML:MD_Georectified.
*
* Inputs:   spatialRepresentationInfo - populated structure from which to make DOMElement.
*           doc - DOMdocument that should be used to create the DOMElement.
*
* Outputs:  Error messages to stderr.
*
* Returns:  Returns a pointer to the DOMElement created.  Returns NULL if an exceptions were encountered.
*
************************************************************************************************/

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

 

        if(strcmp( spatialRepresentationInfo.cellGeometry, "") == 0 ||
		   strcmp( spatialRepresentationInfo.transformationParameterAvailability, "") == 0 ||
		   strcmp( spatialRepresentationInfo.checkPointAvailability, "") == 0 )
        {

            fprintf(stderr, "ERROR: spatialRepresentationInfo.cellGeometry, transformationParameterAvailability,checkPointAvailability must be supplied\n");
            return geoRectifiedElem;
        }



        if(spatialRepresentationInfo.llCornerX == (NV_FLOAT64)INIT_VALUE ||  
           spatialRepresentationInfo.llCornerY == (NV_FLOAT64)INIT_VALUE ||   
           spatialRepresentationInfo.urCornerX == (NV_FLOAT64)INIT_VALUE ||   
           spatialRepresentationInfo.urCornerY == (NV_FLOAT64)(INIT_VALUE))

        {
 
            fprintf(stderr, "ERROR: All four spatialRepresentationInfo corner points must be supplied. \n"); 
            return geoRectifiedElem;

        }

        /* For the number of dimensions speicifed, ensure that there are dimensionNames, sizes and resolutions. */
        for(NV_INT16 i=0; i< spatialRepresentationInfo.numberOfDimensions; i++)
        {

            if((strcmp(spatialRepresentationInfo.dimensionName[i], "") == 0 )|| 
                spatialRepresentationInfo.dimensionSize == 0 || 
                spatialRepresentationInfo.resolutionValue == 0 )
            {
			 
                fprintf(stderr, "ERROR: numberOfDimensions does not match the the number of dimensionNames, sizes and resolutions. \n"); 
                return geoRectifiedElem;
            }

        }




        geoRectifiedElem = doc->createElement(X("smXML:MD_Georectified"));

    
        
        /* Create the numberOfDimensions node an populate it. */
        NV_CHAR dimAsString[2]; /* assume no more than 1 digits in integer specifiying number of dimensions. */
        sprintf(dimAsString, "%d", spatialRepresentationInfo.numberOfDimensions);


        DOMElement *  numberOfDimensionsElem = createElement("numberOfDimensions", dimAsString, doc);
        geoRectifiedElem->appendChild(numberOfDimensionsElem);
   

     
         
        /* For each dimension, create an axisDimensionProperties node. */
        for (int d = 0; d < spatialRepresentationInfo.numberOfDimensions; d++ )
        {


            /* Create the axisDimensionProperties node and populate it. */
            DOMElement *  axisDimensionPropertiesElem = doc->createElement(X("axisDimensionProperties"));
            geoRectifiedElem->appendChild(axisDimensionPropertiesElem);


            DOMElement *  mdDimensionElem = doc->createElement(X("smXML:MD_Dimension"));
            axisDimensionPropertiesElem->appendChild(mdDimensionElem);


            /* Create dimensionName node. */
            DOMElement *  dimensionNameElem = createElement("dimensionName",spatialRepresentationInfo.dimensionName[d], doc );
            mdDimensionElem->appendChild(dimensionNameElem);


            /* Create dimensionSize node. */
            NV_CHAR numberAsString[21]; /* assume no more than 5 digits in integer specifiying dimension size. */
            sprintf(numberAsString, "%d", spatialRepresentationInfo.dimensionSize[d]);
            DOMElement *  dimensionSizeElem = createElement("dimensionSize", numberAsString, doc);
            mdDimensionElem->appendChild(dimensionSizeElem);


            /* Create reolution node. */
            DOMElement *  resolutionElem = doc->createElement(X("resolution"));
            mdDimensionElem->appendChild(resolutionElem);


            /* Create measure node. */
            DOMElement *  measureElem = doc->createElement(X("smXML:Measure"));
            resolutionElem->appendChild(measureElem);


            /* Create spatialValue node. */
            strcpy(numberAsString, "");
            sprintf(numberAsString, "%.15lf", spatialRepresentationInfo.resolutionValue[d]);
            DOMElement *  valueElem = createElement("smXML:value", numberAsString, doc);
            measureElem->appendChild(valueElem);


            /* Create spatial units of measure node.*/
            DOMElement *  uomElem = doc->createElement(X("smXML:uom_r"));
            measureElem->appendChild(uomElem);
             
        }
           

    
        /* Create the cell geometry node and its text value. Use "point" as default */
        if(strcmp(spatialRepresentationInfo.cellGeometry, "") == 0)
        {

            strcpy(spatialRepresentationInfo.cellGeometry,"point" );
        }


        DOMElement *  cellGeometryElem = createElement("cellGeometry",spatialRepresentationInfo.cellGeometry, doc);
        geoRectifiedElem->appendChild(cellGeometryElem);
     

        /* Create the transformationParameterAvailability node and its text value.  Use "false" as default. */
        if(strcmp(spatialRepresentationInfo.transformationParameterAvailability, "") == 0)
        {
            strcpy(spatialRepresentationInfo.transformationParameterAvailability,"false");
        }


        DOMElement *  transParamAvailElem = createElement("transformationParameterAvailability",spatialRepresentationInfo.transformationParameterAvailability, doc);
        geoRectifiedElem->appendChild(transParamAvailElem);


        /* Create the checkPointAvailability node and its text value.  Use "0" as default */
        if(strcmp(spatialRepresentationInfo.checkPointAvailability, "") == 0)
        {
            strcpy(spatialRepresentationInfo.checkPointAvailability,"0" );
        }

        DOMElement *  checkPtAvailElem = createElement("checkPointAvailability",spatialRepresentationInfo.checkPointAvailability, doc);
            geoRectifiedElem->appendChild(checkPtAvailElem);

     
       
            /* Create the corner points node. */
            DOMElement *  cornerPointsElem = doc->createElement(X("cornerPoints"));
            geoRectifiedElem->appendChild(cornerPointsElem);


            DOMElement *  gmlPointElem = doc->createElement(X("gml:Point"));
            cornerPointsElem->appendChild(gmlPointElem);



            /* Create the gml:coordinates node. */
            DOMElement *  gmlCoordinatesElem = doc->createElement(X("gml:coordinates"));
            gmlCoordinatesElem->setAttribute(XMLString::transcode("decimal"),XMLString::transcode("."));
            gmlCoordinatesElem->setAttribute(XMLString::transcode("cs"),XMLString::transcode("," ));
            gmlCoordinatesElem->setAttribute(XMLString::transcode("ts"),XMLString::transcode(" " ));
            gmlPointElem->appendChild(gmlCoordinatesElem);

 
            /* Create string that contains corner points. */
            NV_CHAR * pointsString = (NV_CHAR *) calloc(88, sizeof(NV_CHAR));
            sprintf(pointsString, "%.12lf,%.12lf %.12lf,%.12lf", spatialRepresentationInfo.llCornerX, spatialRepresentationInfo.llCornerY, spatialRepresentationInfo.urCornerX, spatialRepresentationInfo.urCornerY);
   

            DOMText *   gmlCoordinatesText = doc->createTextNode(X(pointsString));
            gmlCoordinatesElem->appendChild(gmlCoordinatesText);



            free(pointsString);

       

    }
    catch(...)
    { 
        fprintf(stderr, "ERROR: Error creating georectified element.\n");
        geoRectifiedElem->release();
        geoRectifiedElem = NULL;
    }


    return geoRectifiedElem;


}




/************************************************************************************************
*
* Function: createMD_CRSNode
*
* Purpose:  This function creates and populates a DOMElement named smXML:MD_CRS for vertical datum metadata.
*
* Inputs:   verticalDatum - populated string containg value for vertical datum.
*           doc - DOMdocument that should be used to create the DOMElement.
*
* Outputs:  Error messages to stderr.
*
* Returns:  Returns a pointer to the DOMElement created.  Returns NULL if an exceptions were encountered or if vertical datum was not supplied.
*
************************************************************************************************/
DOMElement * createMD_CRSNode(char verticalDatum[100], DOMDocument * doc)
{


    DOMElement * crsElem = NULL;

    try
    {

        /* If vertical datum has been supplied, create the MD_CRS node and populate it. */
        if(strcmp(verticalDatum, "") != 0)
        {

            /* Create the MD_CRS node. */
            crsElem = doc->createElement(X("smXML:MD_CRS"));

            DOMComment *  datumComment = doc->createComment(X("vertical datum"));
            crsElem->appendChild(datumComment);
                

            /* Create the datum node. */
            DOMElement *  datumElem = doc->createElement(X("datum"));
            crsElem->appendChild(datumElem);


            /* Create the RS_Identifier node. */
            DOMElement *  rsElem = doc->createElement(X("smXML:RS_Identifier"));
            datumElem->appendChild(rsElem);

   
            /* Create the code node and its text value. */
            DOMElement *  codeElem =createElement("code", verticalDatum, doc);
            rsElem->appendChild(codeElem);
        }
        else
        {
            fprintf(stderr, "INFO: vertical datum not supplied.  Not creating MD_CRS node. \n");
        }


    }
    catch(...)
    { 
        fprintf(stderr, "ERROR: Error creating MD_CRS.\n");
        crsElem = NULL;
    }

    return crsElem;


}




/************************************************************************************************
*
* Function: createMD_CRSNode
*
* Purpose:  This function creates and populates a DOMElement named smXML:MD_CRS.
*
* Inputs:   referenceSystemInfo - populated structure from which to create DOMElement.
*           doc - DOMdocument that should be used to create the DOMElement.
*
* Outputs:  Error messages to stderr.
*
* Returns:  Returns a pointer to the DOMElement created. Returns NULL if an exceptions were encountered.
*
************************************************************************************************/
DOMElement * createMD_CRSNode(REFERENCE_SYSTEM_INFO referenceSystemInfo, DOMDocument * doc)
{


    DOMElement * crsElem = NULL;
    NV_CHAR doubleAsString[30]; /* Used to represent text values of various lats and  lons. */
    NV_CHAR intAsString[12];    /* Used to represent text values of various integers. */



    try{


        /* Check for required fields. If not present, return NULL. */
        if(strcmp(referenceSystemInfo.projection, "") == 0 || 
           strcmp(referenceSystemInfo.ellipsoid, "")  == 0 ||  
           strcmp(referenceSystemInfo.horizontalDatum, "") == 0)

        {

            fprintf(stderr, "ERROR: Unable to create MD_CRS node.  Missing one of the required fields.\n");
            fprintf(stderr, "Check projection, ellipsoid and horizontalDatum codes. All three are required as referenceSystemIdentifier is not yet supported.\n");
            return crsElem;

        }



        /* Create the MD_CRS node. */
        crsElem = doc->createElement(X("smXML:MD_CRS"));


        /* Create the projection node. */
        DOMElement *  projectionElem = doc->createElement(X("projection"));
        crsElem->appendChild(projectionElem);


        /* Create the RS_Identifier node. */
        DOMElement *  rsElem1 = doc->createElement(X("smXML:RS_Identifier"));
        projectionElem->appendChild(rsElem1);

   
        /* Create the code node for projection. */
        DOMElement *  codeElem1 = createElement("code",referenceSystemInfo.projection, doc);
        rsElem1->appendChild(codeElem1);


        /* Create the ellipsoid node. */
        DOMElement *  ellipsoidElem = doc->createElement(X("ellipsoid"));
        crsElem->appendChild(ellipsoidElem);


        /* Create the RS_Identifier node. */
        DOMElement *  rsElem2 = doc->createElement(X("smXML:RS_Identifier"));
        ellipsoidElem->appendChild(rsElem2);

  
        /* Create the ellipsoid node and its text value. */
        DOMElement *  codeElem2 = createElement("code",referenceSystemInfo.ellipsoid, doc);
        rsElem2->appendChild(codeElem2);

                
        /* Create the datum node. */
        DOMElement *  datumElem = doc->createElement(X("datum"));
        crsElem->appendChild(datumElem);


        /* Create the RS_Identifier node. */
        DOMElement *  rsElem3 = doc->createElement(X("smXML:RS_Identifier"));
        datumElem->appendChild(rsElem3);

  
        /* Create the datum node. */
        DOMElement *  codeElem3 = createElement("code",referenceSystemInfo.horizontalDatum, doc);
        rsElem3->appendChild(codeElem3);


        /* Create the projection  parameters node. */
        DOMElement *  projectionParametersElem = doc->createElement(X("projectionParameters"));
        crsElem->appendChild(projectionParametersElem);


        /* Create the MD_ProjectionParameters node. */
        DOMElement * mdProjectionParametersElem = doc->createElement(X("smXML:MD_ProjectionParameters"));
        projectionParametersElem->appendChild(mdProjectionParametersElem);


        /* Create the zone node. */
        if(referenceSystemInfo.zone != (NV_INT16)INIT_VALUE)
        {
            sprintf(intAsString, "%d",referenceSystemInfo.zone);
            DOMElement *  zoneElem = createElement("zone",intAsString, doc);
            mdProjectionParametersElem->appendChild(zoneElem);
        }



        /* Create the standardParallel node. */
        if(referenceSystemInfo.standardParallel != (NV_FLOAT64)INIT_VALUE)
        {
            sprintf(doubleAsString,"%.7f", referenceSystemInfo.standardParallel);
            DOMElement * standardParallelElem = createElement("standardParallel",doubleAsString, doc);
            mdProjectionParametersElem->appendChild(standardParallelElem);
        }



        /* Create the longitudeOfCentralMeridian node. */
        if(referenceSystemInfo.longitudeOfCentralMeridian != (NV_FLOAT64)INIT_VALUE)
        {
            sprintf(doubleAsString,"%.7f", referenceSystemInfo.longitudeOfCentralMeridian);
            DOMElement * longitudeOfCentralMeridianElem = createElement("longitudeOfCentralMeridian",doubleAsString, doc);
            mdProjectionParametersElem->appendChild(longitudeOfCentralMeridianElem);
        }


        /* Create the optional zone node. */
        if(referenceSystemInfo.latitudeOfProjectionOrigin != (NV_FLOAT64)INIT_VALUE)
        {
            sprintf(doubleAsString,"%.7f", referenceSystemInfo.latitudeOfProjectionOrigin);
            DOMElement * latitudeOfProjectionOriginElem = createElement("latitudeOfProjectionOrigin",doubleAsString, doc);
            mdProjectionParametersElem->appendChild(latitudeOfProjectionOriginElem);
        }



        /* Create the falseEasting node. */
        if(referenceSystemInfo.falseEasting != (NV_FLOAT64)INIT_VALUE)
        {
            sprintf(doubleAsString,"%.7f", referenceSystemInfo.falseEasting);
            DOMElement * falseEastingElem = createElement("falseEasting",doubleAsString, doc);
            mdProjectionParametersElem->appendChild(falseEastingElem);
        }



        /* Create the falseNorthinge node. */
        if(referenceSystemInfo.falseNorthing != (NV_FLOAT64)INIT_VALUE)
        {
            sprintf(doubleAsString,"%.7f", referenceSystemInfo.falseNorthing);
            DOMElement * falseNorthingElem = createElement("falseNorthing", doubleAsString, doc);
            mdProjectionParametersElem->appendChild(falseNorthingElem);
        }


        /* Create the scaleFactorAtEquator node. */
        if(referenceSystemInfo.scaleFactorAtEquator != (NV_FLOAT64)INIT_VALUE)
        {
            sprintf(doubleAsString,"%.7f", referenceSystemInfo.scaleFactorAtEquator);
            DOMElement * scaleFactorAtEquatorElem = createElement("scaleFactorAtEquator", doubleAsString, doc);
            mdProjectionParametersElem->appendChild(scaleFactorAtEquatorElem);
        }


        /* Create the heightOfProspectivePointAboveSurface node. */
        if(referenceSystemInfo.heightOfProspectivePointAboveSurface != (NV_FLOAT64)INIT_VALUE)
        {
            sprintf(doubleAsString,"%.7f", referenceSystemInfo.heightOfProspectivePointAboveSurface);
            DOMElement * heightOfProspectivePointAboveSurfaceElem = createElement("heightOfProspectivePointAboveSurface", doubleAsString, doc);
            mdProjectionParametersElem->appendChild(heightOfProspectivePointAboveSurfaceElem);

        }

        /* Create the longitudeOfProjectionCenter node. */
        if(referenceSystemInfo.longitudeOfProjectionCenter != (NV_FLOAT64)INIT_VALUE)
        {
            sprintf(doubleAsString,"%.7f", referenceSystemInfo.longitudeOfProjectionCenter);
            DOMElement * longitudeOfProjectionCenterElem = createElement("longitudeOfProjectionCenter", doubleAsString, doc);
            mdProjectionParametersElem->appendChild(longitudeOfProjectionCenterElem);
        }



        /* Create the latitudeOfProjectionCenter node. */
        if(referenceSystemInfo.latitudeOfProjectionCenter != (NV_FLOAT64)INIT_VALUE)
        {
            sprintf(doubleAsString,"%.7f", referenceSystemInfo.latitudeOfProjectionCenter);
            DOMElement * latitudeOfProjectionCenterElem = createElement("latitudeOfProjectionCenter", doubleAsString, doc);
            mdProjectionParametersElem->appendChild(latitudeOfProjectionCenterElem);
        }


        /* Create the scaleFactorAtCenterLine node. */
        if(referenceSystemInfo.scaleFactorAtCenterLine != (NV_FLOAT64)INIT_VALUE)
        {
            sprintf(doubleAsString,"%.7f", referenceSystemInfo.scaleFactorAtCenterLine);
            DOMElement * scaleFactorAtCenterLineElem = createElement("scaleFactorAtCenterLine", doubleAsString, doc);
            mdProjectionParametersElem->appendChild(scaleFactorAtCenterLineElem);
        }


        /* Create the straightVerticalLongitudeFromPol node. */
        if(referenceSystemInfo.straightVerticalLongitudeFromPole != (NV_FLOAT64)INIT_VALUE)
        {
            sprintf(doubleAsString,"%.7f", referenceSystemInfo.straightVerticalLongitudeFromPole);
            DOMElement * straightVerticalLongitudeFromPoleElem = createElement("straightVerticalLongitudeFromPole", doubleAsString, doc);
            mdProjectionParametersElem->appendChild(straightVerticalLongitudeFromPoleElem);
        }

        /* Create the scaleFactorAtProjectionOrigin node. */
        if(referenceSystemInfo.scaleFactorAtProjectionOrigin != (NV_FLOAT64)INIT_VALUE )
        {
            sprintf(doubleAsString,"%.7f", referenceSystemInfo.scaleFactorAtProjectionOrigin);
            DOMElement * scaleFactorAtProjectionOriginElem = createElement("scaleFactorAtProjectionOrigin", doubleAsString, doc);
            mdProjectionParametersElem->appendChild(scaleFactorAtProjectionOriginElem);
        }

       
        /* Determine if optional element "ellipsoidParameters" should be created.                         */
        /* If semiMajorAxis has been supplied by the user, then create the element.                       */ 
                                                     
        if(referenceSystemInfo.semiMajorAxis != (NV_FLOAT64)INIT_VALUE )
        {

            /* Create the ellipsoid parameters node. */
            DOMElement *  ellipsoidParametersElem = doc->createElement(X("ellipsoidParameters"));
            crsElem->appendChild(ellipsoidParametersElem);


            /* Create the MD_EllipsoidParameters node. */
            DOMElement *  mdEllipsoidParametersElem = doc->createElement(X("smXML:MD_EllipsoidParameters"));
            ellipsoidParametersElem->appendChild(mdEllipsoidParametersElem);

   
            /* Create the semiMajorAxis node and its text. */
            NV_CHAR axisString[30];
            sprintf(axisString,"%lf", referenceSystemInfo.semiMajorAxis);
            DOMElement *  semiMajorAxisElem = createElement("semiMajorAxis",axisString, doc );
            mdEllipsoidParametersElem->appendChild(semiMajorAxisElem);


            /* Create the axisUnits node and its text. */
            DOMElement *  axisUnitsElem = doc->createElement(X("axisUnits"));
            mdEllipsoidParametersElem->appendChild(axisUnitsElem);
        }


    }
    catch(...)
    { 
        fprintf(stderr, "ERROR: Error creating MD_CRS.\n");
        crsElem->release();
        crsElem = NULL;
    }

    return crsElem;


}








/************************************************************************************************
*
* Function: createBAG_DataIdentificationNode
*
* Purpose:  This function creates and populates a DOMElement named BAG_DataIdentification.
*
* Inputs:   identificationInfo - populated structure from which to create DOMElement.
*           doc - DOMdocument that should be used to create the DOMElement.
*
* Outputs:  Error messages to stderr.
*
* Returns:  Returns a pointer to the DOMElement created.  Returns NULL if an exceptions were encountered.
*
* Notes:    Will stop adding cited responsible parties when the first RESPONSIBLE_PARTY is encountered that 
*           does not have a completed "role" field.
*
*           Will not add a cited responsible party where RESPONSIBLE_PARTY does not have at least one of the following:
*           individualName, organisationName, positionName
************************************************************************************************/

DOMElement * createBAG_DataIdentificationNode(IDENTIFICATION_INFO identificationInfo, DOMDocument * doc)
{


    DOMElement * bagIdentificationElem = NULL;
    NV_CHAR doubleAsString[30];

    /* Check for the required fields. If they are not present, return NULL. */
    if(strcmp(identificationInfo.abstract, "") == 0 ||
       strcmp(identificationInfo.language, "") == 0 || 
       strcmp(identificationInfo.verticalUncertaintyType, "") == 0 )
    {
	    fprintf(stderr, "ERROR: can not create BAG identificationInfo.  Missing one or more required fields... abstract, language or verticalUncertaintyType. \n");
	    return bagIdentificationElem;
    }

 
    try{


        /* Create the BAG_DataIdentification node. */
        bagIdentificationElem = doc->createElement(X("smXML:BAG_DataIdentification"));
  
    

        /* Create the citation node. */
        DOMElement *  citationElem = doc->createElement(X("citation"));
        bagIdentificationElem->appendChild(citationElem);


        NV_INT16 status = 0;
        DOMElement *  ciCitationElem =  createCI_CitationNode(identificationInfo.title, 
                                                              identificationInfo.date, 
                                                              identificationInfo.dateType,
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


 
        /* Create the abstract node and its text value. */
        DOMElement *  abstractElem = createElement("abstract",identificationInfo.abstract, doc);
        bagIdentificationElem->appendChild(abstractElem);



        /* Create the optional purpose node and its text value. */
        if(strcmp(identificationInfo.purpose, "") != 0)
        {
            DOMElement *  purposeElem = createElement("purpose",identificationInfo.purpose, doc);
            bagIdentificationElem->appendChild(purposeElem);
        }

        /* Create the optional status node and its text value. */
        if(strcmp(identificationInfo.status, "") != 0)
        {
            DOMElement *  statusElem = createElement("status",identificationInfo.status, doc);
            bagIdentificationElem->appendChild(statusElem);
        }

        /* Create the spatial representaion type node and its text value. */
        if(strcmp(identificationInfo.spatialRepresentationType, "") != 0)
        {
            DOMElement *  spatialRepElem = createElement("spatialRepresentationType",identificationInfo.spatialRepresentationType, doc);
            bagIdentificationElem->appendChild(spatialRepElem);
        }
     

        /* Create the required language node and its text value. */
        DOMElement *  languageElem = createElement("language",identificationInfo.language, doc);
        bagIdentificationElem->appendChild(languageElem);


        /* Create the topic node and its text value. */
        DOMElement *  topicElem = createElement("topicCategory", identificationInfo.topicCategory, doc);
        bagIdentificationElem->appendChild(topicElem);
               

        /* If any lats & lons are set to INIT_VALUE, the bounding box has not been initialized and the optional "extent" node should not be made.*/
        if(identificationInfo.westBoundingLongitude != NV_FLOAT64(INIT_VALUE) && 
           identificationInfo.eastBoundingLongitude != NV_FLOAT64(INIT_VALUE) &&  
           identificationInfo.southBoundingLatitude != NV_FLOAT64(INIT_VALUE) &&  
           identificationInfo.northBoundingLatitude != NV_FLOAT64(INIT_VALUE) )
        {

            /* Create the extent node. */
            DOMElement *  extentElem = doc->createElement(X("extent"));
            bagIdentificationElem->appendChild(extentElem);

            /* Create the EX_Extent node. */
            DOMElement *  exExtentElem = doc->createElement(X("smXML:EX_Extent"));
            extentElem->appendChild(exExtentElem);

            /* Create the geographic node. */
            DOMElement *  geographicElementElem = doc->createElement(X("geographicElement"));
            exExtentElem->appendChild(geographicElementElem);

            /* Create EX_GeographicBoundingBox node. */
            DOMElement *  exGeographicBoundingBoxElem = doc->createElement(X("smXML:EX_GeographicBoundingBox"));
            geographicElementElem->appendChild(exGeographicBoundingBoxElem);
  
            sprintf(doubleAsString,"%.7f", identificationInfo.westBoundingLongitude);
            DOMElement * westBoundLongitudeElem = createElement("westBoundLongitude", doubleAsString, doc);
            exGeographicBoundingBoxElem->appendChild(westBoundLongitudeElem);
  
            sprintf(doubleAsString,"%.7f", identificationInfo.eastBoundingLongitude);
            DOMElement * eastBoundLongitudeElem = createElement("eastBoundLongitude", doubleAsString, doc);
            exGeographicBoundingBoxElem->appendChild(eastBoundLongitudeElem);
 
            sprintf(doubleAsString,"%.7f", identificationInfo.southBoundingLatitude);
            DOMElement * southBoundLatitudeElem = createElement("southBoundLatitude",doubleAsString, doc);
            exGeographicBoundingBoxElem->appendChild(southBoundLatitudeElem);

            sprintf(doubleAsString,"%.7f", identificationInfo.northBoundingLatitude);
            DOMElement * northBoundLatitudeElem = createElement("northBoundLatitude",doubleAsString, doc);
            exGeographicBoundingBoxElem->appendChild(northBoundLatitudeElem);
        }

  
        /* Create the vertical uncertainty node its text value. */
        DOMElement * verticalUncertaintyTypeElem = createElement("verticalUncertaintyType",identificationInfo.verticalUncertaintyType, doc);
        bagIdentificationElem->appendChild(verticalUncertaintyTypeElem);

        /* Create the optional depthCorrectionType node its text value. */
        if(strcmp(identificationInfo.depthCorrectionType, "") != 0)
        {
            DOMElement * depthCorrectionTypeElem = createElement("depthCorrectionType",identificationInfo.depthCorrectionType, doc);
            bagIdentificationElem->appendChild(depthCorrectionTypeElem);
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



/************************************************************************************************
*
* Function: createCI_CitationNode
*
* Purpose:  This function creates and populates a DOMElement named CI_Citation.
*
* Inputs:   title - title of the citation
*           date - date of the citation
*           dateType - type of citation
*           responsibleParties - array of parties responsible for the citation.
*           doc - DOMdocument that should be used to create the DOMElement.
*           status - used to indicate the reason for returnign a NULL element.
*                    1 = element is NULL and this is due to the user not wanting the element. 
*                   -1 = element is NULL and this is due to the user not supplying enough information or an error is encountered. 
*
* Outputs:  Error/information messages to stderr.
*
* Returns:  Returns a pointer to the DOMElement created.  Returns NULL if an exceptions were encountered 
*           or XML supplied indicates that caller does not want this optional element to be created.
*
* Notes:    Will stop adding cited responsible parties when the first RESPONSIBLE_PARTY is encountered that 
*           does not have a completed "role" field.
*
*           Will not add a cited responsible party where RESPONSIBLE_PARTY does not have at least one of the following:
*           individualName, organisationName, positionName
************************************************************************************************/

DOMElement * createCI_CitationNode(NV_CHAR title[100], NV_CHAR date[20], NV_CHAR dateType[20], RESPONSIBLE_PARTY responsibleParties[MAX_CI_RESPONSIBLE_PARTIES], DOMDocument* doc, NV_INT16 * status)
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


        ciCitationElem = doc->createElement(X("smXML:CI_Citation"));


        /* Create the title node and its text value. */
        DOMElement *  titleElem = createElement("title", title, doc);
        ciCitationElem->appendChild(titleElem);


        /* Create the date node. */
        DOMElement *  dateElem = doc->createElement(X("date"));
        ciCitationElem->appendChild(dateElem);


        /* Must have both the date and the dateType in order to make the optional CI_Date node. */
        NV_INT16 numberOfElements = 0;
        if(strcmp(date, "") != 0){numberOfElements++;}
    	if(strcmp(dateType, "") != 0){numberOfElements++;}
	
	//fprintf(stderr, "Number of elements: %d\n", numberOfElements);
 
    	if(numberOfElements == 2)
    	{

            /* Create the CI date node. */
            DOMElement *  ciDateElem = doc->createElement(X("smXML:CI_Date"));
            dateElem->appendChild(ciDateElem);


            /* Create the date node and its text value. */
            DOMElement *  dateElem2 = createElement("date", date, doc);
            ciDateElem->appendChild(dateElem2);


            /* Create the dateType node and its text value. */
            DOMElement *  dateTypeElem = createElement("dateType", dateType, doc);
            ciDateElem->appendChild(dateTypeElem);
	   
 
        }

        /* Only one of the criteria supplied, this is an error. */
        else if(numberOfElements == 1)
        {
                
            fprintf(stderr, "ERROR: Must supply both the date and dateType in order to create a valid CI_Date node.\n ");
            ciCitationElem->release();
            *status = -1;
            ciCitationElem = NULL;
            return ciCitationElem;

        }



    
        /* While there are responsible parties to add, add them. "role" is a required item, so it is used as criteria. */
        NV_INT32 r = 0;
        while(strcmp(responsibleParties[r].role, "")   != 0  && r < MAX_CI_RESPONSIBLE_PARTIES)
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
            DOMElement *  citedResponsiblePartyElem = doc->createElement(X("citedResponsibleParty"));
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




/************************************************************************************************
*
* Function: createElement
*
* Purpose:  This function creates an element and its text value.
*
* Inputs:   elementName - name of element to create.
*           elementValue - string value that should assigned to the newly created element.
*           doc - the DOMDocument that should be used to create the element.
*
* Outputs:  Error messages to stderr.
*
* Returns:  Returns a pointer to specified DOMElement.  
*           Returns a pointer to NULL if not able to create element and set value.
*
************************************************************************************************/
DOMElement * createElement(NV_CHAR * elementName, NV_CHAR * elementValue, DOMDocument * doc){


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
        NV_CHAR * message = XMLString::transcode(toCatch.msg);
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









/************************************************************************************************
*
* Function FindNode
*
* Purpose:  This function finds a node in the node tree defined by the following criteria:
*           tag = a valid tag name, attName = NULL, attValue = NULL. 
*                 Returns the first node with the tag in the node tree.
*
*           tag = a valid tag name, attName = A valide Attribute Name, attValue = a value.

*                 Returns the first node with the tag in the node tree that has the corresponding 
*                 attribute information.
*                 If no element meeting the specified criteria is found, then a NULL DOMElement is returned.
*                 Else the DOMElement meeting the criteria is returned.
*
* Inputs: doc - document in which to find node.
*         tag - The tag name of the node to find.
*         attName - the name of the attribute the check for.
*         attValue - the value of the attribute to check for.
*
* Outputs:  Error messages to stderr.
*
* Returns: DOMNode specified, NULL if DOM node can not be found or if an exception is encountered.

*
****************************************************************************************************/

DOMNode * FindNode(DOMDocument * doc, NV_CHAR *tag, NV_CHAR *attName, NV_CHAR *attValue)
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


/************************************************************************************************
*
* Function FindNode
*
* Purpose:  This function finds a node in the node tree defined by the following criteria:
*           tag = a valid tag name, attName = NULL, attValue = NULL. 
*                 The first element with the tag in the node tree is made the current node.
*           tag = a valid tag name, attName = A valide Attribute Name, attValue = a value.
*                 The first element with the tag in the node tree that has the corresponding 
*                 attribute information is made current.
*
*           If no element meeting the specified criteria is found, then a NULL DOMElement is returned.
*           Else the DOMElement meeting the criteria is returned.
*
* Inputs: searchElement - DOMElement on which to find a node.
*         tag - The tag name of the node to find.
*         attName - the name of the attribute the check for.
*         attValue - the value of the attribute to check for.
*
*
* Outputs:  Error messages to stderr.
*
* Returns: DOMNode specified, NULL if DOM node can not be found or if an exception is encountered.
*
****************************************************************************************************/

DOMNode * FindNode(DOMElement * searchElement, NV_CHAR *tag, NV_CHAR *attName, NV_CHAR *attValue)
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

/***************************************************************************************************
*
* Function: GetNodeValue
*
* Purpose:  This function gets the value of a given DOMNode. 
*
* Inputs:   someNode  - Node from which to get value.
*           val       - String in which to load  node value
*           valLength - maxiumum allowable length of the value to be retrieved from the given node.
*
* Outputs:  Error messages to stderr.
*
* Returns:   1 = Sucessfully got the value
*           -1 = Could not get the value 
*
****************************************************************************************************/

NV_INT16 GetNodeValue(DOMNode * someNode, NV_CHAR * val, NV_INT16 valLength)
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

        NV_CHAR * message = XMLString::transcode(toCatch.msg);
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






/**************************************************************************************************
*
* Function  SetAttribute
*
* Purpose:  This function sets the value of an attribute of the given DOMElement.  
*           If the attribute has already been set then the value is changed.  
*
* Inputs:   someElement - element on which to set attribute.
*           attName  - the name of the attribute on which the value should be set.
*           attValue - the value of the attribute.
*
* Outputs:  Error messages to stderr.
*
* Returns:  NV_INT - status
*            1  =  Successful
*           -2  =  DOM Exception encountered.
*           -3  =  An exception was encountered while attempting to set an attribute.
*
* Method : 
*
****************************************************************************************************/

NV_INT16 SetAttribute(DOMElement * someElement, NV_CHAR *attName, NV_CHAR *attValue)
{


   try 
   {

       /* Set the attribute */
       someElement->setAttribute(XMLString::transcode(attName),XMLString::transcode(attValue));

   }
   catch (const DOMException& toCatch) 
   {

       NV_CHAR * message = XMLString::transcode(toCatch.msg);
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



/******************************************************************************************************
*
* Function: SetContact
*
* Purpose:  Replaces the existing contact in the BAG metadata XML string.
*
* Inputs:   newContact - structure containing contact information that should be used as a replacement contact.
*           xmlString  - address of the BAG metadata XML string to alter.
*
*
* Outputs:      Error messages to stderr.
*
* Returns:      NV_INT32 = Length of XML string created (if successful). 
*                     -1 = Error initializing XERCES
*                     -2 = A DOMException occurred.
*                     -3 = Out of memory
*                    -10 = Could not create new CI_ResponsiblePary node. Make sure the "role" field has been specified in newContact.
*                    -11 = Can't find node to replace.
*                    -20 = an exception occurred while attempting to set the contact node.
*
* Assumptions:  It is assumed that the supplied XML string is 
*               well-formed, valid XML.  
*
* Notes:        Currently the existence of one and only one contact node supported. In the future this method may be expanded to allow multiple "contact" nodes.  
*
*******************************************************************************************************/

NV_INT32 SetContact(RESPONSIBLE_PARTY newContact,  NV_U_CHAR ** xmlString)
{


    NV_INT16 errorCode = 1         /* Assume success. */;
    NV_CHAR *  gMemBufId = "metadata";
 

    DOMBuilder *myParser;    /* This will parse the given XML string into a DOM document. */
    //DOMAbstractParser *myParser;    /* This will parse the given XML string into a DOM document. */
    DOMDocument * xmlDoc = NULL;

    DOMWriter *theSerializer = NULL;

    /* Initialize XERCES. */
    if (!XMLinitialized)
    {
        try
        {
            XMLPlatformUtils::Initialize();
        }
        catch(const XMLException& toCatch)
        {

            NV_CHAR * message = XMLString::transcode(toCatch.getMessage());
            fprintf(stderr,"Exception message is: %s\n", message);
            XMLString::release(&message);
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
 
      
    try
    {

   

        /* Init the parser. */
        myParser = ((DOMImplementationLS*)impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS,NULL);
        //myParser =impl->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS,0);

        /* Set up to ignore whitespace between elements. (Unfortunately this feature does not work in current version of Xerces. )*/
        //myParser->setFeature(XMLUni::fgDOMWhitespaceInElementContent,false);

		
        /* Another attempt to ignore whitespace between elements.  Also does not work!. */
        //XercesDOMParser* parser = new XercesDOMParser();
        //parser->setIncludeIgnorableWhitespace(false); 
		//parser->parse((NV_CHAR *)xmlString);


        /* Set up memory input source (XML string) to be parsed.*/

        MemBufInputSource* memBufIS = new MemBufInputSource (  (const XMLByte*)*xmlString , strlen((NV_CHAR *)*xmlString), gMemBufId, false);
        Wrapper4InputSource wrap(memBufIS,false);   

        xmlDoc = myParser->parse(wrap);
  
     
        //1.9.2009 - E. Warner - Set encoding to UTF-8
        XMLCh *temp = XMLString::transcode("UTF-8");
        xmlDoc->setEncoding(temp);
        XMLString::release(&temp);


        /* Attempt to create the node to add/replace.     */
        /* If node can not be created, return error code. */
        DOMElement * newResponsiblePartyElem =  createCI_ResponsiblePartyNode(newContact, xmlDoc);
    
        if( newResponsiblePartyElem == NULL)
        {

            throw (NV_INT16)-10;
        }

 

           
        /* Find the contact node in the given XML string. */
        DOMNode * contactNode = NULL;  
        contactNode = FindNode(xmlDoc, "contact", NULL, NULL);
	

        if( contactNode == NULL)
        {

        throw (NV_INT16)-11;
        }

        /* Get the responsible party node and replace it. */
        DOMNode * responsiblePartyNode = NULL;
        responsiblePartyNode = FindNode((DOMElement*)contactNode, "smXML:CI_ResponsibleParty", NULL, NULL);
        
        if( responsiblePartyNode == NULL)
        {

            throw (NV_INT16)-11;
        }
		
		                  
        /* Replace the old responsible party node with the new responsible party node.*/
        contactNode->replaceChild(newResponsiblePartyElem, responsiblePartyNode);
	                      
	 

        /* Serialize the altered document. */
        theSerializer = ((DOMImplementationLS*)impl)->createDOMWriter();



        /* 1.9.2009 - E. Warner - Set encoding to UTF-8. */
        /* Write the document to a string.  Done this way because must use writeNode in order for setEncoding method to work.*/
        MemBufFormatTarget *myFormTarget;
        myFormTarget = new MemBufFormatTarget();
        bool wroteBuffer = theSerializer->writeNode(myFormTarget,*xmlDoc);

        if(wroteBuffer)
        {

            errorCode = myFormTarget->getLen();

            *xmlString = (NV_U_CHAR *)realloc(*xmlString, errorCode * sizeof(NV_U_CHAR));

            *xmlString = (NV_U_CHAR *)myFormTarget->getRawBuffer();
        }
        else
        {
            throw (NV_CHAR *)"Could not write buffer. ";

        }

    }

    catch (const DOMException& e)
    {        
        fprintf(stderr,"Exception while attempting to set contact.  DOMException code %d\n", e.code); 
        errorCode = -2;
    }
    catch (const OutOfMemoryException& m)
    {          
            
        fprintf(stderr,"Exception while attempting to set contact.  Exception message is: OutOfMemoryException\n");
        errorCode = -3;

    }
    catch (NV_INT16 intException)
    {          
            
        fprintf(stderr,"ERROR: Exception while attempting to set contact.  Exception message is: %d\n", intException );
        errorCode = intException;
	   

    }
    catch(...)
    {

        fprintf(stderr,"ERROR: Exception while attempting to set contact.\n");
        errorCode = -20;

    }

  
    /* Release the resources. */
    xmlDoc->release();

    if(theSerializer != NULL)
    {
        theSerializer->release();
    }
   
    TerminateXML();

    return errorCode;  



}



/******************************************************************************************************
*
* Function: SetIdentificationInfo
*
* Purpose:  Replaces an existing IdentificationInfo node in a BAG metadata XML string.
*
* Inputs:   newIdentificationInfo - structure containing IdentificationInfo information that should be added or used as a replacement 
*           xmlString - address of the XML string to alter.        
*
* Outputs:  Error messages to stderr.
*
* Returns:  NV_INT32 = Length of XML string created (if successful). 
*                 -1 = Error initializing XERCES
*                 -2 = A DOMException occurred.
*                 -3 = Out of memory
*                 -4 = Could not make node from information supplied in newIdentificationInfo.
*                -11 = Can't find node to replace.
*                -20 = An exception occurred while attempting to set identification info.
*
* Assumptions:  The supplied XML string is well-formed.
*
*
* Notes:        Currently the library supports only occurrence of the identificationInfo node.  In the future this method may be expanded to allow multiple nodes of this type.  
*
*******************************************************************************************************/

NV_INT32 SetIdentificationInfo(IDENTIFICATION_INFO newIdentificationInfo,
                               NV_U_CHAR ** xmlString)
{


    NV_INT16 errorCode = 1         /* Assume success. */;
    NV_CHAR *  gMemBufId = "metadata";
 

    DOMBuilder *myParser;    /* This will parse the given XML string into a DOM document. */
    DOMDocument* xmlDoc = NULL;
    DOMWriter *theSerializer = NULL;


    /* Initialize XERCES. */
    if (!XMLinitialized)
    {
        try
        {
            XMLPlatformUtils::Initialize();
        }
        catch(const XMLException& toCatch)
        {

            NV_CHAR * message = XMLString::transcode(toCatch.getMessage());
            fprintf(stderr,"Exception message is: %s\n", message);
            XMLString::release(&message);
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
 
      
    try
    {


        /* Init the parser. */
        myParser = ((DOMImplementationLS*)impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS,0);



        /* Set up memory input source (XML string) to be parsed.*/
        MemBufInputSource* memBufIS = new MemBufInputSource (  (const XMLByte*)*xmlString , strlen((NV_CHAR *)*xmlString), gMemBufId, false);

        Wrapper4InputSource wrap(memBufIS,false);   
        xmlDoc = myParser->parse(wrap);



        //1.9.2009 - E. Warner - Set encoding to UTF-8
        XMLCh *temp = XMLString::transcode("UTF-8");
        xmlDoc->setEncoding(temp);
        XMLString::release(&temp);


        
        /*Prepare for adding or replacing BAG_DataIdentification node.  Create the new/replacement node. */
        DOMElement * newDataIdentificationElem;
        newDataIdentificationElem =  createBAG_DataIdentificationNode(newIdentificationInfo, xmlDoc);


        /* if there was a problem creating node to replace/add, return. */
        if(newDataIdentificationElem == NULL)
        {

            fprintf(stderr, "ERROR: Could not make a BAG_DataIdentification node to add/replace. Check information supplied to ensure that the required fields have been supplied.\n");
            throw (NV_INT16) -4;

        }



        DOMNode * bagIdentificationNode = NULL;
        bagIdentificationNode = FindNode(xmlDoc, "smXML:BAG_DataIdentification", NULL, NULL);


        if( bagIdentificationNode == NULL)
        {

            throw (NV_INT16)-11;
        }

        DOMNode * parentNode = NULL;
        parentNode = bagIdentificationNode->getParentNode();
        if(parentNode == NULL)
        {

            throw (NV_INT16)-11;
        }

        parentNode->replaceChild(newDataIdentificationElem, bagIdentificationNode);
 

        //identificationInfoNode->replaceChild(newDataIdentificationElem, bagIdentificationNode);



        /* Serialize the altered document. */
        theSerializer = ((DOMImplementationLS*)impl)->createDOMWriter();



        /* 1.9.2009 - E. Warner - Set encoding to UTF-8. */
        /* Write the document to a string.  Done this way because must use writeNode in order for setEncoding method to work.*/
        MemBufFormatTarget *myFormTarget;
        myFormTarget = new MemBufFormatTarget();
        bool wroteBuffer = theSerializer->writeNode(myFormTarget,*xmlDoc);

        if(wroteBuffer)
        {

            errorCode = myFormTarget->getLen();

            *xmlString = (NV_U_CHAR *)realloc(*xmlString, errorCode * sizeof(NV_U_CHAR));

            *xmlString = (NV_U_CHAR *)myFormTarget->getRawBuffer();
        }
        else
        {
            throw (NV_CHAR *)"Could not write buffer. ";

        }

    }

    catch (const DOMException& e)
    {        
        fprintf(stderr,"Exception while attempting to set BAG identification info.  DOMException code %d\n", e.code); 
        errorCode = -2;
    }

    catch (const OutOfMemoryException& m)
    {          
            
        fprintf(stderr,"Exception while attempting to set BAG identification info. Exception message is: OutOfMemoryException\n");
        errorCode = -3;

    }
     catch (NV_INT16 intException)
    {          
            
        fprintf(stderr,"ERROR: Exception while attempting to set BAG identification info.  Exception message is: %d\n", intException );
        errorCode = intException;

    }
    catch(...)
    {

	    fprintf(stderr,"ERROR: An exception was encountered while attempting to set BAG identificationInfo.\n"); 
	    errorCode = -20;

    }

  

    /* Release the resources. */
    xmlDoc->release();
   
    if(theSerializer != NULL)
    {
        theSerializer->release();
    }

    TerminateXML();

    return errorCode; 


}


/******************************************************************************************************
*
* Function: SetLegalConstraints
*
* Purpose:  Replaces an existing legal constraint in the BAG metadata.
*
* Inputs:   newLegalConstraints - structure containing contact information that should be added or used as a replacement 
*           xmlString - address of the XML string to alter.
*
* Outputs:  Error messages to stderr.
*
* Returns:  NV_INT32 = Length of XML string created (if successful). 
*                 -1 = Error initializing XERCES
*                 -2 = A DOMException occurred.
*                 -3 = Out of memory
*                -11 = Can't find node to replace.
*                -20 = An exception occurred while attempting to set legal constraints.
*
* Assumptions:  The supplied XML string is well-formed.
*
*
* Notes:        Currently the library supports only occurrence of the identificationInfo node.
*               In the future this method may be expanded to allow multiple nodes of this type.  
*
*******************************************************************************************************/

NV_INT32 SetLegalConstraints(MD_LEGAL_CONSTRAINTS newLegalConstraints,  
                             NV_U_CHAR ** xmlString)

{



    NV_INT16 errorCode = 1         /* Assume success. */;
    NV_CHAR *  gMemBufId = "metadata";
 

    DOMBuilder *myParser;    /* This will parse the given XML string into a DOM document. */
    DOMDocument* xmlDoc = NULL;
    DOMWriter *theSerializer = NULL;


    /* Initialize XERCES. */
    if (!XMLinitialized)
    {
        try
        {
            XMLPlatformUtils::Initialize();
        }
        catch(const XMLException& toCatch)
        {

            NV_CHAR * message = XMLString::transcode(toCatch.getMessage());
            fprintf(stderr,"Exception message is: %s\n", message);
            XMLString::release(&message);
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
 
      
    try
    {

        /* Init the parser. */
        myParser = ((DOMImplementationLS*)impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS,NULL);



        /* Set up memory input source (XML string) to be parsed.*/
        MemBufInputSource* memBufIS = new MemBufInputSource (  (const XMLByte*)*xmlString , strlen((NV_CHAR *)*xmlString), gMemBufId, false);
        Wrapper4InputSource wrap(memBufIS,false);   
        xmlDoc = myParser->parse(wrap);


        /* 1.9.2009 - E. Warner - Set encoding to UTF-8. */
        XMLCh *temp = XMLString::transcode("UTF-8");
        xmlDoc->setEncoding(temp);
        XMLString::release(&temp);



        /*Prepare for adding or replacing MD_LegalConstraints node.  Create the new/replacement node. */
        DOMElement     *newLegalConstraintsElem;
        newLegalConstraintsElem =  createMD_LegalConstraintsNode(newLegalConstraints, xmlDoc);


         
        /* Find MD_LegalConstraints node in the given XML string. */
        DOMNode * constraintNode = NULL;
        constraintNode = FindNode(xmlDoc, "smXML:MD_LegalConstraints", NULL, NULL);
        if(constraintNode == NULL)
        {

            throw (NV_INT16) -11;
        }

        DOMNode * parentNode = constraintNode->getParentNode();
        parentNode->replaceChild(newLegalConstraintsElem, constraintNode);


        /* Serialize the altered document. */
        theSerializer = ((DOMImplementationLS*)impl)->createDOMWriter();



        /*1.9.2009 - E. Warner - Set encoding to UTF-8. */
        /* Write the document to a string.  Done this way because must use writeNode in order for setEncoding method to work.*/
        MemBufFormatTarget *myFormTarget;
        myFormTarget = new MemBufFormatTarget();
        bool wroteBuffer = theSerializer->writeNode(myFormTarget,*xmlDoc);

        if(wroteBuffer)
        {

            errorCode = myFormTarget->getLen();

            *xmlString = (NV_U_CHAR *)realloc(*xmlString, errorCode * sizeof(NV_U_CHAR));

            *xmlString = (NV_U_CHAR *)myFormTarget->getRawBuffer();
        }
        else
        {
            throw (NV_CHAR *)"Could not write buffer. ";

        }
        
        

    }

    catch (const DOMException& e)
    {        
        fprintf(stderr,"Exception while attempting to set legal constraints.  DOMException code %d\n", e.code);
        errorCode = -2;
    }
    catch (const OutOfMemoryException& m)
    {          
            
        fprintf(stderr,"Exception while attempting to set legal constraints.Exception message is: OutOfMemoryException\n");
        errorCode = -3;

    }
    catch (NV_INT16 intException)
    {          
            
        fprintf(stderr,"ERROR: Exception while attempting to set legal constraints.  Exception message is: %d\n", intException );
        errorCode = intException;

    }
    catch(...)
    {

        fprintf(stderr,"ERROR: Exception while attempting to set legal constraints.\n");
        errorCode = -20;

    }

  
    /* Release the resources. */
    xmlDoc->release();

    if(theSerializer != NULL)
    {
        theSerializer->release();
    }
   
    TerminateXML();
 
    return errorCode; 



}


/******************************************************************************************************
*
* Function: SetSecurityConstraints
*
* Purpose:  Replaces an existing security constraint in the BAG metadata.
*
* Inputs:   newSecurityConstraints - structure containing Security Constraint information that should be added or used as a replacement 
*           xmlString - address of the XML string to alter.
*
* Outputs:  Error messages to stderr.
*
* Returns:  NV_INT32 = Length of XML string created (if successful). 
*                 -1 = Error initializing XERCES
*                 -2 = A DOMException occurred.
*                 -3 = Out of memory
*                -11 = Can't find node to replace.
*                -20 = An exception occurred while attempting to set security constraints.
*
* Assumptions:  The supplied XML string is well-formed.
*
*               For the purpose of replacing oldSecurityConstraints with newSecurityConstraints, it is assumed that the supplied XML string is 
*               well-formed, valid XML.
* 
* Notes:        XML is constrained to contain only one MD_SecurityConstraints node.
*               In the future this method may be expanded to allow multiple nodes of this type. 
*
*******************************************************************************************************/
NV_INT32 SetSecurityConstraints(MD_SECURITY_CONSTRAINTS newSecurityConstraints, 
                                NV_U_CHAR ** xmlString)
{


    NV_INT16 errorCode = 1         /* Assume success. */;
    NV_CHAR *  gMemBufId = "metadata";
 

    DOMBuilder *myParser;    /* This will parse the given XML string into a DOM document. */
    DOMDocument* xmlDoc = NULL;
    DOMWriter *theSerializer = NULL;


    /* Initialize XERCES. */
    if (!XMLinitialized)
    {
        try
        {
            XMLPlatformUtils::Initialize();
        }
        catch(const XMLException& toCatch)
        {

            NV_CHAR * message = XMLString::transcode(toCatch.getMessage());
            fprintf(stderr,"Exception message is: %s\n", message);
            XMLString::release(&message);
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
 
      
    try
    {
        /* Init the parser. */
        myParser = ((DOMImplementationLS*)impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS,NULL);



        /* Set up memory input source (XML string) to be parsed.*/
        MemBufInputSource* memBufIS = new MemBufInputSource (  (const XMLByte*)*xmlString , strlen((NV_CHAR *)*xmlString), gMemBufId, false);
        Wrapper4InputSource wrap(memBufIS,false);   
        xmlDoc = myParser->parse(wrap);


        /* 1.9.2009 - E. Warner - Set encoding to UTF-8. */
        XMLCh *temp = XMLString::transcode("UTF-8");
        xmlDoc->setEncoding(temp);
        XMLString::release(&temp);


        /*Prepare for adding or replacing MD_SecurityConstraints node.  Create the new/replacement node. */
        DOMElement     *newSecurityConstraintsElem;
        newSecurityConstraintsElem =  createMD_SecurityConstraintsNode(newSecurityConstraints, xmlDoc);


        /* Get a list of MD_SecurityConstraints nodes in the given XML string. */
        DOMNode * constraintNode = NULL;
        constraintNode = FindNode(xmlDoc, "smXML:MD_SecurityConstraints", NULL,NULL);


        /* Replace the old constraint node with the new constraint node.*/
        DOMNode * parentNode = constraintNode->getParentNode();
        parentNode->replaceChild(newSecurityConstraintsElem, constraintNode);



        /* Serialize the altered document. */
        theSerializer = ((DOMImplementationLS*)impl)->createDOMWriter();



        /* 1.9.2009 - E. Warner - Set encoding to UTF-8. */
        /* Write the document to a string.  Done this way because must use writeNode in order for setEncoding method to work.*/
        MemBufFormatTarget *myFormTarget;
        myFormTarget = new MemBufFormatTarget();
        bool wroteBuffer = theSerializer->writeNode(myFormTarget,*xmlDoc);

        if(wroteBuffer)
        {

            errorCode = myFormTarget->getLen();

            *xmlString = (NV_U_CHAR *)realloc(*xmlString, errorCode * sizeof(NV_U_CHAR));

            *xmlString = (NV_U_CHAR *)myFormTarget->getRawBuffer();
        }
        else
        {
            throw (NV_CHAR *)"Could not write buffer. ";

        }

    }

    catch (const DOMException& e)
    {        
        fprintf(stderr,"Exception while attempting to set security constraints.  DOMException code %d\n", e.code);
        errorCode = -2;
    }
    catch (const OutOfMemoryException& m)
    {          
            
        fprintf(stderr,"Exception while attempting to set security constraints. Exception message is: OutOfMemoryException\n");
        errorCode = -3;

    }
    catch (NV_INT16 intException)
    {          
            
        fprintf(stderr,"ERROR: Exception while attempting to set security constraints.  Exception message is: %d\n", intException );
        errorCode = intException;

    }
    catch(...)
    {

        fprintf(stderr,"ERROR: Exception while attempting to set security constraints.\n");
        errorCode = -20;

    }

  
    /* Release the resources. */
    xmlDoc->release();

    if(theSerializer != NULL)
    {
        theSerializer->release();
    }
  
    TerminateXML();
 
    return errorCode; 


}


/******************************************************************************************************
*
* Function: SetDataQualityInfo
*
* Purpose:  Replaces an existing dataQualityInfo node in the BAG metadata.
*
* Inputs:   newDataQualityInfo - structure containing DataQualityInfo information that should be added or used as a replacement 
*           xmlString - address of the XML string to alter.
*
* Outputs:  Error messages to stderr.
*
* Returns:  NV_INT32 = Length of XML string created (if successful). 
*                 -1 = Error initializing XERCES
*                 -2 = A DOMException occurred.
*                 -3 = Out of memory
*                -11 = Can't find node to add.
*                -20 = An exception occurred while trying to set the dataQualityInfo node.
*       
* Assumptions:  The supplied XML string is well-formed.
*
*
* Notes:        XML is constrained to contain only one dataQualityInfo node.
*               In the future this method may be expanded to allow multiple nodes of this type. 
*                 
*
*******************************************************************************************************/
NV_INT32 SetDataQualityInfo(DATA_QUALITY_INFO newDataQualityInfo, 
                            NV_U_CHAR ** xmlString)
{


    NV_INT16 errorCode = 1         /* Assume success. */;
    NV_CHAR *  gMemBufId = "metadata";
 

    DOMBuilder *myParser;    /* This will parse the given XML string into a DOM document. */
    DOMDocument* xmlDoc = NULL;
    DOMWriter *theSerializer = NULL;


    /* Initialize XERCES. */
    if (!XMLinitialized)
    {
        try
        {
            XMLPlatformUtils::Initialize();
        }
        catch(const XMLException& toCatch)
        {

            NV_CHAR * message = XMLString::transcode(toCatch.getMessage());
            fprintf(stderr,"Exception message is: %s\n", message);
            XMLString::release(&message);
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
 
      
    try
    {

        /* Init the parser. */
        myParser = ((DOMImplementationLS*)impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS,NULL);



        /* Set up memory input source (XML string) to be parsed.*/
        MemBufInputSource* memBufIS = new MemBufInputSource (  (const XMLByte*)*xmlString , strlen((NV_CHAR *)*xmlString), gMemBufId, false);
        Wrapper4InputSource wrap(memBufIS,false);   
        xmlDoc = myParser->parse(wrap);


        /* 1.9.2009 - E. Warner - Set encoding to UTF-8. */
        XMLCh *temp = XMLString::transcode("UTF-8");
        xmlDoc->setEncoding(temp);
        XMLString::release(&temp);



        /*Prepare for adding or replacing DQ_DataQuality node.  Create the new/replacement node. */
        DOMElement *newDqDataQualityElem;
        newDqDataQualityElem =  createDQ_DataQualityNode(newDataQualityInfo, xmlDoc);


         
        /* Get a list of DQ_DataQuality nodes in the given XML string.    */
        /* There should be only one per Navigation Specific constraints on this element. */
        DOMNode * dataQualityNode = NULL;
        dataQualityNode = FindNode(xmlDoc, "smXML:DQ_DataQuality", NULL, NULL);


        DOMNode * parentNode = dataQualityNode->getParentNode();
        parentNode->replaceChild(newDqDataQualityElem, dataQualityNode);
                  



        /* Serialize the altered document. */
        theSerializer = ((DOMImplementationLS*)impl)->createDOMWriter();



        /* 1.9.2009 - E. Warner - Set encoding to UTF-8. */
        /* Write the document to a string.  Done this way because must use writeNode in order for setEncoding method to work.*/
        MemBufFormatTarget *myFormTarget;
        myFormTarget = new MemBufFormatTarget();
        bool wroteBuffer = theSerializer->writeNode(myFormTarget,*xmlDoc);

        if(wroteBuffer)
        {

            errorCode = myFormTarget->getLen();

            *xmlString = (NV_U_CHAR *)realloc(*xmlString, errorCode * sizeof(NV_U_CHAR));

            *xmlString = (NV_U_CHAR *)myFormTarget->getRawBuffer();
        }
        else
        {
            throw (NV_CHAR *)"Could not write buffer. ";

        }

    }

    catch (const DOMException& e)
    {        
        fprintf(stderr,"ERROR: Exception while attempting to set data quality info.  DOMException code %d\n", e.code);
        errorCode = -2;
    }
    catch (const OutOfMemoryException& m)
    {          
            
        fprintf(stderr,"ERROR:Exception while attempting to set data quality info.  Exception message is: OutOfMemoryException\n");
        errorCode = -3;

    }
    catch (NV_INT16 intException)
    {          
            
        fprintf(stderr,"ERROR: Exception while attempting to set data quality info.  Exception message is: %d\n", intException );
        errorCode = intException;

    }
    catch(...)
    {

        fprintf(stderr,"ERROR:Exception while attempting to set data quality info.\n"); 
        errorCode = -20;

    }

  

     /* Release the resources. */
    xmlDoc->release();
   
    if(theSerializer != NULL)
    {
        theSerializer->release();
    }

    TerminateXML();
    return errorCode; 



}


/******************************************************************************************************
*
* Function: SetSpatialRepresentationInfo
*
* Purpose:  Replaces an existing spatialRepresentationInfo in the BAG metadata.
*
* Inputs:   newSpatialRepresentationInfo - structure containing spatialRepresentationInfo that should be added or uses a replacement 
*           xmlString - address of the XML string to alter.
*
* Outputs:  Error messages to stderr.
*
* Returns:  NV_INT32 = Length of XML string created (if successful). 
*                 -1 = Error initializing XERCES
*                 -2 = A DOMException occurred.
*                 -3 = Out of memory
*                -11 = Can't find node to add.
*                -20 = An exception occurred while attempting to set spatialRepresentationInfo.
*                  
* Assumptions:  The supplied XML string is well-formed.
*.
*
* Notes:       XML is constrained to contain only one spatialRepresentationInfo node.
*              In the future this method may be expanded to allow multiple nodes of this type. 
*
*******************************************************************************************************/
NV_INT32 SetSpatialRepresentationInfo(SPATIAL_REPRESENTATION_INFO newSpatialRepresentationInfo, 
                                      NV_U_CHAR ** xmlString)
{


    NV_INT16 errorCode = 1         /* Assume success. */;
    NV_CHAR *  gMemBufId = "metadata";
 

    DOMBuilder *myParser;    /* This will parse the given XML string into a DOM document. */
    DOMDocument* xmlDoc = NULL;
    DOMWriter *theSerializer = NULL;


    /* Initialize XERCES. */
    if (!XMLinitialized)
    {
        try
        {
            XMLPlatformUtils::Initialize();
        }
        catch(const XMLException& toCatch)
        {

            NV_CHAR * message = XMLString::transcode(toCatch.getMessage());
            fprintf(stderr,"Exception message is: %s\n", message);
            XMLString::release(&message);
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
 
      
    try
    {
        /* Init the parser. */
        myParser = ((DOMImplementationLS*)impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS,NULL);



        /* Set up memory input source (XML string) to be parsed.*/
        MemBufInputSource* memBufIS = new MemBufInputSource (  (const XMLByte*)*xmlString , strlen((NV_CHAR *)*xmlString), gMemBufId, false);
        Wrapper4InputSource wrap(memBufIS,false);   
        xmlDoc = myParser->parse(wrap);


        /* 1.9.2009 - E. Warner - Set encoding to UTF-8. */
        XMLCh *temp = XMLString::transcode("UTF-8");
        xmlDoc->setEncoding(temp);
        XMLString::release(&temp);


        /* Prepare for adding or replacing MD_Georectified node.  Create the new/replacement node. */
        DOMElement     *newMdGeorectifiedElem = NULL;
        newMdGeorectifiedElem =  createMD_GeorectifiedNode(newSpatialRepresentationInfo, xmlDoc);

        if(newMdGeorectifiedElem == NULL)
        {
            throw (NV_INT16) -11;
        }
         
        /* Get a list of MD_Georectified nodes in the given XML string. */
        DOMNode * mdGeorectifiedNode = NULL;
        mdGeorectifiedNode = FindNode(xmlDoc, "smXML:MD_Georectified", NULL, NULL);
        if(mdGeorectifiedNode == NULL)
        {
            throw (NV_INT16) -11;
        }


        /* Replace the old MD_Georectified node with the new MD_Georectified node.*/
        DOMNode * parentNode = mdGeorectifiedNode->getParentNode();
        if(parentNode == NULL)
        {
            throw (NV_INT16) -11;
        }
        parentNode->replaceChild(newMdGeorectifiedElem, mdGeorectifiedNode);


        /* Serialize the altered document. */
        theSerializer = ((DOMImplementationLS*)impl)->createDOMWriter();



        /* 1.9.2009 - E. Warner - Set encoding to UTF-8. */
        /* Write the document to a string.  Done this way because must use writeNode in order for setEncoding method to work.*/
        MemBufFormatTarget *myFormTarget;
        myFormTarget = new MemBufFormatTarget();
        bool wroteBuffer = theSerializer->writeNode(myFormTarget,*xmlDoc);

        if(wroteBuffer)
        {

            errorCode = myFormTarget->getLen();

            *xmlString = (NV_U_CHAR *)realloc(*xmlString, errorCode * sizeof(NV_U_CHAR));

            *xmlString = (NV_U_CHAR *)myFormTarget->getRawBuffer();
        }
        else
        {
            throw (NV_CHAR *)"Could not write buffer. ";

        }
       

    }

    catch (const DOMException& e)
    {             
        fprintf(stderr,"ERROR: Exception when attempting to set spatial representation info. DOMExceptionCode: %d\n", e.code);   
        errorCode = -2;
    }
    catch (const OutOfMemoryException& m)
    {          
            
        fprintf(stderr,"ERROR: Exception when attempting to set spatial representation info. Exception message is: OutOfMemoryException\n");
        errorCode = -3;

    }
    catch (NV_INT16 intException)
    {          
            
        fprintf(stderr,"ERROR: Exception while attempting to set spatial representation info.  Exception message is: %d\n", intException );
        errorCode = intException;

    }
    catch(...)
    {

        fprintf(stderr,"ERROR: Exception when attempting to set spatial representation info.\n");
        errorCode = -20;

    }

    /* Release the resources. */
    xmlDoc->release();

    if(theSerializer != NULL)
    {
        theSerializer->release();
    }
   
    TerminateXML();

    return errorCode; 



}


/******************************************************************************************************
*
* Function: SetReferenceSystemInfo
*
* Purpose:  Replaces an existing referenceSystemInfo in the BAG metadata XML string.
*
* Inputs:   newReferenceSystemInfo - structure containing referenceSystemInfo information that should be added or used as a replacement 
*           xmlString - address of the XML string to alter.

* Outputs:  Error messages to stderr.
*
* Returns:  NV_INT32 = Length of XML string created (if successful). 
*                 -1 = Error initializing XERCES
*                 -2 = A DOMException occurred.
*                 -3 = Out of memory
*                -10 = Problem creating one or both referenceSystemInfo nodes.                  
*                -11 = Can't find node to replace.
*                -20 = An exception occurred while attempting to set referenceSystemInfo
*
*
* Assumptions:  The supplied XML string is well-formed.
*
* Notes:        XML is constrained to contain only two referenceSystemInfo nodes.  One for horizontal datum, etc.  The second node is for vertical datum only,
*               In the future this method may be expanded to allow more than two nodes of this type. 
*
*******************************************************************************************************/
NV_INT32 SetReferenceSystemInfo(REFERENCE_SYSTEM_INFO newReferenceSystemInfo,  
                                NV_U_CHAR ** xmlString)
{

    NV_INT16 errorCode = 1    /* Assume success. */;
    NV_CHAR *  gMemBufId = "metadata";
 

    DOMBuilder *myParser;    /* This will parse the given XML string into a DOM document. */
    DOMDocument* xmlDoc = NULL;
    DOMWriter *theSerializer = NULL;


    /* Initialize XERCES. */
    if (!XMLinitialized)
    {
        try
        {
            XMLPlatformUtils::Initialize();
        }
        catch(const XMLException& toCatch)
        {

            NV_CHAR * message = XMLString::transcode(toCatch.getMessage());
            fprintf(stderr,"Exception message is: %s\n", message);
            XMLString::release(&message);
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
 
      
    try
    {

        /* Init the parser. */
        myParser = ((DOMImplementationLS*)impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS,NULL);

 

        /* Set up memory input source (XML string) to be parsed.*/
        MemBufInputSource* memBufIS = new MemBufInputSource ((const XMLByte*)*xmlString , strlen((NV_CHAR *)*xmlString), gMemBufId, false);
        Wrapper4InputSource wrap(memBufIS,false);   
        xmlDoc = myParser->parse(wrap);


        /* 1.9.2009 - E. Warner - Set encoding to UTF-8. */
        XMLCh *temp = XMLString::transcode("UTF-8");
        xmlDoc->setEncoding(temp);
        XMLString::release(&temp);


        /* Prepare for adding or replacing MD_CRS nodes.  */
        /* We have two nodes to handle.  One contains horizontal dataum, etc.  the second node contains verticalDatum only. */
        /* Create the new/replacement nodes. */
        DOMElement * newMdCrsElem1 =  createMD_CRSNode(newReferenceSystemInfo, xmlDoc);
        DOMElement * newMdCrsElem2 =  createMD_CRSNode(newReferenceSystemInfo. verticalDatum, xmlDoc);



        /* Get a list of MD_CRS nodes in the given XML string. */
        DOMNodeList * mdCrsNodeList  =  xmlDoc->getElementsByTagName(X("smXML:MD_CRS"));
        XMLSize_t length = mdCrsNodeList->getLength();

        if(length < 2)
        {
            throw (NV_INT16) -11;
        }


        
        DOMNode * firstNodeToReplace  = NULL;
        DOMNode * secondNodeToReplace = NULL;


  
        /* Hold this node as the first node to replace. */
        firstNodeToReplace = (DOMNode *)mdCrsNodeList->item(0);

        /* Hold this node as final node to replace. */
        secondNodeToReplace = (DOMNode *)mdCrsNodeList->item(1);



        /* Replace the old MD_CRS nodes with the new MD_CRS nodes.*/
        DOMNode * parentNode = firstNodeToReplace->getParentNode();
        parentNode->replaceChild(newMdCrsElem1, firstNodeToReplace);

        parentNode = secondNodeToReplace->getParentNode();
        parentNode->replaceChild(newMdCrsElem2, secondNodeToReplace);


                       
        /* Serialize the altered document. */
        theSerializer = ((DOMImplementationLS*)impl)->createDOMWriter();



        /* 1.9.2009 - E. Warner - Set encoding to UTF-8. */
        /* Write the document to a string.  Done this way because must use writeNode in order for setEncoding method to work.*/
        MemBufFormatTarget *myFormTarget;
        myFormTarget = new MemBufFormatTarget();
        bool wroteBuffer = theSerializer->writeNode(myFormTarget,*xmlDoc);

        if(wroteBuffer)
        {

            errorCode = myFormTarget->getLen();

            *xmlString = (NV_U_CHAR *)realloc(*xmlString, errorCode * sizeof(NV_U_CHAR));

            *xmlString = (NV_U_CHAR *)myFormTarget->getRawBuffer();
        }
        else
        {
            throw (NV_CHAR *)"Could not write buffer. ";

        }

    }

    catch (const DOMException& e)
    {       
 
        fprintf(stderr,"ERROR: Exception while attempting to set reference system info. DOMException code %d\n", e.code);
        errorCode = -2;

    }
    catch (const OutOfMemoryException& m)
    {          
            
        fprintf(stderr,"ERROR: Exception while attempting to set reference system info. Exception message is: OutOfMemoryException\n");
        errorCode = -3;

    }
    catch (NV_INT16 intException)
    {          
            
        fprintf(stderr,"ERROR: Exception while attempting to set reference system info.  Exception message is: %d\n", intException );
        errorCode = intException;

    }
    catch(...)
    {

        fprintf(stderr,"ERROR: Exception while attempting to set reference system info.\n");
        errorCode = -20;

    }

 


    /* Release the resources. */
    xmlDoc->release();

    if(theSerializer != NULL)
    {
        theSerializer->release();
    }
	
    TerminateXML();

    return errorCode; 


}



/******************************************************************************************************
*
* Function: SetLanguage
*
* Purpose:  Adds the upper level "language" node or replaces it in the BAG metadata XML string.
*
* Inputs:   newLanguage - string containing language that should be added or used as a replacement. 
*           xmlString   - address of the XML string to alter.
*
*           If language node is present, value will be changed to reflect language specifed.
*           If language node is NOT present, language node will be added.
*
* Outputs:     Error messages to stderr.
*
* Returns:     NV_INT32 = Length of XML string created (if successful). 
*                    -1 = Error initializing XERCES
*                    -2 = A DOMException occurred.
*                    -3 = Out of memory
*                   -20 = An exception occurred while attempting to the langauge.
*        
* Assumptions:  For purpose of adding a newLanguage, it is assumed that 
*               1) previous sibling nodes of "language" node exist and that following sibling nodes do not exist. 
*				2) the supplied XML string is well-formed.
*
*               For the purpose of replacing the language node,  it is assumed that the supplied XML string is 
*               well-formed, valid XML.          
*
*******************************************************************************************************/
NV_INT32 SetLanguage(NV_CHAR * newLanguage,  
                     NV_U_CHAR ** xmlString)
{


    NV_INT16 errorCode = 1         /* Assume success. */;
    NV_CHAR *  gMemBufId = "metadata";
 

    DOMBuilder *myParser;    /* This will parse the given XML string into a DOM document. */
    DOMDocument* xmlDoc = NULL;
    DOMWriter *theSerializer = NULL;


    /* Initialize XERCES. */
    if (!XMLinitialized)
    {
        try
        {
            XMLPlatformUtils::Initialize();
        }
        catch(const XMLException& toCatch)
        {

            NV_CHAR * message = XMLString::transcode(toCatch.getMessage());
            fprintf(stderr,"Exception message is: %s\n", message);
            XMLString::release(&message);
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
 
      
    try
    {

        /* Init the parser. */
        myParser = ((DOMImplementationLS*)impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS,NULL);


        /* Set up memory input source (XML string) to be parsed.*/
        MemBufInputSource* memBufIS = new MemBufInputSource ((const XMLByte*)*xmlString , strlen((NV_CHAR *)*xmlString), gMemBufId, false);
        Wrapper4InputSource wrap(memBufIS,false);   
        xmlDoc = myParser->parse(wrap);


        /* 1.9.2009 - E. Warner - Set encoding to UTF-8. */
        XMLCh *temp = XMLString::transcode("UTF-8");
        xmlDoc->setEncoding(temp);
        XMLString::release(&temp);

        DOMNode * metadataNode = FindNode(xmlDoc, "smXML:MD_Metadata", NULL, NULL);


        /* Find the upper level node named "language" */
        DOMNodeList * languageNodeList = metadataNode->getChildNodes();
        XMLSize_t length = languageNodeList->getLength();
        DOMNode * tempNode = NULL;
        NV_CHAR * tempNodeName;
        NV_BOOL found = NVFalse;

        for (XMLSize_t c = 0; c < length; c++)
        {

            tempNode =  languageNodeList->item(c);
            tempNodeName = XMLString::transcode(tempNode->getNodeName());

            if(strcmp(tempNodeName, "language") == 0)
            {

                tempNode->setTextContent(X(newLanguage));
                found = NVTrue;
                break;
            }

        }
        
        /* If we went through all nodes and did not find the language node, create one and insert it.*/ 
        /* Assumes that the "contact" node exists. */	 
        if(!found)
	{
            DOMElement * languageElem = createElement("language", newLanguage, xmlDoc);
       
            /* Assume that previous siblings exist and that following siblings do not exist. */
            metadataNode->appendChild(languageElem);

        }



        /* Serialize the altered document. */
        theSerializer = ((DOMImplementationLS*)impl)->createDOMWriter();



        /* 1.9.2009 - E. Warner - Set encoding to UTF-8. */
        /* Write the document to a string.  Done this way because must use writeNode in order for setEncoding method to work.*/
        MemBufFormatTarget *myFormTarget;
        myFormTarget = new MemBufFormatTarget();
        bool wroteBuffer = theSerializer->writeNode(myFormTarget,*xmlDoc);

        if(wroteBuffer)
        {

            errorCode = myFormTarget->getLen();

            *xmlString = (NV_U_CHAR *)realloc(*xmlString, errorCode * sizeof(NV_U_CHAR));

            *xmlString = (NV_U_CHAR *)myFormTarget->getRawBuffer();
        }
        else
        {
            throw (NV_CHAR *)"Could not write buffer. ";

        }


    }

    catch (const DOMException& e)
    {        

        fprintf(stderr,"ERROR: Exception while attempting to set language. DOMException code %d\n", e.code);
        errorCode = -2;

    }
    catch (const OutOfMemoryException& m)
    {          
            
        fprintf(stderr,"ERROR: Exception while attempting to set language. Exception message is: OutOfMemoryException\n");
        errorCode = -3;

    }
    catch (NV_INT16 intException)
    {          
            
        fprintf(stderr,"ERROR: Exception while attempting to set language.  Exception message is: %d\n", intException );
        errorCode = intException;

    }
    catch(...)
    {

        fprintf(stderr,"ERROR: Exception while attempting to set language.\n");
        errorCode = -20;

    }

 
    /* Release the resources. */
    xmlDoc->release();

    if(theSerializer != NULL)
    {
        theSerializer->release();
    }
   
    TerminateXML();

    return errorCode; 

}


/******************************************************************************************************
*
* Function: SetDateStamp
*
* Purpose:  Adds the upper level "dateStamp" node or replaces it in the BAG metadata XML string.
*
* Inputs:   newDateStamp - string containing dateStamp that should be added or used as a replacement. 
*           xmlString   - address of the XML string to alter.
*
*           If dateStamp node is present, value will be changed to reflect dateStamp specifed.
*           If dateStamp node is NOT present, dateStamp node will be added.
*
* Outputs:     Error messages to stderr.
*
* Returns:     NV_INT32 = Length of XML string created (if successful). 
*                    -1 = Error initializing XERCES
*                    -2 = A DOMException occurred.
*                    -3 = Out of memory
*                   -20 = An exception occurred while attempting to the langauge.
*        
* Assumptions:  For purpose of adding a newDateStamp, it is assumed that 
*               1) previous sibling nodes of "dateStamp" node exist and that following sibling nodes do not exist. 
*				2) the supplied XML string is well-formed.
*
*               For the purpose of replacing the dateStamp node,  it is assumed that the supplied XML string is 
*               well-formed, valid XML.          
*
*******************************************************************************************************/
NV_INT32 SetDateStamp(NV_CHAR * newDateStamp,  NV_U_CHAR ** xmlString)
{


    NV_INT16 errorCode = 1         /* Assume success. */;
    NV_CHAR *  gMemBufId = "metadata";
 

    DOMBuilder *myParser;    /* This will parse the given XML string into a DOM document. */
    DOMDocument* xmlDoc = NULL;
    DOMWriter *theSerializer = NULL;


    /* Initialize XERCES. */
    if (!XMLinitialized)
    {
        try
        {
            XMLPlatformUtils::Initialize();
        }
        catch(const XMLException& toCatch)
        {

            NV_CHAR * message = XMLString::transcode(toCatch.getMessage());
            fprintf(stderr,"Exception message is: %s\n", message);
            XMLString::release(&message);
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
 
      
    try
    {

        /* Init the parser. */
        myParser = ((DOMImplementationLS*)impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS,NULL);


        /* Set up memory input source (XML string) to be parsed.*/
        MemBufInputSource* memBufIS = new MemBufInputSource ((const XMLByte*)*xmlString , strlen((NV_CHAR *)*xmlString), gMemBufId, false);
        Wrapper4InputSource wrap(memBufIS,false);   
        xmlDoc = myParser->parse(wrap);


        /* 1.9.2009 - E. Warner - Set encoding to UTF-8. */
        XMLCh *temp = XMLString::transcode("UTF-8");
        xmlDoc->setEncoding(temp);
        XMLString::release(&temp);


        DOMNode * metadataNode = FindNode(xmlDoc, "smXML:MD_Metadata", NULL, NULL);


        /* Find the upper level node named "dateStamp" */
        DOMNodeList * dateStampNodeList = metadataNode->getChildNodes();
        XMLSize_t length = dateStampNodeList->getLength();
        DOMNode * tempNode = NULL;
        NV_CHAR * tempNodeName;
        NV_BOOL found = NVFalse;

        for (XMLSize_t c = 0; c < length; c++)
        {

            tempNode =  dateStampNodeList->item(c);
            tempNodeName = XMLString::transcode(tempNode->getNodeName());

            if(strcmp(tempNodeName, "dateStamp") == 0)
            {

                tempNode->setTextContent(X(newDateStamp));
                found = NVTrue;
                break;
            }


        }
        
        /* If we went through all nodes and did not find the dateStamp node, create one and insert it.*/ 
        /* Assumes that the "contact" node exists. */	 
        if(!found)
        {
            DOMElement * dateStampElem = createElement("dateStamp", newDateStamp, xmlDoc);
       
            /* Assume that previous siblings exist and that following siblings do not exist. */
            metadataNode->appendChild(dateStampElem);

        }



        /* Serialize the altered document. */
        theSerializer = ((DOMImplementationLS*)impl)->createDOMWriter();




        /* 1.9.2009 - E. Warner - Set encoding to UTF-8. */
        /* Write the document to a string.  Done this way because must use writeNode in order for setEncoding method to work.*/
        MemBufFormatTarget *myFormTarget;
        myFormTarget = new MemBufFormatTarget();
        bool wroteBuffer = theSerializer->writeNode(myFormTarget,*xmlDoc);

        if(wroteBuffer)
        {

            errorCode = myFormTarget->getLen();

            *xmlString = (NV_U_CHAR *)realloc(*xmlString, errorCode * sizeof(NV_U_CHAR));

            *xmlString = (NV_U_CHAR *)myFormTarget->getRawBuffer();
        }
        else
        {
            throw (NV_CHAR *)"Could not write buffer. ";

        }


       

    }

    catch (const DOMException& e)
    {        

        fprintf(stderr,"ERROR: Exception while attempting to set dateStamp. DOMException code %d\n", e.code);
        errorCode = -2;

    }
    catch (const OutOfMemoryException& m)
    {          
            
        fprintf(stderr,"ERROR: Exception while attempting to set dateStamp. Exception message is: OutOfMemoryException\n");
        errorCode = -3;

    }
    catch (NV_INT16 intException)
    {          
            
        fprintf(stderr,"ERROR: Exception while attempting to set dateStamp.  Exception message is: %d\n", intException );
        errorCode = intException;

    }
    catch(...)
    {

        fprintf(stderr,"ERROR: Exception while attempting to set dateStamp.\n");
        errorCode = -20;

    }

 
    /* Release the resources. */
    xmlDoc->release();

    if(theSerializer != NULL)
    {
        theSerializer->release();
    }
	
    TerminateXML();

    return errorCode; 

}


/******************************************************************************************************
*
* Function: SetMetadataStandardName
*
* Purpose:  Adds the upper level "metadataStandardName" node or replaces it in the BAG metadata.
*
* Inputs:   newMetadataStandardName - string containing Metadata Standard Name that should be added or used as a replacement. 
*           xmlString   - address of the XML string to alter.
*
*           If metadataStandardName node is present, value will be changed to reflect metadataStandardName specifed.
*           If metadataStandardName node is NOT present, metadataStandardName node will be added.
*           Only one metadataStandardName is allowed.
*
* Outputs:      Error messages to stderr.
*
* Returns:      NV_INT32 = Length of XML string created (if successful). 
*                     -1 = Error initializing XERCES
*                     -2 = A DOMException occurred.
*                     -3 = Out of memory
*                    -20 = An exception occurred while attempting to set metadataStandardName. 
*             
* Assumptions:  For purpose of adding a newMetadataStandardName, it is assumed that 
*               1) previous sibling nodes of "metadataStandardName" node exist and that following sibling nodes do not exist. 
*				2) the supplied XML string is well-formed.
*
*               For the purpose of replacing the metadataStandardName node,  it is assumed that the supplied XML string is 
*               well-formed, valid XML.     
*
*******************************************************************************************************/
NV_INT32 SetMetadataStandardName(NV_CHAR * newMetadataStandardName,  
                     NV_U_CHAR ** xmlString)
{


    NV_INT16 errorCode = 1         /* Assume success. */;
    NV_CHAR *  gMemBufId = "metadata";
 

    DOMBuilder *myParser;    /* This will parse the given XML string into a DOM document. */
    DOMDocument* xmlDoc = NULL;
    DOMWriter *theSerializer = NULL;


    /* Initialize XERCES. */
    if (!XMLinitialized)
    {
        try
        {
            XMLPlatformUtils::Initialize();
        }
        catch(const XMLException& toCatch)
        {

            NV_CHAR * message = XMLString::transcode(toCatch.getMessage());
            fprintf(stderr,"Exception message is: %s\n", message);
            XMLString::release(&message);
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
 
      
    try
    {

        /* Init the parser. */
        myParser = ((DOMImplementationLS*)impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS,NULL);


        /* Set up memory input source (XML string) to be parsed.*/
        MemBufInputSource* memBufIS = new MemBufInputSource ((const XMLByte*)*xmlString , strlen((NV_CHAR *)*xmlString), gMemBufId, false);
        Wrapper4InputSource wrap(memBufIS,false);   
        xmlDoc = myParser->parse(wrap);


        /* 1.9.2009 - E. Warner - Set encoding to UTF-8. */
        XMLCh *temp = XMLString::transcode("UTF-8");
        xmlDoc->setEncoding(temp);
        XMLString::release(&temp);


        /* Find the upper level node named "metadataStandardName" */
        DOMNode * metadataStandardNameNode = FindNode(xmlDoc, "metadataStandardName", NULL, NULL);
       

        if(metadataStandardNameNode != NULL)
        {

            metadataStandardNameNode->setTextContent(X(newMetadataStandardName));
	  
        }

        /* Otherwise, the metadataStandardName node was not found. Create one and insert it.*/ 
        /* Assumes that the "metadataStandardVersion" node exists. */
        else
        {
            DOMElement * metadataStandardNameElem = createElement("metadataStandardName", newMetadataStandardName, xmlDoc);

            DOMNode * metadataNode = FindNode(xmlDoc, "smXML:MD_Metadata", NULL, NULL);

            /* Assume that previous siblings exist and that following siblings do not exist. */
            metadataNode->appendChild(metadataStandardNameElem);

        }
        
	 
        /* Serialize the altered document. */
        theSerializer = ((DOMImplementationLS*)impl)->createDOMWriter();


 
    
        /* 1.9.2009 - E. Warner - Set encoding to UTF-8. */
        /* Write the document to a string.  Done this way because must use writeNode in order for setEncoding method to work.*/
        MemBufFormatTarget *myFormTarget;
        myFormTarget = new MemBufFormatTarget();
        bool wroteBuffer = theSerializer->writeNode(myFormTarget,*xmlDoc);

        if(wroteBuffer)
        {

            errorCode = myFormTarget->getLen();

            *xmlString = (NV_U_CHAR *)realloc(*xmlString, errorCode * sizeof(NV_U_CHAR));

            *xmlString = (NV_U_CHAR *)myFormTarget->getRawBuffer();
        }
        else
        {
            throw (NV_CHAR *)"Could not write buffer. ";

        }


    }

    catch (const DOMException& e)
    {   
     
        fprintf(stderr,"ERROR: Exception while attempting to set metadata standard name. DOMException code %d\n", e.code);
        errorCode = -2;

    }
    catch (const OutOfMemoryException& m)
    {          
            
        fprintf(stderr,"ERROR: Exception while attempting to set metadata standard name. Exception message is: OutOfMemoryException\n");
        errorCode = -3;

    }
    catch (NV_INT16 intException)
    {          
            
        fprintf(stderr,"ERROR: Exception while attempting to set metadata standard name.  Exception message is: %d\n", intException );
        errorCode = intException;

    }
    catch(...)
    {

        fprintf(stderr,"ERROR: Exception while attempting to set metadata standard name.\n");
        errorCode = -20;

    }

 
    /* Release the resources. */
    xmlDoc->release();

    if(theSerializer != NULL)
    {
        theSerializer->release();
    }
   
    TerminateXML();

    return errorCode; 

}




/******************************************************************************************************
*
* Function: SetMetadataStandardVersion
*
* Purpose:  Adds the upper level "metadataStandardVersion" node or replaces it in the BAG metadata XML string.
*
* Inputs:   newMetadataStandardVersion - string containing Metadata Standard Version  that should be added or used as a replacement. 
*           xmlString   - address of the XML string to alter.
*
*           If metadataStandardVersion node is present, value will be changed to reflect metadataStandardVersion specifed.
*           If metadataStandardVersion node is NOT present, metadataStandardVersion node will be added.
*           Only one metadataStandardVersion is allowed.
*
* Outputs:  Error messages to stderr.
*
* Returns:  NV_INT32 = Length of XML string created (if successful). 
*                 -1 = Error initializing XERCES
*                 -2 = A DOMException occurred.
*                 -3 = Out of memory
*                 -20 = an exception occurred while attempting to set metadataStandardVersion.
*                  
*
* Assumptions:  For purpose of adding a newMetadataStandardVersion, it is assumed that 
*               1) previous sibling nodes of "metadataStandardVerion" node exist. 
*				2) the supplied XML string is well-formed.
*
*               For the purpose of replacing the metadataStandardVersion node,  it is assumed that the supplied XML string is 
*               well-formed, valid XML.
*
*******************************************************************************************************/
NV_INT32 SetMetadataStandardVersion(NV_CHAR * newMetadataStandardVersion,  
                     NV_U_CHAR ** xmlString)
{


    NV_INT16 errorCode = 1         /* Assume success. */;
    NV_CHAR *  gMemBufId = "metadata";
 

    DOMBuilder *myParser;    /* This will parse the given XML string into a DOM document. */
    DOMDocument* xmlDoc = NULL;
    DOMWriter *theSerializer = NULL;


    /* Initialize XERCES. */
    if (!XMLinitialized)
    {
        try
        {
            XMLPlatformUtils::Initialize();
        }
        catch(const XMLException& toCatch)
        {

            NV_CHAR * message = XMLString::transcode(toCatch.getMessage());
            fprintf(stderr,"Exception message is: %s\n", message);
            XMLString::release(&message);
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
 
      
    try
    {

        /* Init the parser. */
        myParser = ((DOMImplementationLS*)impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS,NULL);


        /* Set up memory input source (XML string) to be parsed.*/
        MemBufInputSource* memBufIS = new MemBufInputSource ((const XMLByte*)*xmlString , strlen((NV_CHAR *)*xmlString), gMemBufId, false);
        Wrapper4InputSource wrap(memBufIS,false);   
        xmlDoc = myParser->parse(wrap);

        /* 1.9.2009 - E. Warner - Set encoding to UTF-8. */
        XMLCh *temp = XMLString::transcode("UTF-8");
        xmlDoc->setEncoding(temp);
        XMLString::release(&temp);


        /* Find the upper level node named "metadataStandardName" */
        DOMNode * metadataStandardVersionNode = FindNode(xmlDoc, "metadataStandardVersion", NULL, NULL);
       

        if(metadataStandardVersionNode != NULL)
        {

            metadataStandardVersionNode->setTextContent(X(newMetadataStandardVersion));
	 
	    
        }

        /* Otherwise, the metadataStandardVersion node was not found. Create one and insert it.*/ 
        /* Assumes that all other upper level nodes already exist. */
        else
        {

            DOMElement * metadataStandardVersionElem = createElement("metadataStandardVersion", newMetadataStandardVersion, xmlDoc);

            DOMNode * metadataNode = FindNode(xmlDoc, "smXML:MD_Metadata", NULL, NULL);

            /* Assume that previous siblings exist and that following siblings do not exist. */
            metadataNode->appendChild(metadataStandardVersionElem);

        }
        
	 
        /* Serialize the altered document. */
        theSerializer = ((DOMImplementationLS*)impl)->createDOMWriter();




        /* 1.9.2009 - E. Warner - Set encoding to UTF-8. */
        /* Write the document to a string.  Done this way because must use writeNode in order for setEncoding method to work.*/
        MemBufFormatTarget *myFormTarget;
        myFormTarget = new MemBufFormatTarget();
        bool wroteBuffer = theSerializer->writeNode(myFormTarget,*xmlDoc);

        if(wroteBuffer)
        {

            errorCode = myFormTarget->getLen();

            *xmlString = (NV_U_CHAR *)realloc(*xmlString, errorCode * sizeof(NV_U_CHAR));

            *xmlString = (NV_U_CHAR *)myFormTarget->getRawBuffer();
        }
        else
        {
            throw (NV_CHAR *)"Could not write buffer. ";

        }


    }

    catch (const DOMException& e)
    {        

        fprintf(stderr,"ERROR: Exception while attempting to set metadata standard version. DOMException code %d\n", e.code);
        errorCode = -2;

    }
    catch (const OutOfMemoryException& m)
    {          
            
        fprintf(stderr,"ERROR: Exception while attempting to set metadata standard version.  Exception message is: OutOfMemoryException\n");
        errorCode = -3;

    }
    catch (NV_INT16 intException)
    {          

        fprintf(stderr,"ERROR: Exception while attempting to set metadata standard version.  Exception message is: %d\n", intException );
        errorCode = intException;

    }
    catch(...)
    {

        fprintf(stderr,"ERROR: Exception while attempting to set metadata standard version.\n"); 
        errorCode = -20;

    }


    /* Release the resources. */
    xmlDoc->release();

    if(theSerializer != NULL)
    {
        theSerializer->release();
    }
   
    TerminateXML();

    return errorCode; 

}



/******************************************************************************************************
*
* Function: InitDataIdentificationInfo
*
* Purpose:  Intialize all floats and ints to INIT_VALUE. Initialize all character arrays to empty string.
*
* Inputs:   dataIdentificationInfo - pointer to object to initialize.
*
* Returns:  1 = success
*          -1 = not successful.  An exception was encountered.
*
*******************************************************************************************************/
NV_INT16 InitDataIdentificationInfo(IDENTIFICATION_INFO * dataIdentificationInfo)
{

    NV_INT16 status = 1;

    try
    {
   
        strcpy((*dataIdentificationInfo).title, "\0"); 
        strcpy((*dataIdentificationInfo).date, "\0"); 
        strcpy((*dataIdentificationInfo).dateType, "\0");  


        for(NV_INT16 i = 0; i < MAX_CI_RESPONSIBLE_PARTIES; i++)
        {
            strcpy((*dataIdentificationInfo).responsibleParties[i].individualName, "\0");
            strcpy((*dataIdentificationInfo).responsibleParties[i].positionName, "\0");
            strcpy((*dataIdentificationInfo).responsibleParties[i].organisationName, "\0");
            strcpy((*dataIdentificationInfo).responsibleParties[i].phoneNumber, "\0");
            strcpy((*dataIdentificationInfo).responsibleParties[i].role, "\0");
         
        }


        strcpy((*dataIdentificationInfo).abstract, "\0"); 
        strcpy((*dataIdentificationInfo).purpose, "\0"); 
        strcpy((*dataIdentificationInfo).status, "\0"); 
        strcpy((*dataIdentificationInfo).spatialRepresentationType, "\0"); 
        strcpy((*dataIdentificationInfo).language, ""); 
        strcpy((*dataIdentificationInfo).topicCategory, ""); 

        (*dataIdentificationInfo).westBoundingLongitude = INIT_VALUE;
        (*dataIdentificationInfo).eastBoundingLongitude = INIT_VALUE;   
        (*dataIdentificationInfo).southBoundingLatitude = INIT_VALUE;       
        (*dataIdentificationInfo).northBoundingLatitude = INIT_VALUE;

        strcpy((*dataIdentificationInfo).verticalUncertaintyType, "\0");
        strcpy((*dataIdentificationInfo).depthCorrectionType, "\0");

    }               
    catch(...)
    {

        status = -1;
    }


    return status;


}



/******************************************************************************************************
*
* Function: InitLegalConstraints
*
* Purpose:  Initialize all character arrays to empty string.
*
* Inputs:   legalConstraints - pointer to object to initialize.
*
* Returns:  1 = success
*          -1 = not successful.  An exception was encountered.
*
*******************************************************************************************************/
NV_INT16 InitLegalConstraints(MD_LEGAL_CONSTRAINTS * legalConstraints)
{

    NV_INT16 status = 1;

    try
    {
        strcpy( (*legalConstraints).useConstraints, "\0");
        strcpy( (*legalConstraints).otherConstraints,"\0");
    }               
    catch(...)
    {
        status = -1;
    }


    return status;


}


/******************************************************************************************************
*
* Function: InitSecurityConstraints
*
* Purpose:  Initialize all character arrays to empty string.
*
* Inputs:   securityConstraints - pointer to object to initialize.
*
* Returns:  1 = success
*          -1 = not successful.  An exception was encountered.
*
*******************************************************************************************************/

NV_INT16 InitSecurityConstraints(MD_SECURITY_CONSTRAINTS * securityConstraints)
{

    NV_INT16 status = 1;

    try
    {
        strcpy( (*securityConstraints).classification, "\0");
        strcpy( (*securityConstraints).userNote,"\0");
    }               
    catch(...)
    {
	    status = -1;
    }


    return status;


}



/******************************************************************************************************
*
* Function: InitDataQualityInfo
*
* Purpose:  Initialize all floats and ints to INIT_VALUE. Initialize all character arrays to empty string. Initialize all pointers to NULL;
*
* Inputs:   dataQualityInfo - pointer to object to initialize. 
*
* Returns:  1 = success
*          -1 = not successful.  An exception was encountered.
*
*******************************************************************************************************/
NV_INT16 InitDataQualityInfo(DATA_QUALITY_INFO * dataQualityInfo)
{

    NV_INT16 status = 1;

    try
    {
        strcpy( (*dataQualityInfo).scope, "\0");

        (*dataQualityInfo).numberOfSources = 0;
        (*dataQualityInfo).numberOfProcessSteps = 0;


        (*dataQualityInfo).	lineageSources = NULL;            
        (*dataQualityInfo).lineageProcessSteps = NULL;      
  
   
    }               
    catch(...)
    {
        status = -1;
    }


    return status;


}


/******************************************************************************************************
*
* Function: InitReferenceSystemInfo
*
* Purpose:  Intialize all floats and ints to INIT_VALUE.  Initialize all character arrays to empty string.
*
* Inputs:   referenceSystemInfo - pointer to object to initialize. 
*
* Returns:  1 = success
*          -1 = not successful.  An exception was encountered.
*
*******************************************************************************************************/

NV_INT16 InitReferenceSystemInfo(REFERENCE_SYSTEM_INFO * referenceSystemInfo)
{

    NV_INT16 status = 1;

    try
    {

        /* Intialize all floats and ints. */
        referenceSystemInfo->zone = INIT_VALUE;                                
        referenceSystemInfo->standardParallel = INIT_VALUE;               
        referenceSystemInfo->longitudeOfCentralMeridian = INIT_VALUE;       
        referenceSystemInfo->latitudeOfProjectionOrigin = INIT_VALUE;      
        referenceSystemInfo->falseEasting = INIT_VALUE;                  
        referenceSystemInfo->falseNorthing = INIT_VALUE;                  
        referenceSystemInfo->scaleFactorAtEquator = INIT_VALUE;              
        referenceSystemInfo->heightOfProspectivePointAboveSurface = INIT_VALUE;
        referenceSystemInfo->longitudeOfProjectionCenter = INIT_VALUE;      
        referenceSystemInfo->latitudeOfProjectionCenter = INIT_VALUE;       
        referenceSystemInfo->scaleFactorAtCenterLine = INIT_VALUE;        
        referenceSystemInfo->straightVerticalLongitudeFromPole = INIT_VALUE;  
        referenceSystemInfo->scaleFactorAtProjectionOrigin = INIT_VALUE;    
        referenceSystemInfo->semiMajorAxis = INIT_VALUE;    
    }               
    catch(...)
    {
        status = -1;
    }


    return status;


}

/******************************************************************************************************
*
* Function: InitSpatialRepresentationInfo
*
* Purpose:  Intialize all floats and ints to INIT_VALUE. Initialize all character arrays to empty string.
*
* Inputs:   spatialRepresentationInfo - pointer to object to initialize.
*
* Returns:  1 = success
*          -1 = not successful.  An exception was encountered.
*
*******************************************************************************************************/
NV_INT16 InitSpatialRepresentationInfo(SPATIAL_REPRESENTATION_INFO * spatialRepresentationInfo)
{

    NV_INT16 status = 1;

    try
    {

        (*spatialRepresentationInfo).numberOfDimensions = INIT_VALUE;

        for (NV_INT16 i = 0; i < 3; i++)
        {
            strcpy((*spatialRepresentationInfo).dimensionName[i],  "\0");
            (*spatialRepresentationInfo).dimensionSize[i] = 0; 
            (*spatialRepresentationInfo).resolutionValue[i]  = 0; 
        }
               

        strcpy((*spatialRepresentationInfo).cellGeometry, "\0");
        strcpy((*spatialRepresentationInfo).transformationParameterAvailability, "\0");
        strcpy((*spatialRepresentationInfo).checkPointAvailability, "\0");                

        (*spatialRepresentationInfo).llCornerX = INIT_VALUE;                                 
        (*spatialRepresentationInfo).llCornerY = INIT_VALUE;                                  
        (*spatialRepresentationInfo).urCornerX = INIT_VALUE;                               
        (*spatialRepresentationInfo).urCornerY = INIT_VALUE;  

    }               
    catch(...)
    {
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

NV_INT16 InitResponsibleParty(RESPONSIBLE_PARTY * responsibleParty)
{

    NV_INT16 status = 1;

    try
    {
        strcpy( (*responsibleParty).individualName, "\0");
        strcpy( (*responsibleParty).organisationName, "\0");
        strcpy( (*responsibleParty).positionName, "\0");
        strcpy( (*responsibleParty).phoneNumber, "\0");
        strcpy( (*responsibleParty).role, "\0");

    }               
    catch(...)
    {
        status = -1;
    }


    return status;


}

/******************************************************************************************************
*
* Function: InitSourceInfo
*
* Purpose:  Initialize all character arrays to empty string.
*
* Inputs:   sourceInfo - pointer to object to initialize.
*
* Returns:  1 = success
*          -1 = not successful.  An exception was encountered.
*
*******************************************************************************************************/

NV_INT16 InitSourceInfo(SOURCE_INFO * sourceInfo)
{

    NV_INT16 status = 1;

    try
    {

        strcpy( (*sourceInfo).description, "\0");
        strcpy( (*sourceInfo).title,"\0");
        strcpy( (*sourceInfo).date,"\0");
        strcpy( (*sourceInfo).dateType,"\0");
	    
        for(NV_INT16 i = 0; i < MAX_CI_RESPONSIBLE_PARTIES; i++)
        {
            strcpy((*sourceInfo).responsibleParties[i].individualName, "\0");
            strcpy((*sourceInfo).responsibleParties[i].positionName, "\0");
            strcpy((*sourceInfo).responsibleParties[i].organisationName, "\0");
            strcpy((*sourceInfo).responsibleParties[i].phoneNumber, "\0");
            strcpy((*sourceInfo).responsibleParties[i].role, "\0");
         
        }
			
    }               
    catch(...)
    {
        status = -1;
    }


    return status;


}






/******************************************************************************************************
*
* Function: FreeDataQualityInfo
*
* Purpose:  Free resources that were allocated while reading a DATA_QUALITY_INFO structure.
*
* Inputs:   dataQuality - pointer to object to free.
*
* Returns:  1 = success
*          -1 = not successful.  An exception was encountered.
*
* Notes:  This method should be called after calling getDataQualityInfo.  
*         Specifically, it should be called when a program no longer needs the metadata in the 
*         DATA_QUALITY_INFO structure which was obtained by calling GetDataQualityInfo.
*
*******************************************************************************************************/
NV_INT16  FreeDataQualityInfo(DATA_QUALITY_INFO * dataQuality )
{

    NV_INT16 status = 1;

    try{

        free(dataQuality->lineageSources);
        free(dataQuality->lineageProcessSteps);
        free(dataQuality);

    }
    catch(...)
    {

        status = -1;

    }

    return status;
}






