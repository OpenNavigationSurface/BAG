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

/* Routine:	bagValidateMetadataBuffer
 * Purpose:	Validate the XML data in the supplied memory buffer.
 * Inputs:	*buffer         The memory buffer containing the XML data.
 *          *bufferSize     The size of the memory buffer.
 *          *error          Modified to 0 if the function succeeds, non-zero if
 *                          the function fails.
 * Outputs:	The handle to the valid metaData if the function succeeds, NULL if the
 *          function fails.
 * Comment: 
 */
extern bagMetaData bagValidateMetadataBuffer(char *buffer, s32 bufferSize, bagError *error);

/* Routine:	bagValidateMetadataBuffer
 * Purpose:	Validate the XML data in the supplied memory buffer.
 * Inputs:	*buffer         The memory buffer containing the XML data.
 *          *bufferSize     The size of the memory buffer.
 *          *error          Modified to 0 if the function succeeds, non-zero if
 *                          the function fails.
 * Outputs:	The handle to the valid metaData if the function succeeds, NULL if the
 *          function fails.
 * Comment: 
 */
extern bagMetaData bagValidateMetadataFile(char *fileName, bagError *error);

/* Routine:	bagFreeMetadata
 * Purpose:	Free the memory associated with the given handle.
 * Inputs:	metaData        The meta data to be relelased.
 * Outputs:	
 * Comment: 
 */
extern void bagFreeMetadata(bagMetaData metaData);

/* Accessor routines. */
extern bagError bagGetXMLBuffer(bagMetaData metaData, char *buffer, u32 *maxBufferSize);
extern bagError bagGetCellDims(bagMetaData metaData, u32 *nRows, u32 *nCols);
extern bagError bagGetGeoCover(bagMetaData metaData, f64 *llLat, f64 *llLong, f64 *urLat, f64 *urLong);
extern bagError bagGetProjectedCover(bagMetaData metaData, f64 *llx, f64 *lly, f64 *urx, f64 *ury);
extern bagError bagGetGridSpacing(bagMetaData metaData, f64 *dx, f64 *dy);
extern bagError bagGetProjectionParams(bagMetaData metaData, char *projId, size_t projIdLen, 
                                  s32 *zone, f64 *standrardParallel, f64 *centralMeridian,
                                    f64 *latitudeOfOrigin, f64 *falseEasting, f64 *falseNorthing,
                                    f64 *scaleFactAtEq, f64 *heightOfPersPoint, f64 *longOfProjCenter,
                                    f64 *latOfProjCenter, f64 *scaleAtCenterLine, f64 *vertLongFromPole,
                                    f64 *scaleAtProjOrigin);
extern bagError bagGetHorizDatum(bagMetaData metaData, char *buffer, u32 bufferSize);
extern bagError bagGetUncertantyType(bagMetaData metaData, u32 *uncrtType);
extern bagError bagGetDepthCorrectionType(bagMetaData metaData, u32 *depthCorrectionType);
#ifdef __cplusplus
}
#endif
#endif
