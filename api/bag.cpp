#include "bag.h"
#include "bag_c_types.h"
#include "bag_dataset.h"
#include "bag_layer.h"
#include "bag_metadata.h"
#include "bag_private.h"
#include "bag_trackinglist.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

#include <h5cpp.h>

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
    CS_THROW_ASSERT(out_raster);
    *out_raster = NULL;
    //..
    caris_raster * raster = new caris_raster();
    //..
    *out_raster = raster;
#endif

BagError bagFileOpen(
    BagHandle** handle,
    BAG_OPEN_MODE accessMode,
    const char* fileName)
{
    if (!handle)
        return 9999; // No handle provided.

    *handle = new BagHandle{};

    (*handle)->pDataset = BAG::Dataset::open(std::string{fileName}, accessMode);

    return 0;
}

BagError bagFileClose(BagHandle* handle)
{
    if (!handle)
        return 9999; // No handle provided.

#ifndef NDEBUG
    if (handle->pDataset.use_count() > 1)
        return 9998; // More than one reference to the dataset still.
#endif

    handle->pDataset.reset();

    return 0;
}

BagError bagCreateFromFile(
    const char* file_name,
    const char* metaDataFile,
    BagHandle* handle)
{
    if (!handle)
        return 9999; // No handle provided.

    BAG::Metadata metadata;
    metadata.loadFromFile(std::string{metaDataFile});

    //TODO Where do these values come from?
    constexpr uint64_t chunkSize = 100;
    constexpr unsigned int compressionLevel = 6;

    handle->pDataset = BAG::Dataset::create(std::string{file_name},
        std::move(metadata), chunkSize, compressionLevel);

    return 0;
}

BagError bagCreateFromBuffer(
    const char* file_name,
    const char* metaDataBuffer,
    size_t metaDataBufferSize,
    BagHandle* handle)
{
    if (!handle)
        return 9999; // No handle provided.

    BAG::Metadata metadata;
    metadata.loadFromBuffer(std::string{metaDataBuffer, metaDataBufferSize});

    //TODO Where do these values come from?
    constexpr uint64_t chunkSize = 100;
    constexpr unsigned int compressionLevel = 6;

    handle->pDataset = BAG::Dataset::create(std::string{file_name},
        std::move(metadata), chunkSize, compressionLevel);

    return 0;
}

//TODO maybe change this to bagCreateSimpleLevel (will have to make SurfaceCorrections and CompoundLayer versions)
BagError bagCreateLayer(
    BagHandle* handle,
    BAG_LAYER_TYPE type)
{
    if (!handle)
        return 9999; // No handle provided.

    //TODO Where do these values come from?
    constexpr uint64_t chunkSize = 100;
    constexpr unsigned int compressionLevel = 6;

    handle->pDataset->createLayer(type, chunkSize, compressionLevel);

    return 0;
}

BagError bagGetGridDimensions(
    BagHandle* handle,
    uint32_t* rows,
    uint32_t* cols)
{
    if (!handle)
        return 9999; // No handle provided.

    std::tie(*rows, *cols) =
        handle->pDataset->getDescriptor().getDims();

    return 0;
}

BagError bagGetNodeSpacing(
    BagHandle* handle,
    double* /*xSpacing*/,
    double* /*ySpacing*/)
{
    if (!handle)
        return 9999; // No handle provided.

    const auto& metadata = handle->pDataset->getMetadata();
    metadata;

    //TODO Implement.

    return 0;
}

BagError bagGetGeoCover(
    BagHandle* handle,
    double* /*llx*/,
    double* /*lly*/,
    double* /*urx*/,
    double* /*ury*/)
{
    if (!handle)
        return 9999; // No handle provided.

    const auto& metadata = handle->pDataset->getMetadata();
    metadata;  // llCornerX(), llCornerY(), urCornerX(), urCornerY(), rowResolution(), columnResolution()

    //TODO Implement.

    return 0;
}

const BagMetadata* bagGetMetaData(BagHandle* handle)
{
    if (!handle)
        return nullptr; // No handle provided.

    return &handle->pDataset->getMetadata().getStruct();
}

BagError bagGetMinMax(
    BagHandle* handle,
    BAG_LAYER_TYPE type,
    double* minValue,
    double* maxValue)
{
    if (!handle)
        return 9999; // No handle provided.

    const auto& layer = handle->pDataset->getLayer(type);
    std::tie(*minValue, *maxValue) = layer.getDescriptor().getMinMax();

    return 0;
}

BagError bagSetMinMax(
    BagHandle* handle,
    BAG_LAYER_TYPE /*type*/,
    double* /*minValue*/,
    double* /*maxValue*/)
{
    if (!handle)
        return 9999; // No handle provided.

    //TODO Implement.
    return 0;
}

BagError bagGetNumLayers(
    BagHandle* handle,
    uint32_t* /*numLayers*/)
{
    if (!handle)
        return 9999; // No handle provided.

    //TODO Implement.
    return 0;
}

