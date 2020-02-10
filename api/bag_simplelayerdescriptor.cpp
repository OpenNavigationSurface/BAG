
#include "bag_dataset.h"
#include "bag_simplelayerdescriptor.h"


namespace BAG {

SimpleLayerDescriptor::SimpleLayerDescriptor(
    uint32_t id,
    LayerType type,
    uint64_t chunkSize,
    unsigned int compressionLevel)
    : LayerDescriptor(id, Layer::getInternalPath(type),
        kLayerTypeMapString.at(type), type, chunkSize, compressionLevel)
    , m_dataType(Layer::getDataType(type))
    , m_elementSize(Layer::getElementSize(Layer::getDataType(type)))
{
}

SimpleLayerDescriptor::SimpleLayerDescriptor(
    LayerType type,
    const Dataset& dataset)
    : LayerDescriptor(type, dataset)
    , m_dataType(Layer::getDataType(type))
    , m_elementSize(Layer::getElementSize(Layer::getDataType(type)))
{
}

std::shared_ptr<SimpleLayerDescriptor> SimpleLayerDescriptor::create(
    LayerType type,
    uint64_t chunkSize,
    unsigned int compressionLevel,
    const Dataset& dataset)
{
    return std::shared_ptr<SimpleLayerDescriptor>(
        new SimpleLayerDescriptor{dataset.getNextId(), type, chunkSize,
        compressionLevel});
}

std::shared_ptr<SimpleLayerDescriptor> SimpleLayerDescriptor::open(
    LayerType type,
    const Dataset& dataset)
{
    return std::shared_ptr<SimpleLayerDescriptor>(
        new SimpleLayerDescriptor{type, dataset});
}


DataType SimpleLayerDescriptor::getDataTypeProxy() const noexcept
{
    return m_dataType;
}

uint8_t SimpleLayerDescriptor::getElementSizeProxy() const noexcept
{
    return m_elementSize;
}

}  // namespace BAG

