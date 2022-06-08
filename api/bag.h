/*! \file bag.h
 * \brief Declarations of Open Navigation Surface C API.
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
#include "bag_c_types.h"
#include "bag_metadatatypes.h"
#include "bag_version.h"


typedef struct BagHandle* Handle;

/* Function prototypes */

/* Open/Create/Close Dataset */
BAG_EXTERNAL BagError bagCreateFromBuffer(BagHandle** handle, const char* fileName, uint8_t* metadataBuffer, uint32_t metadataBufferSize);
BAG_EXTERNAL BagError bagCreateFromFile(BagHandle** handle, const char* fileName, const char* metadataFile);
BAG_EXTERNAL BagError bagCreateLayer(BagHandle* handle, BAG_LAYER_TYPE type);
BAG_EXTERNAL BagError bagFileClose(BagHandle* handle);
BAG_EXTERNAL BagError bagFileOpen(BagHandle** handle, BAG_OPEN_MODE accessMode, const char* fileName);
BAG_EXTERNAL BagError bagGetGeoCover(BagHandle* handle, double* llx, double* lly, double* urx, double* ury);
BAG_EXTERNAL BagError bagGetGridDimensions(BagHandle* handle, uint32_t* rows, uint32_t* cols);
BAG_EXTERNAL BagError bagGetSpacing(BagHandle* handle, double* rowSpacing, double* columnSpacing);

/* Layer access */
BAG_EXTERNAL BagError bagGetNumLayers(BagHandle* handle, uint32_t* numLayers);
BAG_EXTERNAL bool bagContainsLayer(BagHandle* handle, BAG_LAYER_TYPE type, const char* layerName, BagError* bagError);
BAG_EXTERNAL BagError bagRead(BagHandle* handle, uint32_t rowStart, uint32_t colStart, uint32_t rowEnd, uint32_t colEnd, BAG_LAYER_TYPE type, const char* layerName, uint8_t** data, double* x, double* y);
BAG_EXTERNAL BagError bagWrite(BagHandle* handle, uint32_t rowStart, uint32_t colStart, uint32_t rowEnd, uint32_t colEnd, BAG_LAYER_TYPE type, const char* layerName, uint8_t* data);

/* Simple layer access */
BAG_EXTERNAL BagError bagGetMinMaxSimple(BagHandle* handle, BAG_LAYER_TYPE type, float* minValue, float* maxValue);
BAG_EXTERNAL BagError bagSetMinMaxSimple(BagHandle* handle, BAG_LAYER_TYPE type, float minValue, float maxValue);

/* Utilities */
BAG_EXTERNAL BagError bagGetErrorString(BagError code, uint8_t** error);
BAG_EXTERNAL BagError bagComputePostion(BagHandle* handle, uint32_t row, uint32_t col, double* x, double* y);
BAG_EXTERNAL BagError bagComputeIndex(BagHandle* handle, double x, double y, uint32_t* row, uint32_t* col);
BAG_EXTERNAL uint8_t* bagAllocateBuffer(BagHandle* handle, uint32_t numRows, uint32_t numCols, BAG_LAYER_TYPE type, const char* layerName, BagError* bagError);
BAG_EXTERNAL uint8_t* bagAllocate(uint32_t numBytes);
BAG_EXTERNAL void bagFree(uint8_t* buffer);

/* Corrector surface access */
BAG_EXTERNAL BagError bagReadCorrectorVerticalDatum(BagHandle* handle, uint8_t corrector, uint8_t*  datum);
BAG_EXTERNAL BagError bagWriteCorrectorVerticalDatum(BagHandle* handle, uint8_t corrector, const uint8_t*  datum);
BAG_EXTERNAL BagError bagReadCorrectedLayer(BagHandle* handle, uint8_t corrector, BAG_LAYER_TYPE type, float** data);
BAG_EXTERNAL BagError bagReadCorrectedRegion(BagHandle* handle, uint32_t rowStart, uint32_t colStart, uint32_t rowEnd, uint32_t colEnd, uint8_t corrector, BAG_LAYER_TYPE type, float** data);
BAG_EXTERNAL BagError bagReadCorrectedRow(BagHandle* handle, uint32_t row, uint8_t corrector, BAG_LAYER_TYPE type, float** data);
BAG_EXTERNAL BagError bagReadCorrectedNode(BagHandle* handle, uint32_t row, uint32_t col, uint8_t corrector, BAG_LAYER_TYPE type, float** data);
BAG_EXTERNAL BagError bagGetNumSurfaceCorrectors(BagHandle* handle, uint8_t* numCorrectors);
BAG_EXTERNAL BagError bagGetSurfaceCorrectionTopography(BagHandle* handle, BAG_SURFACE_CORRECTION_TOPOGRAPHY* topography);
BAG_EXTERNAL BagError bagCreateCorrectorLayer(BagHandle* handle, uint8_t numCorrectors, BAG_SURFACE_CORRECTION_TOPOGRAPHY topography);
BAG_EXTERNAL BagError bagWriteCorrectorDefinition(BagHandle* handle, BagVerticalCorrectorDef *def);
BAG_EXTERNAL BagError bagReadCorrectorDefinition(BagHandle* handle, BagVerticalCorrectorDef *def);

