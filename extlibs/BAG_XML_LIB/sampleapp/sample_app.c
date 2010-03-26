/*****************************************************************************************
*
* Program: Sample Application
*
* Purpose: Illustrates the use of the BAG XML metadata library.
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


/* Local methods.  Here only for convenience.  Not required in order to use BAG XMl metadata library. */
void  readSampleXml(NV_CHAR  pathToFile[100], NV_U_CHAR ** xml_string);
void  writeXmlFile(NV_CHAR  pathToFile[100], NV_U_CHAR * xml_string);


int main(int argc, char *argv[])
{



    /* Step 1:  Declare one of each of the required structures. */

    IDENTIFICATION_INFO         identification_info;
    MD_LEGAL_CONSTRAINTS        legal_constraints;
    MD_SECURITY_CONSTRAINTS     security_constraints;
    DATA_QUALITY_INFO           data_quality;
    SPATIAL_REPRESENTATION_INFO spatial_info;
    REFERENCE_SYSTEM_INFO       reference_info;
    RESPONSIBLE_PARTY           contact;




    /* Step 2:  Initialize the required structures. This will set all integeters and floats to -999.  This will also set all strings to an emapty string. */

    InitDataIdentificationInfo(&identification_info);
    InitLegalConstraints(&legal_constraints);
	InitSecurityConstraints(&security_constraints);
	InitDataQualityInfo(&data_quality);
    InitReferenceSystemInfo(&reference_info);
    InitSpatialRepresentationInfo(&spatial_info);







    /* Step 3:  Call GetAllStructures to populate the structures with the contents the BAG XML metadata string. */

    /* Get a BAG XML metadata string from somewhere.  In this case, the sample XML file from the BAG library is read in. */

    NV_U_CHAR * xml_string;
    readSampleXml("./sample.xml", &xml_string);

    GetAllStructures(xml_string, &identification_info, &legal_constraints, &security_constraint,  &data_quality2, &spatial_info, &reference_info, &contact );





    
    /* Step 4:  Call individual print methods to display contents of structures as populated from the BAG XML metadata string.. */

    PrintIdentificationInfo(identification_info2);
    PrintLegalConstraints(legal_constraints2); 
    PrintSecurityConstraints(security_constraints2);  
    PrintDataQuality(data_quality2);
    PrintSpatialRepresentationInfo(spatial_info2);
    PrintReferenceSystemInfo(reference_info2);
    PrintContact(contact2);





    /* Optional step.  Get the language, metadata standard name and version from the BAG XML metadata string. */

    NV_CHAR * language;
    fprintf(stderr,"\n\nGetting language for BAG file from XML metadata.\n");
    GetLanguage(xml_string, &language);
    fprintf(stderr,"language: %s\n", language);
    free(language);


    NV_CHAR * metadataStandardName;
    fprintf(stderr,"\n\nGetting metadataStandardName from BAG file from XML metadata.\n");
    GetMetdataStandardName(xml_string, &metadataStandardName);
    fprintf(stderr,"metadataStandardNam: %s\n", metadataStandardName);
    free(metadataStandardName);


    NV_CHAR * metadataStandardVersion;
    fprintf(stderr,"\n\nGetting metadataStandardVersion from BAG file from XML metadata.\n");
    GetMetadataStandardVersion(xml_string, &metadataStandardVersion);
    fprintf(stderr,"metadataStandardVersion: %s\n", metadataStandardVersion);
    free(metadataStandardVersion);


    free(xml_string);
}






/* 
 *
* Method to read the sample BAG XML metdata file into a string. 
*
*/
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


