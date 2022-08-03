%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_compoundlayerdescriptor

%{
#include "bag_compoundlayerdescriptor.h"
%}

%import "bag_dataset.i"
%import "bag_compounddatatype.i"
%import "bag_layerdescriptor.i"

%include <std_string.i>
%include <std_shared_ptr.i>

%include "std_weak_ptr.i"
%shared_ptr(BAG::CompoundLayerDescriptor)
%shared_ptr(BAG::Dataset)

namespace std 
{
    %template(WeakDatasetPtr) weak_ptr<BAG::Dataset>;
}

namespace BAG {

class CompoundLayerDescriptor final : public LayerDescriptor
{
public:
    static std::shared_ptr<CompoundLayerDescriptor> create(Dataset& dataset,
        const std::string& name, GeorefMetadataProfile profile, DataType indexType,
        RecordDefinition definition, uint64_t chunkSize,
        int compressionLevel);
    %rename(openDataset) open(Dataset& dataset, const std::string& name);
    static std::shared_ptr<CompoundLayerDescriptor> open(Dataset& dataset,
        const std::string& name);

    CompoundLayerDescriptor(const CompoundLayerDescriptor&) = delete;
    CompoundLayerDescriptor(CompoundLayerDescriptor&&) = delete;

    bool operator==(const CompoundLayerDescriptor &rhs) const noexcept;
    bool operator!=(const CompoundLayerDescriptor &rhs) const noexcept;

    CompoundLayerDescriptor& operator=(const CompoundLayerDescriptor&) = delete;
    CompoundLayerDescriptor& operator=(CompoundLayerDescriptor&&) = delete;

    std::weak_ptr<Dataset> getDataset() const &;
    const RecordDefinition& getDefinition() const & noexcept;
};

}  // namespace BAG


