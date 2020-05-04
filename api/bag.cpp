#include "bag.h"
#include "bag_c_types.h"
#include "bag_compoundlayer.h"
#include "bag_dataset.h"
#include "bag_errors.h"
#include "bag_layer.h"
#include "bag_metadata.h"
#include "bag_metadata_import.h"
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
#include <string.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif


namespace {

//! Convert a BAG::CompoundDataType (C++) into a BagCompoundDataType (C).
/*!
\param field
    The BAG::CompoundDataType.

\return
    The BagCompoundDataType created from \e field.
*/
BagCompoundDataType getValue(
    const BAG::CompoundDataType& field)
{
    BagCompoundDataType result{};

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
        const auto fieldLen = strlen(value) + 1;
        result.data.c = new char[fieldLen];
        memcpy(result.data.c, value, fieldLen);
        break;
    }
    default:
        result.type = DT_UNKNOWN_DATA_TYPE;
        break;
    }

    return result;
}

//! Convert a BagCompoundDataType (C) into a BAG::CompoundDataType (C++).
/*!
\param field
    The BagCompoundDataType.

\return
    The BAG::CompoundDataType created from \e field.
*/
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

}  // namespace

//! Open the specified BAG.
/*!
\param handle
    A handle to the new BAG.
    Cannot be NULL.
\param accessMode
    How to access the BAG.
    Read only or reading and writing.
\param fileName
    The BAG file name.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
BagError bagFileOpen(
    BagHandle** handle,
    BAG_OPEN_MODE accessMode,
    const char* fileName)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!fileName)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    try
    {
        auto pHandle = std::make_unique<BagHandle>();

        pHandle->dataset = BAG::Dataset::open(std::string{fileName}, accessMode);

        *handle = pHandle.release();
    }
    catch(const std::exception& /*e*/)
    {
        return BAG_BAD_FILE_IO_OPERATION;
    }

    return BAG_SUCCESS;
}

//! Close the specified BAG.
/*!
\param handle
    The BAG handle.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
BagError bagFileClose(
    BagHandle* handle)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

#ifndef NDEBUG
    if (handle->dataset.use_count() > 1)
        return BAG_MORE_BAG_INSTANCES_PRESENT;
#endif

    handle->dataset.reset();
    delete handle;

    return BAG_SUCCESS;
}

//! Create a BAG from the specified metadata XML file.
/*!
\param handle
    A handle to the new BAG.
    Cannot be NULL.
\param fileName
    The BAG file name.
    Cannot be NULL.
\param metadataFile
    The metadata file name.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
BagError bagCreateFromFile(
    BagHandle** handle,
    const char* fileName,
    const char* metadataFile)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!fileName || !metadataFile)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    try
    {
        BAG::Metadata metadata;
        metadata.loadFromFile(std::string{metadataFile});

        constexpr uint64_t chunkSize = 100;
        constexpr int compressionLevel = 6;

        auto pHandle = std::make_unique<BagHandle>();

        pHandle->dataset = BAG::Dataset::create(std::string{fileName},
            std::move(metadata), chunkSize, compressionLevel);

        *handle = pHandle.release();
    }
    catch(const std::exception /*e*/)
    {
        return BAG_BAD_FILE_IO_OPERATION;
    }

    return BAG_SUCCESS;
}

//! Create a BAG from the specified metadata XML buffer.
/*!
\param handle
    A handle to the new BAG.
    Cannot be NULL.
\param fileName
    The BAG file name.
    Cannot be NULL.
\param metadataBuffer
    The metadata information in a buffer.
    Cannot be NULL.
\param metadataBufferSize
    The length of \e metadataBuffer.

\return
    0 if successful.
    An error code otherwise.
*/
BagError bagCreateFromBuffer(
    BagHandle** handle,
    const char* fileName,
    uint8_t* metadataBuffer,
    uint32_t metadataBufferSize)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!fileName || !metadataBuffer)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    try
    {
        BAG::Metadata metadata;
        metadata.loadFromBuffer(
            std::string{reinterpret_cast<char*>(metadataBuffer), metadataBufferSize});

        constexpr uint64_t chunkSize = 100;
        constexpr int compressionLevel = 6;

        auto pHandle = std::make_unique<BagHandle>();

        pHandle->dataset = BAG::Dataset::create(std::string{fileName},
            std::move(metadata), chunkSize, compressionLevel);

        *handle =  pHandle.release();
    }
    catch(const std::exception& /*e*/)
    {
        return BAG_HDF_CREATE_DATASET_FAILURE;
    }

    return BAG_SUCCESS;
}

//! Create a simple layer in the BAG.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param type
    The type of simple layer to create.

\return
    0 if successful.
    An error code otherwise.
*/
BagError bagCreateLayer(
    BagHandle* handle,
    BAG_LAYER_TYPE type)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

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

//! Retrieve the BAG grid dimensions.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param rows
    The number of rows in the BAG.
    Cannot be NULL.
\param cols
    The number of columns in the BAG.
    Cannot be NULL.
\param metadataBufferSize
    The length of \e metadataBuffer.

\return
    0 if successful.
    An error code otherwise.
*/
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

//! Retrieve the BAG spacing.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param rows
    The number of rows in the BAG.
    Cannot be NULL.
\param cols
    The number of columns in the BAG.
    Cannot be NULL.
\param metadataBufferSize
    The length of \e metadataBuffer.

\return
    0 if successful.
    An error code otherwise.
*/
BagError bagGetSpacing(
    BagHandle* handle,
    double* rowSpacing,
    double* columnSpacing)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!rowSpacing || !columnSpacing)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto& metadata = handle->dataset->getMetadata();

    *rowSpacing = metadata.rowResolution();
    *columnSpacing = metadata.columnResolution();

    return BAG_SUCCESS;
}

//! Retrieve the BAG geographic cover.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param llx
    The lower left X geographic value.
    Cannot be NULL.
\param lly
    The lower left Y geographic value.
    Cannot be NULL.
\param urx
    The upper right X geographic value.
    Cannot be NULL.
\param ury
    The upper right Y geographic value.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
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

//! Retrieve the minimum and maximum value of a simple layer.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param type
    The simple layer type.
\param minValue
    The minimum value.
    Cannot be NULL.
