%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_interleavedlayerdescriptor

%{
#include "../bag_interleavedlayerdescriptor.h"
%}

#define final

%include <std_shared_ptr.i>
%shared_ptr(BAG::InterleavedLayerDescriptor)

%import "bag_layerdescriptor.i"
%import "bag_types.i"

namespace BAG {
    class InterleavedLayerDescriptor final : public LayerDescriptor
    {
    public:
        static std::shared_ptr<InterleavedLayerDescriptor> create(
            const Dataset& dataset, LayerType layerType, GroupType groupType);

        %rename(openDataset) open(const Dataset& dataset, LayerType layerType,
            GroupType groupType);
        static std::shared_ptr<InterleavedLayerDescriptor> open(
            const Dataset& dataset, LayerType layerType, GroupType groupType);

        InterleavedLayerDescriptor(const InterleavedLayerDescriptor&) = delete;
        InterleavedLayerDescriptor(InterleavedLayerDescriptor&&) = delete;

        InterleavedLayerDescriptor& operator=(const InterleavedLayerDescriptor&) = delete;
        InterleavedLayerDescriptor& operator=(InterleavedLayerDescriptor&&) = delete;

        GroupType getGroupType() const noexcept;
    };
}
