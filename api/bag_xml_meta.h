
#include "stdtypes.h"
#include "ons_xml.h"
#include "bag_metadata_def.h"






#ifdef  __cplusplus
extern "C" {
#endif


/* Maximum number of parties that can be stored for citations. */
#define MAX_CI_RESPONSIBLE_PARTIES 10




/******************************************************************************************************

                                  Methods to set metadata are listed below.

******************************************************************************************************/

bagError  CreateXmlMetadataString( IDENTIFICATION_INFO identificationInfo, 
	LEGAL_CONSTRAINTS legalConstraints, SECURITY_CONSTRAINTS securityConstraints,  
	DATA_QUALITY_INFO dataQuality, SPATIAL_REPRESENTATION_INFO spatialRepresentationInfo, 
	bagLegacyReferenceSystem SystemInfo,  RESPONSIBLE_PARTY contact, 
	char * creationDate, char * file_identifier, unsigned char ** xmlString);




/******************************************************************************************************

                 Methods to clean up dynamically allocated memory for data structures are listed below.

******************************************************************************************************/

/* Currently only the GetDataQualityInfo method allocates memory dynamcially for a data structure. */
u16  FreeDataQualityInfo(u8 * xmlString, DATA_QUALITY_INFO * dataQuality );



#ifdef  __cplusplus
}
#endif








