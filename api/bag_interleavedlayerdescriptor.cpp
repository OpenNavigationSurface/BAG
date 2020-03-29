
#include "bag_exceptions.h"
#include "bag_interleavedlayer.h"
#include "bag_interleavedlayerdescriptor.h"
#include "bag_private.h"


namespace BAG {

InterleavedLayerDescriptor::InterleavedLayerDescriptor(
    uint32_t id,
    LayerType layerType,
    GroupType groupType)
    : LayerDescriptor(id, Layer::getInternalPath(layerType),
        kLayerTypeMapString.at(layerType), layerType, 0, 0)
    , m_groupType(groupType)
    , m_elementSize(Layer::getElementSize(Layer::getDataType(layerType)))
{
    this->setInternalPath(groupType == NODE ?
        NODE_GROUP_PATH : ELEVATION_SOLUTION_GROUP_PATH);

    InterleavedLayerDescriptor::validateTypes(layerType, groupType);
}

InterleavedLayerDescriptor::InterleavedLayerDescriptor(
    const Dataset& dataset,
    LayerType layerType,
    GroupType groupType)
    : LayerDescriptor(dataset, layerType,
        groupType == NODE
            ? NODE_GROUP_PATH
            : groupType == ELEVATION
                ? ELEVATION_SOLUTION_GROUP_PATH
                : "")
    , m_groupType(groupType)
    , m_elementSize(Layer::getElementSize(Layer::getDataType(layerType)))
{
    InterleavedLayerDescriptor::validateTypes(layerType, groupType);
}

std::shared_ptr<InterleavedLayerDescriptor> InterleavedLayerDescriptor::create(
    const Dataset& dataset,
    LayerType layerType,
    GroupType groupType)
{
    return std::shared_ptr<InterleavedLayerDescriptor>(
        new InterleavedLayerDescriptor{dataset.getNextId(), layerType,
            groupType});
}

std::shared_ptr<InterleavedLayerDescriptor> InterleavedLayerDescriptor::open(
    const Dataset& dataset,
    LayerType layerType,
    GroupType groupType)
{
    return std::shared_ptr<InterleavedLayerDescriptor>(
        new InterleavedLayerDescriptor{dataset, layerType, groupType});
}


DataType InterleavedLayerDescriptor::getDataTypeProxy() const noexcept
{
    return Layer::getDataType(this->getLayerType());
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
    GroupType groupType)
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

