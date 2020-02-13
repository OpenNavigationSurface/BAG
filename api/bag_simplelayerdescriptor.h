#ifndef BAG_SIMPLELAYERDESCRIPTOR_H
#define BAG_SIMPLELAYERDESCRIPTOR_H

#include "bag_config.h"
#include "bag_fordec.h"
#include "bag_layerdescriptor.h"
#include "bag_types.h"

#include <memory>


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

namespace BAG {

class BAG_API SimpleLayerDescriptor final : public LayerDescriptor
{
public:
    static std::shared_ptr<SimpleLayerDescriptor> create(LayerType type,
        uint64_t chunkSize, unsigned int compressionLevel,
        const Dataset& dataset);
    static std::shared_ptr<SimpleLayerDescriptor> open(LayerType type,
        const Dataset& dataset);

    //TODO Temp, make sure only move operations are used until development is done.
    SimpleLayerDescriptor(const SimpleLayerDescriptor&) = delete;
    SimpleLayerDescriptor(SimpleLayerDescriptor&&) = delete;
    SimpleLayerDescriptor& operator=(const SimpleLayerDescriptor&) = delete;
    SimpleLayerDescriptor& operator=(SimpleLayerDescriptor&&) = delete;

protected:
    SimpleLayerDescriptor(uint32_t id, LayerType type, uint64_t chunkSize,
        unsigned int compressionLevel);
    SimpleLayerDescriptor(LayerType type, const Dataset& dataset);

private:
    DataType getDataTypeProxy() const noexcept override;
    uint8_t getElementSizeProxy() const noexcept override;

    //! The data type (depends on layer type).
    DataType m_dataType = DT_UNKNOWN_DATA_TYPE;
    //! The size of a single record in the HDF5 file.
    uint8_t m_elementSize = 0;
};

}  // namespace BAG

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif  // BAG_SIMPLELAYERDESCRIPTOR_H

