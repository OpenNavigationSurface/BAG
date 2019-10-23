
#include "bag_dataset.h"
#include "bag_layer.h"
#include "bag_layerdescriptor.h"


namespace BAG {

LayerDescriptor::LayerDescriptor(
    LayerType type)
    : m_layerType(type)
    , m_dataType(Layer::getDataType(type))
    , m_internalPath(Layer::getInternalPath(type))
    , m_name(kLayerTypeMapString.at(type))
{
}

LayerDescriptor::LayerDescriptor(
    LayerType type,
    const Dataset& dataset)
    : m_layerType(type)
    , m_dataType(Layer::getDataType(type))
    , m_internalPath(Layer::getInternalPath(type))
    , m_name(kLayerTypeMapString.at(type))
    , m_dims(dataset.getDims(type))
    , m_compressionLevel(dataset.getCompressionLevel(type))
    , m_chunkSize(dataset.getChunkSize(type))
{
}


const std::string& LayerDescriptor::getName() const & noexcept
{
    return m_name;
}

LayerDescriptor& LayerDescriptor::setName(std::string inName) & noexcept
{
    m_name = std::move(inName);
    return *this;
}

DataType LayerDescriptor::getDataType() const noexcept
{
    return m_dataType;
}

LayerDescriptor& LayerDescriptor::setDataType(DataType inType) & noexcept
{
    m_dataType = inType;
    return *this;
}

LayerType LayerDescriptor::getLayerType() const noexcept
{
    return m_layerType;
}

LayerDescriptor& LayerDescriptor::setLayerType(LayerType inType) & noexcept
{
    m_layerType = inType;
    return *this;
}

std::tuple<float, float> LayerDescriptor::getMinMax() const noexcept
{
    return m_minMax;
}

LayerDescriptor& LayerDescriptor::setMinMax(
    std::tuple<float, float> inMinMax) & noexcept
{
    m_minMax = inMinMax;
    return *this;
}

const std::string& LayerDescriptor::getInternalPath() const & noexcept
{
    return m_internalPath;
}

LayerDescriptor& LayerDescriptor::setInternalPath(std::string inPath) & noexcept
{
    m_internalPath = std::move(inPath);
    return *this;
}

uint8_t LayerDescriptor::getElementSize() const noexcept
{
    return this->getElementSizeProxy();
}

LayerDescriptor& LayerDescriptor::setElementSize(uint8_t inSize) & noexcept
{
    return this->setElementSizeProxy(inSize);
}

size_t LayerDescriptor::getReadBufferSize(
    uint32_t rows,
    uint32_t columns) const noexcept
{
    return rows * columns * this->getElementSize();
}

uint64_t LayerDescriptor::getChunkSize() const noexcept
{
    return m_chunkSize;
}

unsigned int LayerDescriptor::getCompressionLevel() const noexcept
{
    return m_compressionLevel;
}

std::tuple<uint32_t, uint32_t> LayerDescriptor::getDims() const noexcept
{
    return m_dims;
}

LayerDescriptor& LayerDescriptor::setChunkSize(uint64_t inChunkSize) & noexcept
{
    m_chunkSize = inChunkSize;
    return *this;
}

LayerDescriptor& LayerDescriptor::setCompressionLevel(unsigned int inCompressionLevel) & noexcept
{
    m_compressionLevel = inCompressionLevel;
    return *this;
}

LayerDescriptor& LayerDescriptor::setDims(
    const std::tuple<uint32_t, uint32_t>& inDims) & noexcept
{
    m_dims = inDims;
    return *this;
}

}  // namespace BAG

