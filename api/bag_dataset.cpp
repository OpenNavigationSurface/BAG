
#include "bag_attributeinfo.h"
#include "bag_compoundlayer.h"
#include "bag_compoundlayerdescriptor.h"
#include "bag_dataset.h"
#include "bag_interleavedlayer.h"
#include "bag_interleavedlayerdescriptor.h"
#include "bag_metadata_export.h"
#include "bag_private.h"
#include "bag_simplelayer.h"
#include "bag_simplelayerdescriptor.h"
#include "bag_surfacecorrections.h"
#include "bag_surfacecorrectionsdescriptor.h"
#include "bag_version.h"
#include "bag_vrmetadata.h"
#include "bag_vrmetadatadescriptor.h"
#include "bag_vrnode.h"
#include "bag_vrnodedescriptor.h"
#include "bag_vrrefinements.h"
#include "bag_vrrefinementsdescriptor.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <H5Cpp.h>
#include <map>
#include <regex>
#include <string>


namespace BAG {

namespace {

//! Find a layer by type and case-insensitive name.
/*!
\param layers
    The layers to search.
\param type
    The type of layer to find.
\param name
    The case-insensitive name of the layer to find.
    This is optional unless looking for a compound layer.

\return
    The found layer.
    nullptr if not found.
*/
Layer* getLayer(
    const std::vector<std::unique_ptr<Layer>>& layers,
    LayerType type,
    const std::string& name = {})
{
    if (type == Compound && name.empty())
        throw NameRequired{};

    std::string nameLower{name};
    std::transform(begin(nameLower), end(nameLower), begin(nameLower),
        [](char c) noexcept {
            return static_cast<char>(std::tolower(c));
        });

    auto layerIter = std::find_if(cbegin(layers), cend(layers),
        [type, &nameLower](const std::unique_ptr<Layer>& layer) {
            auto pDescriptor = layer->getDescriptor();

            if (pDescriptor->getLayerType() != type)
                return false;

            if (nameLower.empty())
                return true;

            std::string foundNameLower{pDescriptor->getName()};
            std::transform(begin(foundNameLower), end(foundNameLower), begin(foundNameLower),
                [](char c) noexcept {
                    return static_cast<char>(std::tolower(c));
                });

            return foundNameLower == nameLower;
        });
    if (layerIter == cend(layers))
        return {};

    return layerIter->get();
}

//! Get the numerical version.
/*!
    Expecting "major" (2) or "major.minor" (2.0) or "major.minor.patch" (2.0.0),
    which will be turned into a number such that:

        major * 1,000,000 + minor * 1,000 + patch is returned.

\param versionStr
    The version as a string.  Eg., "2.1.4".

\return
    The numerical representation of the version.
    0 if the version string cannot be parsed.
*/
uint32_t getNumericalVersion(
    const std::string& versionStr)
{
    const std::regex expression{R"(\.)"};  // match any period

    auto b = std::sregex_token_iterator{begin(versionStr), end(versionStr),
        expression, -1};
    const auto e = std::sregex_token_iterator{};


    uint32_t version = 0;
    uint32_t multiplier = 1'000'000;

    for (auto iter = b; iter != e; ++iter)
    {
        try
        {
            version += std::stoul(*iter) * multiplier;
        }
        catch(...)
        {
            return 0;
        }

        multiplier /= 1000;
    }

    return version;
}

//! A type alias to use SFINAE to check if a type is not a std::string.
template <typename T>
using IsNotString = std::enable_if_t<!std::is_same<T, std::string>::value>;

//! Helper to read a non-string attribute from an HDF5 DataSet.
/*!
\param h5file
    The HDF5 file to read.
\param dataSetName
    The name of the HDF5 DataSet.
\param attributeName
    The name of the attribute.

\return
    The attribute value.
*/
template <typename T, typename = IsNotString<T>>
T readAttributeFromDataSet(
    const ::H5::H5File& h5file,
    const std::string& dataSetName,
    const std::string& attributeName)
{
    const ::H5::DataSet h5DataSet = h5file.openDataSet(dataSetName);
    const ::H5::Attribute attribute = h5DataSet.openAttribute(attributeName);

    T value{};
    attribute.read(attribute.getDataType(), &value);

    return value;
}

//! Helper to read a string attribute from an HDF5 DataSet.
/*!
\param h5file
    The HDF5 file to read.
\param dataSetName
    The name of the HDF5 DataSet.
\param attributeName
    The name of the attribute.

\return
    The attribute value as a string.
*/
std::string readStringAttributeFromDataSet(
    const ::H5::H5File& h5file,
    const std::string& dataSetName,
    const std::string& attributeName)
{
    const ::H5::DataSet h5DataSet = h5file.openDataSet(dataSetName);
    const ::H5::Attribute attribute = h5DataSet.openAttribute(attributeName);

    std::string value;
    attribute.read(attribute.getDataType(), value);

    return value;
}

//! Helper to read a non-string attribute from an HDF5 Group.
/*!
\param h5file
    The HDF5 file to read.
\param groupName
    The name of the HDF5 Group.
\param attributeName
    The name of the attribute.

\return
    The attribute value.
*/
template <typename T, typename = IsNotString<T>>
T readAttributeFromGroup(
    const ::H5::H5File& h5file,
    const std::string& groupName,
    const std::string& attributeName)
{
    const ::H5::Group h5group = h5file.openGroup(groupName);
    const ::H5::Attribute attribute = h5group.openAttribute(attributeName);

    T value{};
    attribute.read(attribute.getDataType(), &value);

    return value;
}

//! Helper to read a string attribute from an HDF5 Group.
/*!
\param h5file
    The HDF5 file to read.
\param groupName
    The name of the HDF5 Group.
\param attributeName
    The name of the attribute.

\return
    The attribute value as a string.
*/
std::string readStringAttributeFromGroup(
    const ::H5::H5File& h5file,
    const std::string& groupName,
    const std::string& attributeName)
{
    const ::H5::Group h5group = h5file.openGroup(groupName);
    const ::H5::Attribute attribute = h5group.openAttribute(attributeName);

    std::string value;
    attribute.read(attribute.getDataType(), value);

    return value;
}

}  // namespace

//! Open an existing BAG.
/*!
\param fileName
    The name of the BAG.
\param openMode
    The mode to open the BAG with.

\return
    The BAG Dataset.
*/
std::shared_ptr<Dataset> Dataset::open(
    const std::string& fileName,
    OpenMode openMode)
{
#ifdef NDEBUG
    ::H5::Exception::dontPrint();
#endif

    std::shared_ptr<Dataset> pDataset{new Dataset};
    pDataset->readDataset(fileName, openMode);

    return pDataset;
}

//! Create a BAG.
/*!
\param fileName
    The name of the BAG.
\param metadata
    The metadata describing the BAG.
    This parameter will be moved, and not usable after.
\param chunkSize
    The chunk size the HDF5 DataSet will use.
\param compressionLevel
    The compression level the HDF5 DataSet will use.

\return
    The BAG Dataset.
*/
std::shared_ptr<Dataset> Dataset::create(
    const std::string& fileName,
    Metadata&& metadata,
    uint64_t chunkSize,
    int compressionLevel)
{
    std::shared_ptr<Dataset> pDataset{new Dataset};
    pDataset->createDataset(fileName, std::move(metadata), chunkSize,
        compressionLevel);

    return pDataset;
}


//! Add a layer to this dataset.
/*!
\param newLayer
    The new layer.

\return
    A reference to the new layer.
*/
Layer& Dataset::addLayer(
    std::unique_ptr<Layer> newLayer) &
{
    m_layers.emplace_back(std::move(newLayer));

    const auto& layer = m_layers.back();

    m_descriptor.addLayerDescriptor(*layer->getDescriptor());

    return *layer;
}

//! Create a compound layer.
/*!
\param indexType
    The type of index the compound layer will use.
    Valid values are: ???
\param name
    The name of the simple layer this compound layer has metadata for.
\param definition
    The list of fields defining a record of the compound layer.
\param chunkSize
    The chunk size the HDF5 DataSet will use.
\param compressionLevel
    The compression level the HDF5 DataSet will use.

\return
    The new compound layer.
*/
CompoundLayer& Dataset::createCompoundLayer(
    DataType indexType,
    const std::string& name,
    const RecordDefinition& definition,
    uint64_t chunkSize,
    int compressionLevel) &
{
    if (m_descriptor.isReadOnly())
        throw ReadOnlyError{};

    std::string nameLower{name};
    std::transform(begin(nameLower), end(nameLower), begin(nameLower),
        [](char c) noexcept {
            return static_cast<char>(std::tolower(c));
        });

    // Make sure a corresponding simple layer exists.
    const bool simpleLayerExists = std::any_of(cbegin(m_layers), cend(m_layers),
        [&nameLower](const std::unique_ptr<Layer>& layer) {
            auto pDescriptor = layer->getDescriptor();

            const auto layerType = pDescriptor->getLayerType();

            // Skip non-simple layers.
            if (layerType == Compound || layerType == VarRes_Metadata ||
                layerType == VarRes_Refinement || layerType == VarRes_Node)
                return false;

            std::string simpleNameLower{pDescriptor->getName()};
            std::transform(begin(simpleNameLower), end(simpleNameLower), begin(simpleNameLower),
                [](char c) noexcept {
                    return static_cast<char>(std::tolower(c));
                });

            return simpleNameLower == nameLower;
        });
    if (!simpleLayerExists)
        throw LayerNotFound{};

    // Make sure the compound layer does not already exist.
    if (this->getCompoundLayer(name))
        throw LayerExists{};

    // Create the group if it does not exist.
    const auto id = H5Gopen2(m_pH5file->getId(), COMPOUND_PATH, H5P_DEFAULT);
    if (id == -1)
        m_pH5file->createGroup(COMPOUND_PATH);
    else
        H5Gclose(id);

    return dynamic_cast<CompoundLayer&>(this->addLayer(CompoundLayer::create(
        indexType, name, *this, definition, chunkSize, compressionLevel)));
}

//! Create a new Dataset.
/*!
\param fileName
    The name of the new BAG.
\param metadata
    The metadata to be used by the BAG.
\param chunkSize
    The chunk size the HDF5 DataSet will use.
\param compressionLevel
    The compression level the HDF5 DataSet will use.
*/
void Dataset::createDataset(
    const std::string& fileName,
    Metadata&& metadata,
    uint64_t chunkSize,
    int compressionLevel)
{
#ifdef NDEBUG
    ::H5::Exception::dontPrint();
#endif

    m_pH5file = std::unique_ptr<::H5::H5File, DeleteH5File>(new ::H5::H5File{
        fileName.c_str(), H5F_ACC_EXCL}, DeleteH5File{});

    // Group: BAG_root
    {
        auto h5bagGroup = m_pH5file->createGroup(ROOT_PATH);

        const auto versionAttType = ::H5::StrType{0, BAG_VERSION_LENGTH};
        const ::H5::DataSpace versionAttDataSpace{};
        auto versionAtt = h5bagGroup.createAttribute(BAG_VERSION_NAME,
            versionAttType, versionAttDataSpace);

        versionAtt.write(versionAttType, BAG_VERSION);
    }

    // Metadata
    metadata.createH5dataSet(*this);
    metadata.write();
    m_pMetadata = std::make_unique<Metadata>(std::move(metadata));

    m_descriptor = Descriptor{*m_pMetadata};
    m_descriptor.setReadOnly(false);
    m_descriptor.setVersion(BAG_VERSION);

    // TrackingList
    m_pTrackingList = std::unique_ptr<TrackingList>(new TrackingList{*this,
        compressionLevel});

    // Mandatory Layers
    // Elevation
    this->addLayer(SimpleLayer::create(*this, Elevation, chunkSize,
        compressionLevel));

    // Uncertainty
    this->addLayer(SimpleLayer::create(*this, Uncertainty, chunkSize,
        compressionLevel));
}

//! Create an optional simple layer.
/*!
\param type
    The type of layer to create.
    The layer cannot currently exist.
\param chunkSize
    The chunk size the HDF5 DataSet will use.
\param compressionLevel
    The compression level the HDF5 DataSet will use.

\return
    The new layer.
*/
Layer& Dataset::createSimpleLayer(
    LayerType type,
    uint64_t chunkSize,
    int compressionLevel) &
{
    if (m_descriptor.isReadOnly())
        throw ReadOnlyError{};

    // Make sure it doesn't already exist.
    if (BAG::getLayer(m_layers, type))
        throw LayerExists{};

    switch (type)
    {
    case Elevation:  //[[fallthrough]];
    case Uncertainty:  //[[fallthrough]];
    case Hypothesis_Strength:  //[[fallthrough]];
    case Num_Hypotheses:  //[[fallthrough]];
    case Shoal_Elevation:  //[[fallthrough]];
    case Std_Dev:  //[[fallthrough]];
    case Num_Soundings:  //[[fallthrough]];
    case Average_Elevation:  //[[fallthrough]];
    case Nominal_Elevation:
        return this->addLayer(SimpleLayer::create(*this, type, chunkSize,
            compressionLevel));
    case Surface_Correction:  //[[fallthrough]];
    case Compound:  //[[fallthrough]];
    default:
        throw UnsupportedLayerType{};
    }
}

//! Create optional surface corrections layer.
/*!
\param type
    The type of topography.
    Gridded (BAG_SURFACE_GRID_EXTENTS) or sparse (BAG_SURFACE_IRREGULARLY_SPACED).
\param numCorrectors
    The number of correctors to use (1-10).
\param chunkSize
    The chunk size the HDF5 DataSet will use.
\param compressionLevel
    The compression level the HDF5 DataSet will use.

\return
    The new surface corrections layer.
*/
SurfaceCorrections& Dataset::createSurfaceCorrections(
    BAG_SURFACE_CORRECTION_TOPOGRAPHY type,
    uint8_t numCorrectors,
    uint64_t chunkSize,
    int compressionLevel) &
{
    if (m_descriptor.isReadOnly())
        throw ReadOnlyError{};

    // Make sure surface corrections do not already exist.
    if (this->getSurfaceCorrections())
        throw LayerExists{};

    return dynamic_cast<SurfaceCorrections&>(this->addLayer(
        SurfaceCorrections::create(*this, type, numCorrectors, chunkSize,
            compressionLevel)));
}

//! Create optional variable resolution layers.
/*!
\param chunkSize
    The chunk size the HDF5 DataSet will use.
\param compressionLevel
    The compression level the HDF5 DataSet will use.
*/
void Dataset::createVR(
    uint64_t chunkSize,
    int compressionLevel,
    bool createNode)
{
    if (m_descriptor.isReadOnly())
        throw ReadOnlyError{};

    // Make sure VR layers do not already exist.
    if (this->getVRMetadata())
        throw LayerExists{};

    //TODO Consider a try/catch to undo partial creation.

    m_pVRTrackingList = std::unique_ptr<VRTrackingList>(
        new VRTrackingList{*this, compressionLevel});

    this->addLayer(VRMetadata::create(*this, chunkSize, compressionLevel));
    this->addLayer(VRRefinements::create(*this, chunkSize, compressionLevel));

    if (createNode)
        this->addLayer(VRNode::create(*this, chunkSize, compressionLevel));
}

//! Convert a geographic location to grid position.
/*!
\param x
    The X of the geographic location.
\param y
    The Y of the geographic location.

\return
    The grid position (row, column).
*/
std::tuple<uint32_t, uint32_t> Dataset::geoToGrid(
    double x,
    double y) const noexcept
{
    const auto row = static_cast<uint32_t>((x - m_pMetadata->llCornerX()) /
        m_pMetadata->rowResolution());
    const auto column = static_cast<uint32_t>((y - m_pMetadata->llCornerY()) /
        m_pMetadata->columnResolution());

    return {row, column};
}

//! Retrieve an optional compound layer by name.
/*!
\param name
    The name of the simple layer the compound layer has metadata for.

\return
    The specified compound layer, if it exists.  nullptr otherwise
*/
CompoundLayer* Dataset::getCompoundLayer(
    const std::string& name) & noexcept
{
    return dynamic_cast<CompoundLayer*>(BAG::getLayer(m_layers, Compound, name));
}

//! Retrieve an optional compound layer by name.
/*!
\param name
    The name of the simple layer the compound layer has metadata for.

\return
    The specified compound layer, if it exists.  nullptr otherwise
*/
const CompoundLayer* Dataset::getCompoundLayer(
    const std::string& name) const & noexcept
{
    return dynamic_cast<CompoundLayer*>(BAG::getLayer(m_layers, Compound, name));
}

//! Retrieve all the compound layers.
/*!
\return
    All the compound layers.
*/
std::vector<CompoundLayer*> Dataset::getCompoundLayers() & noexcept
{
    std::vector<CompoundLayer*> layers;

    for (const auto& layer : m_layers)
        if (layer->getDescriptor()->getLayerType() == Compound)
            layers.emplace_back(dynamic_cast<CompoundLayer*>(layer.get()));

    return layers;
}

//! Retrieve the dataset's descriptor.
/*!
\return
    The dataset's descriptor.
*/
Descriptor& Dataset::getDescriptor() & noexcept
{
    return m_descriptor;
}

//! Retrieve the dataset's descriptor.
/*!
\return
    The dataset's descriptor.
*/
const Descriptor& Dataset::getDescriptor() const & noexcept
{
    return m_descriptor;
}

//! Retrieve the HDF5 file that contains this BAG.
/*!
\return
    The HDF5 file that contains to this BAG.
*/
::H5::H5File& Dataset::getH5file() const & noexcept
{
    return *m_pH5file;
}

//! Retrieve a layer by its unique id.
/*!
    Retrieve a layer by its unique id.  If it is not found, an InvalidLayerId
    exception is thrown.

\param id
    The unique id of the layer.

\return
    The layer specified by the id.
*/
Layer& Dataset::getLayer(uint32_t id) &
{
    if (id >= m_layers.size())
        throw InvalidLayerId{};

    return *m_layers[id];
}

//! Retrieve a layer by its unique id.
/*!
    Retrieve a layer by its unique id.  If it is not found, an InvalidLayerId
    exception is thrown.

\param id
    The unique id of the layer.

\return
    The layer specified by the id.
*/
const Layer& Dataset::getLayer(uint32_t id) const &
{
    if (id >= m_layers.size())
        throw InvalidLayerId{};

    return *m_layers[id];
}

//! Retrieve a layer based on type and case-insensitive name.
/*!
\param type
    The layer type.
\param name
    The optional, case-insensitive name.
    If the layer type is Compound, the name must be the simple layer it refers to.

\return
    The specified layer.
    nullptr if no layer is found.
*/
Layer* Dataset::getLayer(
    LayerType type,
    const std::string& name) &
{
    return BAG::getLayer(m_layers, type, name);
}

//! Retrieve a layer based on type and case-insensitive name.
/*!
\param type
    The layer type.
\param name
    The optional, case-insensitive name.
    If the layer type is Compound, the name must be the simple layer it refers to.
*/
const Layer* Dataset::getLayer(
    LayerType type,
    const std::string& name) const &
{
    return BAG::getLayer(m_layers, type, name);
}

//! Retrieve all the layers.
/*!
\return
    All the layers.
*/
std::vector<Layer*> Dataset::getLayers() const &
{
    std::vector<Layer*> layers;
    layers.reserve(m_layers.size());

    for (auto&& layer : m_layers)
        layers.push_back(layer.get());

    return layers;
}

//! Retrieve the layer types.
/*!
\return
    The layer types.
    If multiple compound layers exist, Compound_Layer will only be present once.
*/
std::vector<LayerType> Dataset::getLayerTypes() const
{
    std::vector<LayerType> types;
    types.reserve(m_layers.size());

    bool compoundLayerAdded = false;

    for (auto&& layer : m_layers)
    {
        const auto type = layer->getDescriptor()->getLayerType();
        if (type == Compound)
        {
            if (compoundLayerAdded)
                continue;
            else
                compoundLayerAdded = true;
        }

        types.push_back(type);
    }

    return types;
}

//! Retrieve the metadta.
/*!
\return
    The metadata.
*/
const Metadata& Dataset::getMetadata() const & noexcept
{
    return *m_pMetadata;
}

//! Retrieve the minimum and maximum values of a simple layer.
/*!
\param type
    The type of simple layer.
\param path
    The optional internal HDF5 path to use.

\return
    A tuple of results.
    The first value in the tuple indicates whether a value was found or not.
    If the first value is true, the second value is the minimum, and the third value is the maximum.
    If the first value is false, the second and third values are undefined.
*/
std::tuple<bool, float, float> Dataset::getMinMax(
    LayerType type,
    const std::string& path) const
{
    try
    {
        const auto info = getAttributeInfo(type);
        const auto& thePath = path.empty() ? info.path : path;

        return {true,
            readAttributeFromDataSet<float>(*m_pH5file, thePath, info.minName),
            readAttributeFromDataSet<float>(*m_pH5file, thePath, info.maxName)};
    }
    catch(const UnsupportedSimpleLayerType&)
    {
        return {false, 0.f, 0.f};  // No min/max attributes.
    }
}

//! Retrieve the next unique layer id.
/*!
\return
    The next unique layer id.
*/
uint32_t Dataset::getNextId() const noexcept
{
    return static_cast<uint32_t>(m_layers.size());
}

//! Retrieve the specified simple layer.
/*!
\param type
    The layer type.

\return
    The specified simple l ayer.
    nullptr if the layer does not exist.
*/
SimpleLayer* Dataset::getSimpleLayer(
    LayerType type) & noexcept
{
    return dynamic_cast<SimpleLayer*>(BAG::getLayer(m_layers, type));
}

//! Retrieve the specified simple layer.
/*!
\param type
    The layer type.

\return
    The specified simple l ayer.
    nullptr if the layer does not exist.
*/
const SimpleLayer* Dataset::getSimpleLayer(
    LayerType type) const & noexcept
{
    return dynamic_cast<SimpleLayer*>(BAG::getLayer(m_layers, type));
}

//! Retrieve the optional surface corrections layer.
/*!
\return
    The optional surface corrections layer.
*/
SurfaceCorrections* Dataset::getSurfaceCorrections() & noexcept
{
    return dynamic_cast<SurfaceCorrections*>(
        BAG::getLayer(m_layers, Surface_Correction));
}

//! Retrieve the optional surface corrections layer.
/*!
\return
    The optional surface corrections layer.
*/
const SurfaceCorrections* Dataset::getSurfaceCorrections() const & noexcept
{
    return dynamic_cast<SurfaceCorrections*>(
        BAG::getLayer(m_layers, Surface_Correction));
}

//! Retrieve the tracking list.
/*!
\return
    The tracking list.
*/
TrackingList& Dataset::getTrackingList() & noexcept
{
    return *m_pTrackingList;
}

//! Retrieve the tracking list.
/*!
\return
    The tracking list.
*/
const TrackingList& Dataset::getTrackingList() const & noexcept
{
    return *m_pTrackingList;
}

//! Retrieve the optional variable resolution metadata.
/*!
\return
    The optional variable resolution metadata.
*/
VRMetadata* Dataset::getVRMetadata() & noexcept
{
    return dynamic_cast<VRMetadata*>(BAG::getLayer(m_layers, VarRes_Metadata));
}

//! Retrieve the optional variable resolution metadata.
/*!
\return
    The optional variable resolution metadata.
*/
const VRMetadata* Dataset::getVRMetadata() const & noexcept
{
    return dynamic_cast<VRMetadata*>(BAG::getLayer(m_layers, VarRes_Metadata));
}

//! Retrieve the optional variable resolution node group.
/*!
\return
    The optional variable resolution node group.
*/
VRNode* Dataset::getVRNode() & noexcept
{
    return dynamic_cast<VRNode*>(BAG::getLayer(m_layers, VarRes_Node));
}

//! Retrieve the optional variable resolution node group.
/*!
\return
    The optional variable resolution node group.
*/
const VRNode* Dataset::getVRNode() const & noexcept
{
    return dynamic_cast<VRNode*>(BAG::getLayer(m_layers, VarRes_Node));
}

//! Retrieve the optional variable resolution refinements.
/*!
\return
    The optional variable resolution refinements.
*/
VRRefinements* Dataset::getVRRefinements() & noexcept
{
    return dynamic_cast<VRRefinements*>(
        BAG::getLayer(m_layers, VarRes_Refinement));
}

//! Retrieve the optional variable resolution refinements.
/*!
\return
    The optional variable resolution refinements.
*/
const VRRefinements* Dataset::getVRRefinements() const & noexcept
{
    return dynamic_cast<VRRefinements*>(
        BAG::getLayer(m_layers, VarRes_Refinement));
}

//! Retrieve the optional variable resolution tracking list.
/*!
\return
    The optional variable resolution tracking list.
*/
VRTrackingList* Dataset::getVRTrackingList() & noexcept
{
    return m_pVRTrackingList.get();
}

//! Retrieve the optional variable resolution tracking list.
/*!
\return
    The optional variable resolution tracking list.
*/
const VRTrackingList* Dataset::getVRTrackingList() const & noexcept
{
    return m_pVRTrackingList.get();
}

//! Convert a grid position to a geographic location.
/*!
\param row
    The grid row.
\param column
    The grid column.

\return
    The geographic position.
*/
std::tuple<double, double> Dataset::gridToGeo(
    uint32_t row,
    uint32_t column) const noexcept
{
    const auto x = m_pMetadata->llCornerX() +
        (row * m_pMetadata->rowResolution());

    const auto y = m_pMetadata->llCornerY() +
        (column * m_pMetadata->columnResolution());

    return {x, y};
}

//! Read an existing BAG.
/*!
\param fileName
    The name of the BAG.
\param openMode
    The mode to open the BAG with.
*/
void Dataset::readDataset(
    const std::string& fileName,
    OpenMode openMode)
{
    m_pH5file = std::unique_ptr<::H5::H5File, DeleteH5File>(new ::H5::H5File{
        fileName.c_str(),
        (openMode == BAG_OPEN_READONLY) ? H5F_ACC_RDONLY : H5F_ACC_RDWR},
        DeleteH5File{});

    m_pMetadata = std::make_unique<Metadata>(*this);

    m_descriptor = Descriptor{*m_pMetadata};
    m_descriptor.setReadOnly(openMode == BAG_OPEN_READONLY);
    m_descriptor.setVersion(readStringAttributeFromGroup(*m_pH5file,
        ROOT_PATH, BAG_VERSION_NAME));

    const auto bagGroup = m_pH5file->openGroup(ROOT_PATH);

    // Look for the simple layers.
    for (auto layerType : {Elevation, Uncertainty, Hypothesis_Strength,
        Num_Hypotheses, Shoal_Elevation, Std_Dev, Num_Soundings,
        Average_Elevation, Nominal_Elevation})
    {
        const std::string internalPath = Layer::getInternalPath(layerType);
        if (internalPath.empty())
            continue;

        const hid_t id = H5Dopen2(bagGroup.getLocId(), internalPath.c_str(),
            H5P_DEFAULT);
        if (id < 0)
            continue;

        H5Dclose(id);

        auto layerDesc = SimpleLayerDescriptor::open(*this, layerType);
        this->addLayer(SimpleLayer::open(*this, *layerDesc));
    }

    const auto bagVersion = getNumericalVersion(m_descriptor.getVersion());

    // If the BAG is version 1.5+ ...
    if (bagVersion >= 1'005'000)
    {
        hid_t id = H5Dopen2(bagGroup.getLocId(), NODE_GROUP_PATH, H5P_DEFAULT);
        if (id >= 0)
        {
            H5Dclose(id);

            // Hypothesis_Strength
            auto layerDesc = InterleavedLayerDescriptor::open(*this,
                Hypothesis_Strength, NODE);
            this->addLayer(InterleavedLayer::open(*this, *layerDesc));

            // Num_Hypotheses
            layerDesc = InterleavedLayerDescriptor::open(*this, Num_Hypotheses,
                NODE);
            this->addLayer(InterleavedLayer::open(*this, *layerDesc));
        }

        id = H5Dopen2(bagGroup.getLocId(), ELEVATION_SOLUTION_GROUP_PATH,
            H5P_DEFAULT);
        if (id >= 0)
        {
            H5Dclose(id);

            // Shoal_Elevation
            auto layerDesc = InterleavedLayerDescriptor::open(*this,
                Shoal_Elevation, ELEVATION);

            this->addLayer(InterleavedLayer::open(*this, *layerDesc));

            // Std_Dev
            layerDesc = InterleavedLayerDescriptor::open(*this, Std_Dev,
                ELEVATION);
            this->addLayer(InterleavedLayer::open(*this, *layerDesc));

            // Num_Soundings
            layerDesc = InterleavedLayerDescriptor::open(*this, Num_Soundings,
                ELEVATION);
            this->addLayer(InterleavedLayer::open(*this, *layerDesc));
        }
    }

    // If the BAG is version 2.0+ ...
    if (bagVersion >= 2'000'000)
    {
        // Add all existing CompoundLayers
        hid_t id = H5Gopen2(bagGroup.getLocId(), COMPOUND_PATH, H5P_DEFAULT);
        if (id >= 0)
        {
            H5Gclose(id);

            // Look for all ::H5::DataSets in the Group COMPOUND_PATH that don't end with "_records"
            const auto group = m_pH5file->openGroup(COMPOUND_PATH);

            const hsize_t numObjects = group.getNumObjs();
            constexpr size_t kRecordsLen = 8;  // length of "_records".

            for (auto i=0; i<numObjects; ++i)
            {
                try
                {
                    const auto name = group.getObjnameByIdx(i);

                    // Skip any x_records DataSets.
                    const auto foundPos = name.rfind(COMPOUND_RECORDS);
                    if ((foundPos != std::string::npos) &&
                        (foundPos == (name.length() - kRecordsLen)))
                        continue;

                    auto descriptor = CompoundLayerDescriptor::open(*this, name);
                    this->addLayer(CompoundLayer::open(*this, *descriptor));
                }
                catch(...)
                {}
            }
        }
    }

    m_pTrackingList = std::unique_ptr<TrackingList>(new TrackingList{*this});

    // Read optional Surface Corrections
    hid_t id = H5Dopen2(bagGroup.getLocId(), VERT_DATUM_CORR_PATH, H5P_DEFAULT);
    if (id >= 0)
    {
        H5Dclose(id);

        auto descriptor = SurfaceCorrectionsDescriptor::open(*this);
        this->addLayer(SurfaceCorrections::open(*this, *descriptor));
    }

    // Read optional VR
    id = H5Dopen2(bagGroup.getLocId(), VR_TRACKING_LIST_PATH, H5P_DEFAULT);
    if (id >= 0)
    {
        H5Dclose(id);

        m_pVRTrackingList = std::unique_ptr<VRTrackingList>(
            new VRTrackingList{*this});

        {
            auto descriptor = VRMetadataDescriptor::open(*this);
            this->addLayer(VRMetadata::open(*this, *descriptor));
        }

        {
            auto descriptor = VRRefinementsDescriptor::open(*this);
            this->addLayer(VRRefinements::open(*this, *descriptor));
        }

        // optional VRNodeLayer
        id = H5Dopen2(bagGroup.getLocId(), VR_NODE_PATH, H5P_DEFAULT);
        if (id >= 0)
        {
            H5Dclose(id);

            auto descriptor = VRNodeDescriptor::open(*this);
            this->addLayer(VRNode::open(*this, *descriptor));
        }
    }
}

//! Custom deleter to not expose the HDF5 dependency to the user.
/*!
\param ptr
    The H5File to be deleted.
*/
void Dataset::DeleteH5File::operator()(::H5::H5File* ptr) noexcept
{
    delete ptr;
}

}   //namespace BAG

