
#include "bag_georefmetadatalayer.h"
#include "bag_metadataprofiles.h"
#include "bag_georefmetadatalayerdescriptor.h"
#include "bag_exceptions.h"
#include "bag_hdfhelper.h"
#include "bag_private.h"

#include <array>
#include <H5Cpp.h>
#include <limits>
#include <memory>
#include <vector>


namespace BAG {

namespace {

//! Helper function to find the maximum value of the specified DataType.
/*!
\param dataType
    The type of data.
    Supported types are: DT_UINT8, DT_UINT16, DT_UINT32, DT_UINT64.

\return
    The maximum value the specified data type can hold.
*/
hsize_t getDataTypeMax(
    DataType dataType)
{
    switch(dataType)
    {
    case DT_UINT8:
        return std::numeric_limits<uint8_t>::max();
    case DT_UINT16:
        return std::numeric_limits<uint16_t>::max();
    case DT_UINT32:
        return std::numeric_limits<uint32_t>::max();
    case DT_UINT64:
        return std::numeric_limits<uint64_t>::max();
    default:
        throw UnsupportedDataType{};
    }
}

}

//! The constructor.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
\param descriptor
    The descriptor of this layer.
\param pH5keyDataSet
    The HDF5 DataSet that will hold the single resolution keys.
\param pH5vrKeyDataSet
    The HDF5 DataSet that will hold the variable resolution keys.
\param pH5valueDataSet
    The HDF5 DataSet that will hold the spatial metadata values.
*/
GeorefMetadataLayer::GeorefMetadataLayer(
        Dataset& dataset,
        GeorefMetadataLayerDescriptor& descriptor,
        std::unique_ptr<::H5::DataSet, DeleteH5dataSet> pH5keyDataSet,
        std::unique_ptr<::H5::DataSet, DeleteH5dataSet> pH5vrKeyDataSet,
        std::unique_ptr<::H5::DataSet, DeleteH5dataSet> pH5valueDataSet)
    : Layer(dataset, descriptor)
    , m_pH5keyDataSet(std::move(pH5keyDataSet))
    , m_pH5vrKeyDataSet(std::move(pH5vrKeyDataSet))
    , m_pH5valueDataSet(std::move(pH5valueDataSet))
{
}

//! Create a georeferenced metadata layer.
/*!
\param keyType
    The type of key this layer will use.
\param name
    The name of this georeferenced metadata layer.
    Must be a unique name among all georeferenced metadata layers in this BAG Dataset.
\param dataset
    The BAG Dataset this georeferenced metadata layer will belong to.
\param definition
    The list of fields describing a single record/value.
\param chunkSize
    The chunk size the HDF5 DataSet will use.
\param compressionLevel
    The compression level the HDF5 DataSet will use.

\return
    The new georeferenced metadata layer.
*/
std::shared_ptr<GeorefMetadataLayer> GeorefMetadataLayer::create(
            DataType keyType,
            const std::string& name,
            GeorefMetadataProfile profile,
            Dataset& dataset,
            const RecordDefinition& definition,
            uint64_t chunkSize,
            int compressionLevel)
{
    if (keyType != DT_UINT8 && keyType != DT_UINT16 && keyType != DT_UINT32 &&
        keyType != DT_UINT64)
        throw InvalidKeyType{};

    auto pDescriptor = GeorefMetadataLayerDescriptor::create(dataset, name, profile, keyType,
                                                             definition, chunkSize, compressionLevel);

    // Create the H5 Group to hold keys & values.
    const auto& h5file = dataset.getH5file();
    h5file.createGroup(GEOREF_METADATA_PATH + name);

    auto h5keyDataSet = GeorefMetadataLayer::createH5keyDataSet(dataset, *pDescriptor);

    // create optional variable resolution keys.
    std::unique_ptr<::H5::DataSet, DeleteH5dataSet> h5vrKeyDataSet{};

    if (dataset.getVRMetadata())
        h5vrKeyDataSet = GeorefMetadataLayer::createH5vrKeyDataSet(dataset, *pDescriptor);

    auto h5valueDataSet = GeorefMetadataLayer::createH5valueDataSet(dataset, *pDescriptor);

    auto layer = std::make_shared<GeorefMetadataLayer>(dataset,
                                                       *pDescriptor, std::move(h5keyDataSet), std::move(h5vrKeyDataSet),
                                                       std::move(h5valueDataSet));

    layer->setValueTable(std::unique_ptr<ValueTable>(new ValueTable{*layer}));

    return layer;
}

//! Open an existing georeferenced metadata layer.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
\param descriptor
    The descriptor of this layer.

\return
    The georeferenced metadata layer read from dataset.
*/
std::shared_ptr<GeorefMetadataLayer> GeorefMetadataLayer::open(
            Dataset& dataset,
            GeorefMetadataLayerDescriptor& descriptor)
{
    const auto& h5file = dataset.getH5file();
    const std::string& internalPath = descriptor.getInternalPath();
    auto h5keyDataSet = std::unique_ptr<::H5::DataSet, DeleteH5dataSet>(
        new ::H5::DataSet{h5file.openDataSet(internalPath + COMPOUND_KEYS)},
        DeleteH5dataSet{});

    std::unique_ptr<::H5::DataSet, DeleteH5dataSet> h5vrKeyDataSet{};
    if (dataset.getVRMetadata())
        h5vrKeyDataSet = std::unique_ptr<::H5::DataSet, DeleteH5dataSet>(
            new ::H5::DataSet{h5file.openDataSet(internalPath + COMPOUND_VR_KEYS)},
            DeleteH5dataSet{});

    auto h5valueDataSet = std::unique_ptr<::H5::DataSet, DeleteH5dataSet>(
        new ::H5::DataSet{h5file.openDataSet(internalPath + COMPOUND_VALUES)},
        DeleteH5dataSet{});

    auto layer = std::make_shared<GeorefMetadataLayer>(dataset,
                                                       descriptor, std::move(h5keyDataSet), std::move(h5vrKeyDataSet),
                                                       std::move(h5valueDataSet));

    layer->setValueTable(std::unique_ptr<ValueTable>(new ValueTable{*layer}));

    return layer;
}


//! Create an HDF5 DataSet for the keys of a single resolution georeferenced metadata layer with details from the descriptor.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
\param descriptor
    The descriptor of this layer.

\return
    The HDF5 DataSet containing the single resolution keys of a new georeferenced metadata layer.
*/
std::unique_ptr<::H5::DataSet, DeleteH5dataSet>
GeorefMetadataLayer::createH5keyDataSet(
    const Dataset& dataset,
    const GeorefMetadataLayerDescriptor& descriptor)
{
    std::unique_ptr<::H5::DataSet, DeleteH5dataSet> pH5dataSet;

    {
        // Use the dimensions from the descriptor.
        uint32_t dim0 = 0, dim1 = 0;
        std::tie(dim0, dim1) = dataset.getDescriptor().getDims();
        const std::array<hsize_t, kRank> fileDims{dim0, dim1};

        // Create the creation property list.
        const ::H5::DSetCreatPropList h5createPropList{};
        h5createPropList.setFillTime(H5D_FILL_TIME_ALLOC);

        const auto dataType = descriptor.getDataType();
        const auto& memDataType = BAG::getH5memoryType(dataType);

        const std::vector<uint8_t> fillValue(Layer::getElementSize(dataType), 0);
        h5createPropList.setFillValue(memDataType, fillValue.data());

        // Use chunk size and compression level from the descriptor.
        const auto compressionLevel = descriptor.getCompressionLevel();
        const auto chunkSize = descriptor.getChunkSize();
        if (chunkSize > 0)
        {
            const std::array<hsize_t, kRank> chunkDims{chunkSize, chunkSize};
            h5createPropList.setChunk(kRank, chunkDims.data());

            if (compressionLevel > 0 && compressionLevel <= kMaxCompressionLevel)
                h5createPropList.setDeflate(compressionLevel);
        }
        else if (compressionLevel > 0)
            throw CompressionNeedsChunkingSet{};

        const ::H5::DataSpace fileDataSpace{kRank, fileDims.data(), fileDims.data()};

        const auto& fileDataType = BAG::getH5fileType(dataType);

        // Create the DataSet using the above.
        const auto& h5file = dataset.getH5file();

        pH5dataSet = std::unique_ptr<::H5::DataSet, DeleteH5dataSet>(
            new ::H5::DataSet{h5file.createDataSet(
                descriptor.getInternalPath() + COMPOUND_KEYS, fileDataType,
                fileDataSpace, h5createPropList)},
            DeleteH5dataSet{});
    }

    // Create the Record (value) Definition attribute.
    {
        const auto& definition = descriptor.getDefinition();
        const hsize_t dims = definition.size();
        const ::H5::DataSpace fileDataSpace{1, &dims, &dims};

        // Create the compound type for the Record Definition.
        const ::H5::StrType strType{::H5::PredType::C_S1, H5T_VARIABLE};

        //works
        const ::H5::CompType fileDataType{sizeof(FieldDefinition)};
        fileDataType.insertMember("name", HOFFSET(FieldDefinition, name), strType);
        fileDataType.insertMember("type", HOFFSET(FieldDefinition, type), ::H5::PredType::NATIVE_UINT8);

        // Create the attribute.
        const auto att = pH5dataSet->createAttribute(COMPOUND_RECORD_DEFINITION,
            fileDataType, fileDataSpace);

        // Write metadata field definitions to the attribute.
        att.write(fileDataType, definition.data());

        // Specify a string attribute to indicate the metadata profile type
        const auto profileAttType = ::H5::StrType{0, METADATA_PROFILE_LEN};
        const ::H5::DataSpace profileAttSpace{};
        const auto profileAttr = pH5dataSet->createAttribute(METADATA_PROFILE_TYPE,
                                                             profileAttType, profileAttSpace);
        profileAttr.write(profileAttType, kGeorefMetadataProfileMapString.at(descriptor.m_profile).data());
    }

    return pH5dataSet;
}

//! Create an HDF5 DataSet for the keys of a variable resolution georeferenced metadata layer with details from the descriptor.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
\param descriptor
    The descriptor of this layer.

\return
    The HDF5 DataSet containing the variable resolution keys of a new georeferenced metadata layer.
*/
std::unique_ptr<::H5::DataSet, DeleteH5dataSet>
GeorefMetadataLayer::createH5vrKeyDataSet(
    const Dataset& dataset,
    const GeorefMetadataLayerDescriptor& descriptor)
{
    std::unique_ptr<::H5::DataSet, DeleteH5dataSet> pH5dataSet;

    {
        const auto& h5file = dataset.getH5file();

        const auto dataType = descriptor.getDataType();
        const auto& fileDataType = BAG::getH5fileType(dataType);

        constexpr hsize_t kFileLength = 0;
        constexpr hsize_t kMaxFileLength = H5S_UNLIMITED;
        const ::H5::DataSpace fileDataSpace{1, &kFileLength, &kMaxFileLength};

        // Create the creation property list.
        const ::H5::DSetCreatPropList h5createPropList{};
        h5createPropList.setFillTime(H5D_FILL_TIME_ALLOC);

        // Use chunk size and compression level from the layer descriptor.
        const auto compressionLevel = descriptor.getCompressionLevel();
        const auto chunkSize = descriptor.getChunkSize();
        if (chunkSize > 0)
        {
            const auto chunk = static_cast<hsize_t>(chunkSize);
            h5createPropList.setChunk(1, &chunk);

            if (compressionLevel > 0 && compressionLevel <= kMaxCompressionLevel)
                h5createPropList.setDeflate(compressionLevel);
        }
        else if (compressionLevel > 0)
            throw CompressionNeedsChunkingSet{};
        else
            throw LayerRequiresChunkingSet{};

        // Create the DataSet using the above.
        pH5dataSet = std::unique_ptr<::H5::DataSet, DeleteH5dataSet>(
            new ::H5::DataSet{h5file.createDataSet(
                descriptor.getInternalPath() + COMPOUND_VR_KEYS, fileDataType,
                fileDataSpace, h5createPropList)},
            DeleteH5dataSet{});
    }

    return pH5dataSet;
}

//! Create an HDF5 DataSet for the values of a georeferenced metadata layer with details from the descriptor.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
\param descriptor
    The descriptor of this layer.

\return
    The HDF5 DataSet.
*/
std::unique_ptr<::H5::DataSet, DeleteH5dataSet>
GeorefMetadataLayer::createH5valueDataSet(
    const Dataset& dataset,
    const GeorefMetadataLayerDescriptor& descriptor)
{
    constexpr hsize_t numValues = 1;
    const auto keyType = descriptor.getDataType();
    const hsize_t maxNumValues = getDataTypeMax(keyType);

    const ::H5::DataSpace fileDataSpace{1, &numValues, &maxNumValues};

    const auto& definition = descriptor.getDefinition();
    const auto fileDataType = BAG::createH5fileCompType(definition);

    // Create the creation property list.
    const ::H5::DSetCreatPropList h5createPropList{};
    h5createPropList.setFillTime(H5D_FILL_TIME_ALLOC);

    const std::vector<uint8_t> fillValue(BAG::getRecordSize(definition), 0);
    h5createPropList.setFillValue(fileDataType, fillValue.data());

    constexpr hsize_t kChunkSize = 100;
    h5createPropList.setChunk(1, &kChunkSize);

    // Create the DataSet using the above.
    const auto& h5file = dataset.getH5file();

    auto pH5dataSet = std::unique_ptr<::H5::DataSet, DeleteH5dataSet>(
        new ::H5::DataSet{h5file.createDataSet(
            descriptor.getInternalPath() + COMPOUND_VALUES, fileDataType,
            fileDataSpace, h5createPropList)},
        DeleteH5dataSet{});

    return pH5dataSet;
}

//! Retrieve the HDF5 DataSet containing the values.
/*!
\return
    The HDF5 DataSet containing the values.
*/
const ::H5::DataSet& GeorefMetadataLayer::getValueDataSet() const &
{
    return *m_pH5valueDataSet;
}

//! Retrieve the layer's descriptor. Note: this shadows BAG::Layer.getDescriptor()
/*!
\return
    The layer's descriptor.
    Will never be nullptr.
*/
std::shared_ptr<GeorefMetadataLayerDescriptor> GeorefMetadataLayer::getDescriptor() & noexcept
{
    return std::dynamic_pointer_cast<GeorefMetadataLayerDescriptor>(Layer::getDescriptor());
}

//! Retrieve the layer's descriptor. Note: this shadows BAG::Layer.getDescriptor()
/*!
\return
    The layer's descriptor.
    Will never be nullptr.
*/
std::shared_ptr<const GeorefMetadataLayerDescriptor> GeorefMetadataLayer::getDescriptor() const & noexcept {
    return std::dynamic_pointer_cast<const GeorefMetadataLayerDescriptor>(Layer::getDescriptor());
}

//! Retrieve the value table.
/*!
\return
    The value table.
*/
ValueTable& GeorefMetadataLayer::getValueTable() & noexcept
{
    return *m_pValueTable;
}

//! Retrieve the value table.
/*!
\return
    The value table.
*/
const ValueTable& GeorefMetadataLayer::getValueTable() const & noexcept
{
    return *m_pValueTable;
}

//! \copydoc Layer::read
UInt8Array GeorefMetadataLayer::readProxy(
    uint32_t rowStart,
    uint32_t columnStart,
    uint32_t rowEnd,
    uint32_t columnEnd) const
{
    // Query the file for the specified rows and columns.
    const auto h5fileDataSpace = m_pH5keyDataSet->getSpace();

    // Make sure the area being read from does not exceed the file dimensions.
    const auto numDims = h5fileDataSpace.getSimpleExtentNdims();
    if (numDims != kRank)
        throw InvalidReadSize{};

    std::array<hsize_t, kRank> fileDims{};
    h5fileDataSpace.getSimpleExtentDims(fileDims.data());

    if ((rowEnd >= fileDims[0]) || (columnEnd >= fileDims[1]))
        throw InvalidReadSize{};

    const auto rows = (rowEnd - rowStart) + 1;
    const auto columns = (columnEnd - columnStart) + 1;

    const std::array<hsize_t, kRank> count{rows, columns};
    const std::array<hsize_t, kRank> offset{rowStart, columnStart};

    h5fileDataSpace.selectHyperslab(H5S_SELECT_SET, count.data(), offset.data());

    // Initialize the output buffer.
    const auto bufferSize = this->getDescriptor()->getReadBufferSize(rows,
        columns);
    UInt8Array buffer{bufferSize};

    // Prepare the memory space.
    const ::H5::DataSpace h5memSpace{kRank, count.data(), count.data()};

    m_pH5keyDataSet->read(buffer.data(), H5Dget_type(m_pH5keyDataSet->getId()),
        h5memSpace, h5fileDataSpace);

    return buffer;
}

//! Read the variable resolution metadata keys.
/*!
\param indexStart
    The starting index to read.
    Must be less than or equal to indexEnd.
\param indexEnd
    The ending index to read.  (inclusive)

\return
    The specified keys.
*/
UInt8Array GeorefMetadataLayer::readVR(
    uint32_t indexStart,
    uint32_t indexEnd) const
{
    // Make sure the variable resolution key dataset is present.
    if (!m_pH5vrKeyDataSet)
        throw DatasetRequiresVariableResolution{};

    auto pDescriptor = std::dynamic_pointer_cast<const GeorefMetadataLayerDescriptor>(
        this->getDescriptor());
    if (!pDescriptor)
        throw InvalidLayerDescriptor{};

    // Query the file for the specified rows and columns.
    const auto h5fileDataSpace = m_pH5vrKeyDataSet->getSpace();

    // Make sure the area being read from does not exceed the file dimensions.
    const auto numDims = h5fileDataSpace.getSimpleExtentNdims();
    if (numDims != 1)
        throw InvalidReadSize{};

    hsize_t fileLength = 0;
    h5fileDataSpace.getSimpleExtentDims(&fileLength);

    if ((indexStart > indexEnd) || (indexEnd >= fileLength))
        throw InvalidReadSize{};

    // Query the file.
    const hsize_t count = (indexEnd - indexStart) + 1;
    const hsize_t offset = indexStart;

    const auto fileDataSpace = m_pH5vrKeyDataSet->getSpace();
    fileDataSpace.selectHyperslab(H5S_SELECT_SET, &count, &offset);

    // Initialize the output buffer.
    const auto bufferSize = pDescriptor->getReadBufferSize(1,
        static_cast<uint32_t>(count));
    UInt8Array buffer{bufferSize};

    // Prepare the memory space.
    const ::H5::DataSpace memDataSpace{1, &count, &count};

    m_pH5vrKeyDataSet->read(buffer.data(),
        H5Dget_type(m_pH5vrKeyDataSet->getId()), memDataSpace, fileDataSpace);

    return buffer;
}

//! Set the value table.
/*!
\param table
    The new value table.
*/
void GeorefMetadataLayer::setValueTable(
    std::unique_ptr<ValueTable> table) noexcept
{
    m_pValueTable = std::move(table);
}

//! \copydoc Layer::write
void GeorefMetadataLayer::writeProxy(
    uint32_t rowStart,
    uint32_t columnStart,
    uint32_t rowEnd,
    uint32_t columnEnd,
    const uint8_t* buffer)
{
    const auto h5fileDataSpace = m_pH5keyDataSet->getSpace();

    // Make sure the area being written to does not exceed the file dimensions.
    const auto numDims = h5fileDataSpace.getSimpleExtentNdims();
    if (numDims != kRank)
        throw InvalidWriteSize{};

    std::array<hsize_t, kRank> fileDims{};
    h5fileDataSpace.getSimpleExtentDims(fileDims.data());

    if ((rowEnd >= fileDims[0]) || (columnEnd >= fileDims[1]))
        throw InvalidWriteSize{};

    const auto rows = (rowEnd - rowStart) + 1;
    const auto columns = (columnEnd - columnStart) + 1;
    const std::array<hsize_t, kRank> count{rows, columns};
    const std::array<hsize_t, kRank> offset{rowStart, columnStart};

    h5fileDataSpace.selectHyperslab(H5S_SELECT_SET, count.data(), offset.data());

    // Prepare the memory space.
    const ::H5::DataSpace h5memDataSpace{kRank, count.data(), count.data()};

    m_pH5keyDataSet->write(buffer, H5Dget_type(m_pH5keyDataSet->getId()),
        h5memDataSpace, h5fileDataSpace);
}

//! \copydoc Layer::writeAttributes
void GeorefMetadataLayer::writeAttributesProxy() const
{
    // Nothing to be done.  Attributes are not modified.
}

//! Write the variable resolution metadata keys.
/*!
\param indexStart
    The starting index to write.
    Must be less than or equal to indexEnd.
\param indexEnd
    The ending index to write.  (inclusive)
\param buffer
    The keys to be written.
    Must contain at least indexEnd - indexStart + 1 keys!
*/
void GeorefMetadataLayer::writeVR(
    uint32_t indexStart,
    uint32_t indexEnd,
    const uint8_t* buffer)
{
    // Make sure the dataset is available.
    if (this->getDataset().expired())
        throw DatasetNotFound{};

    // Make sure the variable resolution key dataset is present.
    if (!m_pH5vrKeyDataSet)
        throw DatasetRequiresVariableResolution{};

    // Query the file for the specified rows and columns.
    auto h5fileDataSpace = m_pH5vrKeyDataSet->getSpace();

    // Make sure the area being read from does not exceed the file dimensions.
    hsize_t fileLength = 0;
    const auto numDims = h5fileDataSpace.getSimpleExtentDims(&fileLength);
    if (numDims != 1)
        throw InvalidWriteSize{};

    if (indexStart > indexEnd)
        throw InvalidWriteSize{};

    auto pDescriptor = std::dynamic_pointer_cast<GeorefMetadataLayerDescriptor>(
        this->getDescriptor());
    if (!pDescriptor)
        throw InvalidLayerDescriptor{};

    const hsize_t count = (indexEnd - indexStart) + 1;
    const hsize_t offset = indexStart;
    const ::H5::DataSpace memDataSpace{1, &count, &count};

    // Expand the file data space if needed.
    if (fileLength < (indexEnd + 1))
    {
        const auto newMaxLength = std::max<hsize_t>(fileLength, indexEnd + 1);

        m_pH5vrKeyDataSet->extend(&newMaxLength);

        h5fileDataSpace = m_pH5vrKeyDataSet->getSpace();

        // Update the dataset's dimensions.
        auto pDataset = this->getDataset().lock();
        pDataset->getDescriptor().setDims(1, static_cast<uint32_t>(newMaxLength));
    }

    // Write the specified data.
    h5fileDataSpace.selectHyperslab(H5S_SELECT_SET, &count, &offset);

    m_pH5vrKeyDataSet->write(buffer, H5Dget_type(m_pH5vrKeyDataSet->getId()),
        memDataSpace, h5fileDataSpace);
}

}  // namespace BAG

