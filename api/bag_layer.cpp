
#include "bag_layer.h"
#include "bag_metadata.h"
#include "bag_private.h"
#include "bag_trackinglist.h"

#include <array>
#include <H5Cpp.h>


namespace BAG {

Layer::Layer(
    Dataset& dataset,
    LayerDescriptor& descriptor)
    : m_pBagDataset(dataset.shared_from_this())
    , m_pLayerDescriptor(descriptor.shared_from_this())
{
}


Layer::AttributeInfo Layer::getAttributeInfo(LayerType layerType)
{
    switch (layerType)
    {
    case Elevation:
        return AttributeInfo(MIN_ELEVATION_NAME, MAX_ELEVATION_NAME,
            ELEVATION_PATH, ::H5::PredType::NATIVE_FLOAT);
    case Uncertainty:
        return AttributeInfo(MIN_UNCERTAINTY_NAME, MAX_UNCERTAINTY_NAME,
            UNCERTAINTY_PATH, ::H5::PredType::NATIVE_FLOAT);
    case Hypothesis_Strength:
        return AttributeInfo(MIN_HYPOTHESIS_STRENGTH, MAX_HYPOTHESIS_STRENGTH,
            HYPOTHESIS_STRENGTH_PATH, ::H5::PredType::NATIVE_FLOAT);
    case Num_Hypotheses:
        return AttributeInfo(MIN_NUM_HYPOTHESES, MAX_NUM_HYPOTHESES,
            NUM_HYPOTHESES_PATH, ::H5::PredType::NATIVE_UINT32);
    case Shoal_Elevation:
        return AttributeInfo(MIN_SHOAL_ELEVATION, MAX_SHOAL_ELEVATION,
            SHOAL_ELEVATION_PATH, ::H5::PredType::NATIVE_FLOAT);
    case Std_Dev:
        return AttributeInfo(MIN_STANDARD_DEV_NAME, MAX_STANDARD_DEV_NAME,
            STANDARD_DEV_PATH, ::H5::PredType::NATIVE_FLOAT);
    case Num_Soundings:
        return AttributeInfo(MIN_NUM_SOUNDINGS, MAX_NUM_SOUNDINGS,
            NUM_SOUNDINGS_PATH, ::H5::PredType::NATIVE_UINT32);
    case Average_Elevation:
        return AttributeInfo(MIN_AVERAGE, MAX_AVERAGE, AVERAGE_PATH,
            ::H5::PredType::NATIVE_FLOAT);
    case Nominal_Elevation:
        return AttributeInfo(MIN_NOMINAL_ELEVATION, MAX_NOMINAL_ELEVATION,
            NOMINAL_ELEVATION_PATH, ::H5::PredType::NATIVE_FLOAT);
    case Surface_Correction:  //[[fallthrough]];
    case Compound:  //[[fallthrough]];
    default:
        throw UnknownSimpleLayerType{};
    }
}

std::weak_ptr<Dataset> Layer::getDataset() & noexcept
{
    return m_pBagDataset;
}

DataType Layer::getDataType(LayerType layerType) noexcept
{
    switch (layerType)
    {
    case Elevation:  //[[fallthrough]];
    case Uncertainty:  //[[fallthrough]];
    case Hypothesis_Strength:  //[[fallthrough]];
    case Shoal_Elevation:  //[[fallthrough]];
    case Std_Dev:  //[[fallthrough]];
    case Average_Elevation:  //[[fallthrough]];
    case Nominal_Elevation:
        return FLOAT32;
    case Num_Hypotheses:  //[[fallthrough]];
    case Num_Soundings:
        return UINT32;
    case Surface_Correction:
    case Compound:  //[[fallthrough]];
        return COMPOUND;
    default:
        return UNKNOWN_DATA_TYPE;
    }
}

LayerDescriptor& Layer::getDescriptor() & noexcept
{
    return *m_pLayerDescriptor;
}

const LayerDescriptor& Layer::getDescriptor() const & noexcept
{
    return *m_pLayerDescriptor;
}

uint8_t Layer::getElementSize(DataType type)
{
    switch (type)
    {
    case FLOAT32:
        return sizeof(float);
    case UINT32:
        return sizeof(uint32_t);
    case UINT8:
        return sizeof(uint8_t);
    case UINT16:
        return sizeof(uint16_t);
    case UINT64:
        return sizeof(uint64_t);
    case BOOL:
        return sizeof(bool);
    case STRING:
        return sizeof(char*);
    case COMPOUND:  //[[fallthrough]]
    case UNKNOWN_DATA_TYPE:  //[[fallthrough]]
    default:
        throw UnsupportedElementSize{};
    }
}

std::string Layer::getInternalPath(
    LayerType layerType,
    GroupType groupType)
{
    if (groupType == NODE)
        return NODE_GROUP_PATH;

    if (groupType == ELEVATION)
        return ELEVATION_SOLUTION_GROUP_PATH;

    if (groupType != UNKNOWN_GROUP_TYPE)
        throw UnsupportedGroupType{};

    switch (layerType)
    {
    case Elevation:
        return ELEVATION_PATH;
    case Uncertainty:
        return UNCERTAINTY_PATH;
    case Hypothesis_Strength:
        return HYPOTHESIS_STRENGTH_PATH;
    case Num_Hypotheses:
        return NUM_HYPOTHESES_PATH;
    case Shoal_Elevation:
        return SHOAL_ELEVATION_PATH;
    case Std_Dev:
        return STANDARD_DEV_PATH;
    case Num_Soundings:
        return NUM_SOUNDINGS_PATH;
    case Average_Elevation:
        return AVERAGE_PATH;
    case Nominal_Elevation:
        return NOMINAL_ELEVATION_PATH;
    case Surface_Correction:
        return VERT_DATUM_CORR_PATH;
    case Compound:  // [[fallthrough]];
    case UNKNOWN_LAYER_TYPE:  // [[fallthrough]];
    default:
        throw UnsupportedLayerType{};
    }
}

std::unique_ptr<uint8_t[]> Layer::read(
    uint32_t rowStart,
    uint32_t columnStart,
    uint32_t rowEnd,
    uint32_t columnEnd) const
{
    if (rowStart > rowEnd || columnStart > columnEnd)
        InvalidReadSize{};

    if (m_pBagDataset.expired())
        throw DatasetNotFound{};

    const auto pDataset = m_pBagDataset.lock();
    uint32_t numRows = 0, numColumns = 0;
    std::tie(numRows, numColumns) = pDataset->getDescriptor().getDims();

    if (columnEnd >= numColumns || rowEnd >= numRows)
        throw InvalidReadSize{};

    return this->readProxy(rowStart, columnStart, rowEnd, columnEnd);
}

void Layer::write(
    uint32_t rowStart,
    uint32_t columnStart,
    uint32_t rowEnd,
    uint32_t columnEnd,
    const uint8_t* buffer)
{
    if (m_pBagDataset.expired())
        throw DatasetNotFound{};

    if (rowStart > rowEnd || columnStart > columnEnd)
        throw InvalidWriteSize{};

    if (!buffer)
        throw InvalidBuffer{};

    this->writeProxy(rowStart, columnStart, rowEnd, columnEnd, buffer);
    this->writeAttributesProxy();
}

void Layer::writeAttributes() const
{
    if (m_pBagDataset.expired())
        throw DatasetNotFound{};

    this->writeAttributesProxy();
}

}  // namespace BAG

