#ifndef BAG_SIMPLELAYERDESCRIPTOR_H
#define BAG_SIMPLELAYERDESCRIPTOR_H

#include "bag_config.h"
#include "bag_fordec.h"
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
        uint64_t chunkSize, unsigned int compressionLevel);
    static std::shared_ptr<SimpleLayerDescriptor> create(LayerType type,
        const Dataset& dataset);

    //TODO Temp, make sure only move operations are used until development is done.
    SimpleLayerDescriptor(const SimpleLayerDescriptor&) = delete;
    SimpleLayerDescriptor(SimpleLayerDescriptor&&) = delete;
    SimpleLayerDescriptor& operator=(const SimpleLayerDescriptor&) = delete;
    SimpleLayerDescriptor& operator=(SimpleLayerDescriptor&&) = delete;

protected:
    SimpleLayerDescriptor(LayerType type, uint64_t chunkSize,
        unsigned int compressionLevel);
    SimpleLayerDescriptor(LayerType type, const Dataset& dataset);

private:
    uint8_t getElementSizeProxy() const noexcept override;

    //! The size of a single record in the HDF5 file.
    uint8_t m_elementSize = 0;
};

}  // namespace BAG

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif  // BAG_LAYERDESCRIPTOR_H

