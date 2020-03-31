#include "bag.h"
#include "bag_c_types.h"
#include "bag_compoundlayer.h"
#include "bag_dataset.h"
#include "bag_errors.h"
#include "bag_layer.h"
#include "bag_metadata.h"
#include "bag_private.h"
#include "bag_simplelayer.h"
#include "bag_surfacecorrections.h"
#include "bag_surfacecorrectionsdescriptor.h"
#include "bag_trackinglist.h"
#include "bag_valuetable.h"
#include "bag_vrmetadata.h"
#include "bag_vrmetadatadescriptor.h"
#include "bag_vrnode.h"
#include "bag_vrnodedescriptor.h"
#include "bag_vrrefinements.h"
#include "bag_vrrefinementsdescriptor.h"
#include "bag_vrtrackinglist.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)  // std classes do not have DLL-interface when exporting
#endif

#include <H5Cpp.h>
#include <algorithm>
#include <iterator>
#include <numeric>
#include <sstream>
#include <string>

#ifdef _MSC_VER
#pragma warning(pop)
#endif


// how to use a function from C
#if 0
    BagHandle* bagHandle = NULL;
    BagError err = bagFileOpen(&bagHandle, ...);
#endif

// how to populate the out_handle
#if 0
void foo(caris_raster** out_raster, ...)
    CS_THROW_ASSERT(out_raster);
    *out_raster = nullptr;
    //..
    caris_raster * raster = new caris_raster();
    //..
    *out_raster = raster;
#endif

namespace {

BagCompoundDataType getValue(
    const BAG::CompoundDataType& field)
{
    BagCompoundDataType result;

    result.type = field.getType();

    switch(result.type)
    {
    case DT_FLOAT32:
        result.data.f = field.asFloat();
        break;
    case DT_UINT32:
        result.data.ui32 = field.asUInt32();
        break;
    case DT_BOOLEAN:
        result.data.b = field.asBool();
        break;
    case DT_STRING:  // Copy the string as it will go out of scope.
    {
        const char* const value = field.asString().c_str();
        result.data.c = new char[strlen(value) + 1];
        memcpy(result.data.c, value, strlen(value) + 1);
        break;
    }
    default:
        result.type = DT_UNKNOWN_DATA_TYPE;
        break;
    }

    return result;
}

BAG::CompoundDataType getValue(
    const BagCompoundDataType& field)
{
    switch (field.type)
    {
    case DT_FLOAT32:
        return BAG::CompoundDataType{field.data.f};
    case DT_UINT32:
        return BAG::CompoundDataType{field.data.ui32};
    case DT_BOOLEAN:
        return BAG::CompoundDataType{field.data.b};
    case DT_STRING:
    {
        const char* value = field.data.c;
        return BAG::CompoundDataType{std::string{value}};
    }
    default:
        return {};
    }
}

}

BagError bagFileOpen(
    BagHandle** handle,
    BAG_OPEN_MODE accessMode,
    const char* fileName)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!fileName)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    auto pHandle = std::make_unique<BagHandle>();

    pHandle->dataset = BAG::Dataset::open(std::string{fileName}, accessMode);

    *handle = pHandle.release();

    return BAG_SUCCESS;
}

BagError bagFileClose(
    BagHandle* handle)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

#ifndef NDEBUG
    if (handle->dataset.use_count() > 1)
        return 9900; // More than one reference to the dataset still.
#endif

    handle->dataset.reset();
    delete handle;

    return BAG_SUCCESS;
}

BagError bagCreateFromFile(
    BagHandle** handle,
    const char* fileName,
    const char* metaDataFile)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!fileName || !metaDataFile)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    BAG::Metadata metadata;
    metadata.loadFromFile(std::string{metaDataFile});

    //TODO Where do these values come from?
    // Thinking parameters to this function.
    constexpr uint64_t chunkSize = 100;
    constexpr int compressionLevel = 6;

    auto pHandle = std::make_unique<BagHandle>();

    pHandle->dataset = BAG::Dataset::create(std::string{fileName},
        std::move(metadata), chunkSize, compressionLevel);

    *handle = pHandle.release();

    return BAG_SUCCESS;
}

BagError bagCreateFromBuffer(
    BagHandle** handle,
    const char* fileName,
    uint8_t* metaDataBuffer,
    uint32_t metaDataBufferSize)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!fileName || !metaDataBuffer)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    BAG::Metadata metadata;
    metadata.loadFromBuffer(
        std::string{reinterpret_cast<char*>(metaDataBuffer), metaDataBufferSize});

    //TODO Where do these values come from?
    // Thinking parameters to this function.
    constexpr uint64_t chunkSize = 100;
    constexpr int compressionLevel = 6;

    auto pHandle = std::make_unique<BagHandle>();

    try
    {
        pHandle->dataset = BAG::Dataset::create(std::string{fileName},
            std::move(metadata), chunkSize, compressionLevel);
    }
    catch(const std::exception& /*e*/)
    {
        return BAG_HDF_CREATE_DATASET_FAILURE;
    }

    *handle =  pHandle.release();

    return BAG_SUCCESS;
}

BagError bagCreateLayer(
    BagHandle* handle,
    BAG_LAYER_TYPE type)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    //TODO Where do these values come from?
    // Thinking parameters to this function.
    constexpr uint64_t chunkSize = 100;
    constexpr int compressionLevel = 6;

    try
    {
        handle->dataset->createSimpleLayer(type, chunkSize, compressionLevel);
    }
    catch(const std::exception& /*e*/)
    {
        return BAG_HDF_CREATE_DATASET_FAILURE;
    }

    return BAG_SUCCESS;
}

BagError bagGetGridDimensions(
    BagHandle* handle,
    uint32_t* rows,
    uint32_t* cols)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!rows || !cols)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    std::tie(*rows, *cols) = handle->dataset->getDescriptor().getDims();

    return BAG_SUCCESS;
}

BagError bagGetResolution(
    BagHandle* handle,
    double* rowResolution,
    double* columnResolution)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!rowResolution || !columnResolution)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto& metadata = handle->dataset->getMetadata();
    metadata;

    *rowResolution = metadata.rowResolution();
    *columnResolution = metadata.columnResolution();

    return BAG_SUCCESS;
}

BagError bagGetGeoCover(
    BagHandle* handle,
    double* llx,
    double* lly,
    double* urx,
    double* ury)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!llx || !lly || !urx || !ury)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto& metadata = handle->dataset->getMetadata();

    *llx = metadata.llCornerX();
    *lly = metadata.llCornerY();
    *urx = metadata.urCornerX();
    *ury = metadata.urCornerY();

    return BAG_SUCCESS;
}

BagError bagSetMinMaxSimple(
    BagHandle* handle,
    BAG_LAYER_TYPE type,
    float minValue,
    float maxValue)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!minValue || !maxValue)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    auto* layer = handle->dataset->getSimpleLayer(type);
    if (!layer)
        return 9997;  // layer type not found

    layer->getDescriptor().setMinMax(minValue, maxValue);

    return BAG_SUCCESS;
}

BagError bagGetNumLayers(
    BagHandle* handle,
    uint32_t* numLayers)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!numLayers)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    *numLayers = static_cast<uint32_t>(handle->dataset->getLayers().size());

    return BAG_SUCCESS;
}

bool bagContainsLayer(
    BagHandle* handle,
    BAG_LAYER_TYPE type,
    const char* layerName)
{
    if (!handle)
        return false;

    return handle->dataset->getLayer(type, layerName) != nullptr;
}

BagError bagRead(
    BagHandle* handle,
    uint32_t start_row,
    uint32_t start_col,
    uint32_t end_row,
    uint32_t end_col,
    BAG_LAYER_TYPE type,
    const char* layerName,
    uint8_t** data,
    double* x,
    double* y)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!data)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    if (!x || !y)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    if (type == Compound && !layerName)
        return BAG_COMPOUND_LAYER_NAME_MISSING;

    const auto* layer = handle->dataset->getLayer(type, layerName);
    if (!layer)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    try
    {
        auto buffer = layer->read(start_row, start_col, end_row, end_col);
        *data = buffer->release();

        // Get the position of the node.
        std::tie(*x, *y) = handle->dataset->gridToGeo(start_row, start_col);
    }
    catch(const std::exception& /*e*/)
    {
        return BAG_HDF_READ_FAILURE;
    }

    return BAG_SUCCESS;
}

BagError bagWrite(
    BagHandle* handle,
    uint32_t start_row,
    uint32_t start_col,
    uint32_t end_row,
    uint32_t end_col,
    BAG_LAYER_TYPE type,
    const char* layerName,
    const uint8_t* data)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!data)
        return BAG_SUCCESS;  // nothing to write

    if (type == Compound && !layerName)
        return BAG_COMPOUND_LAYER_NAME_MISSING;

    auto* layer = handle->dataset->getLayer(type, layerName);
    if (!layer)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    try
    {
        layer->write(start_row, start_col, end_row, end_col, data);
    }
    catch(const std::exception& /*e*/)
    {
        return BAG_HDF_WRITE_FAILURE;
    }

    return BAG_SUCCESS;
}

