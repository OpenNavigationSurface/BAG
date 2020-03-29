#ifndef BAG_LAYER_H
#define BAG_LAYER_H

#include "bag_config.h"
#include "bag_fordec.h"
#include "bag_types.h"
#include "bag_uint8array.h"

#include <memory>


namespace BAG {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)  // std classes do not have DLL-interface when exporting
#endif

class BAG_API Layer
{
public:
    virtual ~Layer() = default;

    Layer(const Layer&) = delete;
    Layer(Layer&&) = delete;

    Layer& operator=(const Layer&) = delete;
    Layer& operator=(Layer&&) = delete;

    static DataType getDataType(LayerType layerType) noexcept;
    static uint8_t getElementSize(DataType type);
    static std::string getInternalPath(LayerType layerType,
        GroupType groupType = UNKNOWN_GROUP_TYPE);

    LayerDescriptor& getDescriptor() & noexcept;
    const LayerDescriptor& getDescriptor() const & noexcept;

    std::unique_ptr<UInt8Array> read(uint32_t rowStart,
        uint32_t columnStart, uint32_t rowEnd, uint32_t columnEnd) const;

    void write(uint32_t rowStart, uint32_t columnStart, uint32_t rowEnd,
        uint32_t columnEnd, const uint8_t* buffer);

    void writeAttributes() const;

protected:
    Layer(Dataset& dataset, LayerDescriptor& descriptor);

    std::weak_ptr<Dataset> getDataset() & noexcept;
    std::weak_ptr<const Dataset> getDataset() const & noexcept;

private:
    virtual std::unique_ptr<UInt8Array> readProxy(uint32_t rowStart,
        uint32_t columnStart, uint32_t rowEnd, uint32_t columnEnd) const = 0;

    virtual void writeProxy(uint32_t rowStart, uint32_t columnStart,
        uint32_t rowEnd, uint32_t columnEnd, const uint8_t* buffer) = 0;

    virtual void writeAttributesProxy() const = 0;

    //! The dataset this layer is from.
    std::weak_ptr<Dataset> m_pBagDataset;
    //! The layer's descriptor (owned).
    std::shared_ptr<LayerDescriptor> m_pLayerDescriptor;

    friend class Dataset;
    friend class ValueTable;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

}  // namespace BAG

#endif  //BAG_LAYER_H

