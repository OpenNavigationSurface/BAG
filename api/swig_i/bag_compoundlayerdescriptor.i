%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_compoundlayerdescriptor

%{
#include "../bag_compoundlayerdescriptor.h"
%}

%import "bag_dataset.i"
%import "bag_compounddatatype.i"
%import "bag_layerdescriptor.i"

%include <std_string.i>
%include <std_shared_ptr.i>

// TODO implement weak_ptr interface

%shared_ptr(BAG::CompoundLayerDescriptor)
//%template(WeakDataset) std::weak_ptr<Dataset>;


namespace BAG {

class CompoundLayerDescriptor final : public LayerDescriptor
{
public:
    static std::shared_ptr<CompoundLayerDescriptor> create(Dataset& dataset,
        const std::string& name, DataType indexType,
        RecordDefinition definition, uint64_t chunkSize,
        int compressionLevel);
    %rename(openDataset) open(Dataset& dataset, const std::string& name);
    static std::shared_ptr<CompoundLayerDescriptor> open(Dataset& dataset,
        const std::string& name);

    CompoundLayerDescriptor(const CompoundLayerDescriptor&) = delete;
    CompoundLayerDescriptor(CompoundLayerDescriptor&&) = delete;

    CompoundLayerDescriptor& operator=(const CompoundLayerDescriptor&) = delete;
    CompoundLayerDescriptor& operator=(CompoundLayerDescriptor&&) = delete;

    std::weak_ptr<Dataset> getDataset() const &;
    const RecordDefinition& getDefinition() const & noexcept;
};

}  // namespace BAG