/****************************************************************************************/
/*! \brief bagGetErrorString
 *
 * Description:
 *     This function provides a short text description for the last error that
 *     occurred on the BAG specified by bagHandle. Memory for the text string
 *     is maintained within the library and calling applications do not free this memory.
 *
 * \param    code     Error string will be assigned based on the \a bagError argument.
 * \param  **error    The address of a string pointer which will be updated
 *                    to point to a short text string describing the error condition.
 *
 * \return            On success the function returns \a BAG_SUCCESS You can always
 *           call this function again to see what the error was from the results of the
 *           original call to \a bagGetErrorString.
 *
 ****************************************************************************************/
BagError bagGetErrorString(
    BagError code,
    uint8_t** error)
{
    constexpr int MAX_STR = 255;
    constexpr int RANK = 2;

    static char str[MAX_STR];

    str[0] = '\0';

    if (!error)
      return BAG_INVALID_FUNCTION_ARGUMENT;

    *error = reinterpret_cast<uint8_t*>(str);

    switch (code)
    {
    case BAG_SUCCESS:
        strncpy(str, "Bag returned a successful completion", MAX_STR-1);
        break;
    case BAG_BAD_FILE_IO_OPERATION:
        strncpy(str, "Bad status with file IO operation", MAX_STR-1);
        break;
    case BAG_NO_FILE_FOUND:
        strncpy(str, "File not found", MAX_STR-1);
        break;
    case BAG_NO_ACCESS_PERMISSION:
        strncpy(str, "Failed to access Bag because of permissions", MAX_STR-1);
        break;
    case BAG_MEMORY_ALLOCATION_FAILED:
        strncpy(str, "Memory allocation failed", MAX_STR-1);
        break;
    case BAG_INVALID_BAG_HANDLE:
        strncpy(str, "Invalid or NULL bagHandle", MAX_STR-1);
        break;
    case BAG_INVALID_FUNCTION_ARGUMENT:
        strncpy(str, "Invalid function argument or illegal value passed to Bag", MAX_STR-1);
        break;
    case BAG_METADTA_NO_HOME:
        strncpy(str, "The BAG_HOME environment variable must be set to the configdata directory of the openns distribution", MAX_STR-1);
        break;
    case BAG_METADTA_SCHEMA_FILE_MISSING:
        strncpy(str, "Metadata schema file(s) (*.xsd) was missing, they should be within configdata under smXML or ISO19139/bag. Check BAG_HOME path", MAX_STR-1);
        break;
    case BAG_METADTA_PARSE_MEM_EXCEPTION:
        strncpy(str, "Metadata exceeded memory limit during parsing", MAX_STR-1);
        break;
    case BAG_METADTA_PARSE_EXCEPTION:
        strncpy(str, "Metadata parsing encountered an fatal exception", MAX_STR-1);
        break;
    case BAG_METADTA_PARSE_DOM_EXCEPTION:
        strncpy(str, "Metadata parsing encountered a DOM (document object model) exception, impossible to complete operation", MAX_STR-1);
        break;
    case BAG_METADTA_PARSE_UNK_EXCEPTION:
        strncpy(str, "Metadata parsing encountered an unknown error", MAX_STR-1);
        break;
    case BAG_METADTA_PARSE_FAILED:
        strncpy(str, "Metadata parsing failed, unable to parse input file", MAX_STR-1);
        break;
    case BAG_METADTA_PARSE_FAILED_MEM:
        strncpy(str, "Metadata parsing failed, unable to parse specified input buffer memory overflow", MAX_STR-1);
        break;
    case BAG_METADTA_VALIDATE_FAILED:
        strncpy(str, "Metadata XML validation failed", MAX_STR-1);
        break;
    case BAG_METADTA_INVALID_HANDLE:
        strncpy(str, "Metadata Invalid (NULL) bagHandle supplied to an accessor method", MAX_STR-1);
        break;
    case BAG_METADTA_INIT_FAILED:
        strncpy(str, "Metadata Initialization of the low level XML support system failed", MAX_STR-1);
        break;
    case BAG_METADTA_NO_PROJECTION_INFO:
        strncpy(str, "Metadata No projection information was found in the XML supplied", MAX_STR-1);
        break;
    case BAG_METADTA_INSUFFICIENT_BUFFER:
        strncpy(str, "Metadata supplied buffer is not large enough to hold the extracted contents from XML", MAX_STR-1);
        break;
    case BAG_METADTA_UNCRT_MISSING:
        strncpy(str, "Metadata 'uncertaintyType' information is missing from the XML structure", MAX_STR-1);
        break;
    case BAG_METADTA_INCOMPLETE_COVER:
        strncpy(str, "Metadata One or more elements of the requested coverage are missing from the XML file", MAX_STR-1);
        break;
    case BAG_METADTA_INVLID_DIMENSIONS:
        sprintf(str, "Metadata The number of dimensions is incorrect (not equal to %d)", RANK);
        break;
    case BAG_METADTA_BUFFER_EXCEEDED:
        strncpy(str, "Metadata supplied buffer is too large to be stored in the internal array", MAX_STR-1);
        break;
	case BAG_METADTA_DPTHCORR_MISSING:
		strncpy(str, "The 'depthCorrectionType' information is missing from the XML structure", MAX_STR-1);
		break;
    case BAG_METADTA_RESOLUTION_MISSING:
        strncpy(str, "Metadata resolution information is missing from the XML structure", MAX_STR-1);
        break;
    case BAG_METADTA_INVALID_PROJECTION:
        strncpy(str, "Metadata unsupported projection being used", MAX_STR-1);
        break;
    case BAG_METADTA_INVALID_DATUM:
        strncpy(str, "Metadata unsupported datum being used", MAX_STR-1);
        break;
    case BAG_METADTA_INVALID_HREF:
        strncpy(str, "Metadata horizontal reference system is invalid", MAX_STR-1);
        break;
    case BAG_METADTA_INVALID_VREF:
        strncpy(str, "Metadata vertical reference system is invalid", MAX_STR-1);
        break;
	case BAG_METADTA_SCHEMA_SETUP_FAILED:
		strncpy(str, "Failed to setup the xml schema", MAX_STR-1);
        break;
	case BAG_METADTA_SCHEMA_VALIDATION_SETUP_FAILED:
		strncpy(str, "Failed to setup the xml schema validation", MAX_STR-1);
        break;
	case BAG_METADTA_EMPTY_DOCUMENT:
		strncpy(str, "The metadata document is emtpy", MAX_STR-1);
        break;
	case BAG_METADTA_MISSING_MANDATORY_ITEM:
		strncpy(str, "The metadata is missing a mandatory item", MAX_STR-1);
        break;
    case BAG_METADTA_NOT_INITIALIZED:
        strncpy(str, "The metadata has not been initialized correctly", MAX_STR-1);
        break;
    case BAG_NOT_HDF5_FILE:
        strncpy(str, "HDF Bag is not an HDF5 File", MAX_STR-1);
        break;
    case BAG_HDF_RANK_INCOMPATIBLE:
        sprintf(str, "HDF Bag's rank is incompatible with expected Rank of the Datasets: %d", RANK);
        break;
    case BAG_HDF_TYPE_NOT_FOUND:
        strncpy(str, "HDF Bag surface Datatype parameter not available", MAX_STR-1);
        break;
    case BAG_HDF_DATASPACE_CORRUPTED:
        strncpy(str, "HDF Dataspace for a bag surface is corrupted or could not be read", MAX_STR-1);
        break;
    case BAG_HDF_ACCESS_EXTENTS_ERROR:
        strncpy(str, "HDF Failure in request for access outside the extents of a bag surface's Dataset", MAX_STR-1);
        break;
    case BAG_HDF_CANNOT_WRITE_NULL_DATA:
        strncpy(str, "HDF Cannot write NULL or uninitialized data to Dataset", MAX_STR-1);
        break;
    case BAG_HDF_INTERNAL_ERROR:
        strncpy(str, "HDF There was an internal HDF error detected", MAX_STR-1);
        break;
    case BAG_HDF_CREATE_FILE_FAILURE:
        strncpy(str, "HDF Unable to create new HDF Bag File", MAX_STR-1);
        break;
    case BAG_HDF_CREATE_DATASPACE_FAILURE:
        strncpy(str, "HDF Unable to create the Dataspace", MAX_STR-1);
        break;
    case BAG_HDF_CREATE_PROPERTY_CLASS_FAILURE:
        strncpy(str, "HDF Unable to create the Property class", MAX_STR-1);
        break;
    case BAG_HDF_SET_PROPERTY_FAILURE:
        strncpy(str, "HDF Unable to set value of Property class", MAX_STR-1);
        break;
    case BAG_HDF_TYPE_COPY_FAILURE:
        strncpy(str, "HDF Failed to copy Datatype parameter for Dataset access", MAX_STR-1);
        break;
    case BAG_HDF_CREATE_DATASET_FAILURE:
        strncpy(str, "HDF Unable to create the Dataset", MAX_STR-1);
        break;
    case BAG_HDF_DATASET_EXTEND_FAILURE:
        strncpy(str, "HDF Cannot extend Dataset extents", MAX_STR-1);
        break;
    case BAG_HDF_CREATE_ATTRIBUTE_FAILURE:
        strncpy(str, "HDF Unable to create Attribute", MAX_STR-1);
        break;
    case BAG_HDF_CREATE_GROUP_FAILURE:
        strncpy(str, "HDF Unable to create Group", MAX_STR-1);
        break;
    case BAG_HDF_WRITE_FAILURE:
        strncpy(str, "HDF Failure writing to Dataset", MAX_STR-1);
        break;
    case BAG_HDF_READ_FAILURE:
        strncpy(str, "HDF Failure reading from Dataset", MAX_STR-1);
        break;
    case BAG_HDF_GROUP_CLOSE_FAILURE:
        strncpy(str, "HDF Failure closing Group", MAX_STR-1);
        break;
    case BAG_HDF_FILE_CLOSE_FAILURE:
        strncpy(str, "HDF Failure closing File", MAX_STR-1);
        break;
    case BAG_HDF_FILE_OPEN_FAILURE:
        strncpy(str, "HDF Unable to open File", MAX_STR-1);
        break;
    case BAG_HDF_GROUP_OPEN_FAILURE:
        strncpy(str, "HDF Unable to open Group", MAX_STR-1);
        break;
    case BAG_HDF_ATTRIBUTE_OPEN_FAILURE:
        strncpy(str, "HDF Unable to open Attribute", MAX_STR-1);
        break;
    case BAG_HDF_ATTRIBUTE_CLOSE_FAILURE:
        strncpy(str, "HDF Failure closing Attribute", MAX_STR-1);
        break;
    case BAG_HDF_DATASET_CLOSE_FAILURE:
        strncpy(str, "HDF Failure closing Dataset", MAX_STR-1);
        break;
    case BAG_HDF_DATASET_OPEN_FAILURE:
        strncpy(str, "HDF Unable to open Dataset", MAX_STR-1);
        break;
    case BAG_HDF_TYPE_CREATE_FAILURE:
        strncpy(str, "HDF Unable to create Datatype", MAX_STR-1);
        break;
    case BAG_HDF_INVALID_COMPRESSION_LEVEL:
        strncpy(str, "HDF compression level not in acceptable range of 0 to 9", MAX_STR-1);
        break;
    case BAG_CRYPTO_SIGNATURE_OK:
        strncpy(str, "Crypto Signature is OK", MAX_STR-1);
        break;
    case BAG_CRYPTO_NO_SIGNATURE_FOUND:
        strncpy(str, "Crypto No signature was found", MAX_STR-1);
        break;
    case BAG_CRYPTO_BAD_SIGNATURE_BLOCK:
        strncpy(str, "Crypto Bad signature block", MAX_STR-1);
        break;
    case BAG_CRYPTO_BAD_KEY:
        strncpy(str, "Crypto Bad Key", MAX_STR-1);
        break;
    case BAG_CRYPTO_WRONG_KEY:
        strncpy(str, "Crypto Wrong key", MAX_STR-1);
        break;
    case BAG_CRYPTO_GENERAL_ERROR:
        strncpy(str, "Crypto General error was detected", MAX_STR-1);
        break;
    case BAG_CRYPTO_INTERNAL_ERROR:
        strncpy(str, "Crypto Internal error was detected", MAX_STR-1);
        break;
    case BAG_INVALID_ERROR_CODE:
    default:
        strncpy(str, "An undefined bagError code was encountered", MAX_STR-1);
        return BAG_INVALID_ERROR_CODE;
    }
    return BAG_SUCCESS;
}

