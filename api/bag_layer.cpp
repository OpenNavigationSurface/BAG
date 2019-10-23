
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
    const LayerDescriptor& descriptor)
    : m_pBagDataset(dataset.shared_from_this())
    , m_pLayerDescriptor(descriptor.shared_from_this())
{
}


constexpr DataType Layer::getDataType(LayerType layerType) noexcept
{
    switch (layerType)
    {
    case Elevation:
    case Uncertainty:
    case Hypothesis_Strength:
    case Shoal_Elevation:
    case Std_Dev:
    case Average_Elevation:
    case Nominal_Elevation:
        return FLOAT32;

    case Num_Hypotheses:
    case Num_Soundings:
        return UINT32;
    }

    return UNKNOWN_DATA_TYPE;
}

const LayerDescriptor& Layer::getDescriptor() const & noexcept
{
    return *m_pLayerDescriptor;
}

constexpr uint8_t Layer::getElementSize(DataType type) noexcept
{
    switch (type)
    {
    case FLOAT32:
        return sizeof(float);

    case UINT32:
        return sizeof(uint32_t);
    }

    return 0;
}

std::string Layer::getInternalPath(LayerType type)
{
    switch (type)
    {
    case Elevation:
        return ELEVATION_PATH;

    case Uncertainty:
        return UNCERTAINTY_PATH;

    case Hypothesis_Strength:
        return NODE_GROUP_PATH;  // Part of the NODE group.

    case Num_Hypotheses:
        return NODE_GROUP_PATH;  //TODO NUM_HYPOTHESES_PATH could be used as a SimpleLayer.

    case Shoal_Elevation:
        return ELEVATION_SOLUTION_GROUP_PATH;  // Part of the ELEVATION group.

    case Std_Dev:
        return ELEVATION_SOLUTION_GROUP_PATH;  //TODO STANDARD_DEV_PATH could be used as a SimpleLayer.

    case Num_Soundings:
        return ELEVATION_SOLUTION_GROUP_PATH;  // Part of the ELEVATION group.

    case Average_Elevation:
        return AVERAGE_PATH;

    case Nominal_Elevation:
        return NOMINAL_ELEVATION_PATH;
    }

    throw 18181;  // Unknown layer type specified.
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
    const uint8_t* buffer) const
{
    if (rowStart > rowEnd || columnStart > columnEnd)
        throw 32;  //TODO make an exception

    if (!buffer)
        throw 33;  //TODO make an exception

    //TODO Check if buffer being written is larger than current?

    this->writeProxy(rowStart, columnStart, rowEnd, columnEnd, buffer);
}

}   //namespace BAG