\param maxValue
    The maximum value.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
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

    std::tie(*minValue, *maxValue) = layer->getDescriptor()->getMinMax();

    return BAG_SUCCESS;
}

//! Set the minimum and maximum value on a simple layer.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param type
    The simple layer type.
\param minValue
    The new minimum value.
\param maxValue
    The new maximum value.

\return
    0 if successful.
    An error code otherwise.
*/
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
        return BAG_SIMPLE_LAYER_MISSING;

    layer->getDescriptor()->setMinMax(minValue, maxValue);

    return BAG_SUCCESS;
}

//! Retrieve the number of layers in the BAG.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param numLayers
    The number of layers.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
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

//! Determine if the specified layer and optional case-insensitive name exists.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param type
    The layer type.
\param layerName
    The case-insensitive name of the layer.
    Optional unless checking for a compound layer.
\param bagError
    BAG_SUCCESS if successful.
    The error code, otherwise.
    Cannot be NULL.

\return
    \e true if the BAG contains the specified layer
    \e false otherwise
*/
bool bagContainsLayer(
    BagHandle* handle,
    BAG_LAYER_TYPE type,
    const char* layerName,
    BagError* bagError)
{
    if (!handle)
    {
        *bagError = BAG_INVALID_BAG_HANDLE;
        return false;
    }

    if (type == Compound && (!layerName || layerName[0] == '\0'))
    {
        *bagError = BAG_COMPOUND_LAYER_NAME_MISSING;
        return false;
    }

    *bagError = BAG_SUCCESS;

    return handle->dataset->getLayer(type, layerName) != nullptr;
}

//! Read a specific area of a BAG.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param rowStart
    The starting row.
\param colStart
    The starting column.
\param rowEnd
    The end row (inclusive).
\param colEnd
    The end column (inclusive).
\param type
    The layer type.
\param layerName
    The case-insensitive name of the layer.
    Optional unless checking for a compound layer.
\param data
    The buffer the BAG is read into.
    data must be at least large enough to hold the specified number of rows and columns.
    Cannot be NULL.
\param x
    The geographical position of the \e rowStart.
    Cannot be NULL.
\param y
    The geographical position of the \e colStart.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
BagError bagRead(
    BagHandle* handle,
    uint32_t rowStart,
    uint32_t colStart,
    uint32_t rowEnd,
    uint32_t colEnd,
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

    if (type == Compound && (!layerName || layerName[0] == '\0'))
        return BAG_COMPOUND_LAYER_NAME_MISSING;

    const auto* layer = handle->dataset->getLayer(type, layerName);
    if (!layer)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    try
    {
        auto buffer = layer->read(rowStart, colStart, rowEnd, colEnd);
        *data = buffer.release();

        // Get the position of the node.
        std::tie(*x, *y) = handle->dataset->gridToGeo(rowStart, colStart);
    }
    catch(const std::exception& /*e*/)
    {
        return BAG_HDF_READ_FAILURE;
    }

    return BAG_SUCCESS;
}

//! Write to a specific area of a BAG.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param rowStart
    The starting row.
\param colStart
    The starting column.
\param rowEnd
    The end row (inclusive).
\param colEnd
    The end column (inclusive).
\param type
    The layer type.
\param layerName
    The case-insensitive name of the layer.
    Optional unless checking for a compound layer.
\param data
    The buffer to write to the BAG.
    data must be at least large enough to hold the specified number of rows and columns.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
