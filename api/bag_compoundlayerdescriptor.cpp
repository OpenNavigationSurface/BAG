
#include "bag_dataset.h"
#include "bag_compoundlayerdescriptor.h"
#include "bag_dataset.h"
#include "bag_hdfhelper.h"
#include "bag_private.h"
#include "bag_valuetable.h"

#include <array>
#include <H5Cpp.h>


namespace BAG {

//! Constructor.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
\param name
    The name of the simple layer this layer has metadata for.
\param indexType
    The type of index in the indices.
    Must be DT_UINT8, DT_UINT16, DT_UINT32, or DT_UINT64.
\param definition
    The list of fields describing a record this layer contains for each node.
\param chunkSize
    The chunk size the HDF5 DataSet will use.
\param compressionLevel
    The compression level the HDF5 DataSet will use.
*/
CompoundLayerDescriptor::CompoundLayerDescriptor(
    Dataset& dataset,
    const std::string& name,
    DataType indexType,
    RecordDefinition definition,
    uint64_t chunkSize,
    int compressionLevel)
    : LayerDescriptor(dataset.getNextId(), COMPOUND_PATH + name, name, Compound,
        chunkSize, compressionLevel)
    , m_pBagDataset(dataset.shared_from_this())
    , m_dataType(indexType)
    , m_elementSize(Layer::getElementSize(indexType))
    , m_definition(std::move(definition))
{
}

//! Create a compound layer.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
\param name
    The name of the simple layer this layer has metadata for.
\param indexType
    The type of index in the indices.
    Must be DT_UINT8, DT_UINT16, DT_UINT32, or DT_UINT64.
\param definition
    The list of fields describing a record this layer contains for each node.
\param chunkSize
    The chunk size the HDF5 DataSet will use.
\param compressionLevel
    The compression level the HDF5 DataSet will use.

\return
    The new compound layer.
*/
std::shared_ptr<CompoundLayerDescriptor> CompoundLayerDescriptor::create(
    Dataset& dataset,
    const std::string& name,
    DataType indexType,
    RecordDefinition definition,
    uint64_t chunkSize,
    int compressionLevel)
{
    return std::shared_ptr<CompoundLayerDescriptor>(
        new CompoundLayerDescriptor{dataset, name, indexType,
            std::move(definition), chunkSize, compressionLevel});
}

//! Open an existing compound layer.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
\param name
    The name of the simple layer this layer has metadata for.

\return
    The existing compound layer.
*/
std::shared_ptr<CompoundLayerDescriptor> CompoundLayerDescriptor::open(
    Dataset& dataset,
    const std::string& name)
{
    const auto& h5file = dataset.getH5file();

    const std::string internalPath{COMPOUND_PATH + name};
    const auto h5dataSet = ::H5::DataSet{h5file.openDataSet(internalPath)};

    // Determine indexType.
    const auto fileDataType = h5dataSet.getDataType();

    DataType indexType = DT_UNKNOWN_DATA_TYPE;
    switch(fileDataType.getSize())
    {
    case 1:
        indexType = DT_UINT8;
        break;
    case 2:
        indexType = DT_UINT16;
        break;
    case 4:
        indexType = DT_UINT32;
        break;
    case 8:
        indexType = DT_UINT64;
        break;
    default:
        throw InvalidIndexType{};
    }

    // Determine Record Definition.
    const auto attribute = h5dataSet.openAttribute(COMPOUND_RECORD_DEFINITION);

    const auto fileDataSpace = attribute.getSpace();

    const auto numDims = fileDataSpace.getSimpleExtentNdims();
    if (numDims != 1)
        throw InvalidCompoundRecordsSize{};

    hsize_t numFields = 0;
    fileDataSpace.getSimpleExtentDims(&numFields, nullptr);

    RecordDefinition definition(numFields);

    attribute.read(attribute.getDataType(), definition.data());

    // Determine chunk size and compression level.
    const auto chunkSize = BAG::getChunkSize(h5file, internalPath);
    const auto compressionLevel = BAG::getCompressionLevel(h5file, internalPath);

    return std::shared_ptr<CompoundLayerDescriptor>(
        new CompoundLayerDescriptor{dataset, name, indexType, definition,
            chunkSize, compressionLevel});
}


//! \copydoc LayerDescriptor::getDataType
DataType CompoundLayerDescriptor::getDataTypeProxy() const noexcept
{
    return m_dataType;
}

//! Retrieve the BAG Dataset this layer belongs to.
/*!
\return
    The BAG Dataset.
*/
std::weak_ptr<Dataset> CompoundLayerDescriptor::getDataset() const &
{
    return m_pBagDataset;
}

//! \copydoc LayerDescriptor::getElementSize
uint8_t CompoundLayerDescriptor::getElementSizeProxy() const noexcept
{
    return m_elementSize;
}

//! Retrieve the definition of a record in this layer.
/*!
\return
    The definition of a record in this layer.
*/
const RecordDefinition&
CompoundLayerDescriptor::getDefinition() const & noexcept
{
    return m_definition;
}

}  // namespace BAG