BagError bagComputePostion(
    BagHandle* handle,
    uint32_t row,
    uint32_t col,
    double* x,
    double* y)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!x || !y)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    std::tie(*x, *y) = handle->dataset->gridToGeo(row, col);

    return BAG_SUCCESS;
}

BagError bagComputeIndex(
    BagHandle* handle,
    double x,
    double y,
    uint32_t* row,
    uint32_t* col)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!row || !col)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    std::tie(*row, *col) = handle->dataset->geoToGrid(x, y);

    return BAG_SUCCESS;
}

uint8_t* bagAllocateBuffer(
    BagHandle* handle,
    uint32_t start_row,
    uint32_t start_col,
    uint32_t end_row,
    uint32_t end_col,
    BAG_LAYER_TYPE type,
    const char* layerName,
    BagError* bagError)
{
    if (!handle)
        return {};

    if (!bagError)
        return {};

    if (type == Compound && !layerName)
    {
        *bagError = BAG_INVALID_FUNCTION_ARGUMENT;
        return {};
    }

    const auto* layer = handle->dataset->getLayer(type, layerName);
    if (!layer)
    {
        *bagError = BAG_HDF_DATASET_OPEN_FAILURE;
        return {};
    }

    const int8_t elementSize = layer->getDescriptor().getElementSize();

    const auto numRows = (end_row - start_row) + 1;
    const auto numCols = (end_col - start_col) + 1;

    *bagError = BAG_SUCCESS;

    return new uint8_t[numRows * numCols * elementSize];
}

uint8_t* bagAllocate(uint32_t numBytes)
{
    return new uint8_t[numBytes];
}

BagError bagFree(uint8_t* buffer)
{
    delete[] buffer;

    return BAG_SUCCESS;
}

// Surface Corrections
BagError bagReadCorrectorVerticalDatum(
    BagHandle* handle,
    uint32_t type,  // corrector type to read; related to that 10 limit; valid values are 1-10
    uint8_t* datum)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (type < 1 || type > BAG_SURFACE_CORRECTOR_LIMIT)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    if (!datum)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto* layer = handle->dataset->getSurfaceCorrections();
    if (!layer)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    const auto& descriptor =
        dynamic_cast<const BAG::SurfaceCorrectionsDescriptor&>(
            layer->getDescriptor());
    const auto& allVerticalDatums = descriptor.getVerticalDatums();

    if (!allVerticalDatums.empty())
    {
        // Iterate over the datums, looking for the one specified by type-1.
        std::istringstream iss{allVerticalDatums};
        std::string item;
        size_t index = 1;

        while (index <= BAG_SURFACE_CORRECTOR_LIMIT && std::getline(iss, item, ','))
        {
            if (index == type)
            {
                strcpy(reinterpret_cast<char*>(datum), item.c_str());
                return BAG_SUCCESS;
            }

            ++index;
        }

        return BAG_HDF_ATTRIBUTE_OPEN_FAILURE;
    }

    return BAG_SUCCESS;
}

BagError bagWriteCorrectorVerticalDatum(
    BagHandle* handle,
    uint32_t type,
    const uint8_t* inDatum)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (type < 1 || type > BAG_SURFACE_CORRECTOR_LIMIT)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    if (!inDatum)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    auto* layer = handle->dataset->getSurfaceCorrections();
    if (!layer)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    auto& descriptor = dynamic_cast<BAG::SurfaceCorrectionsDescriptor&>(
        layer->getDescriptor());

    // Set/replace the specified datum.
    std::vector<std::string> datums;
    datums.reserve(BAG_SURFACE_CORRECTOR_LIMIT);
    std::istringstream iss{descriptor.getVerticalDatums()};

    while (iss)
    {
        std::string datum;
        std::getline(iss, datum, ',');
        datums.emplace_back(std::move(datum));
    }

    datums[type-1] = reinterpret_cast<const char*>(inDatum);

    std::string joinedDatums = std::accumulate(cbegin(datums), cend(datums),
        std::string{}, [](std::string& dest, const std::string& datum)
        {
            return dest.empty() ? datum : dest + ',' + datum;
        });

    descriptor.setVerticalDatums(joinedDatums);

    // write out the attribute
    try
    {
        layer->writeAttributes();
    }
    catch(const std::exception& /*e*/)
    {
        return BAG_HDF_WRITE_ATTRIBUTE_FAILURE;
    }

    return BAG_SUCCESS;
}

BagError bagReadCorrectedDataset(
    BagHandle* handle,
    uint8_t corrector,
    BAG_LAYER_TYPE type,
    float** data)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!data)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto* corrections = handle->dataset->getSurfaceCorrections();
    if (!corrections)
        return BAG_SURFACE_CORRECTIONS_MISSING;

    const auto* layer = dynamic_cast<BAG::SimpleLayer*>(
        handle->dataset->getSimpleLayer(type));
    if (!layer)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    constexpr uint32_t rowStart = 0;
    constexpr uint32_t columnStart = 0;

    const auto& descriptor = handle->dataset->getDescriptor();

    uint32_t rowEnd = 0;
    uint32_t columnEnd = 0;
    std::tie(rowEnd, columnEnd) = descriptor.getDims();

    auto correctedData = corrections->readCorrected(rowStart, rowEnd,
        columnStart, columnEnd, corrector, *layer);

    *data = reinterpret_cast<float*>(correctedData->release());

    return BAG_SUCCESS;
}

