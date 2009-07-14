/*****************************************************************************************
*
* Program: Sample Application 3
*
* Purpose: Illustrates the use of the "set" functions in the BAG XML metadata library.
*
* History :
*
*     5/2008 - E. Warner - Initial development
*
*
******************************************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include "nvtypes.h"
#include "bag_xml_meta.h"


/* Local methods */
void  readSampleXml(NV_CHAR  pathToFile[100], NV_U_CHAR ** xml_string);
void  writeXmlFile(NV_CHAR  pathToFile[100], NV_U_CHAR * xml_string);


int main(int argc, char *argv[])
{




    /************************************************************************   
    *
    *            Illustrate the use of the SET functions.
    *
    *************************************************************************/

    /* Step 1:  Declare one of each of the required structures. */


    /* Structures to hold edited metadata read from sample XML file. */
    IDENTIFICATION_INFO         edited_identification_info;
    MD_LEGAL_CONSTRAINTS        edited_legal_constraints;
    MD_SECURITY_CONSTRAINTS     edited_security_constraints;
    DATA_QUALITY_INFO           edited_data_quality;
    SPATIAL_REPRESENTATION_INFO edited_spatial_info;
    REFERENCE_SYSTEM_INFO       edited_reference_info;
    RESPONSIBLE_PARTY           edited_contact;


    /* Declare other needed items. */
    /* Holds the result of a method call. */
    NV_INT32 result;


    /* Will hold the XML that is read from sample.xml file. */
    NV_U_CHAR * xml_string;
   




    /* Step 2:  Initialize the required structures. 
                This will set all integers and floats to -999.  This will also set all strings to an empty string. */


    InitDataIdentificationInfo(&edited_identification_info);
    InitLegalConstraints(&edited_legal_constraints);
    InitSecurityConstraints(&edited_security_constraints);
    InitDataQualityInfo(&edited_data_quality);
    InitReferenceSystemInfo(&edited_reference_info);
    InitSpatialRepresentationInfo(&edited_spatial_info);






    /* Step 3:  Populate the metadata structures with the "original" metadata. */

    /* Read file created by sample application 2. */
    readSampleXml("./newXmlFromStructures.xml", &xml_string);
    //readSampleXml("./sample.xml", &xml_string);


    GetAllStructures(xml_string, & edited_identification_info, & edited_legal_constraints, & edited_security_constraints,  & edited_data_quality, & edited_spatial_info, & edited_reference_info, & edited_contact );
  

    /* Print the structures to verify that the retrieval obtained the expected metadata. */
    PrintIdentificationInfo(edited_identification_info);
    PrintLegalConstraints(edited_legal_constraints); 
    PrintSecurityConstraints(edited_security_constraints);  
    PrintDataQuality(edited_data_quality);
    PrintSpatialRepresentationInfo(edited_spatial_info);
    PrintReferenceSystemInfo(edited_reference_info);
    PrintContact(edited_contact);



    /* Steps 4 and 5 are performed below for each of the data structures. 
       4.  Edit the "original" metadata contained in the structures.
       5.  Call the appropriate Set method using the two structures. 
           This will edit the BAG XML metadata string from the contents of the populated structures.
    */


    
    /* Establish a date with which to stamp the edited XML. */
    NV_CHAR * creationDate = (NV_CHAR *)malloc(11 * sizeof(NV_CHAR));
    strcpy(creationDate, "2009-06-17");



    /* Edit original contact. */
    strcpy(edited_contact.individualName, "Bob Bathy");
    //strcpy(edited_contact.role, "new POC");

    fprintf(stderr,"\n\nEditing XML to reflect new contact name: %s \n",  edited_contact.individualName);

    result = SetContact(edited_contact,  &xml_string);
    fprintf(stderr, "Result of SetContact:%d\n", result);

    if(result > 0)
    {
         writeXmlFile("./modifiedContact.xml", xml_string);
    }





    /* Edit  original legal constraint.  */
    fprintf(stderr, "\nEditing legal constraint using SetLegalConstraints. \n");

    strcat(edited_legal_constraints.useConstraints, " new");
    strcat(edited_legal_constraints.otherConstraints, " new");


    result = SetLegalConstraints(edited_legal_constraints, &xml_string);
    fprintf(stderr, "Result of SetLegalConstraints: %d\n", result);

    if(result > 0)
    {
        writeXmlFile("./modifiedLegalConstraint.xml", xml_string);
    }




    /* Edit  original security constraint. */
    fprintf(stderr, "\nEditing security constraint using SetSecurityConstraints. \n");

    strcat(edited_security_constraints.classification," new" );
    strcat(edited_security_constraints.userNote," new" );

    result = SetSecurityConstraints(edited_security_constraints, &xml_string);
    fprintf(stderr, "Result of SetSecurityConstraints: %d\n", result);

    if(result > 0)
    {
        writeXmlFile("./modifiedSecurityConstraint.xml", xml_string);
    }





    /* Edit  original identification Info. */
    fprintf(stderr, "\nEditing identificationInfo using SetIdentificationInfo.\n");

    edited_identification_info.westBoundingLongitude = -100.222;
    edited_identification_info.eastBoundingLongitude = 100.111;

    strcat(edited_identification_info.title, " new");

    result = SetIdentificationInfo(edited_identification_info, &xml_string);
    fprintf(stderr, "Result of SetIdentificationInfo: %d\n", result);

    if(result > 0)
    {
        writeXmlFile("./modIdentificationInfo.xml", xml_string);
    }






    /* Edit  original dataQualityInfo. */
    fprintf(stderr, "\nEditing dataQualityInfo using SetDataQualityInfo. \n");

    /* Edit an existing source. */
    strcat(edited_data_quality.lineageSources[0].description, " new");

    /* Edit an existing processStep. */
    strcat(edited_data_quality.lineageProcessSteps[0].description, " new");

    result = SetDataQualityInfo(edited_data_quality, &xml_string);
    fprintf(stderr, "Result of SetDataQualityInfo:%d\n", result);

    if(result > 0)
    {
        writeXmlFile("./modDataQualityInfo.xml", xml_string);
    }






    /* Edit  original spatialRepresentationInfo . */
    fprintf(stderr, "\nEditing spatialRepresentationInfo using SetSpatialRepresentationInfo\n");

    edited_spatial_info.llCornerX = -170.0;
    edited_spatial_info.llCornerY = -80.0;
    edited_spatial_info.urCornerX = 170;
    edited_spatial_info.urCornerY = 80.0;
  
    result = SetSpatialRepresentationInfo(edited_spatial_info, &xml_string);
    fprintf(stderr, "Result of SetSpatialRepresentationInfo:%d\n", result);

    if(result > 0)
    {
        writeXmlFile("./modSpatialRepresentationInfo.xml", xml_string);
    }






    /* Edit  original referenceSystemInfo. */
    fprintf(stderr, "\nEditing referenceSystemInfo using SetReferenceSystemInfo. \n");

    edited_reference_info.zone = 44;
    edited_reference_info.falseEasting = 4.0;
    edited_reference_info.falseNorthing = 44.0;
    strcpy(edited_reference_info.verticalDatum, "WGS84 2");
  
    result = SetReferenceSystemInfo(edited_reference_info, &xml_string);
    fprintf(stderr, "Result of SetReferenceSystemInfo:%d\n", result);

    if(result > 0)
    {
        writeXmlFile("./modReferenceSystemInfo.xml", xml_string);
    }





    /* Set  language. */
    fprintf(stderr, "\nEditing language using  SetLanguage.\n");
    NV_CHAR * newLanguage = calloc(3, sizeof(NV_CHAR));    

    strcpy(newLanguage, "ch");
 
    result = SetLanguage(newLanguage, &xml_string);
    fprintf(stderr, "Result of SetLanguage: %d\n", result);

    if(result > 0)
    {
        writeXmlFile("./newLanguage.xml", xml_string);
    }

    free(newLanguage);





    /* Set metadataStandardName. */
    fprintf(stderr, "\nEditing metadataStandardName using SetMetadataStandardName. \n");
    NV_CHAR * newMetadataStandardName = malloc(11 * sizeof(NV_CHAR));  
    strcpy(newMetadataStandardName, "ISO 50000");
 
    result = SetMetadataStandardName(newMetadataStandardName, &xml_string);
    fprintf(stderr, "Result of SetMetadataStandardName: %d\n", result);

    if(result > 0)
    {
        writeXmlFile("./newMetadataStandardName.xml", xml_string);
    }

    free(newMetadataStandardName);





    /* Set metadataStandardVersion. */
    fprintf(stderr, "\nEditing metadataStandardVersion using SetMetadataStandardVersion. \n");
    NV_CHAR * newMetadataStandardVersion = malloc(11 * sizeof(NV_CHAR)); 
    strcpy(newMetadataStandardVersion, "2008");
 
    result = SetMetadataStandardVersion(newMetadataStandardVersion, &xml_string);
    fprintf(stderr, "Result of SetMetadataStandardVersion: %d\n", result);

    if(result > 0)
    {
        writeXmlFile("./newMetadataStandardVersion.xml", xml_string);
    }

    free(newMetadataStandardVersion);



    return 1;

}




