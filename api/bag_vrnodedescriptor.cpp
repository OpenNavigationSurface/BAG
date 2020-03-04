
#include "bag_exceptions.h"
#include "bag_private.h"
#include "bag_vrnodedescriptor.h"


namespace BAG {

VRNodeDescriptor::VRNodeDescriptor(
    uint32_t id,
    uint64_t chunkSize,
    unsigned int compressionLevel)
    : LayerDescriptor(id, VR_NODE_PATH,
        kLayerTypeMapString.at(VarRes_Node), VarRes_Node, chunkSize,
        compressionLevel)
{
}

VRNodeDescriptor::VRNodeDescriptor(
    const Dataset& dataset)
    : LayerDescriptor(dataset, VarRes_Node, VR_NODE_PATH)
{
}

std::shared_ptr<VRNodeDescriptor> VRNodeDescriptor::create(
    const Dataset& dataset,
    uint64_t chunkSize,
    unsigned int compressionLevel)
{
    return std::shared_ptr<VRNodeDescriptor>(
        new VRNodeDescriptor{dataset.getNextId(), chunkSize,
            compressionLevel});
}

std::shared_ptr<VRNodeDescriptor> VRNodeDescriptor::open(
    const Dataset& dataset)
{
    return std::shared_ptr<VRNodeDescriptor>(
        new VRNodeDescriptor{dataset});
}


DataType VRNodeDescriptor::getDataTypeProxy() const noexcept
{
    return COMPOUND;
}

uint8_t VRNodeDescriptor::getElementSizeProxy() const noexcept
{
    return sizeof(BagVRNodeItem);
}

std::tuple<float, float>
VRNodeDescriptor::getMinMaxHypStrength() const noexcept
{
    return {m_minHypStrength, m_maxHypStrength};
}

void VRNodeDescriptor::setMinMaxHypStrength(
    float minHypStrength,
    float maxHypStrength) noexcept
{
    m_minHypStrength = minHypStrength;
    m_maxHypStrength = maxHypStrength;
}

std::tuple<uint32_t, uint32_t>
VRNodeDescriptor::getMinMaxNumHypotheses() const noexcept
{
    return {m_minNumHypotheses, m_maxNumHypotheses};
}

void VRNodeDescriptor::setMinMaxNumHypotheses(
    uint32_t minNumHypotheses,
    uint32_t maxNumHypotheses) noexcept
{
    m_minNumHypotheses = minNumHypotheses;
    m_maxNumHypotheses = maxNumHypotheses;
}

std::tuple<uint32_t, uint32_t>
VRNodeDescriptor::getMinMaxNSamples() const noexcept
{
    return {m_minNSamples, m_maxNSamples};
}

void VRNodeDescriptor::setMinMaxNSamples(
    uint32_t minNSamples,
    uint32_t maxNSamples) noexcept
{
    m_minNSamples = minNSamples;
    m_maxNSamples = maxNSamples;
}

}  // namespace BAG

