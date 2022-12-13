%begin %{
#ifdef _MSC_VER
#ifdef SWIGPYTHON
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
#endif
%}

%module bag_georefmetadatalayerdescriptor

%{
#include "bag_georefmetadatalayerdescriptor.h"
%}

%import "bag_dataset.i"
%import "bag_compounddatatype.i"
%import "bag_layerdescriptor.i"

%include <std_string.i>
%include <std_weak_ptr.i>
%include <std_shared_ptr.i>
%shared_ptr(BAG::GeorefMetadataLayerDescriptor)
%shared_ptr(BAG::Dataset)

namespace std 
{
    %template(WeakDatasetPtr) weak_ptr<BAG::Dataset>;
}

namespace BAG {

class GeorefMetadataLayerDescriptor final : public LayerDescriptor
{
public:
    static std::shared_ptr<GeorefMetadataLayerDescriptor> create(Dataset& dataset,
        const std::string& name, GeorefMetadataProfile profile, DataType indexType,
        RecordDefinition definition, uint64_t chunkSize,
        int compressionLevel);
    %rename(openDataset) open(Dataset& dataset, const std::string& name);
    static std::shared_ptr<GeorefMetadataLayerDescriptor> open(Dataset& dataset,
        const std::string& name);

    GeorefMetadataLayerDescriptor(const GeorefMetadataLayerDescriptor&) = delete;
    GeorefMetadataLayerDescriptor(GeorefMetadataLayerDescriptor&&) = delete;

    bool operator==(const GeorefMetadataLayerDescriptor &rhs) const noexcept;
    bool operator!=(const GeorefMetadataLayerDescriptor &rhs) const noexcept;

    GeorefMetadataLayerDescriptor& operator=(const GeorefMetadataLayerDescriptor&) = delete;
    GeorefMetadataLayerDescriptor& operator=(GeorefMetadataLayerDescriptor&&) = delete;

    std::weak_ptr<Dataset> getDataset() const &;
    const RecordDefinition& getDefinition() const & noexcept;
    GeorefMetadataProfile getProfile();
};

}  // namespace BAG


