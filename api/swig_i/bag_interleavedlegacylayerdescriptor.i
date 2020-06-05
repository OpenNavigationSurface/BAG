%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_interleavedlegacylayerdescriptor

%{
#include "../bag_interleavedlegacylayerdescriptor.h"
%}

%import "bag_dataset.i"
%import "bag_layerdescriptor.i"
%import "bag_types.i"

%include <std_shared_ptr.i>

%shared_ptr(BAG::InterleavedLegacyLayerDescriptor)


namespace BAG {

class InterleavedLegacyLayerDescriptor final : public LayerDescriptor
{
public:
    static std::shared_ptr<InterleavedLegacyLayerDescriptor> create(
        const Dataset& dataset, LayerType layerType, GroupType groupType);

    %rename(openDataset) open(const Dataset& dataset, LayerType layerType,
        GroupType groupType);
    static std::shared_ptr<InterleavedLegacyLayerDescriptor> open(
        const Dataset& dataset, LayerType layerType, GroupType groupType);

    InterleavedLegacyLayerDescriptor(const InterleavedLegacyLayerDescriptor&) = delete;
    InterleavedLegacyLayerDescriptor(InterleavedLegacyLayerDescriptor&&) = delete;

    InterleavedLegacyLayerDescriptor& operator=(const InterleavedLegacyLayerDescriptor&) = delete;
    InterleavedLegacyLayerDescriptor& operator=(InterleavedLegacyLayerDescriptor&&) = delete;

    GroupType getGroupType() const noexcept;
};

}  // namespace BAG

