%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_simplelayerdescriptor

%{
#include "../bag_simplelayerdescriptor.h"
%}

#define final

%include <std_shared_ptr.i>
%shared_ptr(BAG::SimpleLayerDescriptor)

%import "bag_layerdescriptor.i"
%import "bag_types.i"

namespace BAG {

    class BAG_API SimpleLayerDescriptor final : public LayerDescriptor
    {
    public:
        static std::shared_ptr<SimpleLayerDescriptor> create(const Dataset& dataset,
            LayerType type, uint64_t chunkSize, unsigned int compressionLevel);

        %rename(openDataset) open(const Dataset&, LayerType);
        static std::shared_ptr<SimpleLayerDescriptor> open(
            const Dataset& dataset, LayerType type);

        //TODO Temp, make sure only move operations are used until development is done.
        SimpleLayerDescriptor(const SimpleLayerDescriptor&) = delete;
        SimpleLayerDescriptor(SimpleLayerDescriptor&&) = delete;
        SimpleLayerDescriptor& operator=(const SimpleLayerDescriptor&) = delete;
        SimpleLayerDescriptor& operator=(SimpleLayerDescriptor&&) = delete;

    };
}