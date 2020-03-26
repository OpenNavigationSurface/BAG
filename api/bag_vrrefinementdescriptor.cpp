
#include "bag_exceptions.h"
#include "bag_private.h"
#include "bag_vrrefinementdescriptor.h"


namespace BAG {

VRRefinementDescriptor::VRRefinementDescriptor(
    uint32_t id,
    uint64_t chunkSize,
    unsigned int compressionLevel)
    : LayerDescriptor(id, VR_REFINEMENT_PATH,
        kLayerTypeMapString.at(VarRes_Refinement), VarRes_Refinement, chunkSize,
        compressionLevel)
{
}

VRRefinementDescriptor::VRRefinementDescriptor(
    const Dataset& dataset)
    : LayerDescriptor(dataset, VarRes_Refinement, VR_REFINEMENT_PATH)
{
}

std::shared_ptr<VRRefinementDescriptor> VRRefinementDescriptor::create(
    const Dataset& dataset,
    uint64_t chunkSize,
    unsigned int compressionLevel)
{
    return std::shared_ptr<VRRefinementDescriptor>(
        new VRRefinementDescriptor{dataset.getNextId(), chunkSize,
            compressionLevel});
}

std::shared_ptr<VRRefinementDescriptor> VRRefinementDescriptor::open(
    const Dataset& dataset)
{
    return std::shared_ptr<VRRefinementDescriptor>(
        new VRRefinementDescriptor{dataset});
}


DataType VRRefinementDescriptor::getDataTypeProxy() const noexcept
{
    return DT_COMPOUND;
}

uint8_t VRRefinementDescriptor::getElementSizeProxy() const noexcept
{
    return sizeof(VRRefinementItem);
}

std::tuple<float, float>
VRRefinementDescriptor::getMinMaxDepth() const noexcept
{
    return {m_minDepth, m_maxDepth};
}

void VRRefinementDescriptor::setMinMaxDepth(
    float minDepth,
    float maxDepth) noexcept
{
    m_minDepth = minDepth;
    m_maxDepth = maxDepth;
}

std::tuple<float, float>
VRRefinementDescriptor::getMinMaxUncertainty() const noexcept
{
    return {m_minUncertainty, m_maxUncertainty};
}

void VRRefinementDescriptor::setMinMaxUncertainty(
    float minUncertainty,
    float maxUncertainty) noexcept
{
    m_minUncertainty = minUncertainty;
    m_maxUncertainty = maxUncertainty;
}

}  // namespace BAG

