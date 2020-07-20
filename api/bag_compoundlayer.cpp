
#include "bag_compoundlayer.h"
#include "bag_compoundlayerdescriptor.h"
#include "bag_hdfhelper.h"
#include "bag_private.h"

#include <array>
#include <H5Cpp.h>
#include <limits>
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
\param pH5indexDataSet
    The HDF5 DataSet that will hold the index values.
\param pH5recordDataSet
    The HDF5 DataSet that will hold the records.
*/
CompoundLayer::CompoundLayer(
    Dataset& dataset,
    CompoundLayerDescriptor& descriptor,
    std::unique_ptr<::H5::DataSet, DeleteH5dataSet> pH5indexDataSet,
    std::unique_ptr<::H5::DataSet, DeleteH5dataSet> pH5recordDataSet)
    : Layer(dataset, descriptor)
    , m_pH5indexDataSet(std::move(pH5indexDataSet))
    , m_pH5recordDataSet(std::move(pH5recordDataSet))
{
}

//! Create a compound layer.
/*!
\param indexType
    The type of index this layer will use.
\param name
    The name of this compound layer.
    Must be a unique name among all compound layers in this BAG Dataset.
\param dataset
    The BAG Dataset this compound layer will belong to.
\param definition
    The list of fields describing a single record.
\param chunkSize
    The chunk size the HDF5 DataSet will use.
\param compressionLevel
    The compression level the HDF5 DataSet will use.

\return
    The new compound layer.
*/
std::unique_ptr<CompoundLayer> CompoundLayer::create(
    DataType indexType,
    const std::string& name,
    Dataset& dataset,
    const RecordDefinition& definition,
    uint64_t chunkSize,
    int compressionLevel)
{
    if (indexType != DT_UINT8 && indexType != DT_UINT16 && indexType != DT_UINT32 &&
        indexType != DT_UINT64)
        throw InvalidIndexType{};

    auto pDescriptor = CompoundLayerDescriptor::create(dataset, name, indexType,
        definition, chunkSize, compressionLevel);

    // Create the H5 Group to hold keys & values.
    const auto& h5file = dataset.getH5file();
    h5file.createGroup(COMPOUND_PATH + name);

    auto h5indexDataSet = CompoundLayer::createH5indexDataSet(dataset, *pDescriptor);
    auto h5recordDataSet = CompoundLayer::createH5recordDataSet(dataset, *pDescriptor);

    auto layer = std::unique_ptr<CompoundLayer>(new CompoundLayer{dataset,
        *pDescriptor, std::move(h5indexDataSet), std::move(h5recordDataSet)});

    layer->setValueTable(std::unique_ptr<ValueTable>(new ValueTable{*layer}));

    return layer;
}

//! Open an existing compound layer.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
\param descriptor
    The descriptor of this layer.

\return
    The compound layer read from dataset.
*/
std::unique_ptr<CompoundLayer> CompoundLayer::open(
    Dataset& dataset,
    CompoundLayerDescriptor& descriptor)
{
    const auto& h5file = dataset.getH5file();
    auto h5indexDataSet = std::unique_ptr<::H5::DataSet, DeleteH5dataSet>(
        new ::H5::DataSet{h5file.openDataSet(descriptor.getInternalPath())},
        DeleteH5dataSet{});

    auto h5recordDataSet = std::unique_ptr<::H5::DataSet, DeleteH5dataSet>(
        new ::H5::DataSet{h5file.openDataSet(descriptor.getValuesPath())},
        DeleteH5dataSet{});

    auto layer = std::unique_ptr<CompoundLayer>(new CompoundLayer{dataset,
        descriptor, std::move(h5indexDataSet), std::move(h5recordDataSet)});

    layer->setValueTable(std::unique_ptr<ValueTable>(new ValueTable{*layer}));

    return layer;
}


//! Create an HDF5 DataSet for the indices of a compound layer with details in from the descriptor.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
\param descriptor
    The descriptor of this layer.

\return
    The HDF5 DataSet containing the indices of a new compound layer.
*/
std::unique_ptr<::H5::DataSet, DeleteH5dataSet>
CompoundLayer::createH5indexDataSet(
    const Dataset& dataset,
    const CompoundLayerDescriptor& descriptor)
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
            new ::H5::DataSet{h5file.createDataSet(descriptor.getInternalPath(),
                fileDataType, fileDataSpace, h5createPropList)},
            DeleteH5dataSet{});
    }

    // Create the Record Definition attribute.
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

        // Write the value.
        att.write(fileDataType, definition.data());
    }

    return pH5dataSet;
}

//! Create an HDF5 DataSet for the records of a compound layer with details from the descriptor.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
\param descriptor
    The descriptor of this layer.

\return
    The HDF5 DataSet.
*/
std::unique_ptr<::H5::DataSet, DeleteH5dataSet>
CompoundLayer::createH5recordDataSet(
    const Dataset& dataset,
    const CompoundLayerDescriptor& descriptor)
{
    constexpr hsize_t numRecords = 1;
    const auto dataType = descriptor.getDataType();
    const hsize_t maxNumRecords = getDataTypeMax(dataType);

    const ::H5::DataSpace fileDataSpace{1, &numRecords, &maxNumRecords};

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
        new ::H5::DataSet{h5file.createDataSet(descriptor.getValuesPath(),
            fileDataType, fileDataSpace, h5createPropList)},
        DeleteH5dataSet{});

    return pH5dataSet;
}

//! Retrieve the HDF5 DataSet containing the records.
/*!
\return
    The HDF5 DataSet containing the records.
*/
const ::H5::DataSet& CompoundLayer::getRecordDataSet() const &
{
    return *m_pH5recordDataSet;
}

//! Retrieve the value table.
/*!
\return
    The value table.
*/
ValueTable& CompoundLayer::getValueTable() & noexcept
{
    return *m_pValueTable;
}

//! Retrieve the value table.
/*!
\return
    The value table.
*/
const ValueTable& CompoundLayer::getValueTable() const & noexcept
{
    return *m_pValueTable;
}

//! \copydoc Layer::read
UInt8Array CompoundLayer::readProxy(
    uint32_t rowStart,
    uint32_t columnStart,
    uint32_t rowEnd,
    uint32_t columnEnd) const
{
    // Query the file for the specified rows and columns.
    const auto h5fileDataSpace = m_pH5indexDataSet->getSpace();

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

    m_pH5indexDataSet->read(buffer.data(), H5Dget_type(m_pH5indexDataSet->getId()),
        h5memSpace, h5fileDataSpace);

    return buffer;
}

//! Set the value table.
/*!
\param table
    The new value table.
*/
void CompoundLayer::setValueTable(
    std::unique_ptr<ValueTable> table) noexcept
{
    m_pValueTable = std::move(table);
}

//! \copydoc Layer::write
void CompoundLayer::writeProxy(
    uint32_t rowStart,
    uint32_t columnStart,
    uint32_t rowEnd,
    uint32_t columnEnd,
    const uint8_t* buffer)
{
    const auto h5fileDataSpace = m_pH5indexDataSet->getSpace();

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

    m_pH5indexDataSet->write(buffer, H5Dget_type(m_pH5indexDataSet->getId()),
        h5memDataSpace, h5fileDataSpace);
}

//! \copydoc Layer::writeAttributes
void CompoundLayer::writeAttributesProxy() const
{
    // Nothing to be done.  Attributes are not modified.
}

}  // namespace BAG

