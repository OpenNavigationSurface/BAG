#ifndef BAG_DATASET_H
#define BAG_DATASET_H

#include "bag_compounddatatype.h"
#include "bag_config.h"
#include "bag_descriptor.h"
#include "bag_exceptions.h"
#include "bag_fordec.h"
#include "bag_layer.h"
#include "bag_metadata.h"
#include "bag_trackinglist.h"
#include "bag_types.h"
#include "bag_vrtrackinglist.h"

#include <functional>
#include <memory>
#include <numeric>
#include <string>
#include <unordered_map>
#include <vector>


namespace H5 {

class H5File;

}   //namespace H5

namespace BAG {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)  // std classes do not have DLL-interface when exporting
#pragma warning(disable: 4275)  // non-DLL-interface class used as base class
#endif

constexpr static uint32_t kInvalidId = std::numeric_limits<uint32_t>::max();

class BAG_API Dataset final
#ifndef SWIG
    : public std::enable_shared_from_this<Dataset>
#endif
{
public:
    static std::shared_ptr<Dataset> open(const std::string &fileName,
        OpenMode openMode);

    static std::shared_ptr<Dataset> create(const std::string &fileName,
        Metadata&& metadata, uint64_t chunkSize, unsigned int compressionLevel);

    Dataset(const Dataset&) = delete;
    Dataset(Dataset&&) = delete;
    Dataset& operator=(const Dataset&) = delete;
    Dataset& operator=(Dataset&&) = delete;

    Layer& getLayer(uint32_t id) &;
    const Layer& getLayer(uint32_t id) const &;
    std::vector<Layer*> getLayers() const &;

    std::vector<LayerType> getLayerTypes() const;

    Layer& createSimpleLayer(LayerType type, uint64_t chunkSize,
        unsigned int compressionLevel) &;
    CompoundLayer& createCompoundLayer(DataType indexType,
        const std::string& name, const RecordDefinition& definition,
        uint64_t chunkSize, unsigned int compressionLevel) &;
    SurfaceCorrections& createSurfaceCorrections(
        BAG_SURFACE_CORRECTION_TOPOGRAPHY type, uint8_t numCorrectors,
        uint64_t chunkSize, unsigned int compressionLevel) &;
    void createVR(uint64_t chunkSize, unsigned int compressionLevel);
    void createVRNode(uint64_t chunkSize, unsigned int compressionLevel);

    TrackingList& getTrackingList() & noexcept;
    const TrackingList& getTrackingList() const & noexcept;
    const Metadata& getMetadata() const & noexcept;
    CompoundLayer* getCompoundLayer(const std::string& name) & noexcept;
    const CompoundLayer* getCompoundLayer(const std::string& name) const & noexcept;
    std::vector<CompoundLayer*> getCompoundLayers() & noexcept;
    SurfaceCorrections* getSurfaceCorrections() & noexcept;
    const SurfaceCorrections* getSurfaceCorrections() const & noexcept;
    VRMetadata* getVRMetadata() & noexcept;
    const VRMetadata* getVRMetadata() const & noexcept;
    VRNode* getVRNode() & noexcept;
    const VRNode* getVRNode() const & noexcept;
    VRRefinement* getVRRefinement() & noexcept;
    const VRRefinement* getVRRefinement() const & noexcept;
    VRTrackingList* getVRTrackingList() & noexcept;
    const VRTrackingList* getVRTrackingList() const & noexcept;

    Descriptor& getDescriptor() & noexcept;
    const Descriptor& getDescriptor() const & noexcept;

    std::tuple<double, double> gridToGeo(uint32_t row, uint32_t column) const noexcept;
    std::tuple<uint32_t, uint32_t> geoToGrid(double x, double y) const noexcept;

private:
    Dataset() = default;
    uint32_t getNextId() const noexcept;

    void readDataset(const std::string& fileName, OpenMode openMode);
    void createDataset(const std::string& fileName, Metadata&& metadata,
        uint64_t chunkSize, unsigned int compressionLevel);

    //TODO What about uint32_t type min/max?
    std::tuple<bool, float, float> getMinMax(LayerType type,
        const std::string& path = {}) const;

    ::H5::H5File& getH5file() const & noexcept;

    Layer& addLayer(std::unique_ptr<Layer> layer) &;

    //! Custom deleter to not require knowledge of ::H5::H5File destructor here.
    struct BAG_API DeleteH5File final {
        void operator()(::H5::H5File* ptr) noexcept;
    };

    //! The HDF5 file.
    std::unique_ptr<::H5::H5File, DeleteH5File> m_pH5file;
    //! The layers.
    std::vector<std::unique_ptr<Layer>> m_layers;
    //! The metadata.
    std::unique_ptr<Metadata> m_pMetadata;
    //! The tracking list.
    std::unique_ptr<TrackingList> m_pTrackingList;
    //! The descriptor.
    Descriptor m_descriptor;
    //! The VR tracking list.
    std::unique_ptr<VRTrackingList> m_pVRTrackingList;

    friend class CompoundLayer;
    friend class CompoundLayerDescriptor;
    friend class InterleavedLayer;
    friend class InterleavedLayerDescriptor;
    friend class LayerDescriptor;
    friend class Metadata;
    friend class SimpleLayer;
    friend class TrackingList;
    friend class SimpleLayerDescriptor;
    friend class SurfaceCorrections;
    friend class SurfaceCorrectionsDescriptor;
    friend class ValueTable;
    friend class VRMetadata;
    friend class VRMetadataDescriptor;
    friend class VRNode;
    friend class VRNodeDescriptor;
    friend class VRRefinement;
    friend class VRRefinementDescriptor;
    friend class VRTrackingList;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

}   //namespace BAG

#endif  //BAG_DATASET_H