BagError bagReadCorrectedRegion(
    BagHandle* handle,
    uint32_t rowStart,
    uint32_t rowEnd,
    uint32_t columnStart,
    uint32_t columnEnd,
    uint8_t corrector,
    BAG_LAYER_TYPE type,
    float** data)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!data)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto* corrections = handle->dataset->getSurfaceCorrections();
    if (!corrections)
        return BAG_SURFACE_CORRECTIONS_MISSING;

    const auto* layer = dynamic_cast<BAG::SimpleLayer*>(
        handle->dataset->getSimpleLayer(type));
    if (!layer)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    auto correctedData = corrections->readCorrected(rowStart, rowEnd,
        columnStart, columnEnd, corrector, *layer);

    *data = reinterpret_cast<float*>(correctedData->release());

    return BAG_SUCCESS;
}

BagError bagReadCorrectedRow(
    BagHandle* handle,
    uint32_t row,
    uint8_t corrector,
    BAG_LAYER_TYPE type,
    float** data)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!data)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto* corrections = handle->dataset->getSurfaceCorrections();
    if (!corrections)
        return BAG_SURFACE_CORRECTIONS_MISSING;

    const auto* layer = dynamic_cast<BAG::SimpleLayer*>(
        handle->dataset->getSimpleLayer(type));
    if (!layer)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    constexpr uint32_t columnStart = 0;

    const auto& descriptor = handle->dataset->getDescriptor();
    const auto columnEnd = std::get<1>(descriptor.getDims());

    auto correctedData = corrections->readCorrectedRow(row, columnStart,
        columnEnd, corrector, *layer);

    *data = reinterpret_cast<float*>(correctedData->release());

    return BAG_SUCCESS;
}

BagError bagReadCorrectedNode(
    BagHandle* handle,
    uint32_t row,
    uint32_t column,
    uint8_t corrector,
    BAG_LAYER_TYPE type,
    float** data)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!data)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto* corrections = handle->dataset->getSurfaceCorrections();
    if (!corrections)
        return BAG_SURFACE_CORRECTIONS_MISSING;

    const auto* layer = dynamic_cast<BAG::SimpleLayer*>(
        handle->dataset->getSimpleLayer(type));
    if (!layer)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    auto correctedData = corrections->readCorrectedRow(row, column,
        column, corrector, *layer);

    *data = reinterpret_cast<float*>(correctedData->release());

    return BAG_SUCCESS;
}

BagError bagGetNumSurfaceCorrectors(
    BagHandle* handle,
    uint32_t* num)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!num)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto* layer = handle->dataset->getSurfaceCorrections();
    if (!layer)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    auto& descriptor = dynamic_cast<const BAG::SurfaceCorrectionsDescriptor&>(
        layer->getDescriptor());

    *num = descriptor.getNumCorrectors();

    return BAG_SUCCESS;
}

BagError bagGetSurfaceCorrectionTopography(
    BagHandle* handle,
    uint8_t* type)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!type)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto* layer = handle->dataset->getSurfaceCorrections();
    if (!layer)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    auto& descriptor = dynamic_cast<const BAG::SurfaceCorrectionsDescriptor&>(
        layer->getDescriptor());

    *type = static_cast<uint8_t>(descriptor.getSurfaceType());

    return BAG_SUCCESS;
}

BagError bagCreateCorrectorDataset(
    BagHandle* handle,
    uint32_t numCorrectors,
    uint8_t type)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    //TODO where to get chunkSize & compressionLevel from?  elevation layer?
    const auto* elevationLayer = handle->dataset->getSimpleLayer(Elevation);
    if (!elevationLayer)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    const auto& descriptor = elevationLayer->getDescriptor();
    const auto chunkSize = descriptor.getChunkSize();
    const auto compressionLevel = descriptor.getCompressionLevel();

    handle->dataset->createSurfaceCorrections(
        static_cast<BAG_SURFACE_CORRECTION_TOPOGRAPHY>(type),
        static_cast<uint8_t>(numCorrectors), chunkSize, compressionLevel);

    return BAG_SUCCESS;
}

BagError bagWriteCorrectorDefinition(
    BagHandle* handle,
    BagVerticalCorrectorDef* def)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!def)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    auto* corrections = handle->dataset->getSurfaceCorrections();
    if (!corrections)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    auto* descriptor = dynamic_cast<BAG::SurfaceCorrectionsDescriptor*>(
        &corrections->getDescriptor());
    if (!descriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    descriptor->setOrigin(def->swCornerX, def->swCornerY)
        .setSpacing(def->nodeSpacingX, def->nodeSpacingY);

    try
    {
        corrections->writeAttributes();
    }
    catch(const std::exception& /*e*/)
    {
        return BAG_HDF_WRITE_ATTRIBUTE_FAILURE;
    }

    return BAG_SUCCESS;
}

BagError bagReadCorrectorDefinition(
    BagHandle* handle,
    BagVerticalCorrectorDef* def)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!def)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    auto* corrections = handle->dataset->getSurfaceCorrections();
    if (!corrections)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    auto* descriptor = dynamic_cast<BAG::SurfaceCorrectionsDescriptor*>(
        &corrections->getDescriptor());
    if (!descriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    std::tie(def->swCornerX, def->swCornerY) = descriptor->getOrigin();
    std::tie(def->nodeSpacingX, def->nodeSpacingY) = descriptor->getSpacing();

    return BAG_SUCCESS;
}

// Tracking List
BagError bagTrackingListLength(
    BagHandle* handle,
    uint32_t* len)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!len)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    *len = static_cast<uint32_t>(handle->dataset->getTrackingList().size());

    return BAG_SUCCESS;
}

BagError bagReadTrackingListNode(
    BagHandle* handle,
    uint32_t row,
    uint32_t col,
    BagTrackingItem** items,
    uint32_t* length)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!items || !length)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto& trackingList = handle->dataset->getTrackingList();

    using std::cbegin;  using std::cend;
    std::vector<BAG::TrackingItem> results;

    std::copy_if(cbegin(trackingList), cend(trackingList),
        std::back_inserter(results),
        [row, col](const BAG::TrackingItem& item) {
            return item.row == row && item.col == col;
        });

    *length = static_cast<uint32_t>(results.size());

    *items = new BAG::TrackingItem[*length];
    memcpy(*items, results.data(), *length);

    return BAG_SUCCESS;
}

BagError bagReadTrackingListCode(
    BagHandle* handle,
    uint8_t code,
    BagTrackingItem** items,
    uint32_t* length)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!items || !length)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto& trackingList = handle->dataset->getTrackingList();

    using std::cbegin;  using std::cend;
    std::vector<BAG::TrackingItem> results;

    std::copy_if(cbegin(trackingList), cend(trackingList),
        std::back_inserter(results),
        [code](const BAG::TrackingItem& item) {
            return item.track_code == code;
        });

    *length = static_cast<uint32_t>(results.size());

    *items = new BAG::TrackingItem[*length];
    memcpy(*items, results.data(), *length);

    return BAG_SUCCESS;
}

BagError bagReadTrackingListSeries(
    BagHandle* handle,
    uint16_t series,
    BagTrackingItem** items,
    uint32_t* length)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!items || !length)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto& trackingList = handle->dataset->getTrackingList();

    using std::cbegin;  using std::cend;
    std::vector<BAG::TrackingItem> results;

    std::copy_if(cbegin(trackingList), cend(trackingList),
        std::back_inserter(results),
        [series](const BAG::TrackingItem& item) {
            return item.list_series == series;
        });

    *length = static_cast<uint32_t>(results.size());

    *items = new BAG::TrackingItem[*length];
    memcpy(*items, results.data(), *length);

    return BAG_SUCCESS;
}

BagError bagWriteTrackingListItem(
    BagHandle* handle,
    BagTrackingItem* item)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!item)
        return BAG_SUCCESS;

    auto& trackingList = handle->dataset->getTrackingList();

    trackingList.emplace_back(*item);

    try
    {
        trackingList.write();
    }
    catch(const std::exception& /*e*/)
    {
        return BAG_HDF_WRITE_FAILURE;
    }

    return BAG_SUCCESS;
}

BagError bagSortTrackingListByNode(BagHandle* handle)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    auto& trackingList = handle->dataset->getTrackingList();

    using std::begin;  using std::end;

    std::sort(begin(trackingList), end(trackingList),
        [](const BAG::TrackingItem& lhs, const BAG::TrackingItem& rhs) {
            return (lhs.row >= rhs.row) && (lhs.col >= rhs.col);
        });

    try
    {
        trackingList.write();
    }
    catch(const std::exception& /*e*/)
    {
        return BAG_HDF_WRITE_FAILURE;
    }

    return BAG_SUCCESS;
}