BagError bagWrite(
    BagHandle* handle,
    uint32_t rowStart,
    uint32_t colStart,
    uint32_t rowEnd,
    uint32_t colEnd,
    BAG_LAYER_TYPE type,
    const char* layerName,
    const uint8_t* data)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!data)
        return BAG_SUCCESS;  // nothing to write

    if (type == Compound && (!layerName || layerName[0] == '\0'))
        return BAG_COMPOUND_LAYER_NAME_MISSING;

    auto* layer = handle->dataset->getLayer(type, layerName);
    if (!layer)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    try
    {
        layer->write(rowStart, colStart, rowEnd, colEnd, data);
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

//! Determine the geographic position of a specified grid row and column.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param row
    The grid row.
\param col
    The grid column.
\param x
    The geographical position of \e row.
    Cannot be NULL.
\param y
    The geographical position of \e col.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
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

//! Determine the grid position from the specified geographic position.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param x
    The X geographical position.
\param y
    The Y geographical position.
\param row
    The grid row.
    Cannot be NULL.
\param col
    The grid column.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
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

//! Allocate a buffer that will hold the specified number of rows and columns of the specified layer.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param numRows
    The number of rows.
\param numCols
    The number of columns.
\param type
    The layer type.
\param layerName
    The case-insensitive name of the layer.
    Optional unless checking for a compound layer.
\param bagError
    The error (if any) generated from this function.

\return
    The allocated buffer of the appropriate size.
    NULL if an error occurs.
*/
uint8_t* bagAllocateBuffer(
    BagHandle* handle,
    uint32_t numRows,
    uint32_t numCols,
    BAG_LAYER_TYPE type,
    const char* layerName,
    BagError* bagError)
{
    if (!handle)
        return {};

    if (!bagError)
        return {};

    if (type == Compound && (!layerName || layerName[0] == '\0'))
    {
        *bagError = BAG_COMPOUND_LAYER_NAME_MISSING;
        return {};
    }

    const auto* layer = handle->dataset->getLayer(type, layerName);
    if (!layer)
    {
        *bagError = BAG_LAYER_MISSING;
        return {};
    }

    const int8_t elementSize = layer->getDescriptor()->getElementSize();

    *bagError = BAG_SUCCESS;

    return new uint8_t[numRows * numCols * elementSize];
}

//! Allocate a buffer of the specified size.
/*!
\param numBytes
    The number of bytes to allocate.

\return
    A buffer of the specified size.
*/
uint8_t* bagAllocate(uint32_t numBytes)
{
    return new uint8_t[numBytes];
}

//! Free a buffer allocated by bagAllocate() or bagAllocateBuffer().
/*!
\param buffer
    The buffer to free.
*/
void bagFree(uint8_t* buffer)
{
    delete[] buffer;
}

// Surface Corrections
//! Retrieve the specified vertical datum from the surface corrections.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param corrector
    The corrector to use.
    Valid values are 1-10.
\param datum
    The retrieved vertical datum.
    The memory must be pre-allocated, and at least 256 bytes.

\return
    0 if successful.
    An error code otherwise.
*/
BagError bagReadCorrectorVerticalDatum(
    BagHandle* handle,
    uint8_t corrector,
    uint8_t* datum)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (corrector < 1 || corrector > BAG_SURFACE_CORRECTOR_LIMIT)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    if (!datum)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto* layer = handle->dataset->getSurfaceCorrections();
    if (!layer)
        return BAG_SURFACE_CORRECTIONS_MISSING;

    auto pDescriptor =
        std::dynamic_pointer_cast<const BAG::SurfaceCorrectionsDescriptor>(
            layer->getDescriptor());
    const auto& allVerticalDatums = pDescriptor->getVerticalDatums();

    if (!allVerticalDatums.empty())
    {
        // Iterate over the datums, looking for the one specified by corrector-1.
        std::istringstream iss{allVerticalDatums};
        std::string item;
        size_t index = 1;

        while (index <= BAG_SURFACE_CORRECTOR_LIMIT && std::getline(iss, item, ','))
        {
            if (index == corrector)
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

//! Write the specified vertical datum to the surface corrections in the BAG.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param corrector
    The corrector to use.
    Valid values are 1-10.
\param inDatum
    The vertical datum.
    Must be NULL terminated.

\return
    0 if successful.
    An error code otherwise.
*/
BagError bagWriteCorrectorVerticalDatum(
    BagHandle* handle,
    uint8_t corrector,
    const uint8_t* inDatum)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (corrector < 1 || corrector > BAG_SURFACE_CORRECTOR_LIMIT)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    if (!inDatum)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    auto* layer = handle->dataset->getSurfaceCorrections();
    if (!layer)
        return BAG_SURFACE_CORRECTIONS_MISSING;

    auto pDescriptor = std::dynamic_pointer_cast<BAG::SurfaceCorrectionsDescriptor>(
        layer->getDescriptor());

    // Set/replace the specified datum.
    std::vector<std::string> datums;
    datums.reserve(BAG_SURFACE_CORRECTOR_LIMIT);

    std::istringstream iss{pDescriptor->getVerticalDatums()};

    while (iss)
    {
        std::string datum;
        std::getline(iss, datum, ',');
        if (!iss)
            break;

        datums.emplace_back(std::move(datum));
    }

    datums[corrector-1] = reinterpret_cast<const char*>(inDatum);

    std::string joinedDatums = std::accumulate(cbegin(datums), cend(datums),
        std::string{}, [](std::string& dest, const std::string& datum)
        {
            return dest.empty() ? datum : dest + ',' + datum;
        });

    pDescriptor->setVerticalDatums(joinedDatums);

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

//! Read a corrected simple layer.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param corrector
    The corrector to use.
    Valid values are 1-10.
\param type
    The simple layer type.
\param data
    The buffer to place the corrected layer data into.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
BagError bagReadCorrectedLayer(
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

    const auto* layer = handle->dataset->getSimpleLayer(type);
    if (!layer)
        return BAG_SIMPLE_LAYER_MISSING;

    constexpr uint32_t rowStart = 0;
    constexpr uint32_t columnStart = 0;

    const auto& descriptor = handle->dataset->getDescriptor();

    uint32_t rowEnd = 0;
    uint32_t columnEnd = 0;
    std::tie(rowEnd, columnEnd) = descriptor.getDims();

    auto correctedData = corrections->readCorrected(rowStart, rowEnd - 1,
        columnStart, columnEnd - 1, corrector, *layer);

    *data = reinterpret_cast<float*>(correctedData.release());

    return BAG_SUCCESS;
}

//! Read a corrected region from a simple layer.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param rowStart
    The starting row.
\param colStart
    The starting column.
\param rowEnd
    The end row (inclusive).
\param colEnd
    The end column (inclusive).
\param corrector
    The corrector to use.
    Valid values are 1-10.
\param type
    The simple layer type.
\param data
    The buffer to place the corrected layer into.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
BagError bagReadCorrectedRegion(
    BagHandle* handle,
    uint32_t rowStart,
    uint32_t colStart,
    uint32_t rowEnd,
    uint32_t colEnd,
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

    auto correctedData = corrections->readCorrected(rowStart, colStart, rowEnd,
        colEnd, corrector, *layer);

    *data = reinterpret_cast<float*>(correctedData.release());

    return BAG_SUCCESS;
}

//! Read a corrected row from a simple layer.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param row
    The row.
\param corrector
    The corrector to use.
    Valid values are 1-10.
\param type
    The simple layer type.
\param data
    The buffer to place the corrected layer's row into.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
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

    *data = reinterpret_cast<float*>(correctedData.release());

    return BAG_SUCCESS;
}

//! Read a corrected node from a simple layer.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param row
    The row.
\param column
    The column.
\param corrector
    The corrector to use.
    Valid values are 1-10.
\param type
    The simple layer type.
\param data
    The buffer to place the corrected layer's row into.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
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
        return BAG_SIMPLE_LAYER_MISSING;

    auto correctedData = corrections->readCorrectedRow(row, column,
        column, corrector, *layer);

    *data = reinterpret_cast<float*>(correctedData.release());

    return BAG_SUCCESS;
}

//! Retrieve the number of correctors.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param numCorrectors
    The number of correctors.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
BagError bagGetNumSurfaceCorrectors(
    BagHandle* handle,
    uint8_t* numCorrectors)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!numCorrectors)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto* layer = handle->dataset->getSurfaceCorrections();
    if (!layer)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    auto pDescriptor =
        std::dynamic_pointer_cast<const BAG::SurfaceCorrectionsDescriptor>(
            layer->getDescriptor());
    if (!pDescriptor)
        return BAG_WRONG_DESCRIPTOR_FOUND;

    *numCorrectors = pDescriptor->getNumCorrectors();

    return BAG_SUCCESS;
}

