
#include "bag_exceptions.h"
#include "bag_private.h"
#include "bag_vrmetadatadescriptor.h"


namespace BAG {

VRMetadataDescriptor::VRMetadataDescriptor(
    uint32_t id,
    uint64_t chunkSize,
    unsigned int compressionLevel)
    : LayerDescriptor(id, VR_METADATA_PATH,
        kLayerTypeMapString.at(VarRes_Metadata), VarRes_Metadata, chunkSize,
        compressionLevel)
{
}

VRMetadataDescriptor::VRMetadataDescriptor(
    const Dataset& dataset)
    : LayerDescriptor(dataset, VarRes_Metadata, VR_METADATA_PATH)
{
}

std::shared_ptr<VRMetadataDescriptor> VRMetadataDescriptor::create(
    const Dataset& dataset,
    uint64_t chunkSize,
    unsigned int compressionLevel)
{
    return std::shared_ptr<VRMetadataDescriptor>(
        new VRMetadataDescriptor{dataset.getNextId(), chunkSize,
            compressionLevel});
}

std::shared_ptr<VRMetadataDescriptor> VRMetadataDescriptor::open(
    const Dataset& dataset)
{
    return std::shared_ptr<VRMetadataDescriptor>(
        new VRMetadataDescriptor{dataset});
}


DataType VRMetadataDescriptor::getDataTypeProxy() const noexcept
{
    return COMPOUND;
}

uint8_t VRMetadataDescriptor::getElementSizeProxy() const noexcept
{
    return sizeof(BagVRMetadataItem);
}

std::tuple<uint32_t, uint32_t>
VRMetadataDescriptor::getMaxDimensions() const noexcept
{
    return {m_maxDimX, m_maxDimY};
}

void VRMetadataDescriptor::setMaxDimensions(
    uint32_t maxDimX,
    uint32_t maxDimY) noexcept
{
    m_maxDimX = maxDimX;
    m_maxDimY = maxDimY;
}

std::tuple<uint32_t, uint32_t>
VRMetadataDescriptor::getMinDimensions() const noexcept
{
    return {m_minDimX, m_minDimY};
}

void VRMetadataDescriptor::setMinDimensions(
    uint32_t minDimX,
    uint32_t minDimY) noexcept
{
    m_minDimX = minDimX;
    m_minDimY = minDimY;
}

std::tuple<float, float>
VRMetadataDescriptor::getMaxResolution() const noexcept
{
    return {m_maxResX, m_maxResY};
}

void VRMetadataDescriptor::setMaxResolution(
    float maxResX,
    float maxResY) noexcept
{
    m_maxResX = maxResX;
    m_maxResY = maxResY;
}

std::tuple<float, float>
VRMetadataDescriptor::getMinResolution() const noexcept
{
    return {m_minResX, m_minResY};
}

void VRMetadataDescriptor::setMinResolution(
    float minResX,
    float minResY) noexcept
{
    m_minResX = minResX;
    m_minResY = minResY;
}

}  // namespace BAG