BagError bagSortTrackingListBySeries(BagHandle* handle)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    auto& trackingList = handle->dataset->getTrackingList();

    using std::begin;  using std::end;

    std::sort(begin(trackingList), end(trackingList),
        [](const BAG::TrackingItem& lhs, const BAG::TrackingItem& rhs) {
            return lhs.list_series > rhs.list_series;
        });

    try
    {
        trackingList.write();
    }
    catch(const std::exception& /*e*/)
    {
        return BAG_HDF_WRITE_FAILURE;
    }

    return BAG_SUCCESS;
}

BagError bagSortTrackingListByCode(BagHandle* handle)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    auto& trackingList = handle->dataset->getTrackingList();

    using std::begin;  using std::end;

    std::sort(begin(trackingList), end(trackingList),
        [](const BAG::TrackingItem& lhs, const BAG::TrackingItem& rhs) {
            return lhs.track_code > rhs.track_code;
        });

    try
    {
        trackingList.write();
    }
    catch(const std::exception& /*e*/)
    {
        return BAG_HDF_WRITE_FAILURE;
    }

    return BAG_SUCCESS;
}

// Metadata
const BagMetadata* bagGetMetaData(BagHandle* handle)
{
    if (!handle)
        return nullptr;

    return &handle->dataset->getMetadata().getStruct();
}

BagError bagGetMinMaxSimple(
    BagHandle* handle,
    BAG_LAYER_TYPE type,
    float* minValue,
    float* maxValue)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!minValue || !maxValue)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto* layer = handle->dataset->getSimpleLayer(type);
    if (!layer)
        return 9997;  // layer type not found

    std::tie(*minValue, *maxValue) = layer->getDescriptor().getMinMax();

    return BAG_SUCCESS;
}

BagError bagSetHomeFolder(const char* metadataFolder)
{
    if (!metadataFolder)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    bagSetHomeFolder(metadataFolder);

    return BAG_SUCCESS;
}

// New Metadata (CompoundLayer)
BagError bagCreateCompoundLayer(
    BagHandle* handle,
    BAG_DATA_TYPE indexType,
    const char* layerName,
    const FieldDefinition* definition,
    uint32_t numDefinitions)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!layerName || !definition || numDefinitions < 1)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    //TODO where to get chunkSize & compressionLevel from?  elevation layer?
    const auto* elevationLayer = handle->dataset->getSimpleLayer(Elevation);
    if (!elevationLayer)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    const auto& descriptor = elevationLayer->getDescriptor();
    const auto chunkSize = descriptor.getChunkSize();
    const auto compressionLevel = descriptor.getCompressionLevel();

    // Convert the FieldDefinition* into a RecordDefinition.
    const BAG::RecordDefinition recordDef(definition, definition + numDefinitions);

    try
    {
        handle->dataset->createCompoundLayer(indexType, layerName, recordDef,
            chunkSize, compressionLevel);
    }
    catch(const std::exception& /*e*/)
    {
        return BAG_HDF_CREATE_DATASET_FAILURE;
    }

    return BAG_SUCCESS;
}

BagError bagGetCompoundLayerDefinition(
    BagHandle* handle,
    const char* layerName,
    FieldDefinition** definition,
    uint32_t* numDefinitions)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!definition || !numDefinitions)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto* compoundLayer = handle->dataset->getCompoundLayer(layerName);
    if (!compoundLayer)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    const auto& recordDef = compoundLayer->getValueTable().getDefinition();

    // Convert the RecordDefinition into a FieldDefinition*.
    *numDefinitions = static_cast<uint32_t>(recordDef.size());

    auto* pDef = *definition;
    pDef = new FieldDefinition[*numDefinitions];  // caller is responsible to clean this up.

    uint32_t index = 0;
    for (const auto& def : recordDef)
        pDef[index++] = def;

    return BAG_SUCCESS;
}

BagError bagGetCompoundLayerRecords(
    BagHandle* handle,
    const char* layerName,
    BagCompoundDataType*** records,
    uint32_t* numRecords,
    uint32_t* numFields)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!layerName || !records || !numRecords || !numFields)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto* compoundLayer = handle->dataset->getCompoundLayer(layerName);
    if (!compoundLayer)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    const auto& recs = compoundLayer->getValueTable().getRecords();
    if (recs.empty())
    {
        *numRecords = 0;
        *numFields = static_cast<uint32_t>(compoundLayer->getValueTable().getDefinition().size());

        *records = nullptr;

        return BAG_SUCCESS;
    }

    // Convert the Records into a BagCompoundDataType*.
    *numRecords = static_cast<uint32_t>(recs.size());
    *numFields = static_cast<uint32_t>(recs[0].size());

    // Allocate the records.
    auto** pRecords = new BagCompoundDataType*[*numRecords];
    size_t recordIndex = 0;

    for (const auto& rec : recs)
    {
        // Allocate the fields.
        auto pRecord = new BagCompoundDataType[*numFields];
        pRecords[recordIndex++] = pRecord;

        size_t fieldIndex = 0;

        for (const auto& field : rec)
        {
            // Copy the field
            auto& outField = pRecord[fieldIndex++];
            outField = getValue(field);
        }
    }

    *records = pRecords;

    return BAG_SUCCESS;
}

BagError bagGetCompoundLayerValueByName(
    BagHandle* handle,
    const char* layerName,
    uint32_t recordIndex,
    const char* fieldName,
    BagCompoundDataType* value)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!value)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto* compoundLayer = handle->dataset->getCompoundLayer(layerName);
    if (!compoundLayer)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    try
    {
        const auto& val = compoundLayer->getValueTable().getValue(recordIndex,
            fieldName);

        *value = getValue(val);
    }
    catch(const BAG::RecordNotFound& /*e*/)
    {
        return BAG_NEWMETADATA_RECORD_NOT_FOUND;
    }
    catch(const BAG::FieldNotFound& /*e*/)
    {
        return BAG_NEWMETADATA_FIELD_NOT_FOUND;
    }
    catch(const std::exception& /*e*/)
    {
        return BAG_NEWMETADATA_NO_VALUE_FOUND;
    }

    return BAG_SUCCESS;
}

BagError bagGetCompoundLayerValueByIndex(
    BagHandle* handle,
    const char* layerName,
    uint32_t recordIndex,
    uint32_t fieldIndex,
    BagCompoundDataType* value)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!value)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto* compoundLayer = handle->dataset->getCompoundLayer(layerName);
    if (!compoundLayer)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    try
    {
        const auto& val = compoundLayer->getValueTable().getValue(recordIndex,
            fieldIndex);

        *value = getValue(val);
    }
    catch(const BAG::RecordNotFound& /*e*/)
    {
        return BAG_NEWMETADATA_RECORD_NOT_FOUND;
    }
    catch(const BAG::FieldNotFound& /*e*/)
    {
        return BAG_NEWMETADATA_FIELD_NOT_FOUND;
    }
    catch(const std::exception& /*e*/)
    {
        return BAG_NEWMETADATA_NO_VALUE_FOUND;
    }

    return BAG_SUCCESS;
}

BagError bagGetCompoundLayerFieldIndex(
    BagHandle* handle,
    const char* layerName,
    const char* fieldName,
    uint32_t* fieldIndex)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!layerName || !fieldName || !fieldIndex)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto* compoundLayer = handle->dataset->getCompoundLayer(layerName);
    if (!compoundLayer)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    try
    {
        *fieldIndex = static_cast<uint32_t>(
            compoundLayer->getValueTable().getFieldIndex(fieldName));
    }
    catch(const BAG::FieldNotFound& /*e*/)
    {
        return BAG_NEWMETADATA_FIELD_NOT_FOUND;
    }

    return BAG_SUCCESS;
}

BagError bagGetCompoundLayerFieldName(
    BagHandle* handle,
    const char* layerName,
    uint32_t fieldIndex,
    const char** fieldName)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!layerName)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto* compoundLayer = handle->dataset->getCompoundLayer(layerName);
    if (!compoundLayer)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    try
    {
        *fieldName = compoundLayer->getValueTable().getFieldName(fieldIndex);
    }
    catch(const BAG::FieldNotFound& /*e*/)
    {
        return BAG_NEWMETADATA_FIELD_NOT_FOUND;
    }

    return BAG_SUCCESS;
}

BagError bagAddCompoundLayerRecord(
    BagHandle* handle,
    const char* layerName,
    const BagCompoundDataType* record,
    uint32_t numFields,
    uint32_t* fieldIndex)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!layerName || !record || !fieldIndex)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    auto* compoundLayer = handle->dataset->getCompoundLayer(layerName);
    if (!compoundLayer)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    // Convert BagCompoundDataType into a BAG::Record.
    BAG::Record rec(numFields);

    size_t index = 0;
    for (auto& field : rec)
        field = getValue(record[index++]);

    try
    {
        *fieldIndex = static_cast<uint32_t>(
            compoundLayer->getValueTable().addRecord(rec));
    }
    catch(const BAG::InvalidRecord& /*e*/)
    {
        return BAG_NEWMETADATA_INVALID_RECORD_DEFINITION;
    }
    catch(const BAG::InvalidRecordsIndex&)
    {
        return BAG_NEWMETADATA_RECORD_NOT_FOUND;
    }
    catch(const std::exception& /*e*/)
    {
        return BAG_HDF_READ_FAILURE;
    }

    return BAG_SUCCESS;
}

