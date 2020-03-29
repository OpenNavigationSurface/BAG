#ifndef BAG_SIMPLELAYERDESCRIPTOR_H
#define BAG_SIMPLELAYERDESCRIPTOR_H

#include "bag_config.h"
#include "bag_fordec.h"
#include "bag_layerdescriptor.h"
#include "bag_types.h"

#include <memory>


namespace BAG {

class BAG_API SimpleLayerDescriptor final : public LayerDescriptor
{
public:
    static std::shared_ptr<SimpleLayerDescriptor> create(const Dataset& dataset,
        LayerType type, uint64_t chunkSize, int compressionLevel);

    static std::shared_ptr<SimpleLayerDescriptor> open(const Dataset& dataset,
        LayerType type);

    SimpleLayerDescriptor(const SimpleLayerDescriptor&) = delete;
    SimpleLayerDescriptor(SimpleLayerDescriptor&&) = delete;

    SimpleLayerDescriptor& operator=(const SimpleLayerDescriptor&) = delete;
    SimpleLayerDescriptor& operator=(SimpleLayerDescriptor&&) = delete;

protected:
    SimpleLayerDescriptor(uint32_t id, LayerType type, uint64_t chunkSize,
        int compressionLevel);
    SimpleLayerDescriptor(const Dataset& dataset, LayerType type);

private:
    DataType getDataTypeProxy() const noexcept override;
    uint8_t getElementSizeProxy() const noexcept override;

    //! The size of a single record in the HDF5 file.
    uint8_t m_elementSize = 0;
};

}  // namespace BAG

#endif  // BAG_SIMPLELAYERDESCRIPTOR_H

