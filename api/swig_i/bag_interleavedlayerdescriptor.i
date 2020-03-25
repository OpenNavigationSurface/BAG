%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_interleavedlayerdescriptor

%include <std_shared_ptr.i>
%shared_ptr(BAG::InterleavedLayerDescriptor)

%{
#include "../bag_interleavedlayerdescriptor.h"
%}

#define final

%import "bag_layerdescriptor.i"

namespace BAG {

class BAG_API InterleavedLayerDescriptor final : public LayerDescriptor
{
public:
    static std::shared_ptr<InterleavedLayerDescriptor> create(
        LayerType layerType, GroupType groupType, uint64_t chunkSize,
        unsigned int compressionLevel, const Dataset& dataset);

    %rename(openDataset) open(LayerType, GroupType, const Dataset&);
    static std::shared_ptr<InterleavedLayerDescriptor> open(
        LayerType layerType, GroupType groupType, const Dataset& dataset);

    //TODO Temp, make sure only move operations are used until development is done.
    InterleavedLayerDescriptor(const InterleavedLayerDescriptor&) = delete;
    InterleavedLayerDescriptor(InterleavedLayerDescriptor&&) = delete;
    InterleavedLayerDescriptor& operator=(const InterleavedLayerDescriptor&) = delete;
    InterleavedLayerDescriptor& operator=(InterleavedLayerDescriptor&&) = delete;

    GroupType getGroupType() const noexcept;
};

}