//! Retrieve the surface correction topography.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param topography
    The surface correction topography
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
BagError bagGetSurfaceCorrectionTopography(
    BagHandle* handle,
    BAG_SURFACE_CORRECTION_TOPOGRAPHY* type)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!type)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto* layer = handle->dataset->getSurfaceCorrections();
    if (!layer)
        return BAG_SURFACE_CORRECTIONS_MISSING;

    auto pDescriptor =
        std::dynamic_pointer_cast<const BAG::SurfaceCorrectionsDescriptor>(
            layer->getDescriptor());
    if (!pDescriptor)
        return BAG_WRONG_DESCRIPTOR_FOUND;

    *type = pDescriptor->getSurfaceType();

    return BAG_SUCCESS;
}

//! Retrieve the surface correction topography.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param numCorrectors
    The number of correctors the surface corrections layer will have.
\param topography
    The surface correction topography
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
BagError bagCreateCorrectorLayer(
    BagHandle* handle,
    uint8_t numCorrectors,
    BAG_SURFACE_CORRECTION_TOPOGRAPHY topography)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    // Get chunkSize & compressionLevel from the elevation layer.
    const auto* elevationLayer = handle->dataset->getSimpleLayer(Elevation);
    if (!elevationLayer)
        return BAG_SIMPLE_LAYER_MISSING;

    auto pDescriptor = elevationLayer->getDescriptor();
    const auto chunkSize = pDescriptor->getChunkSize();
    const auto compressionLevel = pDescriptor->getCompressionLevel();

    handle->dataset->createSurfaceCorrections(topography, numCorrectors,
        chunkSize, compressionLevel);

    return BAG_SUCCESS;
}

//! Write the attributes to the surface corrections layer.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param def
    The new attribute values.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
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
        return BAG_SURFACE_CORRECTIONS_MISSING;

    auto pDescriptor =
        std::dynamic_pointer_cast<BAG::SurfaceCorrectionsDescriptor>(
            corrections->getDescriptor());
    if (!pDescriptor)
        return BAG_WRONG_DESCRIPTOR_FOUND;

    pDescriptor->setOrigin(def->swCornerX, def->swCornerY)
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

//! Write the attributes to the surface corrections layer.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param def
    The new attribute values.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
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

    auto pDescriptor =
        std::dynamic_pointer_cast<BAG::SurfaceCorrectionsDescriptor>(
            corrections->getDescriptor());
    if (!pDescriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    std::tie(def->swCornerX, def->swCornerY) = pDescriptor->getOrigin();
    std::tie(def->nodeSpacingX, def->nodeSpacingY) = pDescriptor->getSpacing();

    return BAG_SUCCESS;
}

// Tracking List
//! Retrieve the tracking list length.
/*
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param length
    The number of items in the tracking list.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
BagError bagTrackingListLength(
    BagHandle* handle,
    uint32_t* length)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!length)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    *length = static_cast<uint32_t>(handle->dataset->getTrackingList().size());

    return BAG_SUCCESS;
}

//! Read items from a tracking list.
/*
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param row
    The row.
\param col
    The column.
\param items
    The items read.
    Cannot be NULL.
\param numItems
    The number of items read.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
BagError bagReadTrackingListNode(
    BagHandle* handle,
    uint32_t row,
    uint32_t col,
    BagTrackingItem** items,
    uint32_t* numItems)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!items || !numItems)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto& trackingList = handle->dataset->getTrackingList();

    using std::cbegin;  using std::cend;
    std::vector<BAG::TrackingItem> results;

    std::copy_if(cbegin(trackingList), cend(trackingList),
        std::back_inserter(results),
        [row, col](const BAG::TrackingItem& item) {
            return item.row == row && item.col == col;
        });

    *numItems = static_cast<uint32_t>(results.size());

    *items = new BAG::TrackingItem[*numItems];
    memcpy(*items, results.data(), *numItems);

    return BAG_SUCCESS;
}

//! Read the items with matching track code from the entire tracking list.
/*
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param code
    The tracking code to match.
\param items
    The items read.
    Cannot be NULL.
\param numItems
    The number of items read.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
BagError bagReadTrackingListCode(
    BagHandle* handle,
    uint8_t code,
    BagTrackingItem** items,
    uint32_t* numItems)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!items || !numItems)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto& trackingList = handle->dataset->getTrackingList();

    using std::cbegin;  using std::cend;
    std::vector<BAG::TrackingItem> results;

    std::copy_if(cbegin(trackingList), cend(trackingList),
        std::back_inserter(results),
        [code](const BAG::TrackingItem& item) {
            return item.track_code == code;
        });

    *numItems = static_cast<uint32_t>(results.size());

    *items = new BAG::TrackingItem[*numItems];
    memcpy(*items, results.data(), *numItems);

    return BAG_SUCCESS;
}

//! Read the items with matching list series from the entire tracking list.
/*
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param series
    The list series to match.
\param items
    The items read.
    Cannot be NULL.
\param numItems
    The number of items read.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
BagError bagReadTrackingListSeries(
    BagHandle* handle,
    uint16_t series,
    BagTrackingItem** items,
    uint32_t* numItems)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!items || !numItems)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto& trackingList = handle->dataset->getTrackingList();

    using std::cbegin;  using std::cend;
    std::vector<BAG::TrackingItem> results;

    std::copy_if(cbegin(trackingList), cend(trackingList),
        std::back_inserter(results),
        [series](const BAG::TrackingItem& item) {
            return item.list_series == series;
        });

    *numItems = static_cast<uint32_t>(results.size());

    *items = new BAG::TrackingItem[*numItems];
    memcpy(*items, results.data(), *numItems);

    return BAG_SUCCESS;
}

//! Add an item to the end of the tracking list.
/*
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param item
    The tracking list item.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
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

//! Sort the tracking list descending by node.
/*
\param handle
    A handle to the BAG.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
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

//! Sort the tracking list descending by series list.
/*
\param handle
    A handle to the BAG.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
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

//! Sort the tracking list descending by track code.
/*
\param handle
    A handle to the BAG.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
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
//! Retrieve the metadata.
/*!
\param handle
    A handle to the BAG.

\return
    The metadata.
    NULL if the handle is NULL.
*/
const BagMetadata* bagGetMetaData(BagHandle* handle)
{
    if (!handle)
        return nullptr;

    return &handle->dataset->getMetadata().getStruct();
}

//! Set the home folder.  This will override the BAG_HOME environment variable.
/*!
\param metadataFolder
    The new metadata folder.
    Does not verify if it exists.
\return
    0 if successful.
    An error code otherwise.
*/
BagError bagSetHomeFolder(const char* metadataFolder)
{
    if (!metadataFolder)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    BAG::bagSetHomeFolder(metadataFolder);

    return BAG_SUCCESS;
}

