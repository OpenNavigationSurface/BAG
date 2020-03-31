
#include "bag_layer.h"
#include "bag_metadata.h"
#include "bag_private.h"
#include "bag_trackinglist.h"

#include <array>

namespace BAG {

//! Constructor.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
\param descriptor
    The descriptor of this layer.
*/
Layer::Layer(
    Dataset& dataset,
    LayerDescriptor& descriptor)
    : m_pBagDataset(dataset.shared_from_this())
    , m_pLayerDescriptor(descriptor.shared_from_this())
{
}

//! Retrieve the BAG Dataset this layer belongs to.
/*!
\return
    The BAG Dataset this layer belongs to.
*/
std::weak_ptr<Dataset> Layer::getDataset() & noexcept
{
    return m_pBagDataset;
}

//! Retrieve the BAG Dataset this layer belongs to.
/*!
\return
    The BAG Dataset this layer belongs to.
*/
std::weak_ptr<const Dataset> Layer::getDataset() const & noexcept
{
    return m_pBagDataset;
}

//! Determine the type of data stored in the specified layer type.
/*!
\return
    The type of data stored in the specified layer type.
*/
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
        return DT_FLOAT32;
    case Num_Hypotheses:  //[[fallthrough]];
    case Num_Soundings:
        return DT_UINT32;
    case Surface_Correction:
    case Compound:  //[[fallthrough]];
        return DT_COMPOUND;
    default:
        return DT_UNKNOWN_DATA_TYPE;
    }
}

//! Retrieve the layer's descriptor.
/*!
\return
    The layer's descriptor.
*/
LayerDescriptor& Layer::getDescriptor() & noexcept
{
    return *m_pLayerDescriptor;
}

//! Retrieve the layer's descriptor.
/*!
\return
    The layer's descriptor.
*/
const LayerDescriptor& Layer::getDescriptor() const & noexcept
{
    return *m_pLayerDescriptor;
}

//! Retrieve the size of the specified data type.
/*!
\param
    The type of data.

\return
    The size of the specified data type.
*/
uint8_t Layer::getElementSize(DataType type)
{
    switch (type)
    {
    case DT_FLOAT32:
        return sizeof(float);
    case DT_UINT32:
        return sizeof(uint32_t);
    case DT_UINT8:
        return sizeof(uint8_t);
    case DT_UINT16:
        return sizeof(uint16_t);
    case DT_UINT64:
        return sizeof(uint64_t);
    case DT_BOOLEAN:
        return sizeof(bool);
    case DT_STRING:
        return sizeof(char*);
    case DT_COMPOUND:  //[[fallthrough]]
    case DT_UNKNOWN_DATA_TYPE:  //[[fallthrough]]
    default:
        throw UnsupportedElementSize{};
    }
}

//! Retrieve the HDF5 path of the specified layer.
/*!
\param layerType
    The type of layer.
\param groupType
    The type of group.
    NODE or ELEVATION.

\return
    The HDF5 path to the specified layer and group types.
*/
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

//! Read a section of data from this layer.
/*!
    Read data from this layer starting at rowStart, columnStart, and continue
    until rowEnd, columnEnd (inclusive).

\param rowStart
    The starting row.
\param columnStart
    The starting column.
\param rowEnd
    The ending row (inclusive).
\param columnEnd
    The ending column (inclusive).

\return
    The section of data specified by the rows and columns.
*/
std::unique_ptr<UInt8Array> Layer::read(
    uint32_t rowStart,
    uint32_t columnStart,
    uint32_t rowEnd,
    uint32_t columnEnd) const
{
    if (rowStart > rowEnd || columnStart > columnEnd)
        throw InvalidReadSize{};

    if (m_pBagDataset.expired())
        throw DatasetNotFound{};

    const auto pDataset = m_pBagDataset.lock();
    uint32_t numRows = 0, numColumns = 0;
    std::tie(numRows, numColumns) = pDataset->getDescriptor().getDims();

    if (columnEnd >= numColumns || rowEnd >= numRows)
        throw InvalidReadSize{};

    return this->readProxy(rowStart, columnStart, rowEnd, columnEnd);
}

//! Write a section of data to this layer.
/*!
    Write data to this layer starting at rowStart, columnStart, and continue
    until rowEnd, columnEnd (inclusive).

    After writing the data, write the attributes.

\param rowStart
    The starting row.
\param columnStart
    The starting column.
\param rowEnd
    The ending row (inclusive).
\param columnEnd
    The ending column (inclusive).
\param buffer
    The data to be written.
    It must contain at least (rowEnd - rowStart + 1) * (columnEnd - columnStart + 1) elements!
*/
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

//! Write the attributes this layer contains to disk.
void Layer::writeAttributes() const
{
    if (m_pBagDataset.expired())
        throw DatasetNotFound{};

    this->writeAttributesProxy();
}

}  // namespace BAG

