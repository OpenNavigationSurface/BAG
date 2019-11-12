
#include "bag_dataset.h"
#include "bag_layer.h"
#include "bag_layerdescriptor.h"
#include "bag_private.h"

#include <array>
#include <h5cpp.h>


namespace BAG {

namespace {

std::tuple<uint32_t, uint32_t> getDims(
    const ::H5::H5File& h5file,
    const std::string& path)
{
    const auto h5dataset = h5file.openDataSet(path);
    const auto h5dataSpace = h5dataset.getSpace();
    if (!h5dataSpace.isSimple())
        throw 97;  // Can only work with simple data spaces.

    const int fileRank = h5dataSpace.getSimpleExtentNdims();
    std::array<hsize_t, RANK> fileDims{};
    const int dimsRank = h5dataSpace.getSimpleExtentDims(fileDims.data());

    if (fileRank != RANK || dimsRank != RANK)
        throw 99;  // Unexpected dimensions.

    const auto rows = static_cast<uint32_t>(fileDims[0]);
    const auto columns = static_cast<uint32_t>(fileDims[1]);

    return std::make_tuple(rows, columns);
}

unsigned int getCompressionLevel(
    const ::H5::H5File& h5file,
    const std::string& path)
{
    //Get the elevation HD5 dataset.
    const auto h5dataset = h5file.openDataSet(path);
    const auto h5pList = h5dataset.getCreatePlist();

    for (int i=0; i<h5pList.getNfilters(); ++i)
    {
        unsigned int flags = 0;
        size_t cdNelmts = 10;
        constexpr size_t nameLen = 64;
        std::array<unsigned int, 10> cdValues{};
        std::array<char, 64> name{};
        unsigned int filterConfig = 0;

        const auto filter = h5pList.getFilter(i, flags, cdNelmts,
            cdValues.data(), nameLen, name.data(), filterConfig);
        if (filter == H5Z_FILTER_DEFLATE)
            if (cdNelmts >= 1)
                return cdValues.front();
    }

    return 0;
}

uint64_t getChunkSize(
    const ::H5::H5File& h5file,
    const std::string& path)
{
    //Get the elevation HD5 dataset.
    const auto h5dataset = h5file.openDataSet(path);
    const auto h5pList = h5dataset.getCreatePlist();

    if (h5pList.getLayout() == H5D_CHUNKED)
    {
        std::array<hsize_t, RANK> maxDims{};

        const int rankChunk = h5pList.getChunk(RANK, maxDims.data());
        if (rankChunk == RANK)
            return {maxDims[0]};  // Using {} to prevent narrowing.
    }

    return 0;
}

}  // namespace

LayerDescriptor::LayerDescriptor(
    LayerType type,
    uint64_t chunkSize,
    unsigned int compressionLevel)
    : m_layerType(type)
    , m_dataType(Layer::getDataType(type))
    , m_internalPath(Layer::getInternalPath(type))
    , m_name(kLayerTypeMapString.at(type))
    , m_compressionLevel(compressionLevel)
    , m_chunkSize(chunkSize)
    , m_minMax(std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest())
{
}

LayerDescriptor::LayerDescriptor(
    LayerType type,
    const Dataset& dataset,
    std::string internalPath)
    : m_layerType(type)
    , m_dataType(Layer::getDataType(type))
    , m_name(kLayerTypeMapString.at(type))
    , m_minMax(std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest())
{
    m_internalPath = internalPath.empty()
        ? Layer::getInternalPath(type)
        : std::move(internalPath);

    const auto& h5file = dataset.getH5file();

    m_dims = BAG::getDims(h5file, m_internalPath);
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
    return m_dataType;
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

std::tuple<uint32_t, uint32_t> LayerDescriptor::getDims() const noexcept
{
    return m_dims;
}

LayerDescriptor& LayerDescriptor::setDims(
    const std::tuple<uint32_t, uint32_t>& inDims) & noexcept
{
    m_dims = inDims;
    return *this;
}

}  // namespace BAG

