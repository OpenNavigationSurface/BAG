#ifndef BAG_LAYERDESCRIPTOR_H
#define BAG_LAYERDESCRIPTOR_H

#include "bag_config.h"
#include "bag_fordec.h"
#include "bag_types.h"

#include <memory>
#include <string>
#include <tuple>


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

namespace BAG {

class BAG_API LayerDescriptor : public std::enable_shared_from_this<LayerDescriptor>
{
public:
    //TODO Temp, make sure only move operations are used until development is done.
    LayerDescriptor(const LayerDescriptor&) = delete;
    LayerDescriptor(LayerDescriptor&&) = delete;
    LayerDescriptor& operator=(const LayerDescriptor&) = delete;
    LayerDescriptor& operator=(LayerDescriptor&&) = delete;
    virtual ~LayerDescriptor() = default;

    const std::string& getName() const & noexcept;
    LayerDescriptor& setName(std::string inName) & noexcept;

    DataType getDataType() const noexcept;
    LayerDescriptor& setDataType(DataType inType) & noexcept;

    LayerType getLayerType() const noexcept;
    LayerDescriptor& setLayerType(LayerType inType) & noexcept;

    std::tuple<float, float> getMinMax() const noexcept;
    LayerDescriptor& setMinMax(std::tuple<float, float> inMinMax) & noexcept;

    const std::string& getInternalPath() const & noexcept;
    LayerDescriptor& setInternalPath(std::string inPath) & noexcept;

    uint8_t getElementSize() const noexcept;
    LayerDescriptor& setElementSize(uint8_t inSize) & noexcept;

    uint64_t getChunkSize() const noexcept;
    LayerDescriptor& setChunkSize(uint64_t inChunkSize) & noexcept;

    unsigned int getCompressionLevel() const noexcept;
    LayerDescriptor& setCompressionLevel(unsigned int inCompressionLevel) & noexcept;

    std::tuple<uint32_t, uint32_t> getDims() const noexcept;
    LayerDescriptor& setDims(const std::tuple<uint32_t, uint32_t>& inDims) & noexcept;

protected:
    LayerDescriptor(LayerType type);
    LayerDescriptor(LayerType type, const Dataset& dataset,
        std::string internalPath = {});

    size_t getReadBufferSize(uint32_t rows, uint32_t columns) const noexcept;

private:
    virtual uint8_t getElementSizeProxy() const noexcept = 0;
    virtual LayerDescriptor& setElementSizeProxy(uint8_t) & noexcept = 0;

    LayerType m_layerType = UNKNOWN_LAYER_TYPE;
    DataType m_dataType = UNKNOWN_DATA_TYPE;
    std::string m_internalPath;
    std::string m_name;
    std::tuple<uint32_t, uint32_t> m_dims;
    unsigned int m_compressionLevel = 0;
    uint64_t m_chunkSize = 0;
    std::tuple<float, float> m_minMax{};

    friend InterleavedLayer;
    friend SurfaceCorrections;
    friend SimpleLayer;
};

}  // namespace BAG

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif  // BAG_LAYERDESCRIPTOR_H

