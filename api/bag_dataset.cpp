
#include "bag_dataset.h"
#include "bag_interleavedlayer.h"
#include "bag_interleavedlayerdescriptor.h"
#include "bag_metadata_export.h"
#include "bag_private.h"
#include "bag_simplelayerdescriptor.h"
#include "bag_simplelayer.h"
#include "bag_version.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

#include <algorithm>
#include <array>
#include <h5cpp.h>
#include <map>
#include <regex>
#include <string>

#ifdef _MSC_VER
#pragma warning(pop)
#endif


namespace BAG {

namespace {

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

Layer& getLayer(
    LayerType layerType,
    const std::unordered_map<LayerType, std::unique_ptr<Layer>>& layers)
{
    const auto iter = layers.find(layerType);
    if (iter == cend(layers))
        throw LayerNotFound{};

    return *(iter->second);
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
    Metadata&& metadata)
{
    std::shared_ptr<Dataset> pDataset{new Dataset};
    pDataset->createDataset(fileName, std::move(metadata));

    return pDataset;
}


Layer& Dataset::addLayer(std::unique_ptr<Layer> newLayer) &
{
    auto result = m_layers.emplace(newLayer->getDescriptor().getLayerType(),
        std::move(newLayer));
    auto& layer = *(result.first->second);

    m_descriptor.addLayerDescriptor(layer.getDescriptor());

    return layer;
}

Layer& Dataset::createLayer(LayerType type) &
{
    if (this->m_descriptor.isReadOnly())
        throw ReadOnlyError{};

    //Make sure it doesn't already exist.
    const auto iter = m_layers.find(type);
    if (iter != cend(m_layers))
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
        return this->addLayer(SimpleLayer::create(*this, type));
    case Compound:  //[[fallthrough]];
    default:
        throw UnsupportedLayerType{};
    }
}

void Dataset::createDataset(
    const std::string& fileName,
    Metadata&& metadata)
{
#ifdef NDEBUG
    ::H5::Exception::dontPrint();
#endif

    m_pH5file = std::unique_ptr<::H5::H5File, DeleteH5File>(new ::H5::H5File{
        fileName.c_str(), H5F_ACC_EXCL}, DeleteH5File{});

    // Group: BAG_root
    auto h5bagGroup = m_pH5file->createGroup(ROOT_PATH);

    const auto versionAttType = ::H5::StrType{0, BAG_VERSION_LENGTH};
    const ::H5::DataSpace versionAttDataSpace{};
    auto versionAtt = h5bagGroup.createAttribute(BAG_VERSION_NAME, versionAttType, versionAttDataSpace);

    versionAtt.write(versionAttType, BAG_VERSION);
    versionAtt.close();

    // Metadata
    metadata.createH5dataSet(*this);
    metadata.write();
    m_pMetadata = std::make_unique<Metadata>(std::move(metadata));

    // TrackingList
    m_pTrackingList = std::unique_ptr<TrackingList>(new TrackingList{*this, 5});  //TODO Where does compressionLevel come from?

    Descriptor descriptor;
    descriptor.setReadOnly(false);
    m_descriptor = std::move(descriptor);

    // Mandatory Layers (Elevation, Uncertainty)
    this->addLayer(SimpleLayer::create(*this, Elevation));
    this->addLayer(SimpleLayer::create(*this, Uncertainty));

    // All mandatory items exist in the HDF5 file now.
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

uint64_t Dataset::getChunkSize(LayerType type) const
{
    //Get the elevation HD5 dataset.
    const auto h5dataset = m_pH5file->openDataSet(Layer::getInternalPath(type));
    const auto h5pList = h5dataset.getCreatePlist();

    if (h5pList.getLayout() == H5D_CHUNKED)
    {
        std::array<hsize_t, RANK> maxDims{};

        const int rankChunk = h5pList.getChunk(RANK, maxDims.data());
        if (rankChunk == RANK)
            return {maxDims[0]};  // Using {} to prevent narrowing.
    }

    return 0;
}

unsigned int Dataset::getCompressionLevel(LayerType type) const
{
    //Get the elevation HD5 dataset.
    const auto h5dataset = m_pH5file->openDataSet(Layer::getInternalPath(type));
    const auto h5pList = h5dataset.getCreatePlist();

    for (int i=0; i<h5pList.getNfilters(); ++i)
    {
        unsigned int flags = 0;
        size_t cdNelmts = 10;
        constexpr size_t nameLen = 64;
        std::array<unsigned int, 10> cdValues{};
        std::array<char, 64> name{};
        unsigned int filterConfig = 0;

        const auto filter = h5pList.getFilter(i, flags, cdNelmts,
            cdValues.data(), nameLen, name.data(), filterConfig);
        if (filter == H5Z_FILTER_DEFLATE)
            if (cdNelmts >= 1)
                return cdValues.front();
    }

    return 0;
}

const Descriptor& Dataset::getDescriptor() const & noexcept
{
    return m_descriptor;
}

std::tuple<uint32_t, uint32_t> Dataset::getDims(LayerType type) const
{
    //Get the elevation HD5 dataset.
    const auto h5dataset = m_pH5file->openDataSet(Layer::getInternalPath(type));
    const auto h5dataSpace = h5dataset.getSpace();
    if (!h5dataSpace.isSimple())
        throw 97;  // Can only work with simple data spaces.

    const int nDimsRank = h5dataSpace.getSimpleExtentNdims();
    std::array<hsize_t, RANK> size{};
    const int dimsRank = h5dataSpace.getSimpleExtentDims(size.data());

    if (nDimsRank != RANK || dimsRank != RANK)
        throw 99;  // Unexpected dimensions.

    const auto numRows = static_cast<uint32_t>(size[0]);
    const auto numColumns = static_cast<uint32_t>(size[1]);

    return std::make_tuple(numRows, numColumns);
}

::H5::H5File& Dataset::getH5file() const & noexcept
{
    return *m_pH5file;
}

Layer& Dataset::getLayer(LayerType layerType) &
{
    return BAG::getLayer(layerType, m_layers);
}

const Layer& Dataset::getLayer(LayerType layerType) const &
{
    return BAG::getLayer(layerType, m_layers);
}

std::vector<LayerType> Dataset::getLayerTypes() const
{
    std::vector<LayerType> types;
    types.reserve(m_layers.size());

    for (auto&& layer : m_layers)
        types.push_back(layer.first);

    return types;
}

const Metadata& Dataset::getMetadata() const & noexcept
{
    return *m_pMetadata;
}

std::tuple<bool, float, float> Dataset::getMinMax(
    LayerType type) const
{
    // Defaults for Elevation.
    const char* dataSetPath = ELEVATION_PATH;
    const char* minAttName = MIN_ELEVATION_NAME;
    const char* maxAttName = MAX_ELEVATION_NAME;

    switch (type)
    {
    case Elevation:
        break;  // Default, set above.
    case Uncertainty:
        dataSetPath = UNCERTAINTY_PATH;
        minAttName = MIN_UNCERTAINTY_NAME;
        maxAttName = MAX_UNCERTAINTY_NAME;
        break;
    case Hypothesis_Strength:
        dataSetPath = HYPOTHESIS_STRENGTH_PATH;
        minAttName = MIN_HYPOTHESIS_STRENGTH;
        maxAttName = MAX_HYPOTHESIS_STRENGTH;
        break;
    case Num_Hypotheses:
        dataSetPath = NUM_HYPOTHESES_PATH;
        minAttName = MIN_NUM_HYPOTHESES;
        maxAttName = MAX_NUM_HYPOTHESES;
        break;
    case Shoal_Elevation:
        dataSetPath = SHOAL_ELEVATION_PATH;
        minAttName = MIN_SHOAL_ELEVATION;
        maxAttName = MAX_SHOAL_ELEVATION;
        break;
    case Std_Dev:
        dataSetPath = STANDARD_DEV_PATH;
        minAttName = MIN_STANDARD_DEV_NAME;
        maxAttName = MAX_STANDARD_DEV_NAME;
        break;
    case Num_Soundings:
        dataSetPath = NUM_SOUNDINGS_PATH;
        minAttName = MIN_NUM_SOUNDINGS;
        maxAttName = MAX_NUM_SOUNDINGS;
        break;
    case Average_Elevation:
        dataSetPath = AVERAGE_PATH;
        minAttName = MIN_AVERAGE;
        maxAttName = MAX_AVERAGE;
        break;
    case Nominal_Elevation:
        dataSetPath = NOMINAL_ELEVATION_PATH;
        minAttName = MIN_NOMINAL_ELEVATION;
        maxAttName = MAX_NOMINAL_ELEVATION;
        break;
    case Compound:
        throw UnsupportedLayerType{};
    default:
        // No known min/max attributes.
        return std::make_tuple(false, 0.f, 0.f);
    }

    return std::make_tuple(true,
        readAttributeFromDataSet<float>(*m_pH5file, dataSetPath, minAttName),
        readAttributeFromDataSet<float>(*m_pH5file, dataSetPath, maxAttName));
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

std::unique_ptr<::H5::DataSet, Dataset::DeleteH5DataSet>
Dataset::openLayerH5DataSet(const LayerDescriptor& descriptor)
{
    return std::unique_ptr<::H5::DataSet, DeleteH5DataSet>(new ::H5::DataSet{
        m_pH5file->openDataSet(descriptor.getInternalPath())}, DeleteH5DataSet{});
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

    Descriptor descriptor;

    descriptor.setReadOnly(openMode == BAG_OPEN_READONLY);
    descriptor.setVersion(readStringAttributeFromGroup(*m_pH5file, ROOT_PATH,
        BAG_VERSION_NAME));

    const auto bagGroup = m_pH5file->openGroup(ROOT_PATH);

    // Look for the simple layers.
    //TODO Add rest of the layers (Hypothesis_Strength, Num_Hypotheses, Shoal_Elevation, Std_Dev, Num_Soundings)
    for (auto layerType : {Elevation, Uncertainty, Average_Elevation, Nominal_Elevation})
    {
        const std::string internalPath = Layer::getInternalPath(layerType);
        if (internalPath.empty())
            continue;

        const hid_t id = H5Dopen2(bagGroup.getLocId(), internalPath.c_str(),
            H5P_DEFAULT);
        if (id < 0)
            continue;

        H5Dclose(id);

        auto layerDesc = SimpleLayerDescriptor::create(layerType, *this);
        this->addLayer(SimpleLayer::open(*this, *layerDesc));
    }

    const auto bagVersion = getNumericalVersion(descriptor.getVersion());

    // If the BAG is version 1.5+ ...
    if (bagVersion >= 1'005'000)
    {
        hid_t id = H5Dopen2(bagGroup.getLocId(), NODE_GROUP_PATH, H5P_DEFAULT);
        if (id >= 0)
        {
            H5Dclose(id);

            // Hypothesis_Strength
            auto layerDesc = InterleavedLayerDescriptor::create(
                Hypothesis_Strength, NODE, *this);
            this->addLayer(InterleavedLayer::open(*this, *layerDesc));

            // Num_Hypotheses
            layerDesc = InterleavedLayerDescriptor::create(Num_Hypotheses, NODE,
                *this);
            this->addLayer(InterleavedLayer::open(*this, *layerDesc));
        }

        id = H5Dopen2(bagGroup.getLocId(), ELEVATION_SOLUTION_GROUP_PATH, H5P_DEFAULT);
        if (id >= 0)
        {
            H5Dclose(id);

            // Shoal_Elevation
            auto layerDesc = InterleavedLayerDescriptor::create(Shoal_Elevation,
                ELEVATION, *this);
            layerDesc->setInternalPath(std::string{ELEVATION_SOLUTION_GROUP_PATH});

            this->addLayer(InterleavedLayer::open(*this, *layerDesc));

            // Std_Dev
            layerDesc = InterleavedLayerDescriptor::create(Std_Dev, ELEVATION,
                *this);
            this->addLayer(InterleavedLayer::open(*this, *layerDesc));

            // Num_Soundings
            layerDesc = InterleavedLayerDescriptor::create(Num_Soundings,
                ELEVATION, *this);
            this->addLayer(InterleavedLayer::open(*this, *layerDesc));
        }
    }

    // If the BAG is version 2.0+ ...
    if (bagVersion >= 2'000'000)
    {
        //TODO handle Compound layers
    }

    m_descriptor = std::move(descriptor);

    m_pMetadata = std::make_unique<Metadata>(*this);

    m_pTrackingList = std::unique_ptr<TrackingList>(new TrackingList{*this});

    const hid_t id = H5Dopen2(bagGroup.getLocId(), VERT_DATUM_CORR_PATH,
        H5P_DEFAULT);
    if (id >= 0)
    {
        H5Dclose(id);
        m_pVerticalDatumCorrections =
            std::make_unique<VerticalDatumCorrections>(*this);
    }
}

void Dataset::DeleteH5File::operator()(::H5::H5File* ptr) noexcept
{
    delete ptr;
}

void Dataset::DeleteH5DataSet::operator()(::H5::DataSet* ptr) noexcept
{
    delete ptr;
}

}   //namespace BAG

