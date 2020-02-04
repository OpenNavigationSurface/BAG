
#include "bag_compoundlayer.h"
#include "bag_compoundlayerdescriptor.h"
#include "bag_dataset.h"
#include "bag_interleavedlayer.h"
#include "bag_interleavedlayerdescriptor.h"
#include "bag_metadata_export.h"
#include "bag_private.h"
#include "bag_simplelayer.h"
#include "bag_simplelayerdescriptor.h"
#include "bag_surfacecorrectionsdescriptor.h"
#include "bag_version.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

#include <algorithm>
#include <array>
#include <H5Cpp.h>
#include <map>
#include <regex>
#include <string>

#ifdef _MSC_VER
#pragma warning(pop)
#endif


namespace BAG {

namespace {

//! Get the specified compound layer.
CompoundLayer* getCompoundLayer(
    const std::string& name,
    const std::vector<std::unique_ptr<Layer>>& layers)
{
    auto layerIter = std::find_if(cbegin(layers), cend(layers),
        [&name](const std::unique_ptr<Layer>& layer) {
            const auto& descriptor = layer->getDescriptor();

            return descriptor.getLayerType() == Compound &&
                descriptor.getName() == name;
        });
    if (layerIter == cend(layers))
        return nullptr;

    return dynamic_cast<CompoundLayer*>(layerIter->get());
}

// Expecting "major" or "major.minor" or "major.minor.patch", which will be
// turned into a number such that:
// major * 1,000,000 + minor * 1,000 + patch is returned.
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

template <typename T>
using IsNotString = std::enable_if_t<!std::is_same_v<T, std::string>>;

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

std::shared_ptr<Dataset> Dataset::open(
    const std::string& fileName,
    OpenMode openMode)
{
    std::shared_ptr<Dataset> pDataset{new Dataset};
    pDataset->readDataset(fileName, openMode);

    return pDataset;
}

std::shared_ptr<Dataset> Dataset::create(
    const std::string& fileName,
    Metadata&& metadata,
    uint64_t chunkSize,
    unsigned int compressionLevel)
{
    std::shared_ptr<Dataset> pDataset{new Dataset};
    pDataset->createDataset(fileName, std::move(metadata), chunkSize,
        compressionLevel);

    return pDataset;
}


Layer& Dataset::addLayer(
    std::unique_ptr<Layer> newLayer) &
{
    m_layers.emplace_back(std::move(newLayer));

    const auto& layer = m_layers.back();

    m_descriptor.addLayerDescriptor(layer->getDescriptor());

    return *layer;
}

CompoundLayer& Dataset::createCompoundLayer(
    DataType indexType,
    const std::string& name,
    const RecordDefinition& definition,
    uint64_t chunkSize,
    unsigned int compressionLevel) &
{
    if (m_descriptor.isReadOnly())
        throw ReadOnlyError{};

    // Make sure the layer does not already exist.
    const auto layerIter = std::find_if(cbegin(m_layers), cend(m_layers),
        [&name](const std::unique_ptr<Layer>& layer) {
            const auto& descriptor = layer->getDescriptor();

            return descriptor.getLayerType() == Compound &&
                descriptor.getName() == name;
        });
    if (layerIter != cend(m_layers))
        throw LayerExists{};

    //TODO make sure a corresponding simple layer exists.

    // Create the group if it does not exist.
    const auto id = H5Gopen2(m_pH5file->getId(), COMPOUND_PATH, H5P_DEFAULT);
    if (id == -1)
        m_pH5file->createGroup(COMPOUND_PATH);
    else
        H5Gclose(id);

    return dynamic_cast<CompoundLayer&>(this->addLayer(CompoundLayer::create(
        indexType, name, *this, definition, chunkSize, compressionLevel)));
}

void Dataset::createDataset(
    const std::string& fileName,
    Metadata&& metadata,
    uint64_t chunkSize,
    unsigned int compressionLevel)
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

