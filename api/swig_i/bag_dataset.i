%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_dataset

%{
#include "../bag_dataset.h"
%}

#define final

%import "bag_types.i"
%include <stl.i>
namespace std 
{
    %template(CompoundLayerVector) vector<BAG::CompoundLayer*>;
    %template(LayerVector) vector<BAG::Layer*>;
}

%import "../bag_config.h"
%import "bag_compoundlayer.i"
%import "bag_descriptor.i"
%import "bag_metadata.i"
%import "bag_surfacecorrections.i"
%import "bag_trackinglist.i"
%import "bag_types.i"
%import "bag_vrmetadata.i"

%include <pyabc.i>
%include <std_string.i>
%include <std_shared_ptr.i>
%include <stdint.i>

%shared_ptr(BAG::Dataset)

namespace BAG {

class BAG_API Dataset final
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
    std::vector<Layer*> getLayers() const&;

    std::vector<LayerType> getLayerTypes() const;

    Layer& createSimpleLayer(LayerType type, uint64_t chunkSize,
        int compressionLevel) &;
    CompoundLayer& createCompoundLayer(DataType indexType,
        const std::string& name, const RecordDefinition& definition,
        uint64_t chunkSize, int compressionLevel) &;
    SurfaceCorrections& createSurfaceCorrections(
        BAG_SURFACE_CORRECTION_TOPOGRAPHY type, uint8_t numCorrectors,
        uint64_t chunkSize, int compressionLevel) &;

    void createVR(uint64_t chunkSize, int compressionLevel, bool makeNode);

    TrackingList& getTrackingList() & noexcept;
    %ignore getTrackingList() const& noexcept;
    const Metadata& getMetadata() const & noexcept;
    CompoundLayer* getCompoundLayer(const std::string& name) & noexcept;
    %ignore getCompoundLayer(const std::string& name) const & noexcept;
    std::vector<CompoundLayer*> getCompoundLayers() & noexcept;
    SurfaceCorrections* getSurfaceCorrections() & noexcept;
    %ignore getSurfaceCorrections() const & noexcept;

    VRMetadata* getVRMetadata() & noexcept;
    %ignore getVRMetadata() const & noexcept;

#if 0  //TODO Enable rest as implemented.
    VRNode* getVRNode() & noexcept;
    %ignore getVRNode() const & noexcept;
    VRRefinement* getVRRefinement() & noexcept;
    %ignore getVRRefinement() const & noexcept;
    VRTrackingList* getVRTrackingList() & noexcept;
    %ignore getVRTrackingList() const & noexcept;
#endif

    Descriptor& getDescriptor() & noexcept;
    %ignore getDescriptor() const & noexcept;

#if 0
    //! Intentionally omit exposing of std::tuple methods (unsupported by SWIG), 
    //! so they can be exposed with std::pair below.
    std::tuple<double, double> gridToGeo(uint32_t row, uint32_t column) const noexcept;
    std::tuple<uint32_t, uint32_t> geoToGrid(double x, double y) const noexcept;
#endif
};
}


%extend BAG::Dataset
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