// CompoundLayer
//! Create a compound layer.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param indexType
    The type of index the compound layer will use to index the record.
    Valid types are: DT_UINT8, DT_UINT16, DT_UINT32, DT_UINT64
\param layerName
    The case-insensitive name of the simple layer this compound layer contains metadata about.
    Cannot be NULL.
\param definition
    The list of fields making up the record definition.
    Cannot be NULL.
\param numFields
    The number of fields in the definition.

\return
    0 if successful.
    An error code otherwise.
*/
BagError bagCreateCompoundLayer(
    BagHandle* handle,
    BAG_DATA_TYPE indexType,
    const char* layerName,
    const FieldDefinition* definition,
    uint32_t numFields)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!layerName || !definition || numFields < 1)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    // Get the chunkSize & compressionLevel from the elevation layer.
    const auto* elevationLayer = handle->dataset->getSimpleLayer(Elevation);
    if (!elevationLayer)
        return BAG_SIMPLE_LAYER_MISSING;

    auto pDescriptor = elevationLayer->getDescriptor();
    const auto chunkSize = pDescriptor->getChunkSize();
    const auto compressionLevel = pDescriptor->getCompressionLevel();

    // Convert the FieldDefinition* into a RecordDefinition.
    const BAG::RecordDefinition recordDef(definition, definition + numFields);

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

//! Get the compound layer record definition.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param layerName
    The case-insensitive name of the simple layer this compound layer contains metadata about.
    Cannot be NULL.
\param definition
    The list of fields making up the record definition.
    Cannot be NULL.
\param numFields
    The number of fields in the definition.

\return
    0 if successful.
    An error code otherwise.
*/
BagError bagGetCompoundLayerDefinition(
    BagHandle* handle,
    const char* layerName,
    FieldDefinition** definition,
    uint32_t* numFields)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!definition || !numFields)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto* compoundLayer = handle->dataset->getCompoundLayer(layerName);
    if (!compoundLayer)
        return BAG_COMPOUND_LAYER_MISSING;

    const auto& recordDef = compoundLayer->getValueTable().getDefinition();

    // Convert the RecordDefinition into a FieldDefinition*.
    *numFields = static_cast<uint32_t>(recordDef.size());

    auto* pDef = *definition;
    pDef = new FieldDefinition[*numFields];

    uint32_t index = 0;
    for (const auto& def : recordDef)
        pDef[index++] = def;

    return BAG_SUCCESS;
}

//! Get the compound layer records.
/*!
    Read all the records from the compound layer.
    Note that the first record is the no data value record.

\param handle
    A handle to the BAG.
    Cannot be NULL.
\param layerName
    The case-insensitive name of the simple layer the compound layer contains metadata about.
    Cannot be NULL.
\param records
    The records read from the compound layer.
    Cannot be NULL.
\param numRecords
    The number of records.
\param numFields
    The number of fields in a record.

\return
    0 if successful.
    An error code otherwise.
*/
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
        return BAG_COMPOUND_LAYER_MISSING;

    const auto& recs = compoundLayer->getValueTable().getRecords();
    if (recs.size() == 1)  // No user defined records; just the single no data value record.
    {
        *numRecords = 0;
        *numFields = static_cast<uint32_t>(
            compoundLayer->getValueTable().getDefinition().size());

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

//! Get a specific value from the compound layer.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param layerName
    The case-insensitive name of the simple layer the compound layer contains metadata about.
    Cannot be NULL.
\param recordIndex
    The record index to be read from the compound layer.
    Valid values are 1 to number of records - 1.
    Index 0 contains the no data value so is not queried.
\param fieldName
    The name of the field.
    Cannot be NULL.
\param value
    The value read.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
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
        return BAG_COMPOUND_LAYER_MISSING;

    try
    {
        const auto& val = compoundLayer->getValueTable().getValue(recordIndex,
            fieldName);

        *value = getValue(val);
    }
    catch(const BAG::RecordNotFound& /*e*/)
    {
        return BAG_COMPOUND_LAYER_RECORD_NOT_FOUND;
    }
    catch(const BAG::FieldNotFound& /*e*/)
    {
        return BAG_COMPOUND_LAYER_FIELD_NOT_FOUND;
    }
    catch(const std::exception& /*e*/)
    {
        return BAG_COMPOUND_LAYER_NO_VALUE_FOUND;
    }

    return BAG_SUCCESS;
}

//! Get a specific value from the compound layer.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param layerName
    The case-insensitive name of the simple layer the compound layer contains metadata about.
    Cannot be NULL.
\param recordIndex
    The record index to be read from the compound layer.
    Valid values are 1 to number of records - 1.
    Index 0 contains the no data value so is not queried.
\param fieldIndex
    The name of the field.
\param value
    The value read.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
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
        return BAG_COMPOUND_LAYER_MISSING;

    try
    {
        const auto& val = compoundLayer->getValueTable().getValue(recordIndex,
            fieldIndex);

        *value = getValue(val);
    }
    catch(const BAG::RecordNotFound& /*e*/)
    {
        return BAG_COMPOUND_LAYER_RECORD_NOT_FOUND;
    }
    catch(const BAG::FieldNotFound& /*e*/)
    {
        return BAG_COMPOUND_LAYER_FIELD_NOT_FOUND;
    }
    catch(const std::exception& /*e*/)
    {
        return BAG_COMPOUND_LAYER_NO_VALUE_FOUND;
    }

    return BAG_SUCCESS;
}

//! Retrieve the field index of a field name from a specified compound layer.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param layerName
    The case-insensitive name of the simple layer the compound layer contains metadata about.
    Cannot be NULL.
\param fieldName
    The name of the field.
    Cannot be NULL.
\param fieldIndex
    The index of the field.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
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
        return BAG_COMPOUND_LAYER_MISSING;

    try
    {
        *fieldIndex = static_cast<uint32_t>(
            compoundLayer->getValueTable().getFieldIndex(fieldName));
    }
    catch(const BAG::FieldNotFound& /*e*/)
    {
        return BAG_COMPOUND_LAYER_FIELD_NOT_FOUND;
    }

    return BAG_SUCCESS;
}