    // Mandatory Layers (Elevation, Uncertainty)
    this->addLayer(SimpleLayer::create(*this, Elevation, chunkSize,
        compressionLevel));

    this->addLayer(SimpleLayer::create(*this, Uncertainty, chunkSize,
        compressionLevel));

    // All mandatory items exist in the HDF5 file now.
}

Layer& Dataset::createSimpleLayer(
    LayerType type,
    uint64_t chunkSize,
    unsigned int compressionLevel) &
{
    if (m_descriptor.isReadOnly())
        throw ReadOnlyError{};

    // Make sure it doesn't already exist.
    const auto foundIter = std::find_if(cbegin(m_layers), cend(m_layers),
        [type](const auto& layer) {
            return layer->getDescriptor().getLayerType() == type;
        });
    if (foundIter != cend(m_layers))
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

SurfaceCorrections& Dataset::createSurfaceCorrections(
    BAG_SURFACE_CORRECTION_TOPOGRAPHY type,
    uint8_t numCorrectors,
    uint64_t chunkSize,
    unsigned int compressionLevel) &
{
    if (m_descriptor.isReadOnly())
        throw ReadOnlyError{};

    // Make sure it doesn't already exist.
    const auto layerIter = std::find_if(cbegin(m_layers), cend(m_layers),
        [](const auto& layer) {
            return layer->getDescriptor().getLayerType() == Surface_Correction;
        });
    if (layerIter != cend(m_layers))
        throw LayerExists{};

    return dynamic_cast<SurfaceCorrections&>(this->addLayer(
        SurfaceCorrections::create(*this, type, numCorrectors, chunkSize,
            compressionLevel)));
}

std::tuple<uint32_t, uint32_t> Dataset::geoToGrid(
    double x,
    double y) const noexcept
{
    const auto row = static_cast<uint32_t>((x - m_pMetadata->llCornerX()) /
        m_pMetadata->rowResolution());
    const auto column = static_cast<uint32_t>((y - m_pMetadata->llCornerY()) /
        m_pMetadata->columnResolution());

    return std::make_tuple(row, column);
}

CompoundLayer* Dataset::getCompoundLayer(
    const std::string& name) & noexcept
{
    return BAG::getCompoundLayer(name, m_layers);
}

const CompoundLayer* Dataset::getCompoundLayer(
    const std::string& name) const & noexcept
{
    return BAG::getCompoundLayer(name, m_layers);
}

Descriptor& Dataset::getDescriptor() & noexcept
{
    return m_descriptor;
}

const Descriptor& Dataset::getDescriptor() const & noexcept
{
    return m_descriptor;
}

::H5::H5File& Dataset::getH5file() const & noexcept
{
    return *m_pH5file;
}

Layer& Dataset::getLayer(uint32_t id) &
{
    if (id >= m_layers.size())
        throw InvalidLayerId{};

    return *m_layers[id];
}

const Layer& Dataset::getLayer(uint32_t id) const &
{
    if (id >= m_layers.size())
        throw InvalidLayerId{};

    return *m_layers[id];
}

std::vector<LayerType> Dataset::getLayerTypes() const
{
    std::vector<LayerType> types;
    types.reserve(m_layers.size());

    for (auto&& layer : m_layers)
        types.push_back(layer->getDescriptor().getLayerType());

    return types;
}

const Metadata& Dataset::getMetadata() const & noexcept
{
    return *m_pMetadata;
}

//TODO What about uint32_t?
std::tuple<bool, float, float> Dataset::getMinMax(
    LayerType type,
    const std::string& path) const
{
    try
    {
        const auto info = Layer::getAttributeInfo(type);
        const auto& thePath = path.empty() ? info.path : path;

        return std::make_tuple(true,
            readAttributeFromDataSet<float>(*m_pH5file, thePath, info.minName),
            readAttributeFromDataSet<float>(*m_pH5file, thePath, info.maxName));
    }
    catch(const UnknownSimpleLayerType&)
    {
        return std::make_tuple(false, 0.f, 0.f);  // No min/max attributes.
    }
}

uint32_t Dataset::getNextId() const noexcept
{
    return static_cast<uint32_t>(m_layers.size());
}

TrackingList& Dataset::getTrackingList() & noexcept
{
    return *m_pTrackingList;
}

const TrackingList& Dataset::getTrackingList() const & noexcept
{
    return *m_pTrackingList;
}

std::tuple<double, double> Dataset::gridToGeo(
    uint32_t row,
    uint32_t column) const noexcept
{
    const auto x = m_pMetadata->llCornerX() +
        (row * m_pMetadata->rowResolution());

    const auto y = m_pMetadata->llCornerY() +
        (column * m_pMetadata->columnResolution());

    return std::make_tuple(x, y);
}

SurfaceCorrections* Dataset::getSurfaceCorrections() & noexcept
{
    const auto layerIter = std::find_if(cbegin(m_layers), cend(m_layers),
        [](const auto& layer) {
            return layer->getDescriptor().getLayerType() == Surface_Correction;
        });
    if (layerIter == cend(m_layers))
        return nullptr;

    return dynamic_cast<SurfaceCorrections*>((*layerIter).get());
}

const SurfaceCorrections* Dataset::getSurfaceCorrections() const & noexcept
{
    const auto layerIter = std::find_if(cbegin(m_layers), cend(m_layers),
        [](const auto& layer) {
            return layer->getDescriptor().getLayerType() == Surface_Correction;
        });
    if (layerIter == cend(m_layers))
        return nullptr;

    return dynamic_cast<SurfaceCorrections*>((*layerIter).get());
}

void Dataset::readDataset(
    const std::string& fileName,
    OpenMode openMode)
{
#ifdef NDEBUG
    ::H5::Exception::dontPrint();
#endif
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

        auto layerDesc = SimpleLayerDescriptor::open(layerType, *this);
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
            auto layerDesc = InterleavedLayerDescriptor::open(
                Hypothesis_Strength, NODE, *this);
            this->addLayer(InterleavedLayer::open(*this, *layerDesc));

            // Num_Hypotheses
            layerDesc = InterleavedLayerDescriptor::open(Num_Hypotheses, NODE,
                *this);
            this->addLayer(InterleavedLayer::open(*this, *layerDesc));
        }

        id = H5Dopen2(bagGroup.getLocId(), ELEVATION_SOLUTION_GROUP_PATH, H5P_DEFAULT);
        if (id >= 0)
        {
            H5Dclose(id);

            // Shoal_Elevation
            auto layerDesc = InterleavedLayerDescriptor::open(Shoal_Elevation,
                ELEVATION, *this);

            this->addLayer(InterleavedLayer::open(*this, *layerDesc));

            // Std_Dev
            layerDesc = InterleavedLayerDescriptor::open(Std_Dev, ELEVATION,
                *this);
            this->addLayer(InterleavedLayer::open(*this, *layerDesc));

            // Num_Soundings
            layerDesc = InterleavedLayerDescriptor::open(Num_Soundings,
                ELEVATION, *this);
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

                    auto descriptor = CompoundLayerDescriptor::open(name, *this);
                    this->addLayer(CompoundLayer::open(*this, *descriptor));
                }
                catch(...)
                {}
            }
        }
    }

    m_pTrackingList = std::unique_ptr<TrackingList>(new TrackingList{*this});

    hid_t id = H5Dopen2(bagGroup.getLocId(), VERT_DATUM_CORR_PATH, H5P_DEFAULT);
    if (id >= 0)
    {
        H5Dclose(id);

        auto descriptor = SurfaceCorrectionsDescriptor::open(*this);
        this->addLayer(SurfaceCorrections::open(*this, *descriptor));
    }
}

void Dataset::DeleteH5File::operator()(::H5::H5File* ptr) noexcept
{
    delete ptr;
}

}   //namespace BAG

