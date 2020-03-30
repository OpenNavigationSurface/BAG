%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_compoundlayerdescriptor

%{
#include "../bag_compoundlayerdescriptor.h"
%}

#define final

%import "../bag_config.h"
%import "bag_compoundlayer.i"
%import "bag_layerdescriptor.i"
%include <std_string.i>

// TODO implement weak_ptr interface

%include <std_shared_ptr.i>
%shared_ptr(BAG::CompoundLayerDescriptor)

namespace BAG {

class BAG_API CompoundLayerDescriptor final : public LayerDescriptor
{
public:
    static std::shared_ptr<CompoundLayerDescriptor> create(
        const std::string& name, DataType indexType,
        RecordDefinition definition, uint64_t chunkSize,
        unsigned int compressionLevel, Dataset& dataset);
    %rename(openDataset) open(const std::string&, Dataset&);
    static std::shared_ptr<CompoundLayerDescriptor> open(
        const std::string& name, Dataset& dataset);

    //TODO Temp, make sure only move operations are used until development is done.
    CompoundLayerDescriptor(const CompoundLayerDescriptor&) = delete;
    CompoundLayerDescriptor(CompoundLayerDescriptor&&) = delete;
    CompoundLayerDescriptor& operator=(const CompoundLayerDescriptor&) = delete;
    CompoundLayerDescriptor& operator=(CompoundLayerDescriptor&&) = delete;

    std::weak_ptr<Dataset> getDataset() const &;
    const RecordDefinition& getDefinition() const & noexcept;
};
}


