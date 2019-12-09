/*! \file bag.h
 * \brief Declarations of Open Navigation Surface API.
 ********************************************************************
 *
 * Authors/Date : Fri Jan 27 14:41:13 2006
 *     Brian Calder      (CCOM/JHC)
 *     Rick Brennan      (CCOM/JHC)
 *     Bill Lamey        (CARIS Ltd)
 *     Mark Paton        (IVS Ltd)
 *     Shannon Byrne     (SAIC, Newport)
 *     Jim Case          (SAIC, Newport)
 *     Webb McDonald     (SAIC, Newport)
 *     Dave Fabre        (NAVOCEANO)
 *     Jan Depner        (NAVOCEANO)
 *     Wade Ladner       (NAVOCEANO)
 *     Barry Gallagher   (NOAA HSTP)
 *     Friedhelm Moggert (7Cs GmbH)
 *     Shep Smith        (NOAA)
 *     Jack Riley        (NOAA HSTP)
 *
 * Initial concepts developed during The Open Navigation Surface Workshop
 *   attended by above contributors meeting at CCOM/JHC.
 *
 * Description :
 *   This is the header file for the Bathymetric Attributed Grid (BAG)
 *   access library.  This file defines the data structures and function
 *   prototypes for accessing BAG files.
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 * who when      what
 * --- ----      ----
 *
 * Classification :
 *  Unclassified
 *
 * Distribution :
 *
 * Usage And Licensing :
 *
 * References :
 *     Bathymetric Attributed Grid Format Specification Document
 *     http://hdf.ncsa.uiuc.edu/HDF5/
 *
 ********************************************************************/
#ifndef BAG_H
#define BAG_H

#include "bag_config.h"
#include "bag_metadatatypes.h"
#include "bag_types.h"
#include "bag_version.h"

#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct BagHandle BagHandle;

/* Function prototypes */

/* Open/Create/Close */
BAG_EXTERNAL BagError bagFileOpen(BagHandle* BagHandle, BAG_OPEN_MODE accessMode, const char* fileName);
BAG_EXTERNAL BagError bagFileClose(BagHandle BagHandle);
BAG_EXTERNAL BagError bagCreateFromFile(const char* file_name, const char* metaDataFile, BagHandle* bag_handle);
BAG_EXTERNAL BagError bagCreateFromBuffer(const char* file_name, uint8_t* metaDataBuffer, uint32_t metaDataBufferSize, BagHandle* bag_handle);
BAG_EXTERNAL BagError bagCreateLayer(BagHandle hnd, BAG_LAYER_TYPE type);
BAG_EXTERNAL BagError bagGetGridDimensions(BagHandle hnd, uint32_t* rows, uint32_t* cols);
BAG_EXTERNAL BagError bagGetNodeSpacing(BagHandle handle, double* xSpacing, double* ySpacing);
BAG_EXTERNAL BagError bagGetGeoCover(BagHandle handle, double* llx, double* lly, double* urx, double* ury);
BAG_EXTERNAL const BagMetadata* bagGetMetaData(BagHandle handle);

/* Layer access */
BAG_EXTERNAL BagError bagGetMinMax(BagHandle hnd, BAG_LAYER_TYPE type, double* minValue, double* maxValue);
BAG_EXTERNAL BagError bagSetMinMax(BagHandle hnd, BAG_LAYER_TYPE type, double* minValue, double* maxValue);
BAG_EXTERNAL BagError bagGetNumLayers(BagHandle hnd, uint32_t* numLayers);
BAG_EXTERNAL bool bagContainsLayer(BagHandle hnd, BAG_LAYER_TYPE type);
BAG_EXTERNAL BagError bagRead (BagHandle BagHandle, uint32_t start_row, uint32_t start_col, uint32_t end_row, uint32_t end_col, BAG_LAYER_TYPE type, uint8_t* data, double* x, double* y);
BAG_EXTERNAL BagError bagWrite (BagHandle BagHandle, uint32_t start_row, uint32_t start_col, uint32_t end_row, uint32_t end_col, BAG_LAYER_TYPE type, uint8_t* data);

/* Utilities */
BAG_EXTERNAL BagError bagGetErrorString(BagError code, uint8_t** error);
BAG_EXTERNAL BagError bagComputePostion(BagHandle hnd, uint32_t row, uint32_t col, double* x, double* y);
BAG_EXTERNAL void* bagGetNullValue(BAG_LAYER_TYPE type);
BAG_EXTERNAL BagError bagComputeIndex(BagHandle hnd, double x, double y, uint32_t* row, uint32_t* col);
BAG_EXTERNAL BagError bagSetCompressionLevel(uint8_t level);
BAG_EXTERNAL uint8_t* bagAllocateBuffer(uint32_t start_row, uint32_t start_col, uint32_t end_row, uint32_t end_col, BAG_LAYER_TYPE type);
BAG_EXTERNAL uint8_t* bagAllocate(uint32_t numBytes);
BAG_EXTERNAL BagError bagFree(uint8_t* buffer);

