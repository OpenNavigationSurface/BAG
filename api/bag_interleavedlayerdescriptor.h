#ifndef BAG_INTERLEAVEDLAYERDESCRIPTOR_H
#define BAG_INTERLEAVEDLAYERDESCRIPTOR_H

#include "bag_config.h"
#include "bag_fordec.h"
#include "bag_layerdescriptor.h"
#include "bag_types.h"


namespace BAG {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)  // std classes do not have DLL-interface when exporting
#endif

//! Describe an interleaved layer.
class BAG_API InterleavedLayerDescriptor final : public LayerDescriptor
{
public:
    static std::shared_ptr<InterleavedLayerDescriptor> create(
        const Dataset& dataset, LayerType layerType, GroupType groupType);

    static std::shared_ptr<InterleavedLayerDescriptor> open(
        const Dataset& dataset, LayerType layerType, GroupType groupType);

    InterleavedLayerDescriptor(const InterleavedLayerDescriptor&) = delete;
    InterleavedLayerDescriptor(InterleavedLayerDescriptor&&) = delete;

    InterleavedLayerDescriptor& operator=(const InterleavedLayerDescriptor&) = delete;
    InterleavedLayerDescriptor& operator=(InterleavedLayerDescriptor&&) = delete;

    GroupType getGroupType() const noexcept;

protected:
    InterleavedLayerDescriptor(uint32_t id, LayerType layerType,
        GroupType groupType);
    InterleavedLayerDescriptor(const Dataset& dataset, LayerType layerType,
        GroupType groupType);

private:
    static void validateTypes(LayerType layerType, GroupType groupType);

    DataType getDataTypeProxy() const noexcept override;
    uint8_t getElementSizeProxy() const noexcept override;

    //! The group type.
    GroupType m_groupType = UNKNOWN_GROUP_TYPE;
    //! The size of a node in the HDF5 DataSet (depends on group type).
    uint8_t m_elementSize = 0;

};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

}  // namespace BAG

#endif  // BAG_INTERLEAVEDLAYERDESCRIPTOR_H