BagError bagAddCompoundLayerRecords(
    BagHandle* handle,
    const char* layerName,
    const BagCompoundDataType** records,
    uint32_t numRecords,
    uint32_t numFields)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!layerName || !records)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    auto* compoundLayer = handle->dataset->getCompoundLayer(layerName);
    if (!compoundLayer)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    // Convert BagCompoundDataType* into a BAG::Records.
    BAG::Records recs(numRecords);
    size_t recordIndex = 0;

    for (auto& rec : recs)
    {
        rec.resize(numFields);
        size_t fieldIndex = 0;

        for (auto& field : rec)
            field = getValue(records[recordIndex][fieldIndex++]);

        ++recordIndex;
    }

    try
    {
        compoundLayer->getValueTable().addRecords(recs);
    }
    catch(const BAG::InvalidRecord& /*e*/)
    {
        return BAG_NEWMETADATA_INVALID_RECORD_DEFINITION;
    }
    catch(const BAG::InvalidRecordsIndex&)
    {
        return BAG_NEWMETADATA_RECORD_NOT_FOUND;
    }
    catch(const std::exception& /*e*/)
    {
        return BAG_HDF_READ_FAILURE;
    }

    return BAG_SUCCESS;
}

BagError bagCompoundLayerSetValueByName(
    BagHandle* handle,
    const char* layerName,
    uint32_t recordIndex,
    const char* fieldName,
    const BagCompoundDataType* value)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!layerName || !fieldName || !value)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    auto* compoundLayer = handle->dataset->getCompoundLayer(layerName);
    if (!compoundLayer)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    // Convert BagCompoundDataType into a BAG::CompoundDataType.
    const auto val = getValue(*value);

    try
    {
        compoundLayer->getValueTable().setValue(recordIndex, fieldName, val);
    }
    catch(const BAG::RecordNotFound& /*e*/)
    {
        return BAG_NEWMETADATA_RECORD_NOT_FOUND;
    }
    catch(const BAG::InvalidRecord& /*e*/)
    {
        return BAG_NEWMETADATA_INVALID_RECORD_DEFINITION;
    }
    catch(const BAG::InvalidRecordsIndex&)
    {
        return BAG_NEWMETADATA_RECORD_NOT_FOUND;
    }
    catch(const std::exception& /*e*/)
    {
        return BAG_HDF_READ_FAILURE;
    }

    return BAG_SUCCESS;
}

BagError bagCompoundLayerSetValueByIndex(
    BagHandle* handle,
    const char* layerName,
    uint32_t recordIndex,
    uint32_t fieldIndex,
    const BagCompoundDataType* value)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!layerName || !value)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    auto* compoundLayer = handle->dataset->getCompoundLayer(layerName);
    if (!compoundLayer)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    // Convert BagCompoundDataType into a BAG::CompoundDataType.
    const auto val = getValue(*value);

    try
    {
        compoundLayer->getValueTable().setValue(recordIndex, fieldIndex, val);
    }
    catch(const BAG::RecordNotFound& /*e*/)
    {
        return BAG_NEWMETADATA_RECORD_NOT_FOUND;
    }
    catch(const BAG::InvalidRecord& /*e*/)
    {
        return BAG_NEWMETADATA_INVALID_RECORD_DEFINITION;
    }
    catch(const BAG::InvalidRecordsIndex&)
    {
        return BAG_NEWMETADATA_RECORD_NOT_FOUND;
    }
    catch(const std::exception& /*e*/)
    {
        return BAG_HDF_READ_FAILURE;
    }

    return BAG_SUCCESS;
}

// Variable Resolution
BagError bagCreateVRLayers(
    BagHandle* handle,
    bool makeNode)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    //TODO where to get chunkSize & compressionLevel from?  elevation layer?
    const auto* elevationLayer = handle->dataset->getSimpleLayer(Elevation);
    if (!elevationLayer)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    const auto& descriptor = elevationLayer->getDescriptor();
    const auto chunkSize = descriptor.getChunkSize();
    const auto compressionLevel = descriptor.getCompressionLevel();

    try
    {
        handle->dataset->createVR(chunkSize, compressionLevel, makeNode);
    }
    catch(const BAG::ReadOnlyError& /*e*/)
    {
        return BAG_NO_ACCESS_PERMISSION;
    }
    catch(const BAG::LayerExists& /*e*/)
    {
        return BAG_COMPOUND_LAYER_EXISTS;
    }

    return BAG_SUCCESS;
}

// Variable Resolution Metadata
BagError bagVRMetadataGetMinDimensions(
    BagHandle* handle,
    uint32_t* minX,
    uint32_t* minY)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!minX || !minY)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto* vrMetadata = handle->dataset->getVRMetadata();
    if (!vrMetadata)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    const auto* descriptor =
        dynamic_cast<const BAG::VRMetadataDescriptor*>(&vrMetadata->getDescriptor());
    if (!descriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    std::tie(*minX, *minY) = descriptor->getMinDimensions();

    return BAG_SUCCESS;
}

BagError bagVRMetadataGetMaxDimensions(
    BagHandle* handle,
    uint32_t* maxX,
    uint32_t* maxY)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!maxX || !maxY)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto* vrMetadata = handle->dataset->getVRMetadata();
    if (!vrMetadata)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    const auto* descriptor =
        dynamic_cast<const BAG::VRMetadataDescriptor*>(&vrMetadata->getDescriptor());
    if (!descriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    std::tie(*maxX, *maxY) = descriptor->getMinDimensions();

    return BAG_SUCCESS;
}

BagError bagVRMetadataGetMinResolution(
    BagHandle* handle,
    float* minX,
    float* minY)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!minX || !minY)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto* vrMetadata = handle->dataset->getVRMetadata();
    if (!vrMetadata)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    const auto* descriptor =
        dynamic_cast<const BAG::VRMetadataDescriptor*>(&vrMetadata->getDescriptor());
    if (!descriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    std::tie(*minX, *minY) = descriptor->getMinResolution();

    return BAG_SUCCESS;
};

BagError bagVRMetadataGetMaxResolution(
    BagHandle* handle,
    float* maxX,
    float* maxY)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!maxX || !maxY)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto* vrMetadata = handle->dataset->getVRMetadata();
    if (!vrMetadata)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    const auto* descriptor =
        dynamic_cast<const BAG::VRMetadataDescriptor*>(&vrMetadata->getDescriptor());
    if (!descriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    std::tie(*maxX, *maxY) = descriptor->getMaxResolution();

    return BAG_SUCCESS;
}

BagError bagVRMetadataSetMinDimensions(
    BagHandle* handle,
    uint32_t minX,
    uint32_t minY)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    auto* vrMetadata = handle->dataset->getVRMetadata();
    if (!vrMetadata)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    auto* descriptor =
        dynamic_cast<BAG::VRMetadataDescriptor*>(&vrMetadata->getDescriptor());
    if (!descriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    descriptor->setMinDimensions(minX, minY);

    try
    {
        vrMetadata->writeAttributes();
    }
    catch(const BAG::DatasetNotFound& /*e*/)
    {
        return BAG_INVALID_BAG_HANDLE;
    }
    catch(const BAG::InvalidWriteSize& /*e*/)
    {
        return BAG_HDF_WRITE_FAILURE;
    }
    catch(const BAG::InvalidBuffer& /*e*/)
    {
        return BAG_HDF_WRITE_ATTRIBUTE_FAILURE;
    }

    return BAG_SUCCESS;
}

BagError bagVRMetadataSetMaxDimensions(
    BagHandle* handle,
    uint32_t maxX,
    uint32_t maxY)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    auto* vrMetadata = handle->dataset->getVRMetadata();
    if (!vrMetadata)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    auto* descriptor =
        dynamic_cast<BAG::VRMetadataDescriptor*>(&vrMetadata->getDescriptor());
    if (!descriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    descriptor->setMaxDimensions(maxX, maxY);

    try
    {
        vrMetadata->writeAttributes();
    }
    catch(const BAG::DatasetNotFound& /*e*/)
    {
        return BAG_INVALID_BAG_HANDLE;
    }
    catch(const BAG::InvalidWriteSize& /*e*/)
    {
        return BAG_HDF_WRITE_FAILURE;
    }
    catch(const BAG::InvalidBuffer& /*e*/)
    {
        return BAG_HDF_WRITE_ATTRIBUTE_FAILURE;
    }

    return BAG_SUCCESS;
}

