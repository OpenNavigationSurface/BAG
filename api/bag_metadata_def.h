#ifndef __BAG_METADATA_DEF_H__
#define __BAG_METADATA_DEF_H__


#ifndef __BAG_H__
#include "bag.h"
#endif

#ifndef __ONS_BAG_XML_H__
#include "ons_xml.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif

/* Maximum number of parties that can be stored for citations. */
#define MAX_CI_RESPONSIBLE_PARTIES 10

	
/* Value to which floats and ints will be initialized. */
#define INIT_VALUE  -999    


typedef struct
{

  u8             individualName[100];            /* REQUIRED: One of the following must be supplied: individualName,organisationName, positionName. */  
  u8             organisationName[100];
  u8             positionName[100];
  u8             contactInfo[17];
  u8             role[100];                      /* REQUIRED                                                                                        */

} RESPONSIBLE_PARTY;

typedef struct
{
  u8             useConstraints[40];
  u8             otherConstraints[100];          /* OPTIONAL: For Nav Spec, this should be set to "grid".                  */
} LEGAL_CONSTRAINTS;

typedef struct
{
  u8            classification[40];             /* REQUIRED   */
  u8            *userNote;                 /* REQUIRED: Contains distribution, declass authority, declass date.   */

} SECURITY_CONSTRAINTS;

typedef struct
{
  u8            description[200];      /* REQUIRED: Navigation Specific required if source is desired.          */
  u8            title[100];            /* OPTIONAL: but this field is required if a citation is desired.        */
  u8            date[20];              /* OPTIONAL: but this field is required if a citation date is desired.   */
  u8            dateType[20];          /* OPTIONAL: but this field is required if a citation date is desired.   */
  RESPONSIBLE_PARTY  responsibleParties[MAX_CI_RESPONSIBLE_PARTIES];   /* Placeholder.  Not implemented yet.         */

} SOURCE_INFO;

typedef struct
{
  u8            description[200];						    /* REQUIRED  */
  u8            dateTime[30];
  RESPONSIBLE_PARTY  processors[MAX_CI_RESPONSIBLE_PARTIES];
  u8            trackingId[5];			                    /* REQUIRED  */

} PROCESS_STEP_INFO;

typedef struct
{

  u8            scope[100];                   /* Navigation specific: always set to "dataset"                 */
  SOURCE_INFO   *lineageSources;             /* REQUIRED: Navigation specific, require at least 1 as scope is set to 'dataset'        */
  u16           numberOfSources;              /* REQUIRED: Navigation specific */
  PROCESS_STEP_INFO  *lineageProcessSteps;   /* REQUIRED: Navigation specific, require at least 1 as scope is set to 'dataset'        */
  u16           numberOfProcessSteps;         /* REQUIRED: Navigation specific */

} DATA_QUALITY_INFO;

typedef struct
{

  u16        numberOfDimensions;                        /* REQUIRED */
  u8         dimensionName[3][20];                      /* REQUIRED: Navigation specific */
  u32        dimensionSize[3];                          /* REQUIRED: Navigation specific */
  f64        resolutionValue[3];                        /* REQUIRED: Navigation specific */
  u8         cellGeometry[10];                          /* REQUIRED: Nav Spec should always set this to "point"             */
  u8         transformationParameterAvailability[6];    /* REQUIRED: Nav Spec should always set this to "false"             */
  u8         checkPointAvailability[2];                 /* REQUIRED: Nav Spec should always set this to 0 indicating false. */
  f64        llCornerX;                                
  f64        llCornerY;                                 
  f64        urCornerX;                              
  f64        urCornerY;                            

} SPATIAL_REPRESENTATION_INFO;

typedef struct
{

  u8         projection[100];                      /* REQUIRED Navigation Specific */
  u8         ellipsoid[100];                       /* REQUIRED Navigation Specific */
  u8         horizontalDatum[100];                 /* REQUIRED Navigation Specific */
  s32        zone;                                
  f64        standardParallel;                 
  f64        longitudeOfCentralMeridian;        
  f64        latitudeOfProjectionOrigin;       
  f64        falseEasting;                      
  f64        falseNorthing;                     
  f64        scaleFactorAtEquator;                 
  f64        heightOfProspectivePointAboveSurface; 
  f64        longitudeOfProjectionCenter;        
  f64        latitudeOfProjectionCenter;        
  f64        scaleFactorAtCenterLine;            
  f64        straightVerticalLongitudeFromPole;   
  f64        scaleFactorAtProjectionOrigin;       
  f64        semiMajorAxis;                      
  u8         verticalDatum[100];                   /* REQUIRED Navigation Specific */

} REFERENCE_SYSTEM_INFO;

