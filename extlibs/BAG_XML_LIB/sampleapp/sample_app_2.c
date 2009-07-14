/*****************************************************************************************
*
* Program: Sample Application 2
*
* Purpose: Illustrates the use of the BAG XML metadata library to create a BAG XML metadata string from individual metadata components.
*
* History :
*
*     5/2008 - E. Warner - Initial development
*
*
******************************************************************************************/

#include <stdlib.h>
#include <stdio.h>


#include "bag_xml_meta.h"


/* Local methods.  Here only for convenience.  Not required in order to use BAG XMl metadata library. */
void  readSampleXml(NV_CHAR  pathToFile[100], NV_U_CHAR ** xml_string);
void  writeXmlFile(NV_CHAR  pathToFile[100], NV_U_CHAR * xml_string);


int main(int argc, char *argv[])
{

    NV_INT32 result; /* Will hold the result of trying to create XML string. */


    /* Step 1:  Declare one of each of the required structures. */

    IDENTIFICATION_INFO         identification_info;
    MD_LEGAL_CONSTRAINTS        legal_constraints;
    MD_SECURITY_CONSTRAINTS     security_constraints;
    DATA_QUALITY_INFO           data_quality;
    SPATIAL_REPRESENTATION_INFO spatial_info;
    REFERENCE_SYSTEM_INFO       reference_info;
    RESPONSIBLE_PARTY           contact;

    /* Also declare items that required components but are represented as strings not structures. */
    NV_CHAR * language;
    NV_CHAR * date_stamp;




    /* Step 2:  Initialize the required structures. This will set all integeters and floats to -999.  This will also set all strings to an emapty string. */

    InitDataIdentificationInfo(&identification_info);
    InitLegalConstraints(&legal_constraints);
    InitSecurityConstraints(&security_constraints);
    InitDataQualityInfo(&data_quality);
    InitReferenceSystemInfo(&reference_info);
    InitSpatialRepresentationInfo(&spatial_info);





    /* Step 3:  Populate the metadata structures and other non complex components that are required. */


    /* Provide the required fields for IDENTIFICATION_INFO. */
    strcpy(identification_info.abstract, "My abstract goes here.");
    strcpy(identification_info.language, "en");
    strcpy(identification_info.topicCategory, "elevation");
    strcpy(identification_info.verticalUncertaintyType, "My vertical uncertainty");

    /* Provide the some optional fields for IDENTIFICATION_INFO. */
    strcpy(identification_info.spatialRepresentationType, "grid");
    strcpy(identification_info.purpose, "My purpose goes here.");
    strcpy(identification_info.status, "My status goes here.");
    strcpy(identification_info.depthCorrectionType, "Mixed");
    identification_info.westBoundingLongitude = -145.123456;
    identification_info.eastBoundingLongitude = -143.654321;
    identification_info.southBoundingLatitude = 10.0;
    identification_info.northBoundingLatitude = 12.0;

  

    /* Provide the optional fields for MD_LEGAL_CONSTRAINTS. */
    strcpy(legal_constraints.useConstraints, "Restrictions");
    strcpy(legal_constraints.otherConstraints, "Not for Navigation");


    /* Provide the required fields for MD_SECURITY_CONSTRAINTS. */
    strcpy(security_constraints.classification, "UNCLASSIFIED");
    strcpy(security_constraints.userNote, "DOD distribution statement goes here.");


    /* Provide the required fields for DATA_QUALITY_INFO. */
	strcpy(data_quality.scope, "dataset");
	data_quality.numberOfSources = 1;
    data_quality.lineageSources = (SOURCE_INFO *)calloc(data_quality.numberOfSources,  sizeof(SOURCE_INFO));
    strcpy(data_quality.lineageSources[0].description, "file1");

    data_quality.numberOfProcessSteps = 1;
    data_quality.lineageProcessSteps = (PROCESS_STEP_INFO *)calloc(data_quality.numberOfProcessSteps,  sizeof(PROCESS_STEP_INFO));
    strcpy(data_quality.lineageProcessSteps[0].description, "some editing process");
    strcpy(data_quality.lineageProcessSteps[0].trackingId, "001");

 
    /* Provide the required fields for SPATIAL_REPRESENTATION_INFO. */
    /* 10 x 10 evenly spaced grid. */
    spatial_info.numberOfDimensions = 2;                    
    strcpy(spatial_info.dimensionName[0], "row");  
    spatial_info.dimensionSize[0] = 10;
    spatial_info.resolutionValue[0] = .2;

    strcpy(spatial_info.dimensionName[1], "column");        
    spatial_info.dimensionSize[1] = 10;                       
    spatial_info.resolutionValue[1] = .2; 
                    
    strcpy(spatial_info.cellGeometry, "point");                       
    strcpy(spatial_info.transformationParameterAvailability, "false");  
    strcpy(spatial_info.checkPointAvailability, "0");                   


    /* Provide some optional fields for SPATIAL_REPRESENTATION_INFO. */
    spatial_info.llCornerX = 145.123456789;
    spatial_info.llCornerY = 15.123456789;
    spatial_info.urCornerX = 147.003;
    spatial_info.urCornerY = 20.1;


   
    /* Provide the required fields for REFERENCE_SYSTEM_INFO. */
    strcpy(reference_info.projection, "Geodetic");                
    strcpy(reference_info.ellipsoid, "WGS84");            
	strcpy(reference_info.horizontalDatum, "WGS84");
    strcpy(reference_info.verticalDatum, "WGS84");


    /* Provide the required fields for contact. */
    strcpy(contact.role, "point of contact");                
    strcpy(contact.organisationName, "NAVO");      




    /* Provide the language of the BAG XML metadata. */
    language = calloc(3, sizeof(NV_CHAR));
    strcpy(language, "en");

  
    /* Provide the date that the BAG XML metadata was created. */
    date_stamp = calloc(11, sizeof(NV_CHAR));
    strcpy(date_stamp, "2008-08-20");



    
    /* Step 4:  Call CreateXmlMetadataString to populate an XML string in memory. */
    /* Remember to free the xml string later. */
    fprintf(stderr, "\nCreating XML string\n");
    NV_U_CHAR * formatted_xml_string = NULL;
    result = CreateXmlMetadataString(identification_info, legal_constraints,security_constraints, data_quality, spatial_info, reference_info, contact, date_stamp, &formatted_xml_string);

  

    /* OPTIONAL: Write the XML string to a file.  This is done only for the convenience of viewing the XML in a web browser application. */
    /* It is not necessary to write the XML string to a file. */
    if(result > 0)
    {	    

        fprintf(stderr, "XML string is %d  chars long.\n", result);
        writeXmlFile("./newXmlFromStructures.xml", formatted_xml_string);
    }
    else
    {

        fprintf(stderr, "Error creating XML string.  Error code is %d\n", result);
	   

    }


    free(language);
    free(date_stamp);
    free(formatted_xml_string);

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