bool bagContainsLayer(
    BagHandle* handle,
    BAG_LAYER_TYPE type)
{
    if (!handle)
        return false; // No handle provided.

    try
    {
        handle->pDataset->getLayer(type);
    }
    catch (...)
    {
        return false;
    }

    return true;
}

BagError bagRead(
    BagHandle* handle,
    uint32_t start_row,
    uint32_t start_col,
    uint32_t end_row,
    uint32_t end_col,
    BAG_LAYER_TYPE type,
    uint8_t** data,
    double* x,
    double* y)
{
    if (!handle)
        return 9999; // No handle provided.

    if (!data)
        return 9997; // No output buffer provided.

    const auto& layer = handle->pDataset->getLayer(type);
    auto buffer = layer.read(start_row, start_col, end_row, end_col);

    //TODO Implement.

    x;
    y;

    *data = buffer.release();

    return 0;
}

BagError bagWrite(
    BagHandle* handle,
    uint32_t start_row,
    uint32_t start_col,
    uint32_t end_row,
    uint32_t end_col,
    BAG_LAYER_TYPE type,
    const uint8_t* data)
{
    if (!handle)
        return 9999; // No handle provided.

    auto& layer = handle->pDataset->getLayer(type);

    layer.write(start_row, start_col, end_row, end_col, data);

    return 0;
}

BagError bagGetErrorString(
    BagError /*code*/,
    uint8_t** /*error*/)
{
    //TODO Implement.
    return 0;
}

BagError bagComputePostion(
    BagHandle* handle,
    uint32_t row,
    uint32_t col,
    double* x,
    double* y)
{
    if (!handle)
        return 9999; // No handle provided.

    std::tie(*x, *y) = handle->pDataset->gridToGeo(row, col);

    return 0;
}

void* bagGetNullValue(BAG_LAYER_TYPE type)
{
    //TODO Implement.
    switch (type)
    {
        case Elevation:
        case Uncertainty:
        case Hypothesis_Strength:
        case Shoal_Elevation:
        case Std_Dev:
        case Average_Elevation:
        case Nominal_Elevation:
            return nullptr;

        case Num_Hypotheses:
        case Num_Soundings:
            return nullptr;
    }

    return nullptr;
}

BagError bagComputeIndex(
    BagHandle* handle,
    double x,
    double y,
    uint32_t* row,
    uint32_t* col)
{
    if (!handle)
        return 9999; // No handle provided.

    std::tie(*row, *col) = handle->pDataset->geoToGrid(x, y);

    return 0;
}

BagError bagSetCompressionLevel(uint8_t /*level*/)
{
    //TODO Implement.
    return 0;
}

uint8_t* bagAllocateBuffer(
    uint32_t start_row,
    uint32_t start_col,
    uint32_t end_row,
    uint32_t end_col,
    BAG_LAYER_TYPE type)
{
    int8_t elementSize = 0;

    switch (type)
    {
        case Elevation:
        case Uncertainty:
        case Hypothesis_Strength:
        case Shoal_Elevation:
        case Std_Dev:
        case Average_Elevation:
        case Nominal_Elevation:
            elementSize = sizeof(float);
            break;

        case Num_Hypotheses:
        case Num_Soundings:
            elementSize = sizeof(uint32_t);
            break;
        default:
            return nullptr;
    }

    const auto numRows = (end_row - start_row) + 1;
    const auto numCols = (end_col - start_col) + 1;

    //TODO Implement. calloc ? (if not, why is calloc used everywhere??)
    return new uint8_t[numRows * numCols * elementSize];
}

uint8_t* bagAllocate(uint32_t numBytes)
{
    return new uint8_t[numBytes];
}

BagError bagFree(uint8_t* buffer)
{
    delete[] buffer;
    return 0;
}

BagError bagReadCorrectorVerticalDatum(
    BagHandle* handle,
    uint32_t,
    uint8_t* /*datum*/)
{
    if (!handle)
        return 9999; // No handle provided.

    //TODO Implement.
    return 0;
}

BagError bagWriteCorrectorVerticalDatum(
    BagHandle* handle,
    uint32_t,
    uint8_t* /*datum*/)
{
    if (!handle)
        return 9999; // No handle provided.

    //TODO Implement.
    return 0;
}

BagError bagReadCorrectedDataset(
    BagHandle* handle,
    uint32_t /*corrIndex*/,
    uint32_t /*surfIndex*/,
    float* /*data*/)
{
    if (!handle)
        return 9999; // No handle provided.

    //TODO Implement.
    return 0;
}

BagError bagReadCorrectedRegion(
    BagHandle* handle,
    uint32_t /*startrow*/,
    uint32_t /*endrow*/,
    uint32_t /*startcol*/,
    uint32_t /*endcol*/,
    uint32_t /*corrIndex*/,
    uint32_t /*surfIndex*/,
    float* /*data*/)
{
    if (!handle)
        return 9999; // No handle provided.

    //TODO Implement.
    return 0;
}