typedef struct
{

  u8             title[100];                     /* OPTIONAL: but this field is required in order to make a citation.     */
  u8              date[20];                       /* OPTIONAL: but this field is required in order to make a citation date */
  u8              dateType[20];                   /* OPTIONAL: but this field is required in order to make a citation date */
  RESPONSIBLE_PARTY   responsibleParties[MAX_CI_RESPONSIBLE_PARTIES];      /* OPTIONAL                                         */

  u8              *abstract;                 /* REQUIRED                                                              */
  u8              purpose[100];                   
  u8              status[100];                   
  u8              spatialRepresentationType[100]; /* OPTIONAL: For Nav Spec, this should be set to "grid".                  */
  u8              language[100];                  /* REQUIRED: For Nav Spec, this should be set to "en"                     */
  u8				character_set[16];				
  u8              topicCategory[100];             /* REQUIRED: For Nav Spec, this should be set to "grid"                   */
  f64          westBoundingLongitude;          /* OPTIONAL: but this field is required in order to make an extent.       */
  f64          eastBoundingLongitude;          /* OPTIONAL: but this field is required in order to make an extent.       */
  f64          southBoundingLatitude;          /* OPTIONAL: but this field is required in order to make an extent.       */
  f64          northBoundingLatitude;          /* OPTIONAL: but this field is required in order to make an extent.       */
  u8              verticalUncertaintyType[40];    /* REQUIRED                                                               */
  u8              depthCorrectionType[32];        /* OPTIONAL: Navigation specific */
  u8	   nodeGroupType[40];    	                      /* The type of optional node group */
  u8	   elevationSolutionGroupType[40];	              /* The type of optional elevation solution group */


} IDENTIFICATION_INFO;


extern bagError bagGetContact(bagMetaData metaData, const char *version, RESPONSIBLE_PARTY *contact);
extern bagError bagGetLegalConstraints(bagMetaData metaData, const char *version, LEGAL_CONSTRAINTS *legal);
extern bagError bagGetSecurityConstraints(bagMetaData metaData, const char *version, SECURITY_CONSTRAINTS *security);
extern bagError bagGetSourceInfo(bagMetaData metaData, const char *version, SOURCE_INFO *sourceInfo);
extern bagError bagGetProcessStepInfo(bagMetaData metaData, const char *version, PROCESS_STEP_INFO *processStepInfo);
extern bagError bagGetSpatialRepresentationInfo(bagMetaData metaData, const char *version, SPATIAL_REPRESENTATION_INFO *spatial_rep_info);
extern bagError bagGetDataQualityInfo(bagMetaData metaData, const char *version, DATA_QUALITY_INFO *dataQualityInfo);
extern bagError bagGetIdentificationInfo(bagMetaData metaData, const char *version, IDENTIFICATION_INFO *identification_info);
extern bagError bagGetFileIdentifier(bagMetaData metaData, const char *version, char **);


/******************************************************************************************************

                                  Methods to initialize structures are listed below.

******************************************************************************************************/

bagError InitDataIdentificationInfo(IDENTIFICATION_INFO * dataIdentificationInfo);

bagError InitLegalConstraints(LEGAL_CONSTRAINTS * legalConstraints);

bagError InitSecurityConstraints(SECURITY_CONSTRAINTS * securityConstraints);

bagError InitDataQualityInfo(DATA_QUALITY_INFO * dataQualityInfo);

bagError InitReferenceSystemInfo(bagLegacyReferenceSystem * referenceSystemInfo);

bagError InitSpatialRepresentationInfo(SPATIAL_REPRESENTATION_INFO * spatialRepresentationInfo);

bagError InitResponsibleParty(RESPONSIBLE_PARTY * responsibleParty);

bagError InitSourceInfo(SOURCE_INFO * sourceInfo);





#ifdef __cplusplus
}
#endif
#endif
