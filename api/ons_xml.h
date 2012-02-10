/*************************************************************************
        File:    ons_xml.h
  
        Open Navigation Surface Working Group, 2005
  
        - Initial implementation
          Bill Lamey, 7/20/2005
  
*************************************************************************/
#ifndef ONS_BAG_XML_H
#define ONS_BAG_XML_H

#ifndef __BAG_H__
#include "bag.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define XML_ATTR_MAXSTR 256


/* Type definition of the bag metadata handle. */
typedef struct bagXMLHandle *bagMetaData;


/* Routine:	bagInitMetadata
 * Purpose:	Initialize the metadata module.
 * Inputs:	
 * Outputs:	Returns an error code.  0 if the function succeeds, non-zero if the
 *          function fails.
 * Comment: This function must be called before any of the other functions in
 *          this module can be used.
 */
extern bagError bagInitMetadata();

/* Routine:	bagTermMetadata
 * Purpose:	Terminate the metadata module.
 * Inputs:	
 * Outputs:	Returns an error code.  0 if the function succeeds, non-zero if the
 *          function fails.
 * Comment: This function must be called when the metadata module is no longer
 *          needed.
 */
extern bagError bagTermMetadata();

/* Routine:	bagGetMetadataFile
 * Purpose:	Retreive the metadata from the specified file.
 * Inputs:	*fileName       The file containing the metadata.
 *          *doValidation   true if the metadata should be validated against the schema,
 *                          false for no validation.
 *          *error          Modified to 0 if the function succeeds, non-zero if
 *                          the function fails.
 * Outputs:	The handle to the valid metaData if the function succeeds, NULL if the
 *          function fails.
 * Comment: 
 */
extern bagMetaData bagGetMetadataFile(char *fileName, Bool doValidation, bagError *error);

/* Routine:	bagGetMetadataBuffer
 * Purpose:	Retreive the metadata from the specified XML memory buffer.
 * Inputs:	*buffer         The memory buffer containing the XML data.
 *          *doValidation   true if the metadata should be validated against the schema,
 *                          false for no validation.
 *          *bufferSize     The size of the memory buffer.
 *          *error          Modified to 0 if the function succeeds, non-zero if
 *                          the function fails.
 * Outputs:	The handle to the valid metaData if the function succeeds, NULL if the
 *          function fails.
 * Comment: 
 */
extern bagMetaData bagGetMetadataBuffer(char *buffer, s32 bufferSize, Bool doValidation, bagError *error);

/* Routine:	bagFreeMetadata
 * Purpose:	Free the memory associated with the given handle.
 * Inputs:	metaData        The meta data to be relelased.
 * Outputs:	
 * Comment: 
 */
extern void bagFreeMetadata(bagMetaData metaData);

/* Accessor routines. */
extern bagError bagGetXMLBuffer(bagMetaData metaData, char *buffer, u32 *maxBufferSize);
extern bagError bagGetCellDims(bagMetaData metaData, const char *version, u32 *nRows, u32 *nCols);
extern bagError bagGetGeoCover(bagMetaData metaData, f64 *llLat, f64 *llLong, f64 *urLat, f64 *urLong);
extern bagError bagGetProjectedCover(bagMetaData metaData, const char *version, f64 *llx, f64 *lly, f64 *urx, f64 *ury);
extern bagError bagGetGridSpacing(bagMetaData metaData, const char *version, f64 *dx, f64 *dy);
extern bagError bagGetUncertantyType(bagMetaData metaData, const char *version, u32 *uncrtType);
extern bagError bagGetDepthCorrectionType(bagMetaData metaData, const char *version, u32 *depthCorrectionType);
extern bagError bagGetHReferenceSystem(bagMetaData metaData, const char *version, char *buffer, u32 bufferSize);
extern bagError bagGetVReferenceSystem(bagMetaData metaData, const char *version, char *buffer, u32 bufferSize);

extern bagError bagGetNodeGroupType(bagMetaData metaData, u8 *);
extern bagError bagGetElevationSolutionType(bagMetaData metaData, u8 *);
#ifdef __cplusplus
}
#endif
#endif
