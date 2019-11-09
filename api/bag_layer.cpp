
#include "bag_layer.h"
#include "bag_metadata.h"
#include "bag_private.h"
#include "bag_trackinglist.h"

#include <array>
#include <h5cpp.h>


namespace BAG {

//TODO Update to only work with an existing ::H5::DataSet.
//TODO Maybe make this a static open().
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
        return {MIN_ELEVATION_NAME, MAX_ELEVATION_NAME, ELEVATION_PATH,
            ::H5::PredType::NATIVE_FLOAT};
    case Uncertainty:
        return {MIN_UNCERTAINTY_NAME, MAX_UNCERTAINTY_NAME, UNCERTAINTY_PATH,
            ::H5::PredType::NATIVE_FLOAT};
    case Hypothesis_Strength:
        return{MIN_HYPOTHESIS_STRENGTH, MAX_HYPOTHESIS_STRENGTH,
            HYPOTHESIS_STRENGTH_PATH, ::H5::PredType::NATIVE_FLOAT};
    case Num_Hypotheses:
        return {MIN_NUM_HYPOTHESES, MAX_NUM_HYPOTHESES, NUM_HYPOTHESES_PATH,
            ::H5::PredType::NATIVE_UINT32};
    case Shoal_Elevation:
        return {MIN_SHOAL_ELEVATION, MAX_SHOAL_ELEVATION, SHOAL_ELEVATION_PATH,
            ::H5::PredType::NATIVE_FLOAT};
    case Std_Dev:
        return {MIN_STANDARD_DEV_NAME, MAX_STANDARD_DEV_NAME, STANDARD_DEV_PATH,
            ::H5::PredType::NATIVE_FLOAT};
    case Num_Soundings:
        return {MIN_NUM_SOUNDINGS, MAX_NUM_SOUNDINGS, NUM_SOUNDINGS_PATH,
            ::H5::PredType::NATIVE_UINT32};
    case Average_Elevation:
        return {MIN_AVERAGE, MAX_AVERAGE, AVERAGE_PATH,
            ::H5::PredType::NATIVE_FLOAT};
    case Nominal_Elevation:
        return {MIN_NOMINAL_ELEVATION, MAX_NOMINAL_ELEVATION,
            NOMINAL_ELEVATION_PATH, ::H5::PredType::NATIVE_FLOAT};
    case Surface_Correction:  //[[fallthrough]];
    case Compound:  //[[fallthrough]];
    default:
        throw UnknownSimpleLayerType{};
    }
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
    case COMPOUND:
        throw 4321;  //TODO How to deal with this?
    default:
        return 0;
    }
}

std::string Layer::getInternalPath(
    LayerType layerType,
    GroupType groupType)
{
    if (groupType == NODE)
        return NODE_GROUP_PATH;
    else if (groupType == ELEVATION)
        return ELEVATION_SOLUTION_GROUP_PATH;
    else if (groupType != UNKNOWN_GROUP_TYPE)
        throw UnsupportedGroupType{};

    switch (layerType)
    {
    case Elevation:
        return ELEVATION_PATH;
    case Uncertainty:
        return UNCERTAINTY_PATH;
    case Hypothesis_Strength:
        return HYPOTHESIS_STRENGTH_PATH;  // Also part of the NODE (NODE_GROUP_PATH).
    case Num_Hypotheses:
        return NUM_HYPOTHESES_PATH;  // Also part of the NODE group (NODE_GROUP_PATH).
    case Shoal_Elevation:
        return SHOAL_ELEVATION_PATH;  // Also part of the ELEVATION group (ELEVATION_SOLUTION_GROUP_PATH).
    case Std_Dev:
        return STANDARD_DEV_PATH;  // Also part of the ELEVATION group (ELEVATION_SOLUTION_GROUP_PATH).
    case Num_Soundings:
        return NUM_SOUNDINGS_PATH;  // Also part of the ELEVATION group (ELEVATION_SOLUTION_GROUP_PATH).
    case Average_Elevation:
        return AVERAGE_PATH;
    case Nominal_Elevation:
        return NOMINAL_ELEVATION_PATH;
    case Surface_Correction:
        return VERT_DATUM_CORR_PATH;
    case Compound:  // [[fallthrough]];
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
        throw 42;  // starting row or column is after the end.

    if (m_pBagDataset.expired())
        throw 23;  // No dataset to read from.

    uint32_t numRows = 0, numColumns = 0;
    std::tie(numRows, numColumns) = this->getDescriptor().getDims();

    if (columnEnd >= numColumns || rowEnd >= numRows)
        throw 43;  // requesting more columns or rows than the data has

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
        throw 32;  //TODO make an exception

    if (!buffer)
        throw 33;  //TODO make an exception

    this->writeProxy(rowStart, columnStart, rowEnd, columnEnd, buffer);
}

}   //namespace BAG

