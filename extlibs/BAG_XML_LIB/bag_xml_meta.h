#ifndef _BAG_XML_META_H_
#define _BAG_XML_META_H_

#ifdef  __cplusplus
extern "C" {
#endif


/* Purpose:  Contains structures and method prototypes for the BAG metadata.  
 *
 * History:  
 *
 * 3/2008   - J. Depner - Created initial data structures.       
 * 6/2008   - E. Warner - Added get methods and one createXML method.

 * 7/1/2008 - E. Warner - Renamed CONTACT structure to RESPONSIBLE_PARTY and added positionName field.
 *                        Added SOURCE_INFO and PROCESS_STEP_INFO type definitions to represent lineage metadata.
 *                        Modified DATA_QUALITY_INFO structure to include lineage metadata.
 *                        Modified IDENTIFICATION_INFO to include an array of cited responsible parties.
 *                        Modified SPATIAL_REPRESENTATION_INFO to represent cellGeometry, transformationParameterAvailability, and checkPointAvailability

 *                        Added SetContact method.
 *                        Added defines for maximum sizes.
 *
 * 7/23/2008 - E. Warner - Added set methods.   
 *                         Corrected bug where incorrect size was returnd for XML string in method CreateXmlMetadataString.   
 * 
 * 1/6/2009 - E. Warner - Added InitResponsibleParty method.     
 *        
 * 5/7/2009 - E. Warner - Changed dateTime field in PROCESS_STEP_INFO structure so that it is 30 chars long instead of 21.  
 *                         This new length accommodates the hours minutes, seconds and time zone that may be specified.
 * 
 * 5/18/2009 - E. Warner - Added method to free memory that was allocated during reading of XML into data structures.
 * 6/18/2009 - E. Warner - Modified "Get" method signatures to use NV_U_CH AR instead of NV_CHAR.
 *                         Done for consistency in method signature.
 * 6/30/2009 - M. Russell - Modified the length of the depthCorrectionType in the IDENTIFICATION_INFO
 *							structure from 22 to 32 to adjust for the types agreed on from the
 *							ONSWG 2009-06-12 teleconference
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "nvtypes.h"


/* Maximum number of parties that can be stored for citations. */
#define MAX_CI_RESPONSIBLE_PARTIES 10


/* Value to which floats and ints will be initialized. */
#define INIT_VALUE  -999    






/* OPTIONAL */

typedef struct
{

  NV_CHAR             individualName[100];            /* REQUIRED: One of the following must be supplied: individualName,organisationName, positionName. */  
  NV_CHAR             organisationName[100];
  NV_CHAR             positionName[100];
  NV_CHAR             phoneNumber[17];
  NV_CHAR             role[100];                      /* REQUIRED                                                                                        */

} RESPONSIBLE_PARTY;




/* REQUIRED */

typedef struct
{

  NV_CHAR             title[100];                     /* OPTIONAL: but this field is required in order to make a citation.     */
  NV_CHAR             date[20];                       /* OPTIONAL: but this field is required in order to make a citation date */
  NV_CHAR             dateType[20];                   /* OPTIONAL: but this field is required in order to make a citation date */
  RESPONSIBLE_PARTY   responsibleParties[MAX_CI_RESPONSIBLE_PARTIES];      /* OPTIONAL                                         */

  NV_CHAR             abstract[8000];                 /* REQUIRED                                                              */
  NV_CHAR             purpose[100];                   
  NV_CHAR             status[100];                   
  NV_CHAR             spatialRepresentationType[100]; /* OPTIONAL: For Nav Spec, this should be set to "grid".                  */
  NV_CHAR             language[100];                  /* REQUIRED: For Nav Spec, this should be set to "en"                     */
  NV_CHAR             topicCategory[100];             /* REQUIRED: For Nav Spec, this should be set to "grid"                   */
  NV_FLOAT64          westBoundingLongitude;          /* OPTIONAL: but this field is required in order to make an extent.       */
  NV_FLOAT64          eastBoundingLongitude;          /* OPTIONAL: but this field is required in order to make an extent.       */
  NV_FLOAT64          southBoundingLatitude;          /* OPTIONAL: but this field is required in order to make an extent.       */
  NV_FLOAT64          northBoundingLatitude;          /* OPTIONAL: but this field is required in order to make an extent.       */
  NV_CHAR             verticalUncertaintyType[40];    /* REQUIRED                                                               */
  NV_CHAR             depthCorrectionType[32];        /* OPTIONAL: Navigation specific */

} IDENTIFICATION_INFO;





typedef struct
{
  NV_CHAR             useConstraints[40];
  NV_CHAR             otherConstraints[100];          /* OPTIONAL: For Nav Spec, this should be set to "grid".                  */
} MD_LEGAL_CONSTRAINTS;






/* REQUIRED structure Navigation Specific */

typedef struct
{
  NV_CHAR            classification[40];             /* REQUIRED   */
  NV_CHAR            userNote[4000];                 /* REQUIRED: Contains distribution, declass authority, declass date.   */

} MD_SECURITY_CONSTRAINTS;





/* OPTIONAL */

typedef struct
{
  NV_CHAR            description[200];      /* REQUIRED: Navigation Specific required if source is desired.          */
  NV_CHAR            title[100];            /* OPTIONAL: but this field is required if a citation is desired.        */
  NV_CHAR            date[20];              /* OPTIONAL: but this field is required if a citation date is desired.   */
  NV_CHAR            dateType[20];          /* OPTIONAL: but this field is required if a citation date is desired.   */
  RESPONSIBLE_PARTY  responsibleParties[MAX_CI_RESPONSIBLE_PARTIES];   /* Placeholder.  Not implemented yet.         */

} SOURCE_INFO;





/* OPTIONAL */

typedef struct
{
  NV_CHAR            description[200];                          /* REQUIRED  */
  NV_CHAR            dateTime[30];
  RESPONSIBLE_PARTY  processors[MAX_CI_RESPONSIBLE_PARTIES];
  NV_CHAR            trackingId[5];                             /* REQUIRED  */

} PROCESS_STEP_INFO;




/* REQUIRED: Navigation Specific */

typedef struct
{

  NV_CHAR            scope[100];                   /* Navigation specific: always set to "dataset"                 */
  SOURCE_INFO        * lineageSources;             /* REQUIRED: Navigation specific, require at least 1 as scope is set to 'dataset'        */
  NV_INT16           numberOfSources;              /* REQUIRED: Navigation specific */
  PROCESS_STEP_INFO  * lineageProcessSteps;        /* REQUIRED: Navigation specific, require at least 1 as scope is set to 'dataset'        */
  NV_INT16           numberOfProcessSteps;         /* REQUIRED: Navigation specific */

} DATA_QUALITY_INFO;





/* REQUIRED: Navigation Specific */

typedef struct
{

  NV_INT16          numberOfDimensions;                        /* REQUIRED */
  NV_CHAR           dimensionName[3][20];                      /* REQUIRED: Navigation specific */
  NV_INT32          dimensionSize[3];                          /* REQUIRED: Navigation specific */
  NV_FLOAT64        resolutionValue[3];                        /* REQUIRED: Navigation specific */
  NV_CHAR           cellGeometry[10];                          /* REQUIRED: Nav Spec should always set this to "point"             */
  NV_CHAR           transformationParameterAvailability[6];    /* REQUIRED: Nav Spec should always set this to "false"             */
  NV_CHAR           checkPointAvailability[2];                 /* REQUIRED: Nav Spec should always set this to 0 indicating false. */
  NV_FLOAT64        llCornerX;                                
  NV_FLOAT64        llCornerY;                                 
  NV_FLOAT64        urCornerX;                              
  NV_FLOAT64        urCornerY;                            

} SPATIAL_REPRESENTATION_INFO;                            




/* REQUIRED:  Navigation Specific */

typedef struct
{

  NV_CHAR           projection[100];                      /* REQUIRED Navigation Specific */
  NV_CHAR           ellipsoid[100];                       /* REQUIRED Navigation Specific */
  NV_CHAR           horizontalDatum[100];                 /* REQUIRED Navigation Specific */
  NV_INT16          zone;                                
  NV_FLOAT64        standardParallel;                 
  NV_FLOAT64        longitudeOfCentralMeridian;        
  NV_FLOAT64        latitudeOfProjectionOrigin;       
  NV_FLOAT64        falseEasting;                      
  NV_FLOAT64        falseNorthing;                     
  NV_FLOAT64        scaleFactorAtEquator;                 
  NV_FLOAT64        heightOfProspectivePointAboveSurface; 
  NV_FLOAT64        longitudeOfProjectionCenter;        
  NV_FLOAT64        latitudeOfProjectionCenter;        
  NV_FLOAT64        scaleFactorAtCenterLine;            
  NV_FLOAT64        straightVerticalLongitudeFromPole;   
  NV_FLOAT64        scaleFactorAtProjectionOrigin;       
  NV_FLOAT64        semiMajorAxis;                      
  NV_CHAR           verticalDatum[100];                   /* REQUIRED Navigation Specific */

} REFERENCE_SYSTEM_INFO;



/******************************************************************************************************

                                  Methods to initialize structures are listed below.

******************************************************************************************************/

NV_INT16 InitDataIdentificationInfo(IDENTIFICATION_INFO * dataIdentificationInfo);

NV_INT16 InitLegalConstraints(MD_LEGAL_CONSTRAINTS * legalConstraints);

NV_INT16 InitSecurityConstraints(MD_SECURITY_CONSTRAINTS * securityConstraints);

NV_INT16 InitDataQualityInfo(DATA_QUALITY_INFO * dataQualityInfo);

NV_INT16 InitReferenceSystemInfo(REFERENCE_SYSTEM_INFO * referenceSystemInfo);

NV_INT16 InitSpatialRepresentationInfo(SPATIAL_REPRESENTATION_INFO * spatialRepresentationInfo);

NV_INT16 InitResponsibleParty(RESPONSIBLE_PARTY * responsibleParty);

NV_INT16 InitSourceInfo(SOURCE_INFO * sourceInfo);

/******************************************************************************************************

                                  Methods to get metadata are listed below.

******************************************************************************************************/


NV_INT16  GetDataIdentification(NV_U_CHAR * xmlString, IDENTIFICATION_INFO * dataIdentificationInfo);


NV_INT16  GetLegalConstraints(NV_U_CHAR * xmlString, MD_LEGAL_CONSTRAINTS * legalConstraints);


NV_INT16  GetSecurityConstraints(NV_U_CHAR * xmlString, MD_SECURITY_CONSTRAINTS * securityConstraints);


NV_INT16  GetDataQualityInfo(NV_U_CHAR * xmlString, DATA_QUALITY_INFO * dataQuality );


NV_INT16  GetReferenceSystemInfo(NV_U_CHAR * xmlString, REFERENCE_SYSTEM_INFO * referenceSystemInfo);


NV_INT16  GetSpatialRepresentationInfo(NV_U_CHAR * xmlString, SPATIAL_REPRESENTATION_INFO * spatialRepresentationInfo);


NV_INT16  GetDateStamp(NV_U_CHAR * xmlString, NV_CHAR ** dateString);


NV_INT16  GetLanguage(NV_U_CHAR * xmlString, NV_CHAR ** language);  


NV_INT16  GetContact(NV_U_CHAR * xmlString, RESPONSIBLE_PARTY * contact);


NV_INT16  GetMetadataStandardName(NV_U_CHAR * xmlString, NV_CHAR ** metadataStandardName);


NV_INT16  GetMetadataStandardVersion( NV_U_CHAR * xmlString, NV_CHAR ** metadataStandardVersion);



NV_INT16  GetAllStructures( NV_U_CHAR * xmlString,IDENTIFICATION_INFO * identificationInfo, MD_LEGAL_CONSTRAINTS * legalConstraints, MD_SECURITY_CONSTRAINTS * securityConstraints,  DATA_QUALITY_INFO * dataQuality, SPATIAL_REPRESENTATION_INFO * spatialRepresentationInfo, REFERENCE_SYSTEM_INFO * referenceSystemInfo, RESPONSIBLE_PARTY * contact );











/******************************************************************************************************

                                  Methods to set metadata are listed below.

******************************************************************************************************/


NV_INT32  CreateXmlMetadataString(IDENTIFICATION_INFO identificationInfo, 
                                  MD_LEGAL_CONSTRAINTS legalConstraints, 
                                  MD_SECURITY_CONSTRAINTS securityConstraints,  
                                  DATA_QUALITY_INFO dataQuality, 
                                  SPATIAL_REPRESENTATION_INFO spatialRepresentationInfo, 
                                  REFERENCE_SYSTEM_INFO referenceSystemInfo, 
                                  RESPONSIBLE_PARTY contact, 
                                  NV_CHAR * creationDate, 
                                  NV_U_CHAR ** xmlString);



NV_INT32 SetContact(RESPONSIBLE_PARTY newContact,  
                    NV_U_CHAR ** xmlString);


NV_INT32 SetIdentificationInfo(IDENTIFICATION_INFO newIdentificationInfo, 
                               NV_U_CHAR ** xmlString);



NV_INT32 SetLegalConstraints(MD_LEGAL_CONSTRAINTS newLegalConstraints,
                             NV_U_CHAR ** xmlString);




NV_INT32 SetSecurityConstraints(MD_SECURITY_CONSTRAINTS newSecurityConstraints,  
                                  NV_U_CHAR ** xmlString);


NV_INT32 SetDataQualityInfo(DATA_QUALITY_INFO newDataQualityInfo,   
                            NV_U_CHAR ** xmlString);


NV_INT32 SetSpatialRepresentationInfo(SPATIAL_REPRESENTATION_INFO newSpatialRepresentationInfo, 
                                      NV_U_CHAR ** xmlString);


NV_INT32 SetReferenceSystemInfo(REFERENCE_SYSTEM_INFO newIdentificationInfo, 
                                NV_U_CHAR ** xmlString);



NV_INT32 SetLanguage(NV_CHAR * newLanguage,  
                     NV_U_CHAR ** xmlString);

NV_INT32 SetDateStamp(NV_CHAR * newDateStamp,  
                     NV_U_CHAR ** xmlString);

NV_INT32 SetMetadataStandardName(NV_CHAR * newMetadataStandardName,  
				 NV_U_CHAR ** xmlString);


NV_INT32 SetMetadataStandardVersion(NV_CHAR * newMetadataStandardVersion,  
                                    NV_U_CHAR ** xmlString);










/******************************************************************************************************

                                  Methods to print data structures are listed below.

******************************************************************************************************/

void PrintIdentificationInfo(IDENTIFICATION_INFO identificationInfo);
void PrintLegalConstraints(MD_LEGAL_CONSTRAINTS legalConstraints); 
void PrintSecurityConstraints(MD_SECURITY_CONSTRAINTS securityConstraints);  
void PrintDataQuality(DATA_QUALITY_INFO dataQuality);
void PrintSpatialRepresentationInfo(SPATIAL_REPRESENTATION_INFO spatialRepresentationInfo);
void PrintReferenceSystemInfo(REFERENCE_SYSTEM_INFO referenceSystemInfo);
void PrintContact(RESPONSIBLE_PARTY contact);



/******************************************************************************************************

                 Methods to clean up dynamically allocated memory for data structures are listed below.

******************************************************************************************************/

/* Currently only the GetDataQualityInfo method allocates memory dynamcially for a data structure. */
NV_INT16  FreeDataQualityInfo(NV_U_CHAR * xmlString, DATA_QUALITY_INFO * dataQuality );





#ifdef  __cplusplus
}
#endif

#endif
