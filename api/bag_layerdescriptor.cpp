

#include "bag_dataset.h"
#include "bag_hdfhelper.h"
#include "bag_layer.h"
#include "bag_layerdescriptor.h"
#include "bag_private.h"

#include <H5Cpp.h>


namespace BAG {

LayerDescriptor::LayerDescriptor(
    uint32_t id,
    std::string internalPath,
    std::string name,
    LayerType type,
    uint64_t chunkSize,
    unsigned int compressionLevel)
    : m_id(id)
    , m_layerType(type)
    , m_internalPath(std::move(internalPath))
    , m_name(std::move(name))
    , m_compressionLevel(compressionLevel)
    , m_chunkSize(chunkSize)
    , m_minMax(std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest())
{
}

LayerDescriptor::LayerDescriptor(
    LayerType type,
    const Dataset& dataset,
    std::string internalPath,
    std::string name)
    : m_id(dataset.getNextId())
    , m_layerType(type)
    , m_minMax(std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest())
{
    m_internalPath = internalPath.empty()
        ? Layer::getInternalPath(type)
        : std::move(internalPath);

    m_name = name.empty()
        ? kLayerTypeMapString.at(type)
        : std::move(name);

    const auto& h5file = dataset.getH5file();

    m_compressionLevel = BAG::getCompressionLevel(h5file, m_internalPath);
    m_chunkSize = BAG::getChunkSize(h5file, m_internalPath);
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
    return this->getDataTypeProxy();
}

LayerType LayerDescriptor::getLayerType() const noexcept
{
    return m_layerType;
}

std::tuple<float, float> LayerDescriptor::getMinMax() const noexcept
{
    return m_minMax;
}

LayerDescriptor& LayerDescriptor::setMinMax(
    float min,
    float max) & noexcept
{
    m_minMax = {min, max};
    return *this;
}

uint32_t LayerDescriptor::getId() const noexcept
{
    return m_id;
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

}  // namespace BAG

