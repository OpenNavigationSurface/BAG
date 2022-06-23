%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_dataset

%{
#include "bag_dataset.h"
%}

%import "bag_compoundlayer.i"
%import "bag_descriptor.i"
%import "bag_simplelayer.i"
%import "bag_surfacecorrections.i"
%import "bag_trackinglist.i"
%import "bag_types.i"
%import "bag_vrmetadata.i"
%import "bag_vrnode.i"
%import "bag_vrrefinements.i"
%import "bag_vrtrackinglist.i"

%include <std_vector.i>

namespace std
{
    %template(CompoundLayerVector) vector<BAG::CompoundLayer*>;
    %template(LayerVector) vector<BAG::Layer*>;
}

%include <std_string.i>
%include <std_shared_ptr.i>
%include <stdint.i>

%shared_ptr(BAG::Dataset)


namespace BAG {

class Dataset final
{
public:
    %rename(openDataset) open(const std::string &, OpenMode);
    static std::shared_ptr<Dataset> open(const std::string & fileName,
        OpenMode openMode);
    static std::shared_ptr<Dataset> create(const std::string& fileName,
        Metadata&& metadata, uint64_t chunkSize, int compressionLevel);

    Dataset(const Dataset&) = delete;
    Dataset(Dataset&&) = delete;

    Dataset& operator=(const Dataset&) = delete;
    Dataset& operator=(Dataset&&) = delete;

    Layer& getLayer(uint32_t id)&;
    %ignore getLayer(uint32_t id) const&;
    Layer* getLayer(LayerType type, const std::string& name) &;
    %ignore getLayer(LayerType type, const std::string& name) const &;
    std::vector<Layer*> getLayers() const&;

    std::vector<LayerType> getLayerTypes() const;

    Layer& createSimpleLayer(LayerType type, uint64_t chunkSize,
        int compressionLevel) &;
    CompoundLayer& createCompoundLayer(DataType indexType, GeorefMetadataProfile profile,
        const std::string& name, const RecordDefinition& definition,
        uint64_t chunkSize, int compressionLevel) &;
    CompoundLayer& createMetadataProfileCompoundLayer(GeorefMetadataProfile profile,
        const std::string& name,
        uint64_t chunkSize, int compressionLevel,
        DataType keyType = DT_UINT16) &;
    SurfaceCorrections& createSurfaceCorrections(
        BAG_SURFACE_CORRECTION_TOPOGRAPHY type, uint8_t numCorrectors,
        uint64_t chunkSize, int compressionLevel) &;

    void createVR(uint64_t chunkSize, int compressionLevel, bool makeNode);

    const Metadata& getMetadata() const & noexcept;

    TrackingList& getTrackingList() & noexcept;
    //const TrackingList& getTrackingList() const& noexcept;

    CompoundLayer* getCompoundLayer(const std::string& name) & noexcept;
    //const CompoundLayer* getCompoundLayer(const std::string& name) const & noexcept;
    std::vector<CompoundLayer*> getCompoundLayers() & noexcept;

    SurfaceCorrections* getSurfaceCorrections() & noexcept;
    //const SurfaceCorrections* getSurfaceCorrections() const & noexcept;

    SimpleLayer* getSimpleLayer(LayerType type) & noexcept;
    //const SimpleLayer* getSimpleLayer(LayerType type) const & noexcept;

    VRMetadata* getVRMetadata() & noexcept;
    //const VRMetadata* getVRMetadata() const & noexcept;

    VRNode* getVRNode() & noexcept;
    //const VRNode* getVRNode() const & noexcept;

    VRRefinements* getVRRefinements() & noexcept;
    // const VRRefinements* getVRRefinements() const & noexcept;

    VRTrackingList* getVRTrackingList() & noexcept;
    //const VRTrackingList* getVRTrackingList() const & noexcept;

    Descriptor& getDescriptor() & noexcept;
    //const Descriptor& getDescriptor() const & noexcept;

#if 0
    //! Intentionally omit exposing of std::tuple methods (unsupported by SWIG), 
    //! so they can be exposed with std::pair below.
    std::tuple<double, double> gridToGeo(uint32_t row, uint32_t column) const noexcept;
    std::tuple<uint32_t, uint32_t> geoToGrid(double x, double y) const noexcept;
#endif
};

%extend Dataset
{
    std::pair<double, double> gridToGeo(uint32_t row, uint32_t column) const noexcept
    {
        double x=0.0, y=0.0;
        std::tie(x, y) = self->gridToGeo(row, column);
        return std::pair<double, double>(x, y);
    }

    std::pair<uint32_t, uint32_t> geoToGrid(double x, double y) const noexcept
    {
        uint32_t row=0.0, column=0.0;
        std::tie(row, column) = self->geoToGrid(x, y);
        return std::pair<uint32_t, uint32_t>(row, column);
    }
}

}  // namespace BAG

