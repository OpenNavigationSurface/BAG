#ifndef BAG_LAYERDESCRIPTOR_H
#define BAG_LAYERDESCRIPTOR_H

#include "bag_config.h"
#include "bag_fordec.h"
#include "bag_types.h"

#include <memory>
#include <string>
#include <tuple>


namespace BAG {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)  // std classes do not have DLL-interface when exporting
#endif

class BAG_API LayerDescriptor : public std::enable_shared_from_this<LayerDescriptor>
{
public:
    //TODO Temp, make sure only move operations are used until development is done.
    LayerDescriptor(const LayerDescriptor&) = delete;
    LayerDescriptor(LayerDescriptor&&) = delete;
    LayerDescriptor& operator=(const LayerDescriptor&) = delete;
    LayerDescriptor& operator=(LayerDescriptor&&) = delete;
    virtual ~LayerDescriptor() = default;

    uint64_t getChunkSize() const noexcept;
    unsigned int getCompressionLevel() const noexcept;
    DataType getDataType() const noexcept;
    uint8_t getElementSize() const noexcept;
    uint32_t getId() const noexcept;
    const std::string& getInternalPath() const & noexcept;
    LayerType getLayerType() const noexcept;
    std::tuple<float, float> getMinMax() const noexcept;
    const std::string& getName() const & noexcept;

    LayerDescriptor& setName(std::string inName) & noexcept;
    LayerDescriptor& setMinMax(float min, float max) & noexcept;

protected:
    LayerDescriptor(uint32_t id, std::string internalPath, std::string name,
        LayerType type, uint64_t chunkSize, unsigned int compressionLevel);
    LayerDescriptor(const Dataset& dataset, LayerType type,
        std::string internalPath = {}, std::string name = {});

    size_t getReadBufferSize(uint32_t rows, uint32_t columns) const noexcept;

    LayerDescriptor& setInternalPath(std::string inPath) & noexcept;

private:
    virtual DataType getDataTypeProxy() const noexcept = 0;
    virtual uint8_t getElementSizeProxy() const noexcept = 0;

    //! The unique id.
    uint32_t m_id = std::numeric_limits<uint32_t>::max();
    //! The layer type.
    LayerType m_layerType = UNKNOWN_LAYER_TYPE;
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

    friend class CompoundLayer;
    friend class InterleavedLayer;
    friend class SurfaceCorrections;
    friend class SimpleLayer;
    friend class VRMetadata;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

}  // namespace BAG

#endif  // BAG_LAYERDESCRIPTOR_H