//! Retrieve the field name of a field index from a specified compound layer.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param layerName
    The case-insensitive name of the simple layer the compound layer contains metadata about.
    Cannot be NULL.
\param fieldIndex
    The index of the field.
\param fieldName
    The name of the field.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
BagError bagGetCompoundLayerFieldName(
    BagHandle* handle,
    const char* layerName,
    uint32_t fieldIndex,
    const char** fieldName)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!layerName || !fieldName)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto* compoundLayer = handle->dataset->getCompoundLayer(layerName);
    if (!compoundLayer)
        return BAG_COMPOUND_LAYER_MISSING;

    try
    {
        *fieldName = compoundLayer->getValueTable().getFieldName(fieldIndex);
    }
    catch(const BAG::FieldNotFound& /*e*/)
    {
        return BAG_COMPOUND_LAYER_FIELD_NOT_FOUND;
    }

    return BAG_SUCCESS;
}

//! Add a record to the end of the specified compound layer.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param layerName
    The case-insensitive name of the simple layer the compound layer contains metadata about.
    Cannot be NULL.
\param record
    The record to add.
    Cannot be NULL.
\param numFields
    The number of fields in the record.
\param recordIndex
    The index of new record.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
BagError bagAddCompoundLayerRecord(
    BagHandle* handle,
    const char* layerName,
    const BagCompoundDataType* record,
    uint32_t numFields,
    uint32_t* recordIndex)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!layerName || !record || !recordIndex)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    auto* compoundLayer = handle->dataset->getCompoundLayer(layerName);
    if (!compoundLayer)
        return BAG_COMPOUND_LAYER_MISSING;

    // Convert BagCompoundDataType into a BAG::Record.
    BAG::Record rec(numFields);

    size_t index = 0;
    for (auto& field : rec)
        field = getValue(record[index++]);

    try
    {
        *recordIndex = static_cast<uint32_t>(
            compoundLayer->getValueTable().addRecord(rec));
    }
    catch(const BAG::InvalidRecord& /*e*/)
    {
        return BAG_COMPOUND_LAYER_INVALID_RECORD_DEFINITION;
    }
    catch(const BAG::InvalidRecordsIndex&)
    {
        return BAG_COMPOUND_LAYER_RECORD_NOT_FOUND;
    }
    catch(const std::exception& /*e*/)
    {
        return BAG_HDF_READ_FAILURE;
    }

    return BAG_SUCCESS;
}

//! Add records to the end of the specified compound layer.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param layerName
    The case-insensitive name of the simple layer the compound layer contains metadata about.
    Cannot be NULL.
\param record
    The record to add.
    Cannot be NULL.
\param numRecords
    The number of records.
\param numFields
    The number of fields in each record.

\return
    0 if successful.
    An error code otherwise.
*/
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
        return BAG_COMPOUND_LAYER_MISSING;

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
        return BAG_COMPOUND_LAYER_INVALID_RECORD_DEFINITION;
    }
    catch(const BAG::InvalidRecordsIndex&)
    {
        return BAG_COMPOUND_LAYER_RECORD_NOT_FOUND;
    }
    catch(const std::exception& /*e*/)
    {
        return BAG_HDF_READ_FAILURE;
    }

    return BAG_SUCCESS;
}

//! Set the value of a field in a record.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param layerName
    The case-insensitive name of the simple layer the compound layer contains metadata about.
    Cannot be NULL.
\param recordIndex
    The record to update.
\param fieldName
    The name of the field.
    Cannot be NULL.
\param value
    The value to write.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
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
        return BAG_COMPOUND_LAYER_MISSING;

    // Convert BagCompoundDataType into a BAG::CompoundDataType.
    const auto val = getValue(*value);

    try
    {
        compoundLayer->getValueTable().setValue(recordIndex, fieldName, val);
    }
    catch(const BAG::RecordNotFound& /*e*/)
    {
        return BAG_COMPOUND_LAYER_RECORD_NOT_FOUND;
    }
    catch(const BAG::InvalidRecord& /*e*/)
    {
        return BAG_COMPOUND_LAYER_INVALID_RECORD_DEFINITION;
    }
    catch(const BAG::InvalidRecordsIndex&)
    {
        return BAG_COMPOUND_LAYER_RECORD_NOT_FOUND;
    }
    catch(const std::exception& /*e*/)
    {
        return BAG_HDF_READ_FAILURE;
    }

    return BAG_SUCCESS;
}

//! Set the value of a field in a record.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param layerName
    The case-insensitive name of the simple layer the compound layer contains metadata about.
    Cannot be NULL.
\param recordIndex
    The record to update.
\param fieldIndex
    The index of the field.
\param value
    The value to write.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
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
        return BAG_COMPOUND_LAYER_MISSING;

    // Convert BagCompoundDataType into a BAG::CompoundDataType.
    const auto val = getValue(*value);

    try
    {
        compoundLayer->getValueTable().setValue(recordIndex, fieldIndex, val);
    }
    catch(const BAG::RecordNotFound& /*e*/)
    {
        return BAG_COMPOUND_LAYER_RECORD_NOT_FOUND;
    }
    catch(const BAG::InvalidRecord& /*e*/)
    {
        return BAG_COMPOUND_LAYER_INVALID_RECORD_DEFINITION;
    }
    catch(const BAG::InvalidRecordsIndex&)
    {
        return BAG_COMPOUND_LAYER_RECORD_NOT_FOUND;
    }
    catch(const std::exception& /*e*/)
    {
        return BAG_HDF_READ_FAILURE;
    }

    return BAG_SUCCESS;
}

// Variable Resolution
//! Create the optional variable resolution layers.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param makeNode
    If \e true, create the optional variable resolution node group layer.
    If e false, do not

\return
    0 if successful.
    An error code otherwise.
*/
BagError bagCreateVRLayers(
    BagHandle* handle,
    bool makeNode)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    //  Get chunkSize & compressionLevel from the elevation layer.
    const auto* elevationLayer = handle->dataset->getSimpleLayer(Elevation);
    if (!elevationLayer)
        return BAG_SIMPLE_LAYER_MISSING;

    auto pDescriptor = elevationLayer->getDescriptor();
    const auto chunkSize = pDescriptor->getChunkSize();
    const auto compressionLevel = pDescriptor->getCompressionLevel();

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

