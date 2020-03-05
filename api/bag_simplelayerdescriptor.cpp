
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
    const Dataset& dataset,
    LayerType type)
    : LayerDescriptor(dataset, type)
    , m_dataType(Layer::getDataType(type))
    , m_elementSize(Layer::getElementSize(Layer::getDataType(type)))
{
}

std::shared_ptr<SimpleLayerDescriptor> SimpleLayerDescriptor::create(
    const Dataset& dataset,
    LayerType type,
    uint64_t chunkSize,
    unsigned int compressionLevel)
{
    return std::shared_ptr<SimpleLayerDescriptor>(
        new SimpleLayerDescriptor{dataset.getNextId(), type, chunkSize,
        compressionLevel});
}

std::shared_ptr<SimpleLayerDescriptor> SimpleLayerDescriptor::open(
    const Dataset& dataset,
    LayerType type)
{
    return std::shared_ptr<SimpleLayerDescriptor>(
        new SimpleLayerDescriptor{dataset, type});
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