BagError bagVRMetadataSetMinResolution(
    BagHandle* handle,
    float minX,
    float minY)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    auto* vrMetadata = handle->dataset->getVRMetadata();
    if (!vrMetadata)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    auto* descriptor =
        dynamic_cast<BAG::VRMetadataDescriptor*>(&vrMetadata->getDescriptor());
    if (!descriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    descriptor->setMinResolution(minX, minY);

    try
    {
        vrMetadata->writeAttributes();
    }
    catch(const BAG::DatasetNotFound& /*e*/)
    {
        return BAG_INVALID_BAG_HANDLE;
    }
    catch(const BAG::InvalidWriteSize& /*e*/)
    {
        return BAG_HDF_WRITE_FAILURE;
    }
    catch(const BAG::InvalidBuffer& /*e*/)
    {
        return BAG_HDF_WRITE_ATTRIBUTE_FAILURE;
    }

    return BAG_SUCCESS;
}

BagError bagVRMetadataSetMaxResolution(
    BagHandle* handle,
    float maxX,
    float maxY)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    auto* vrMetadata = handle->dataset->getVRMetadata();
    if (!vrMetadata)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    auto* descriptor =
        dynamic_cast<BAG::VRMetadataDescriptor*>(&vrMetadata->getDescriptor());
    if (!descriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    descriptor->setMaxResolution(maxX, maxY);

    try
    {
        vrMetadata->writeAttributes();
    }
    catch(const BAG::DatasetNotFound& /*e*/)
    {
        return BAG_INVALID_BAG_HANDLE;
    }
    catch(const BAG::InvalidWriteSize& /*e*/)
    {
        return BAG_HDF_WRITE_FAILURE;
    }
    catch(const BAG::InvalidBuffer& /*e*/)
    {
        return BAG_HDF_WRITE_ATTRIBUTE_FAILURE;
    }

    return BAG_SUCCESS;
}

// Variable Resolution Node
BagError bagVRNodeGetMinMaxHypStrength(
    BagHandle* handle,
    float* minHypStr,
    float* maxHypStr)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!minHypStr || !maxHypStr)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    auto* vrNode = handle->dataset->getVRNode();
    if (!vrNode)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    auto* descriptor =
        dynamic_cast<BAG::VRNodeDescriptor*>(&vrNode->getDescriptor());
    if (!descriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    std::tie(*minHypStr, *maxHypStr) = descriptor->getMinMaxHypStrength();

    return BAG_SUCCESS;
}

BagError bagVRNodeGetMinMaxNumHypotheses(
    BagHandle* handle,
    uint32_t* minNumHyp,
    uint32_t* maxNumHyp)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!minNumHyp || !maxNumHyp)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    auto* vrNode = handle->dataset->getVRNode();
    if (!vrNode)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    auto* descriptor =
        dynamic_cast<BAG::VRNodeDescriptor*>(&vrNode->getDescriptor());
    if (!descriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    std::tie(*minNumHyp, *maxNumHyp) = descriptor->getMinMaxNumHypotheses();

    return BAG_SUCCESS;
}

BagError bagVRNodeGetMinMaxNSamples(
    BagHandle* handle,
    uint32_t* minNSamples,
    uint32_t* maxNSamples)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!minNSamples || !maxNSamples)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    auto* vrNode = handle->dataset->getVRNode();
    if (!vrNode)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    auto* descriptor =
        dynamic_cast<BAG::VRNodeDescriptor*>(&vrNode->getDescriptor());
    if (!descriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    std::tie(*minNSamples, *maxNSamples) = descriptor->getMinMaxNSamples();

    return BAG_SUCCESS;
}

BagError bagVRNodeSetMinMaxHypStrength(
    BagHandle* handle,
    float minHypStr,
    float maxHypStr)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    auto* vrNode = handle->dataset->getVRMetadata();
    if (!vrNode)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    auto* descriptor =
        dynamic_cast<BAG::VRNodeDescriptor*>(&vrNode->getDescriptor());
    if (!descriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    descriptor->setMinMaxHypStrength(minHypStr, maxHypStr);

    try
    {
        vrNode->writeAttributes();
    }
    catch(const BAG::DatasetNotFound& /*e*/)
    {
        return BAG_INVALID_BAG_HANDLE;
    }
    catch(const BAG::InvalidWriteSize& /*e*/)
    {
        return BAG_HDF_WRITE_FAILURE;
    }
    catch(const BAG::InvalidBuffer& /*e*/)
    {
        return BAG_HDF_WRITE_ATTRIBUTE_FAILURE;
    }

    return BAG_SUCCESS;
}

BagError bagVRNodeSetMinMaxNumHypotheses(
    BagHandle* handle,
    uint32_t minNumHyp,
    uint32_t maxNumHyp)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    auto* vrNode = handle->dataset->getVRMetadata();
    if (!vrNode)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    auto* descriptor =
        dynamic_cast<BAG::VRNodeDescriptor*>(&vrNode->getDescriptor());
    if (!descriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    descriptor->setMinMaxNumHypotheses(minNumHyp, maxNumHyp);

    try
    {
        vrNode->writeAttributes();
    }
    catch(const BAG::DatasetNotFound& /*e*/)
    {
        return BAG_INVALID_BAG_HANDLE;
    }
    catch(const BAG::InvalidWriteSize& /*e*/)
    {
        return BAG_HDF_WRITE_FAILURE;
    }
    catch(const BAG::InvalidBuffer& /*e*/)
    {
        return BAG_HDF_WRITE_ATTRIBUTE_FAILURE;
    }

    return BAG_SUCCESS;
}

BagError bagVRNodeSetMinMaxNSamples(
    BagHandle* handle,
    uint32_t minNSamples,
    uint32_t maxNSamples)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    auto* vrNode = handle->dataset->getVRMetadata();
    if (!vrNode)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    auto* descriptor =
        dynamic_cast<BAG::VRNodeDescriptor*>(&vrNode->getDescriptor());
    if (!descriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    descriptor->setMinMaxNSamples(minNSamples, maxNSamples);

    try
    {
        vrNode->writeAttributes();
    }
    catch(const BAG::DatasetNotFound& /*e*/)
    {
        return BAG_INVALID_BAG_HANDLE;
    }
    catch(const BAG::InvalidWriteSize& /*e*/)
    {
        return BAG_HDF_WRITE_FAILURE;
    }
    catch(const BAG::InvalidBuffer& /*e*/)
    {
        return BAG_HDF_WRITE_ATTRIBUTE_FAILURE;
    }

    return BAG_SUCCESS;
}

// Variable Resolution Refinement
BAG_EXTERNAL BagError bagVRRefinementGetMinMaxDepth(
    BagHandle* handle,
    float* minDepth,
    float* maxDepth)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!minDepth || !maxDepth)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    auto* vrRefinement = handle->dataset->getVRRefinements();
    if (!vrRefinement)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    auto* descriptor =
        dynamic_cast<BAG::VRRefinementsDescriptor*>(&vrRefinement->getDescriptor());
    if (!descriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    std::tie(*minDepth, *maxDepth) = descriptor->getMinMaxDepth();

    return BAG_SUCCESS;
}

BAG_EXTERNAL BagError bagVRRefinementGetMinMaxUncertainty(
    BagHandle* handle,
    float* minUncert,
    float* maxUncert)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!minUncert || !maxUncert)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    auto* vrRefinement = handle->dataset->getVRRefinements();
    if (!vrRefinement)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    auto* descriptor =
        dynamic_cast<BAG::VRRefinementsDescriptor*>(&vrRefinement->getDescriptor());
    if (!descriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    std::tie(*minUncert, *maxUncert) = descriptor->getMinMaxUncertainty();

    return BAG_SUCCESS;
}

BAG_EXTERNAL BagError bagVRRefinementSetMinMaxDepth(
    BagHandle* handle,
    float minDepth,
    float maxDepth)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    auto* vrRefinement = handle->dataset->getVRRefinements();
    if (!vrRefinement)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    auto* descriptor =
        dynamic_cast<BAG::VRRefinementsDescriptor*>(&vrRefinement->getDescriptor());
    if (!descriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    descriptor->setMinMaxDepth(minDepth, maxDepth);

    try
    {
        vrRefinement->writeAttributes();
    }
    catch(const BAG::DatasetNotFound& /*e*/)
    {
        return BAG_INVALID_BAG_HANDLE;
    }
    catch(const BAG::InvalidWriteSize& /*e*/)
    {
        return BAG_HDF_WRITE_FAILURE;
    }
    catch(const BAG::InvalidBuffer& /*e*/)
    {
        return BAG_HDF_WRITE_ATTRIBUTE_FAILURE;
    }

    return BAG_SUCCESS;
}