// Variable resolution Metadata
//! Retrieve the minimum X and Y dimensions from the variable resolution metadata.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param minX
    The minimum X dimension.
    Cannot be NULL.
\param minY
    The minimum Y dimension.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
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

    auto pDescriptor =
        std::dynamic_pointer_cast<const BAG::VRMetadataDescriptor>(
            vrMetadata->getDescriptor());
    if (!pDescriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    std::tie(*minX, *minY) = pDescriptor->getMinDimensions();

    return BAG_SUCCESS;
}

//! Retrieve the maximum X and Y dimensions from the variable resolution metadata.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param maxX
    The maximum X dimension.
    Cannot be NULL.
\param maxY
    The maximum Y dimension.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
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

    auto pDescriptor =
        std::dynamic_pointer_cast<const BAG::VRMetadataDescriptor>(
            vrMetadata->getDescriptor());
    if (!pDescriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    std::tie(*maxX, *maxY) = pDescriptor->getMinDimensions();

    return BAG_SUCCESS;
}

//! Retrieve the minimum X and Y resolution from the variable resolution metadata.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param minX
    The minimum X resolution.
    Cannot be NULL.
\param minY
    The minimum Y resolution.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
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

    auto pDescriptor =
        std::dynamic_pointer_cast<const BAG::VRMetadataDescriptor>(
            vrMetadata->getDescriptor());
    if (!pDescriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    std::tie(*minX, *minY) = pDescriptor->getMinResolution();

    return BAG_SUCCESS;
};

//! Retrieve the maximum X and Y resolution from the variable resolution metadata.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param maxX
    The maximum X resolution.
    Cannot be NULL.
\param maxY
    The maximum Y resolution.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
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

    auto pDescriptor = std::dynamic_pointer_cast<const BAG::VRMetadataDescriptor>(
        vrMetadata->getDescriptor());
    if (!pDescriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    std::tie(*maxX, *maxY) = pDescriptor->getMaxResolution();

    return BAG_SUCCESS;
}

//! Set the minimum X and Y dimensions for the variable resolution metadata.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param minX
    The minimum X dimension.
\param minY
    The minimum Y dimension.

\return
    0 if successful.
    An error code otherwise.