BagError bagReadCorrectedRow(
    BagHandle* handle,
    uint32_t /*row*/,
    uint32_t /*corrIndex*/,
    uint32_t /*surfIndex*/,
    float* /*data*/)
{
    if (!handle)
        return 9999; // No handle provided.

    //TODO Implement.
    return 0;
}

BagError bagReadCorrectedNode(
    BagHandle* handle,
    uint32_t /*row*/,
    uint32_t /*col*/,
    uint32_t /*corrIndex*/,
    uint32_t /*surfIndex*/,
    float* /*data*/)
{
    if (!handle)
        return 9999; // No handle provided.

    //TODO Implement.
    return 0;
}

BagError bagGetNumSurfaceCorrectors(
    BagHandle* handle,
    uint32_t* /*num*/)
{
    if (!handle)
        return 9999; // No handle provided.

    //TODO Implement.
    return 0;
}

BagError bagGetSurfaceCorrectionTopography(
    BagHandle* handle,
    uint8_t* /*type*/)
{
    if (!handle)
        return 9999; // No handle provided.

    //TODO Implement.
    return 0;
}

BagError bagCreateCorrectorDataset(
    BagHandle* handle,
    uint32_t /*numCorrectors*/,
    uint8_t /*type*/)
{
    if (!handle)
        return 9999; // No handle provided.

    //TODO Implement.
    return 0;
}

/*
BagError bagWriteCorrectorDefinition(
    BagHandle* handle,
    bagVerticalCorrectorDef* def)
{
    if (!handle)
        return 9999; // No handle provided.

    return 0;
}

BagError bagReadCorrectorDefinition(
    BagHandle* handle,
    bagVerticalCorrectorDef* def)
{
    if (!handle)
        return 9999; // No handle provided.

    return 0;
}
*/

BagError bagTrackingListLength(
    BagHandle* handle,
    uint32_t* /*len*/)
{
    if (!handle)
        return 9999; // No handle provided.

    //TODO Implement.
    return 0;
}

BagError bagReadTrackingListNode(
    BagHandle* handle,
    uint32_t /*row*/,
    uint32_t /*col*/,
    BagTrackingItem** /*items*/,
    uint32_t* /*length*/)
{
    if (!handle)
        return 9999; // No handle provided.

    //TODO Implement.
    return 0;
}

BagError bagReadTrackingListCode(
    BagHandle* handle,
    uint8_t /*code*/,
    BagTrackingItem** /*items*/,
    uint32_t* /*length*/)
{
    if (!handle)
        return 9999; // No handle provided.

    //TODO Implement.
    return 0;
}

BagError bagReadTrackingListSeries(
    BagHandle* handle,
    uint16_t /*index*/,
    BagTrackingItem** /*items*/,
    uint32_t* /*length*/)
{
    if (!handle)
        return 9999; // No handle provided.

    //TODO Implement.
    return 0;
}

BagError bagWriteTrackingListItem(
    BagHandle* handle,
    BagTrackingItem* /*item*/)
{
    if (!handle)
        return 9999; // No handle provided.

    //TODO Implement.
    return 0;
}

BagError bagSortTrackingListByNode(BagHandle* handle)
{
    if (!handle)
        return 9999; // No handle provided.

    //TODO Implement.
    return 0;
}

BagError bagSortTrackingListBySeries(BagHandle* handle)
{
    if (!handle)
        return 9999; // No handle provided.

    //TODO Implement.
    return 0;
}

BagError bagSortTrackingListByCode(BagHandle* handle)
{
    if (!handle)
        return 9999; // No handle provided.

    //TODO Implement.
    return 0;
}

/*
uint8_t* bagComputeMessageDigest(
    const char* file,
    uint32_t signatureID,
    uint32_t* nBytes)
{
    return NULL;
}

uint8_t* bagSignMessageDigest(
    uint8_t* md,
    uint32_t mdLen,
    uint8_t* secKey,
    BagError* errcode)
{
    return NULL;
}

BagError bagReadCertification(
    const char* file,
    uint8_t* sig,
    uint32_t nBuffer,
    uint32_t* sigID)
{
    return 0;
}

BagError bagWriteCertification(
    const char* file,
    uint8_t* sig,
    uint32_t sigID)
{
    return 0;
}

bool bagVerifyCertification(
    uint8_t* sig,
    uint8_t* pubKey,
    uint8_t* md,
    uint32_t mdLen)
{
    return false;
}

uint8_t* bagComputeFileSignature(
    const char* name,
    uint32_t sigID,
    uint8_t* secKey)
{
    return NULL;
}

bool bagSignFile(
    const char* name,
    uint8_t* secKey,
    uint32_t sigID)
{
    return false;
}

bool bagVerifyFile(
    const char* name,
    uint8_t* pubKey,
    uint32_t sigID)
{
    return false;
}

BagError bagGenerateKeyPair(
    uint8_t** pubKey,
    uint8_t** secKey)
{
    return 0;
}

BagError bagConvertCryptoFormat(
    uint8_t* object,
    bagCryptoObject objType,
    bagConvDir convDir,
    uint8_t** converted)
{
    return 0;
}
*/