/*****************************************************************
*
* Method to read the sample XML file into a string. 
*
*****************************************************************/
void  readSampleXml(NV_CHAR  pathToFile[100], NV_U_CHAR **xml_string)
{

    FILE * fp;

    NV_CHAR tempChar;

    int i=0;

    *xml_string = malloc (30000 * sizeof(NV_CHAR));



    if((fp =  fopen(pathToFile, "r")) != NULL)
    {
        fprintf(stderr, "Reading XML file %s.\n", pathToFile);

        while((tempChar = getc(fp)) != EOF)
        {

    
            (*xml_string)[i] = tempChar;

            i++;


        }
    
        (*xml_string)[i] = '\0';

    }
    else
    {
        fprintf(stderr, "Could not open file: %s\n", pathToFile);
    }

    fprintf(stderr, "chars in file: %d\n", i );

    fclose(fp);

}




/*****************************************************************
*
* Method to write the sample XML string into a file. 
*
******************************************************************/
void  writeXmlFile(NV_CHAR  pathToFile[100], NV_U_CHAR *xml_string)
{

    FILE * fp;

    int status = 0;

 

    if((fp =  fopen(pathToFile, "w")) != NULL)
    {
        fprintf(stderr, "Writing XML file %s.\n", pathToFile);

        status = fputs(xml_string,fp);

        if (status < 0 || status == EOF )
	{
            fprintf(stderr, "Error writing file: %s\n", pathToFile);
        }

    }
    else
    {
        fprintf(stderr, "Could not open file for writing: %s\n", pathToFile);
    }


    fclose(fp);

}
