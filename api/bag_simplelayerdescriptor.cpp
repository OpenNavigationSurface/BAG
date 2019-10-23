
#include "bag_dataset.h"
#include "bag_simplelayerdescriptor.h"


namespace BAG {

SimpleLayerDescriptor::SimpleLayerDescriptor(
    LayerType type)
    : LayerDescriptor(type)
    , m_elementSize(Layer::getElementSize(Layer::getDataType(type)))
{
}

SimpleLayerDescriptor::SimpleLayerDescriptor(
    LayerType type,
    const Dataset& dataset)
    : LayerDescriptor(type, dataset)
    ,m_elementSize(Layer::getElementSize(Layer::getDataType(type)))
{
}

std::shared_ptr<SimpleLayerDescriptor> SimpleLayerDescriptor::create(
    LayerType type)
{
    return std::shared_ptr<SimpleLayerDescriptor>(
        new SimpleLayerDescriptor{type});
}

std::shared_ptr<SimpleLayerDescriptor> SimpleLayerDescriptor::create(
    LayerType type,
    const Dataset& dataset)
{
    return std::shared_ptr<SimpleLayerDescriptor>(
        new SimpleLayerDescriptor{type, dataset});
}


uint8_t SimpleLayerDescriptor::getElementSizeProxy() const noexcept
{
    return m_elementSize;
}

SimpleLayerDescriptor& SimpleLayerDescriptor::setElementSizeProxy(
    uint8_t inSize) & noexcept
{
    m_elementSize = inSize;
    return *this;
}

}  // namespace BAG