/* Tracking list access */
BAG_EXTERNAL BagError bagTrackingListLength (BagHandle* handle, uint32_t* len);
BAG_EXTERNAL BagError bagReadTrackingListNode(BagHandle* handle, uint32_t row, uint32_t col, BagTrackingItem** items, uint32_t* numItems);
BAG_EXTERNAL BagError bagReadTrackingListCode(BagHandle* handle, uint8_t code, BagTrackingItem** items, uint32_t* length);
BAG_EXTERNAL BagError bagReadTrackingListSeries(BagHandle* handle, uint16_t series, BagTrackingItem** items, uint32_t* length);
BAG_EXTERNAL BagError bagWriteTrackingListItem(BagHandle* handle, BagTrackingItem* item);
BAG_EXTERNAL BagError bagSortTrackingListByNode(BagHandle* handle);
BAG_EXTERNAL BagError bagSortTrackingListBySeries(BagHandle* handle);
BAG_EXTERNAL BagError bagSortTrackingListByCode(BagHandle* handle);

/* Metadata */
//BAG_EXTERNAL BagError bagInitMetadata(BagHandle* handle, BagMetadata* metadata);
//BAG_EXTERNAL BagError bagFreeMetadata(BagHandle* handle, BagMetadata* metadata);
BAG_EXTERNAL const BagMetadata* bagGetMetaData(BagHandle* handle);
BAG_EXTERNAL BagError bagSetHomeFolder(const char* metadataFolder);

/* CompoundLayer */
BAG_EXTERNAL BagError bagCreateCompoundLayer(BagHandle* handle, BAG_DATA_TYPE indexType, GEOREF_METADATA_PROFILE profile, const char* layerName, const FieldDefinition* definition, uint32_t numFields);
BAG_EXTERNAL BagError bagCreateMetadataProfileCompoundLayer(BagHandle* handle, BAG_DATA_TYPE indexType, GEOREF_METADATA_PROFILE profile, const char* layerName);
BAG_EXTERNAL BagError bagGetCompoundLayerDefinition(BagHandle* handle, const char* layerName, FieldDefinition** definition, uint32_t* numDefinitions);
BAG_EXTERNAL BagError bagGetCompoundLayerRecords(BagHandle* handle, const char* layerName, BagCompoundDataType*** records, uint32_t* numRecords, uint32_t* numFields);
BAG_EXTERNAL BagError bagGetCompoundLayerValueByName(BagHandle* handle, const char* layerName, uint32_t recordIndex, const char* fieldName, BagCompoundDataType* value);
BAG_EXTERNAL BagError bagGetCompoundLayerValueByIndex(BagHandle* handle, const char* layerName, uint32_t recordIndex, uint32_t fieldIndex, BagCompoundDataType* value);
BAG_EXTERNAL BagError bagGetCompoundLayerFieldIndex(BagHandle* handle, const char* layerName, const char* fieldName, uint32_t* fieldIndex);
BAG_EXTERNAL BagError bagGetCompoundLayerFieldName(BagHandle* handle, const char* layerName, uint32_t fieldIndex, const char** fieldName);
BAG_EXTERNAL BagError bagAddCompoundLayerRecord(BagHandle* handle, const char* layerName, const BagCompoundDataType* record, uint32_t numFields, uint32_t* recordIndex);
BAG_EXTERNAL BagError bagAddCompoundLayerRecords(BagHandle* handle, const char* layerName, const BagCompoundDataType** records, uint32_t numRecords, uint32_t numFields);
BAG_EXTERNAL BagError bagCompoundLayerSetValueByName(BagHandle* handle, const char* layerName, uint32_t recordIndex, const char* fieldName, const BagCompoundDataType* value);
BAG_EXTERNAL BagError bagCompoundLayerSetValueByIndex(BagHandle* handle, const char* layerName, uint32_t recordIndex, uint32_t fieldIndex, const BagCompoundDataType* value);


