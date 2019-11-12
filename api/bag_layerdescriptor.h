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

    LayerType getLayerType() const noexcept;

    //TODO What about layers with uint32_t min/max values?
    std::tuple<float, float> getMinMax() const noexcept;
    LayerDescriptor& setMinMax(float min, float max) & noexcept;

    const std::string& getInternalPath() const & noexcept;

    uint8_t getElementSize() const noexcept;

    uint64_t getChunkSize() const noexcept;

    unsigned int getCompressionLevel() const noexcept;

protected:
    LayerDescriptor(LayerType type, uint64_t chunkSize,
        unsigned int compressionLevel);
    LayerDescriptor(LayerType type, const Dataset& dataset,
        std::string internalPath = {});

    size_t getReadBufferSize(uint32_t rows, uint32_t columns) const noexcept;

    LayerDescriptor& setInternalPath(std::string inPath) & noexcept;

private:
    virtual uint8_t getElementSizeProxy() const noexcept = 0;

    //! The layer type.
    LayerType m_layerType = UNKNOWN_LAYER_TYPE;
    //! The data type (depends on layer type).
    DataType m_dataType = UNKNOWN_DATA_TYPE;
    //! The path of the DataSet in the HDF5 file.
    std::string m_internalPath;
    //! The name of the layer.
    std::string m_name;
    //! The compression level of this dataset (0-9).
    unsigned int m_compressionLevel = 0;
    //! The chunk size of this dataset (0x0, 10x10 or 100x100).
    uint64_t m_chunkSize = 0;
    //! The minimum and maximum value of this dataset.
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

