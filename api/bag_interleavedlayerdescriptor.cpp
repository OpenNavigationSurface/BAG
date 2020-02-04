
#include "bag_exceptions.h"
#include "bag_interleavedlayer.h"
#include "bag_interleavedlayerdescriptor.h"
#include "bag_private.h"


namespace BAG {

InterleavedLayerDescriptor::InterleavedLayerDescriptor(
    uint32_t id,
    LayerType layerType,
    GroupType groupType,
    uint64_t chunkSize,
    unsigned int compressionLevel)
    : LayerDescriptor(id, Layer::getInternalPath(layerType),
        kLayerTypeMapString.at(layerType), layerType, chunkSize,
        compressionLevel)
    , m_groupType(groupType)
    , m_dataType(Layer::getDataType(layerType))
    , m_elementSize(Layer::getElementSize(Layer::getDataType(layerType)))
{
    this->setInternalPath(groupType == NODE ?
        NODE_GROUP_PATH : ELEVATION_SOLUTION_GROUP_PATH);

    this->validateTypes(layerType, groupType);
}

InterleavedLayerDescriptor::InterleavedLayerDescriptor(
    LayerType layerType,
    GroupType groupType,
    const Dataset& dataset)
    : LayerDescriptor(layerType, dataset,
        groupType == NODE
            ? NODE_GROUP_PATH
            : groupType == ELEVATION
                ? ELEVATION_SOLUTION_GROUP_PATH
                : "")
    , m_groupType(groupType)
    , m_dataType(Layer::getDataType(layerType))
    , m_elementSize(Layer::getElementSize(Layer::getDataType(layerType)))
{
    this->validateTypes(layerType, groupType);
}

std::shared_ptr<InterleavedLayerDescriptor> InterleavedLayerDescriptor::create(
    LayerType layerType,
    GroupType groupType,
    uint64_t chunkSize,
    unsigned int compressionLevel,
    const Dataset& dataset)
{
    return std::shared_ptr<InterleavedLayerDescriptor>(
        new InterleavedLayerDescriptor{dataset.getNextId(), layerType,
            groupType, chunkSize, compressionLevel});
}

std::shared_ptr<InterleavedLayerDescriptor> InterleavedLayerDescriptor::open(
    LayerType layerType,
    GroupType groupType,
    const Dataset& dataset)
{
    return std::shared_ptr<InterleavedLayerDescriptor>(
        new InterleavedLayerDescriptor{layerType, groupType, dataset});
}


DataType InterleavedLayerDescriptor::getDataTypeProxy() const noexcept
{
    return m_dataType;
}

uint8_t InterleavedLayerDescriptor::getElementSizeProxy() const noexcept
{
    return m_elementSize;
}

GroupType InterleavedLayerDescriptor::getGroupType() const noexcept
{
    return m_groupType;
}

//! Verify the proper group and layer combination is given.
void InterleavedLayerDescriptor::validateTypes(
    LayerType layerType,
    GroupType groupType) const
{
    if (groupType == ELEVATION)
    {
        if (layerType != Shoal_Elevation && layerType != Std_Dev &&
            layerType != Num_Soundings)
            throw UnsupportedInterleavedLayer{};
    }
    else if (groupType == NODE)
    {
        if (layerType != Hypothesis_Strength && layerType != Num_Hypotheses)
            throw UnsupportedInterleavedLayer{};
    }
    else
        throw UnsupportedInterleavedLayer{};
}

}  // namespace BAG

