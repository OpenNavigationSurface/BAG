%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_dataset

%{
#include "bag_dataset.h"
%}

#define final


%include <std_shared_ptr.i>
%shared_ptr(BAG::Dataset)

%import "bag_compoundlayer.i"
%include <stl.i>
namespace std 
{
    %template(CompoundLayerVector) vector<BAG::CompoundLayer*>;
}

%import "bag_config.h"
%import "bag_descriptor.i"
%import "bag_metadata.i"
%import "bag_surfacecorrections.i"
%import "bag_trackinglist.i"
%import "bag_types.i"

namespace BAG {

class BAG_API Dataset final
#ifndef SWIG
    : public std::enable_shared_from_this<Dataset>
#endif
{
public:
    static std::shared_ptr<Dataset> read(const std::string & fileName,
        OpenMode openMode);
    static std::shared_ptr<Dataset> create(const std::string& fileName,
        Metadata&& metadata, uint64_t chunkSize, unsigned int compressionLevel);

    Dataset(const Dataset&) = delete;
    Dataset(Dataset&&) = delete;
    Dataset& operator=(const Dataset&) = delete;
    Dataset& operator=(Dataset&&) = delete;

    Layer& getLayer(uint32_t id)&;
    %ignore getLayer(uint32_t id) const&;
    std::vector<Layer*> getLayers() const&;

    Layer& createSimpleLayer(LayerType type, uint64_t chunkSize,
        unsigned int compressionLevel)&;
    CompoundLayer& createCompoundLayer(DataType indexType,
        const std::string& name, const RecordDefinition& definition,
        uint64_t chunkSize, unsigned int compressionLevel)&;
    SurfaceCorrections& createSurfaceCorrections(
        BAG_SURFACE_CORRECTION_TOPOGRAPHY type, uint8_t numCorrectors,
        uint64_t chunkSize, unsigned int compressionLevel)&;

    std::vector<LayerType> getLayerTypes() const;

    TrackingList& getTrackingList() & noexcept;
    %ignore getTrackingList() const& noexcept;
    const Metadata& getMetadata() const& noexcept;
    CompoundLayer* getCompoundLayer(const std::string& name) & noexcept;
    %ignore getCompoundLayer(const std::string& name) const& noexcept;
    
    //std::vector<CompoundLayer*> getCompoundLayers() & noexcept;  //TODO implement
    
    SurfaceCorrections* getSurfaceCorrections() & noexcept;
    %ignore getSurfaceCorrections() const& noexcept;

    Descriptor& getDescriptor() & noexcept;
    %ignore getDescriptor() const& noexcept;

    //TODO Perhaps a struct GridPoint {uint32_t column; uint32_t row;}; ?
    //TODO Perhaps a struct GeoPoint {double x; double y;}; ?
    std::tuple<double, double> gridToGeo(uint32_t row, uint32_t column) const noexcept;
    std::tuple<uint32_t, uint32_t> geoToGrid(double x, double y) const noexcept;
};
}
