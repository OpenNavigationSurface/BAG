
#include "bag_dataset.h"
#include "bag_compoundlayerdescriptor.h"
#include "bag_dataset.h"
#include "bag_hdfhelper.h"
#include "bag_private.h"
#include "bag_valuetable.h"

#include <array>
#include <H5Cpp.h>


namespace BAG {

CompoundLayerDescriptor::CompoundLayerDescriptor(
    const std::string& name,
    DataType indexType,
    RecordDefinition definition,
    uint64_t chunkSize,
    unsigned int compressionLevel,
    Dataset& dataset)
    : LayerDescriptor(dataset.getNextId(), COMPOUND_PATH + name, name, Compound,
        chunkSize, compressionLevel)
    , m_pBagDataset(dataset.shared_from_this())
    , m_dataType(indexType)
    , m_elementSize(Layer::getElementSize(indexType))
    , m_definition(std::move(definition))
{
}

std::shared_ptr<CompoundLayerDescriptor> CompoundLayerDescriptor::create(
    const std::string& name,
    DataType indexType,
    RecordDefinition definition,
    uint64_t chunkSize,
    unsigned int compressionLevel,
    Dataset& dataset)
{
    return std::shared_ptr<CompoundLayerDescriptor>(
        new CompoundLayerDescriptor{name, indexType, std::move(definition),
            chunkSize, compressionLevel, dataset});
}

std::shared_ptr<CompoundLayerDescriptor> CompoundLayerDescriptor::open(
    const std::string& name,
    Dataset& dataset)
{
    const auto& h5file = dataset.getH5file();

    const std::string internalPath{COMPOUND_PATH + name};
    const auto h5dataSet = ::H5::DataSet{h5file.openDataSet(internalPath)};

    // Determine indexType.
    const auto fileDataType = h5dataSet.getDataType();

    DataType indexType = UNKNOWN_DATA_TYPE;
    switch(fileDataType.getSize())
    {
    case 1:
        indexType = UINT8;
        break;
    case 2:
        indexType = UINT16;
        break;
    case 4:
        indexType = UINT32;
        break;
    case 8:
        indexType = UINT64;
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
        new CompoundLayerDescriptor{name, indexType, definition, chunkSize,
            compressionLevel, dataset});
}


DataType CompoundLayerDescriptor::getDataTypeProxy() const noexcept
{
    return m_dataType;
}

std::weak_ptr<Dataset> CompoundLayerDescriptor::getDataset() const &
{
    return m_pBagDataset;
}

uint8_t CompoundLayerDescriptor::getElementSizeProxy() const noexcept
{
    return m_elementSize;
}

const RecordDefinition&
CompoundLayerDescriptor::getDefinition() const & noexcept
{
    return m_definition;
}

}  // namespace BAG