BAG_EXTERNAL BagError bagVRRefinementSetMinMaxUncertainty(
    BagHandle* handle,
    float minUncert,
    float maxUncert)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    auto* vrRefinement = handle->dataset->getVRRefinements();
    if (!vrRefinement)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    auto* descriptor =
        dynamic_cast<BAG::VRRefinementsDescriptor*>(&vrRefinement->getDescriptor());
    if (!descriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    descriptor->setMinMaxUncertainty(minUncert, maxUncert);

    try
    {
        vrRefinement->writeAttributes();
    }
    catch(const BAG::DatasetNotFound& /*e*/)
    {
        return BAG_INVALID_BAG_HANDLE;
    }
    catch(const BAG::InvalidWriteSize& /*e*/)
    {
        return BAG_HDF_WRITE_FAILURE;
    }
    catch(const BAG::InvalidBuffer& /*e*/)
    {
        return BAG_HDF_WRITE_ATTRIBUTE_FAILURE;
    }

    return BAG_SUCCESS;
}

// Variable Resolution Tracking List
BagError bagVRTrackingListLength(
    BagHandle* handle,
    uint32_t* len)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!len)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto* vrTrackingList = handle->dataset->getVRTrackingList();
    if (!vrTrackingList)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    *len = static_cast<uint32_t>(vrTrackingList->size());

    return BAG_SUCCESS;
}

BagError bagReadVRTrackingListNode(
    BagHandle* handle,
    uint32_t row,
    uint32_t col,
    BagVRTrackingItem** items,
    uint32_t* length)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!items || !length)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto* vrTrackingList = handle->dataset->getVRTrackingList();
    if (!vrTrackingList)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    using std::cbegin;  using std::cend;
    std::vector<BAG::VRTrackingItem> results;

    std::copy_if(cbegin(*vrTrackingList), cend(*vrTrackingList),
        std::back_inserter(results),
        [row, col](const BAG::VRTrackingItem& item) {
            return item.row == row && item.col == col;
        });

    *length = static_cast<uint32_t>(results.size());

    *items = new BAG::VRTrackingItem[*length];
    memcpy(*items, results.data(), *length);

    return BAG_SUCCESS;
}

BagError bagReadVRTrackingListSubNode(
    BagHandle* handle,
    uint32_t row,
    uint32_t col,
    BagVRTrackingItem** items,
    uint32_t* length)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!items || !length)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto* vrTrackingList = handle->dataset->getVRTrackingList();
    if (!vrTrackingList)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    using std::cbegin;  using std::cend;
    std::vector<BAG::VRTrackingItem> results;

    std::copy_if(cbegin(*vrTrackingList), cend(*vrTrackingList),
        std::back_inserter(results),
        [row, col](const BAG::VRTrackingItem& item) {
            return item.sub_row == row && item.sub_col == col;
        });

    *length = static_cast<uint32_t>(results.size());

    *items = new BAG::VRTrackingItem[*length];
    memcpy(*items, results.data(), *length);

    return BAG_SUCCESS;
}

BagError bagReadVRTrackingListCode(
    BagHandle* handle,
    uint8_t code,
    BagVRTrackingItem** items,
    uint32_t* length)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!items || !length)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto* vrTrackingList = handle->dataset->getVRTrackingList();
    if (!vrTrackingList)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    using std::cbegin;  using std::cend;
    std::vector<BAG::VRTrackingItem> results;

    std::copy_if(cbegin(*vrTrackingList), cend(*vrTrackingList),
        std::back_inserter(results),
        [code](const BAG::VRTrackingItem& item) {
            return item.track_code == code;
        });

    *length = static_cast<uint32_t>(results.size());

    *items = new BAG::VRTrackingItem[*length];
    memcpy(*items, results.data(), *length);

    return BAG_SUCCESS;
}

BagError bagReadVRTrackingListSeries(
    BagHandle* handle,
    uint16_t series,
    BagVRTrackingItem** items,
    uint32_t* length)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!items || !length)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto* vrTrackingList = handle->dataset->getVRTrackingList();
    if (!vrTrackingList)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    using std::cbegin;  using std::cend;
    std::vector<BAG::VRTrackingItem> results;

    std::copy_if(cbegin(*vrTrackingList), cend(*vrTrackingList),
        std::back_inserter(results),
        [series](const BAG::VRTrackingItem& item) {
            return item.list_series == series;
        });

    *length = static_cast<uint32_t>(results.size());

    *items = new BAG::VRTrackingItem[*length];
    memcpy(*items, results.data(), *length);

    return BAG_SUCCESS;
}

BagError bagWriteVRTrackingListItem(
    BagHandle* handle,
    BagVRTrackingItem* item)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!item)
        return BAG_SUCCESS;

    auto* vrTrackingList = handle->dataset->getVRTrackingList();
    if (!vrTrackingList)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    vrTrackingList->emplace_back(*item);

    try
    {
        vrTrackingList->write();
    }
    catch(const BAG::DatasetNotFound& /*e*/)
    {
        return BAG_INVALID_BAG_HANDLE;
    }
    catch(const BAG::InvalidWriteSize& /*e*/)
    {
        return BAG_HDF_WRITE_FAILURE;
    }
    catch(const BAG::InvalidBuffer& /*e*/)
    {
        return BAG_HDF_WRITE_ATTRIBUTE_FAILURE;
    }

    return BAG_SUCCESS;
}

BagError bagSortVRTrackingListByNode(
    BagHandle* handle)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    auto* vrTrackingList = handle->dataset->getVRTrackingList();
    if (!vrTrackingList)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    using std::begin;  using std::end;

    std::sort(begin(*vrTrackingList), end(*vrTrackingList),
        [](const BAG::VRTrackingItem& lhs, const BAG::VRTrackingItem& rhs) {
            return (lhs.row >= rhs.row) && (lhs.col >= rhs.col);
        });

    try
    {
        vrTrackingList->write();
    }
    catch(const BAG::DatasetNotFound& /*e*/)
    {
        return BAG_INVALID_BAG_HANDLE;
    }
    catch(const BAG::InvalidWriteSize& /*e*/)
    {
        return BAG_HDF_WRITE_FAILURE;
    }
    catch(const BAG::InvalidBuffer& /*e*/)
    {
        return BAG_HDF_WRITE_ATTRIBUTE_FAILURE;
    }

    return BAG_SUCCESS;
}

BagError bagSortVRTrackingListBySubNode(
    BagHandle* handle)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    auto* vrTrackingList = handle->dataset->getVRTrackingList();
    if (!vrTrackingList)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    using std::begin;  using std::end;

    std::sort(begin(*vrTrackingList), end(*vrTrackingList),
        [](const BAG::VRTrackingItem& lhs, const BAG::VRTrackingItem& rhs) {
            return (lhs.sub_row >= rhs.sub_row) && (lhs.sub_col >= rhs.sub_col);
        });

    try
    {
        vrTrackingList->write();
    }
    catch(const BAG::DatasetNotFound& /*e*/)
    {
        return BAG_INVALID_BAG_HANDLE;
    }
    catch(const BAG::InvalidWriteSize& /*e*/)
    {
        return BAG_HDF_WRITE_FAILURE;
    }
    catch(const BAG::InvalidBuffer& /*e*/)
    {
        return BAG_HDF_WRITE_ATTRIBUTE_FAILURE;
    }

    return BAG_SUCCESS;
}

BagError bagSortVRTrackingListBySeries(
    BagHandle* handle)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    auto* vrTrackingList = handle->dataset->getVRTrackingList();
    if (!vrTrackingList)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    using std::begin;  using std::end;

    std::sort(begin(*vrTrackingList), end(*vrTrackingList),
        [](const BAG::VRTrackingItem& lhs, const BAG::VRTrackingItem& rhs) {
            return lhs.list_series > rhs.list_series;
        });

    try
    {
        vrTrackingList->write();
    }
    catch(const BAG::DatasetNotFound& /*e*/)
    {
        return BAG_INVALID_BAG_HANDLE;
    }
    catch(const BAG::InvalidWriteSize& /*e*/)
    {
        return BAG_HDF_WRITE_FAILURE;
    }
    catch(const BAG::InvalidBuffer& /*e*/)
    {
        return BAG_HDF_WRITE_ATTRIBUTE_FAILURE;
    }

    return BAG_SUCCESS;
}

BagError bagSortVRTrackingListByCode(
    BagHandle* handle)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    auto* vrTrackingList = handle->dataset->getVRTrackingList();
    if (!vrTrackingList)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    using std::begin;  using std::end;

    std::sort(begin(*vrTrackingList), end(*vrTrackingList),
        [](const BAG::VRTrackingItem& lhs, const BAG::VRTrackingItem& rhs) {
            return lhs.track_code > rhs.track_code;
        });

    try
    {
        vrTrackingList->write();
    }
    catch(const BAG::DatasetNotFound& /*e*/)
    {
        return BAG_INVALID_BAG_HANDLE;
    }
    catch(const BAG::InvalidWriteSize& /*e*/)
    {
        return BAG_HDF_WRITE_FAILURE;
    }
    catch(const BAG::InvalidBuffer& /*e*/)
    {
        return BAG_HDF_WRITE_ATTRIBUTE_FAILURE;
    }

    return BAG_SUCCESS;
}

