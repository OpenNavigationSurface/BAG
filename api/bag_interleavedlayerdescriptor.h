#ifndef BAG_INTERLEAVEDLAYERDESCRIPTOR_H
#define BAG_INTERLEAVEDLAYERDESCRIPTOR_H

#include "bag_config.h"
#include "bag_fordec.h"
#include "bag_layerdescriptor.h"
#include "bag_types.h"


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

namespace BAG {

class BAG_API InterleavedLayerDescriptor final : public LayerDescriptor
{
public:

    static std::shared_ptr<InterleavedLayerDescriptor> create(
        LayerType layerType, GroupType groupType, uint64_t chunkSize,
        unsigned int compressionLevel);
    static std::shared_ptr<InterleavedLayerDescriptor> create(
        LayerType layerType, GroupType groupType,
        const Dataset& dataset);

    //TODO Temp, make sure only move operations are used until development is done.
    InterleavedLayerDescriptor(const InterleavedLayerDescriptor&) = delete;
    InterleavedLayerDescriptor(InterleavedLayerDescriptor&&) = delete;
    InterleavedLayerDescriptor& operator=(const InterleavedLayerDescriptor&) = delete;
    InterleavedLayerDescriptor& operator=(InterleavedLayerDescriptor&&) = delete;

    GroupType getGroupType() const noexcept;

protected:

    InterleavedLayerDescriptor(LayerType layerType, GroupType groupType,
        uint64_t chunkSize, unsigned int compressionLevel);
    InterleavedLayerDescriptor(LayerType layerType, GroupType groupType,
        const Dataset& dataset);

private:
    void validateTypes(LayerType layerType, GroupType groupType) const;

    uint8_t getElementSizeProxy() const noexcept override;

    GroupType m_groupType = UNKNOWN_GROUP_TYPE;
    uint8_t m_elementSize = 0;

};

}  // namespace BAG

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif  // BAG_INTERLEAVEDLAYERDESCRIPTOR_H

