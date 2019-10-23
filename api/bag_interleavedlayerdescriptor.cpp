
#include "bag_exceptions.h"
#include "bag_interleavedlayer.h"
#include "bag_interleavedlayerdescriptor.h"
#include "bag_private.h"


namespace BAG {

InterleavedLayerDescriptor::InterleavedLayerDescriptor(
    LayerType layerType,
    GroupType groupType)
    : LayerDescriptor(layerType)
    , m_groupType(groupType)
    , m_elementSize(Layer::getElementSize(Layer::getDataType(layerType)))
{
    this->setInternalPath(groupType == NODE ?
        NODE_GROUP_PATH : ELEVATION_SOLUTION_GROUP_PATH);

    // Verify the proper group and layer combination is given.
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

InterleavedLayerDescriptor::InterleavedLayerDescriptor(
    LayerType layerType,
    GroupType groupType,
    const Dataset& dataset)
    : LayerDescriptor(layerType, dataset)
    , m_groupType(groupType)
    , m_elementSize(Layer::getElementSize(Layer::getDataType(layerType)))
{
    this->setInternalPath(groupType == NODE ?
        NODE_GROUP_PATH : ELEVATION_SOLUTION_GROUP_PATH);

    this->setInternalPath(groupType == NODE ?
        NODE_GROUP_PATH : ELEVATION_SOLUTION_GROUP_PATH);

    // Verify the proper group and layer combination is given.
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

std::shared_ptr<InterleavedLayerDescriptor> InterleavedLayerDescriptor::create(
    LayerType layerType,
    GroupType groupType)
{
    return std::shared_ptr<InterleavedLayerDescriptor>(
        new InterleavedLayerDescriptor{layerType, groupType});
}

std::shared_ptr<InterleavedLayerDescriptor> InterleavedLayerDescriptor::create(
    LayerType layerType,
    GroupType groupType,
    const Dataset& dataset)
{
    return std::shared_ptr<InterleavedLayerDescriptor>(
        new InterleavedLayerDescriptor{layerType, groupType, dataset});
}


GroupType InterleavedLayerDescriptor::getGroupType() const noexcept
{
    return m_groupType;
}

uint8_t InterleavedLayerDescriptor::getElementSizeProxy() const noexcept
{
    return m_elementSize;
}

InterleavedLayerDescriptor& InterleavedLayerDescriptor::setElementSizeProxy(
    uint8_t size) & noexcept
{
    m_elementSize = size;
    return *this;
}

}  // namespace BAG