/* Variable Resolution */
BAG_EXTERNAL BagError bagCreateVRLayers(BagHandle* handle, bool makeNode);

/* Variable Resolution Metadata */
BAG_EXTERNAL BagError bagVRMetadataGetMinDimensions(BagHandle* handle, uint32_t* minX, uint32_t* minY);
BAG_EXTERNAL BagError bagVRMetadataGetMaxDimensions(BagHandle* handle, uint32_t* maxX, uint32_t* maxY);
BAG_EXTERNAL BagError bagVRMetadataGetMinResolution(BagHandle* handle, float* minX, float* minY);
BAG_EXTERNAL BagError bagVRMetadataGetMaxResolution(BagHandle* handle, float* maxX, float* maxY);
BAG_EXTERNAL BagError bagVRMetadataSetMinDimensions(BagHandle* handle, uint32_t minX, uint32_t minY);
BAG_EXTERNAL BagError bagVRMetadataSetMaxDimensions(BagHandle* handle, uint32_t maxX, uint32_t maxY);
BAG_EXTERNAL BagError bagVRMetadataSetMinResolution(BagHandle* handle, float minX, float minY);
BAG_EXTERNAL BagError bagVRMetadataSetMaxResolution(BagHandle* handle, float maxX, float maxY);

/* Variable Resolution Node */
BAG_EXTERNAL BagError bagVRNodeGetMinMaxHypStrength(BagHandle* handle, float* minHypStr, float* maxHypStr);
BAG_EXTERNAL BagError bagVRNodeGetMinMaxNumHypotheses(BagHandle* handle, uint32_t* minNumHyp, uint32_t* maxNumHyp);
BAG_EXTERNAL BagError bagVRNodeGetMinMaxNSamples(BagHandle* handle, uint32_t* minNSamples, uint32_t* maxNSamples);
BAG_EXTERNAL BagError bagVRNodeSetMinMaxHypStrength(BagHandle* handle, float minHypStr, float maxHypStr);
BAG_EXTERNAL BagError bagVRNodeSetMinMaxNumHypotheses(BagHandle* handle, uint32_t minNumHyp, uint32_t maxNumHyp);
BAG_EXTERNAL BagError bagVRNodeSetMinMaxNSamples(BagHandle* handle, uint32_t minNSamples, uint32_t maxNSamples);

/* Variable Resolution Refinement */
BAG_EXTERNAL BagError bagVRRefinementGetMinMaxDepth(BagHandle* handle, float* minDepth, float* maxDepth);
BAG_EXTERNAL BagError bagVRRefinementGetMinMaxUncertainty(BagHandle* handle, float* minUncert, float* maxUncert);
BAG_EXTERNAL BagError bagVRRefinementSetMinMaxDepth(BagHandle* handle, float minDepth, float maxDepth);
BAG_EXTERNAL BagError bagVRRefinementSetMinMaxUncertainty(BagHandle* handle, float minUncert, float maxUncert);

/* Variable Resolution Tracking List */
BAG_EXTERNAL BagError bagVRTrackingListLength(BagHandle* handle, uint32_t* numItems);
BAG_EXTERNAL BagError bagReadVRTrackingListNode(BagHandle* handle, uint32_t row, uint32_t col, BagVRTrackingItem** items, uint32_t* numItems);
BAG_EXTERNAL BagError bagReadVRTrackingListSubNode(BagHandle* handle, uint32_t row, uint32_t col, BagVRTrackingItem** items, uint32_t* numItems);
BAG_EXTERNAL BagError bagReadVRTrackingListCode(BagHandle* handle, uint8_t code, BagVRTrackingItem** items, uint32_t* numItems);
BAG_EXTERNAL BagError bagReadVRTrackingListSeries(BagHandle* handle, uint16_t series, BagVRTrackingItem** items, uint32_t* numItems);
BAG_EXTERNAL BagError bagWriteVRTrackingListItem(BagHandle* handle, BagVRTrackingItem* item);
BAG_EXTERNAL BagError bagSortVRTrackingListByNode(BagHandle* handle);
BAG_EXTERNAL BagError bagSortVRTrackingListBySubNode(BagHandle* handle);
BAG_EXTERNAL BagError bagSortVRTrackingListBySeries(BagHandle* handle);
BAG_EXTERNAL BagError bagSortVRTrackingListByCode(BagHandle* handle);


#endif // BAG_H