/* Corrector surface access */
BAG_EXTERNAL BagError bagReadCorrectorVerticalDatum(BagHandle hnd, uint32_t, uint8_t*  datum);
BAG_EXTERNAL BagError bagWriteCorrectorVerticalDatum(BagHandle hnd, uint32_t, uint8_t*  datum);
BAG_EXTERNAL BagError bagReadCorrectedDataset(BagHandle BagHandle, uint32_t corrIndex, uint32_t surfIndex, float* data);
BAG_EXTERNAL BagError bagReadCorrectedRegion(BagHandle BagHandle, uint32_t startrow, uint32_t endrow, uint32_t startcol, uint32_t endcol, uint32_t corrIndex, uint32_t surfIndex, float* data);
BAG_EXTERNAL BagError bagReadCorrectedRow(BagHandle BagHandle, uint32_t row, uint32_t corrIndex, uint32_t surfIndex, float* data);
BAG_EXTERNAL BagError bagReadCorrectedNode(BagHandle BagHandle, uint32_t row, uint32_t col, uint32_t corrIndex, uint32_t surfIndex, float* data);
BAG_EXTERNAL BagError bagGetNumSurfaceCorrectors(BagHandle hnd_opt, uint32_t* num);
BAG_EXTERNAL BagError bagGetSurfaceCorrectionTopography(BagHandle hnd, uint8_t* type);
BAG_EXTERNAL BagError bagCreateCorrectorDataset(BagHandle hnd, uint32_t numCorrectors, uint8_t type);
//BAG_EXTERNAL BagError bagWriteCorrectorDefinition(BagHandle hnd, bagVerticalCorrectorDef *def);
//BAG_EXTERNAL BagError bagReadCorrectorDefinition(BagHandle hnd, bagVerticalCorrectorDef *def);

/* Tracking list access */
BAG_EXTERNAL BagError bagTrackingListLength (BagHandle BagHandle, uint32_t* len);
BAG_EXTERNAL BagError bagReadTrackingListNode(BagHandle BagHandle, uint32_t row, uint32_t col, BagTrackingItem* *items, uint32_t* length);
BAG_EXTERNAL BagError bagReadTrackingListCode(BagHandle BagHandle, uint8_t code, BagTrackingItem* *items, uint32_t* length);
BAG_EXTERNAL BagError bagReadTrackingListSeries(BagHandle BagHandle, uint16_t index, BagTrackingItem* *items, uint32_t* length);
BAG_EXTERNAL BagError bagWriteTrackingListItem(BagHandle BagHandle, BagTrackingItem* item);
BAG_EXTERNAL BagError bagSortTrackingListByNode(BagHandle BagHandle);
BAG_EXTERNAL BagError bagSortTrackingListBySeries(BagHandle BagHandle);
BAG_EXTERNAL BagError bagSortTrackingListByCode(BagHandle BagHandle);

/* Digital signature */
/*BAG_EXTERNAL uint8_t* bagComputeMessageDigest(const char* fileName, uint32_t signatureID, uint32_t* nBytes);
BAG_EXTERNAL uint8_t* bagSignMessageDigest(uint8_t* md, uint32_t mdLen, uint8_t* secKey, BagError* errcode);
BAG_EXTERNAL BagError bagReadCertification(const char* fileName, uint8_t* sig, uint32_t nBuffer, uint32_t* sigID);
BAG_EXTERNAL BagError bagWriteCertification(const char* fileName, uint8_t* sig, uint32_t sigID);
BAG_EXTERNAL bool bagVerifyCertification(uint8_t* sig, uint8_t* pubKey, uint8_t* md, uint32_t mdLen);
BAG_EXTERNAL uint8_t* bagComputeFileSignature(const char* fileName, uint32_t sigID, uint8_t* secKey);
BAG_EXTERNAL bool bagSignFile(const char* fileName, uint8_t* secKey, uint32_t sigID);
BAG_EXTERNAL bool bagVerifyFile(const char* fileName, uint8_t* pubKey, uint32_t sigID);
BAG_EXTERNAL BagError bagGenerateKeyPair(uint8_t* *pubKey, uint8_t** secKey);
BAG_EXTERNAL BagError bagConvertCryptoFormat(uint8_t* object, bagCryptoObject objType, bagConvDir convDir, uint8_t** converted);
*/

/* Metadata */
/*BAG_EXTERNAL BagError bagInitMetadata(BagMetadata* metadata);
BAG_EXTERNAL void bagFreeMetadata(BagMetadata* metadata);
BAG_EXTERNAL void bagSetHomeFolder(const char* metadataFolder);*/


#endif // BAG_H

