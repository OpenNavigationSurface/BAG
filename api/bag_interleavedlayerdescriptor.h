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
        LayerType layerType, GroupType groupType);
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

    InterleavedLayerDescriptor(LayerType layerType, GroupType groupType);
    InterleavedLayerDescriptor(LayerType layerType, GroupType groupType,
        const Dataset& dataset);

private:

    uint8_t getElementSizeProxy() const noexcept override;
    InterleavedLayerDescriptor& setElementSizeProxy(
        uint8_t inSize) & noexcept override;

    GroupType m_groupType = UNKNOWN_GROUP_TYPE;
    uint8_t m_elementSize = 0;

    //friend InterleavedLayer;

};

}  // namespace BAG

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif  // BAG_INTERLEAVEDLAYERDESCRIPTOR_H

