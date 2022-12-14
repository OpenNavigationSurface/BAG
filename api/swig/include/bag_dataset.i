%begin %{
#ifdef _MSC_VER
#ifdef SWIGPYTHON
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
#endif
%}

%module bag_dataset

%{
#include "bag_dataset.h"
%}

%import "bag_layer.i"
%import "bag_georefmetadatalayer.i"
%import "bag_descriptor.i"
%import "bag_simplelayer.i"
%import "bag_surfacecorrections.i"
%import "bag_trackinglist.i"
%import "bag_types.i"
%import "bag_vrmetadata.i"
%import "bag_vrnode.i"
%import "bag_vrrefinements.i"
%import "bag_vrtrackinglist.i"

%include <std_string.i>
%include <stdint.i>

%include <std_shared_ptr.i>
%shared_ptr(BAG::Dataset)
%shared_ptr(BAG::Layer)

%include <std_vector.i>
namespace std
{
    %template(GeorefMetadataLayerVector) vector<shared_ptr<BAG::GeorefMetadataLayer>>;
    %template(LayerVector) vector<shared_ptr<BAG::Layer>>;
}


namespace BAG {

class Dataset final
{
public:
    %rename(openDataset) open(const std::string &, OpenMode);
    static std::shared_ptr<Dataset> open(const std::string & fileName,
        OpenMode openMode);

    static std::shared_ptr<Dataset> create(const std::string& fileName,
        Metadata&& metadata, uint64_t chunkSize, int compressionLevel);

    void close();

    Dataset(const Dataset&) = delete;
    Dataset(Dataset&&) = delete;

    Dataset& operator=(const Dataset&) = delete;
    Dataset& operator=(Dataset&&) = delete;

    bool operator==(const Dataset &rhs) const noexcept;
    bool operator!=(const Dataset &rhs) const noexcept;

    Layer& getLayer(uint32_t id)&;
    %ignore getLayer(uint32_t id) const&;
    std::shared_ptr<Layer> getLayer(LayerType type, const std::string& name) &;
    %ignore getLayer(LayerType type, const std::string& name) const &;
    %ignore getLayers() const&;

    std::vector<LayerType> getLayerTypes() const;

    Layer& createSimpleLayer(LayerType type, uint64_t chunkSize,
        int compressionLevel) &;
    GeorefMetadataLayer& createGeorefMetadataLayer(DataType indexType, GeorefMetadataProfile profile,
        const std::string& name, const RecordDefinition& definition,
        uint64_t chunkSize, int compressionLevel) &;
    %rename(createMetadataProfileGeorefMetadataLayer) createGeorefMetadataLayer(GeorefMetadataProfile profile,
                                                                                const std::string& name,
                                                                                uint64_t chunkSize,
                                                                                int compressionLevel,
                                                                                DataType keyType = DT_UINT16) &;
    GeorefMetadataLayer& createGeorefMetadataLayer(GeorefMetadataProfile profile,
        const std::string& name,
        uint64_t chunkSize, int compressionLevel,
        DataType keyType = DT_UINT16) &;
    SurfaceCorrections& createSurfaceCorrections(
        BAG_SURFACE_CORRECTION_TOPOGRAPHY type, uint8_t numCorrectors,
        uint64_t chunkSize, int compressionLevel) &;

    void createVR(uint64_t chunkSize, int compressionLevel, bool makeNode);

    const Metadata& getMetadata() const & noexcept;

    TrackingList& getTrackingList() & noexcept;

    std::shared_ptr<GeorefMetadataLayer> getGeorefMetadataLayer(const std::string& name) & noexcept;
    std::vector<std::shared_ptr<GeorefMetadataLayer>> getGeorefMetadataLayers() & noexcept;

    std::shared_ptr<SurfaceCorrections> getSurfaceCorrections() & noexcept;

    std::shared_ptr<SimpleLayer> getSimpleLayer(LayerType type) & noexcept;

    std::shared_ptr<VRMetadata> getVRMetadata() & noexcept;

    std::shared_ptr<VRNode> getVRNode() & noexcept;

    std::shared_ptr<VRRefinements> getVRRefinements() & noexcept;

    std::shared_ptr<VRTrackingList> getVRTrackingList() & noexcept;

    Descriptor& getDescriptor() & noexcept;

    // Converted to std::pair<T, T> below.
    //! Intentionally omit exposing of std::tuple methods (unsupported by SWIG), 
    //! so they can be exposed with std::pair below.
    //std::tuple<double, double> gridToGeo(uint32_t row, uint32_t column) const noexcept;
    //std::tuple<uint32_t, uint32_t> geoToGrid(double x, double y) const noexcept;
};

%extend Dataset
{
    // Manually create non-const getLayers() due to errors in how SWIG handles vectors of const shared_ptr
    //   For more information, see: https://github.com/swig/swig/issues/753
    std::vector<std::shared_ptr<BAG::Layer>> Dataset::getLayers() &
    {
        std::vector<std::shared_ptr<BAG::Layer const>> smartPtrLayers = $self->getLayers();
        std::vector<std::shared_ptr<BAG::Layer>> rawPtrlayers;
        rawPtrlayers.reserve(smartPtrLayers.size());

        for (std::shared_ptr<BAG::Layer const> smartPtrLayer : smartPtrLayers) {
            rawPtrlayers.push_back(std::const_pointer_cast<BAG::Layer>(smartPtrLayer));
        }

        return rawPtrlayers;
    }

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

    #ifdef SWIGPYTHON
    %pythoncode %{
        def __del__(self):
            self.close()
    %}
    // TODO: Add context manager for Dataset to allow use with ``with``.
    #endif
}

}  // namespace BAG

