
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
    Metadata& metadata/*,
    Descriptor descriptor*/)
{
    std::shared_ptr<Dataset> pDataset{new Dataset};
    pDataset->createDataset(fileName, metadata/*, std::move(descriptor)*/);

    return pDataset;
}


Layer& Dataset::createLayer(LayerType type) &
{
    if (this->m_descriptor.isReadOnly())
        throw ReadOnlyError{};

    //Make sure it doesn't already exist.
    const auto iter = m_layers.find(type);
    if (iter != cend(m_layers))
        throw LayerExists{};

    //TODO Rename this as createSimpleLayer, or handle Groups and non-simple
    // types below

    auto result = m_layers.emplace(type, SimpleLayer::create(*this, type));
    auto& newLayer = *(result.first->second);

    m_descriptor.addLayerDescriptor(newLayer.getDescriptor());

    return newLayer;
}

void Dataset::createDataset(
    const std::string& fileName,
    Metadata& metadata/*,
    Descriptor descriptor*/)
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

    // TrackingList
    auto trackingList = std::make_unique<TrackingList>();
    trackingList->createH5dataSet(*this, 5);  //TODO Where does compressionLevel come from?

    Descriptor descriptor;
    descriptor.setReadOnly(false);
    m_descriptor = std::move(descriptor);

    // Mandatory Layers (Elevation, Uncertainty)
    auto elevationLayer = SimpleLayer::create(*this, Elevation);
    m_descriptor.addLayerDescriptor(elevationLayer->getDescriptor());

    auto uncertaintyLayer = SimpleLayer::create(*this, Uncertainty);
    m_descriptor.addLayerDescriptor(elevationLayer->getDescriptor());

    // All mandatory items exist in the HDF5 file now.
}

std::tuple<uint32_t, uint32_t> Dataset::geoToGrid(
    double x,
    double y) const noexcept
{
    x;  y;
    //TODO Implement.
    return std::make_tuple(0, 0);
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
        throw 97;  // Can only work with simple dataspaces.

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
    const char* dataSetName = ELEVATION_PATH;
    const char* minAttName = MIN_ELEVATION_NAME;
    const char* maxAttName = MAX_ELEVATION_NAME;

    switch (type)
    {
    case Elevation:
        break;  // Default, set above.
    case Uncertainty:
        dataSetName = UNCERTAINTY_PATH;
        minAttName = MIN_UNCERTAINTY_NAME;
        maxAttName = MAX_UNCERTAINTY_NAME;
        break;
    case Nominal_Elevation:
        dataSetName = NOMINAL_ELEVATION_PATH;
        minAttName = "min_value";  //TODO Why not in bag_private.h?
        maxAttName = "max_value";  //TODO Why not in bag_private.h?
        break;
    default:
        // No known min/max attributes.
        return std::make_tuple(false, 0.f, 0.f);
    }

    return std::make_tuple(true,
        readAttributeFromDataSet<float>(*m_pH5file, dataSetName, minAttName),
        readAttributeFromDataSet<float>(*m_pH5file, dataSetName, maxAttName));
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

    //Create all of our Simple layers.
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

        const auto possibleMinMax = this->getMinMax(layerType);
        if (std::get<0>(possibleMinMax))
            layerDesc->setMinMax(std::make_tuple(std::get<1>(possibleMinMax),
                std::get<2>(possibleMinMax)));

        m_layers.emplace(layerType, SimpleLayer::open(*this, *layerDesc));

        descriptor.addLayerDescriptor(*layerDesc);
    }

    // If the BAG is version 1.5+ ...
    if (getNumericalVersion(descriptor.getVersion()) >= 1'005'000)
    {
        hid_t id = H5Dopen2(bagGroup.getLocId(), NODE_GROUP_PATH, H5P_DEFAULT);
        if (id >= 0)
        {
            H5Dclose(id);

            // Hypothesis_Strength
            auto possibleMinMax = this->getMinMax(Hypothesis_Strength);

            auto layerDesc = InterleavedLayerDescriptor::create(
                Hypothesis_Strength, NODE, *this);

            if (std::get<0>(possibleMinMax))
                layerDesc->setMinMax(std::make_tuple(std::get<1>(possibleMinMax),
                    std::get<2>(possibleMinMax)));

            m_layers.emplace(Hypothesis_Strength, InterleavedLayer::open(*this,
                *layerDesc));

            descriptor.addLayerDescriptor(*layerDesc);

            // Num_Hypotheses
            possibleMinMax = this->getMinMax(Num_Hypotheses);

            layerDesc = InterleavedLayerDescriptor::create(Num_Hypotheses, NODE,
                *this);

            if (std::get<0>(possibleMinMax))
                layerDesc->setMinMax(std::make_tuple(std::get<1>(possibleMinMax),
                    std::get<2>(possibleMinMax)));

            m_layers.emplace(Num_Hypotheses, InterleavedLayer::open(*this,
                    *layerDesc));

            descriptor.addLayerDescriptor(*layerDesc);
        }

        id = H5Dopen2(bagGroup.getLocId(), ELEVATION_SOLUTION_GROUP_PATH, H5P_DEFAULT);
        if (id >= 0)
        {
            H5Dclose(id);

            // Shoal_Elevation
            auto possibleMinMax = this->getMinMax(Shoal_Elevation);

            auto layerDesc = InterleavedLayerDescriptor::create(Shoal_Elevation,
                ELEVATION, *this);
            layerDesc->setInternalPath(std::string{ELEVATION_SOLUTION_GROUP_PATH});

            if (std::get<0>(possibleMinMax))
                layerDesc->setMinMax(std::make_tuple(std::get<1>(possibleMinMax),
                    std::get<2>(possibleMinMax)));

            m_layers.emplace(Shoal_Elevation, InterleavedLayer::open(*this,
                    *layerDesc));

            descriptor.addLayerDescriptor(*layerDesc);

            // Std_Dev
            possibleMinMax = this->getMinMax(Std_Dev);

            layerDesc = InterleavedLayerDescriptor::create(Std_Dev, ELEVATION,
                *this);

            if (std::get<0>(possibleMinMax))
                layerDesc->setMinMax(std::make_tuple(std::get<1>(possibleMinMax),
                    std::get<2>(possibleMinMax)));

            m_layers.emplace(Std_Dev, InterleavedLayer::open(*this, *layerDesc));

            descriptor.addLayerDescriptor(*layerDesc);

            // Num_Soundings
            possibleMinMax = this->getMinMax(Num_Soundings);

            layerDesc = InterleavedLayerDescriptor::create(Num_Soundings,
                ELEVATION, *this);

            if (std::get<0>(possibleMinMax))
                layerDesc->setMinMax(std::make_tuple(std::get<1>(possibleMinMax),
                    std::get<2>(possibleMinMax)));

            m_layers.emplace(Num_Soundings, InterleavedLayer::open(*this,
                    *layerDesc));

            descriptor.addLayerDescriptor(*layerDesc);
        }
    }

    m_descriptor = std::move(descriptor);

    m_pMetadata = std::make_unique<Metadata>(*this);

    m_pTrackingList = std::make_unique<TrackingList>(*this);

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

