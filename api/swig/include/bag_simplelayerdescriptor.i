%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_simplelayerdescriptor

%{
#include "bag_simplelayerdescriptor.h"
%}

%import "bag_dataset.i"
%import "bag_layerdescriptor.i"
%import "bag_types.i"

%include <std_shared_ptr.i>
%include <stdint.i>

%shared_ptr(BAG::SimpleLayerDescriptor)


namespace BAG {

class SimpleLayerDescriptor final : public LayerDescriptor
{
public:
    static std::shared_ptr<SimpleLayerDescriptor> create(const Dataset& dataset,
        LayerType type, uint64_t chunkSize, int compressionLevel);

    %rename(openDataset) open(const Dataset&, LayerType);
    static std::shared_ptr<SimpleLayerDescriptor> open(
        const Dataset& dataset, LayerType type);

    SimpleLayerDescriptor(const SimpleLayerDescriptor&) = delete;
    SimpleLayerDescriptor(SimpleLayerDescriptor&&) = delete;

    SimpleLayerDescriptor& operator=(const SimpleLayerDescriptor&) = delete;
    SimpleLayerDescriptor& operator=(SimpleLayerDescriptor&&) = delete;
};

}  // namespace BAG