*/
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

    auto pDescriptor = std::dynamic_pointer_cast<BAG::VRMetadataDescriptor>(
        vrMetadata->getDescriptor());
    if (!pDescriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    pDescriptor->setMinDimensions(minX, minY);

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

//! Set the maximum X and Y dimensions for the variable resolution metadata.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param maxX
    The maximum X dimension.
\param maxY
    The maximum Y dimension.

\return
    0 if successful.
    An error code otherwise.
*/
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

    auto pDescriptor = std::dynamic_pointer_cast<BAG::VRMetadataDescriptor>(
        vrMetadata->getDescriptor());
    if (!pDescriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    pDescriptor->setMaxDimensions(maxX, maxY);

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

//! Set the minimum X and Y resolution for the variable resolution metadata.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param minX
    The minimum X dimension.
\param minY
    The minimum Y dimension.

\return
    0 if successful.
    An error code otherwise.
*/
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

    auto pDescriptor = std::dynamic_pointer_cast<BAG::VRMetadataDescriptor>(
        vrMetadata->getDescriptor());
    if (!pDescriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    pDescriptor->setMinResolution(minX, minY);

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

//! Set the maximum X and Y resolution for the variable resolution metadata.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param maxX
    The maximum X dimension.
\param maxY
    The maximum Y dimension.

\return
    0 if successful.
    An error code otherwise.
*/
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

    auto pDescriptor = std::dynamic_pointer_cast<BAG::VRMetadataDescriptor>(
        vrMetadata->getDescriptor());
    if (!pDescriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    pDescriptor->setMaxResolution(maxX, maxY);

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
//! Retrieve the minimum and maximum hypotheses strength from the variable resolution node group.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param minHypStr
    The minimum hypotheses strength.
    Cannot be NULL.
\param maxY
    The maximum hypotheses strength.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
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

    auto pDescriptor = std::dynamic_pointer_cast<BAG::VRNodeDescriptor>(
        vrNode->getDescriptor());
    if (!pDescriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    std::tie(*minHypStr, *maxHypStr) = pDescriptor->getMinMaxHypStrength();

    return BAG_SUCCESS;
}

//! Retrieve the minimum and maximum number of hypotheses from the variable resolution node group.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param minNumHyp
    The minimum number of hypotheses.
    Cannot be NULL.
\param maxNumHyp
    The maximum number of hypotheses.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
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

    auto pDescriptor = std::dynamic_pointer_cast<BAG::VRNodeDescriptor>(
        vrNode->getDescriptor());
    if (!pDescriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    std::tie(*minNumHyp, *maxNumHyp) = pDescriptor->getMinMaxNumHypotheses();

    return BAG_SUCCESS;
}

//! Retrieve the minimum and maximum number of samples from the variable resolution node group.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param minNSamples
    The minimum number of samples.
    Cannot be NULL.
\param maxNSamples
    The maximum number of samples.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
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

    auto pDescriptor = std::dynamic_pointer_cast<BAG::VRNodeDescriptor>(
        vrNode->getDescriptor());
    if (!pDescriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    std::tie(*minNSamples, *maxNSamples) = pDescriptor->getMinMaxNSamples();

    return BAG_SUCCESS;
}

//! Set the minimum and maximum hypotheses strength on the variable resolution metadata.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param minHypStr
    The minimum hypotheses strength.
\param maxHypStr
    The maximum hypotheses strength.

\return
    0 if successful.
    An error code otherwise.
*/
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

    auto pDescriptor = std::dynamic_pointer_cast<BAG::VRNodeDescriptor>(
        vrNode->getDescriptor());
    if (!pDescriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    pDescriptor->setMinMaxHypStrength(minHypStr, maxHypStr);

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

//! Set the minimum and maximum number of hypotheses on the variable resolution metadata.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param minNumHyp
    The minimum number of hypotheses.
\param maxNumHyp
    The maximum number of hypotheses.

\return
    0 if successful.
    An error code otherwise.
*/
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

    auto pDescriptor = std::dynamic_pointer_cast<BAG::VRNodeDescriptor>(
        vrNode->getDescriptor());
    if (!pDescriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    pDescriptor->setMinMaxNumHypotheses(minNumHyp, maxNumHyp);

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

//! Set the minimum and maximum number of samples on the variable resolution metadata.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param minNSamples
    The minimum number of samples.
\param maxNSamples
    The maximum number of samples.

\return
    0 if successful.
    An error code otherwise.
*/
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

    auto pDescriptor = std::dynamic_pointer_cast<BAG::VRNodeDescriptor>(
        vrNode->getDescriptor());
    if (!pDescriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    pDescriptor->setMinMaxNSamples(minNSamples, maxNSamples);

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

// Variable resolution Refinements
//! Retrieve the minimum and maximum depth from the variable resolution refinements.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param minDepth
    The maximum depth.
    Cannot be NULL.
\param maxDepth
    The maximum depth.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
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

    auto pDescriptor = std::dynamic_pointer_cast<BAG::VRRefinementsDescriptor>(
        vrRefinement->getDescriptor());
    if (!pDescriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    std::tie(*minDepth, *maxDepth) = pDescriptor->getMinMaxDepth();

    return BAG_SUCCESS;
}

//! Retrieve the minimum and maximum uncertainty from the variable resolution refinements.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param minUncert
    The maximum uncertainty.
    Cannot be NULL.
\param maxUncert
    The maximum uncertainty.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
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

    auto pDescriptor = std::dynamic_pointer_cast<BAG::VRRefinementsDescriptor>(
        vrRefinement->getDescriptor());
    if (!pDescriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    std::tie(*minUncert, *maxUncert) = pDescriptor->getMinMaxUncertainty();

    return BAG_SUCCESS;
}

//! Set the minimum and maximum depths for the variable resolution refinements.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param minDepth
    The minimum depth.
\param maxDepth
    The minimum depth.

\return
    0 if successful.
    An error code otherwise.
*/
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

    auto pDescriptor = std::dynamic_pointer_cast<BAG::VRRefinementsDescriptor>(
        vrRefinement->getDescriptor());
    if (!pDescriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    pDescriptor->setMinMaxDepth(minDepth, maxDepth);

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

//! Set the minimum and maximum uncertainty for the variable resolution refinements.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param minUncert
    The minimum uncertainty.
\param maxUncert
    The minimum uncertainty.

\return
    0 if successful.
    An error code otherwise.
*/
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

    auto pDescriptor = std::dynamic_pointer_cast<BAG::VRRefinementsDescriptor>(
        vrRefinement->getDescriptor());
    if (!pDescriptor)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    pDescriptor->setMinMaxUncertainty(minUncert, maxUncert);

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
//! Retrieve the number of items in the variable resolution tracking list.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param numItems
    The number of items.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
BagError bagVRTrackingListLength(
    BagHandle* handle,
    uint32_t* numItems)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!numItems)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    const auto* vrTrackingList = handle->dataset->getVRTrackingList();
    if (!vrTrackingList)
        return BAG_HDF_DATASET_OPEN_FAILURE;

    *numItems = static_cast<uint32_t>(vrTrackingList->size());

    return BAG_SUCCESS;
}

//! Retrieve the items from the variable resolution tracking list matching row and column.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param row
    The row.
\param col
    The column.
\param items
    The items to retrieve.
    Cannot be NULL.
\param numItems
    The number of items that were retrieved.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
BagError bagReadVRTrackingListNode(
    BagHandle* handle,
    uint32_t row,
    uint32_t col,
    BagVRTrackingItem** items,
    uint32_t* numItems)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!items || !numItems)
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

    *numItems = static_cast<uint32_t>(results.size());

    *items = new BAG::VRTrackingItem[*numItems];
    memcpy(*items, results.data(), *numItems);

    return BAG_SUCCESS;
}

//! Retrieve the items from the variable resolution tracking list matching the sub row and column.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param row
    The sub row.
\param col
    The sub column.
\param items
    The items to retrieve.
    Cannot be NULL.
\param numItems
    The number of items that were retrieved.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
BagError bagReadVRTrackingListSubNode(
    BagHandle* handle,
    uint32_t row,
    uint32_t col,
    BagVRTrackingItem** items,
    uint32_t* numItems)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!items || !numItems)
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

    *numItems = static_cast<uint32_t>(results.size());

    *items = new BAG::VRTrackingItem[*numItems];
    memcpy(*items, results.data(), *numItems);

    return BAG_SUCCESS;
}

//! Retrieve the items from the variable resolution tracking list matching track code.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param code
    The track code.
\param items
    The items to retrieve.
    Cannot be NULL.
\param numItems
    The number of items that were retrieved.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
BagError bagReadVRTrackingListCode(
    BagHandle* handle,
    uint8_t code,
    BagVRTrackingItem** items,
    uint32_t* numItems)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!items || !numItems)
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

    *numItems = static_cast<uint32_t>(results.size());

    *items = new BAG::VRTrackingItem[*numItems];
    memcpy(*items, results.data(), *numItems);

    return BAG_SUCCESS;
}

//! Retrieve the items from the variable resolution tracking list matching list series.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param series
    The list series.
\param items
    The items to retrieve.
    Cannot be NULL.
\param numItems
    The number of items that were retrieved.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
BagError bagReadVRTrackingListSeries(
    BagHandle* handle,
    uint16_t series,
    BagVRTrackingItem** items,
    uint32_t* numItems)
{
    if (!handle)
        return BAG_INVALID_BAG_HANDLE;

    if (!items || !numItems)
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

    *numItems = static_cast<uint32_t>(results.size());

    *items = new BAG::VRTrackingItem[*numItems];
    memcpy(*items, results.data(), *numItems);

    return BAG_SUCCESS;
}

//! Write an item to the end of the variable resolution tracking list.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.
\param item
    The item.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
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

//! Sort the variable resolution tracking list by row and column, descending.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
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

//! Sort the variable resolution tracking list by sub row and column, descending.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
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

//! Sort the variable resolution tracking list by list series, descending.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
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

//! Sort the variable resolution tracking list by track code, descending.
/*!
\param handle
    A handle to the BAG.
    Cannot be NULL.

\return
    0 if successful.
    An error code otherwise.
*/
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

