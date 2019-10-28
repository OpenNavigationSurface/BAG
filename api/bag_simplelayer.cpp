
#include "bag_private.h"
#include "bag_simplelayer.h"
#include "bag_simplelayerdescriptor.h"

#include <array>
#include <h5cpp.h>


namespace BAG {

namespace {

const ::H5::PredType& getH5PredType(DataType type)
{
    switch(type)
    {
    case UINT32:
        return ::H5::PredType::NATIVE_UINT32;
    case FLOAT32:
        return ::H5::PredType::NATIVE_FLOAT;
    default:
        throw 1122;  // Unsupported DataType.
    }
}

}  // namespace

SimpleLayer::SimpleLayer(
    Dataset& dataset,
    LayerDescriptor& descriptor,
    std::unique_ptr<::H5::DataSet, Dataset::DeleteH5DataSet> h5dataSet)
    : Layer(dataset, descriptor)
    , m_pH5dataSet(std::move(h5dataSet))
{
}

std::unique_ptr<SimpleLayer> SimpleLayer::create(
    Dataset& dataset,
    LayerType type)
{
    auto descriptor = SimpleLayerDescriptor::create(type);
    auto h5DataSet = SimpleLayer::createH5dataSet(dataset, *descriptor);

    return std::unique_ptr<SimpleLayer>(new SimpleLayer{dataset, *descriptor,
        std::move(h5DataSet)});
}

std::unique_ptr<SimpleLayer> SimpleLayer::open(
    Dataset& dataset,
    LayerDescriptor& descriptor)
{
    auto h5DataSet = dataset.openLayerH5DataSet(descriptor);

    // Read the min/max attribute values.
    const auto possibleMinMax = dataset.getMinMax(descriptor.getLayerType());
    if (std::get<0>(possibleMinMax))
        descriptor.setMinMax({std::get<1>(possibleMinMax),
            std::get<1>(possibleMinMax)});

    return std::unique_ptr<SimpleLayer>(new SimpleLayer{dataset, descriptor,
        std::move(h5DataSet)});
}


std::unique_ptr<::H5::DataSet, Dataset::DeleteH5DataSet>
SimpleLayer::createH5dataSet(
    const Dataset& dataset,
    const LayerDescriptor& descriptor)
{
    const auto dims = descriptor.getDims();
    std::array<hsize_t, 2> size{std::get<0>(dims), std::get<1>(dims)};
    ::H5::DataSpace h5dataSpace{RANK, size.data()};  //TODO Check that max dim size == starting size

    ::H5::AtomType h5dataType{::H5::PredType::NATIVE_FLOAT};
    h5dataType.setOrder(H5T_ORDER_LE);

    const ::H5::DSetCreatPropList h5createPropList{};
    h5createPropList.setFillTime(H5D_FILL_TIME_ALLOC);

    constexpr float kFillValue = BAG_NULL_ELEVATION;
    h5createPropList.setFillValue(h5dataType, &kFillValue);

    const int compressionLevel = descriptor.getCompressionLevel();
    if (compressionLevel > 0 && compressionLevel <= 9)
    {
        h5createPropList.setLayout(H5D_CHUNKED);

        const std::array<uint64_t, 2> chunkSize{descriptor.getChunkSize(),
            descriptor.getChunkSize()};
        h5createPropList.setChunk(RANK, chunkSize.data());
        h5createPropList.setDeflate(compressionLevel);
    }

    const auto& h5file = dataset.getH5file();

    auto pH5dataSet = std::unique_ptr<::H5::DataSet, Dataset::DeleteH5DataSet>(
            new ::H5::DataSet{h5file.createDataSet(descriptor.getInternalPath(),
                h5dataType, h5dataSpace, h5createPropList)},
            Dataset::DeleteH5DataSet{});

    const char* minAttName = MIN_ELEVATION_NAME;
    const char* maxAttName = MAX_ELEVATION_NAME;

    switch (descriptor.getLayerType())
    {
    case Elevation:
        break;
    case Uncertainty:
        minAttName = MIN_UNCERTAINTY_NAME;
        maxAttName = MAX_UNCERTAINTY_NAME;
        break;
    case Hypothesis_Strength:
        minAttName = MIN_HYPOTHESIS_STRENGTH;
        maxAttName = MAX_HYPOTHESIS_STRENGTH;
        break;
    case Num_Hypotheses:
        minAttName = MIN_NUM_HYPOTHESES;
        maxAttName = MAX_NUM_HYPOTHESES;
        break;
    case Shoal_Elevation:
        minAttName = MIN_SHOAL_ELEVATION;
        maxAttName = MAX_SHOAL_ELEVATION;
        break;
    case Std_Dev:
        minAttName = MIN_STANDARD_DEV_NAME;
        maxAttName = MAX_STANDARD_DEV_NAME;
        break;
    case Num_Soundings:
        minAttName = MIN_NUM_SOUNDINGS;
        maxAttName = MAX_NUM_SOUNDINGS;
        break;
    case Average_Elevation:
        minAttName = MIN_AVERAGE;
        maxAttName = MAX_AVERAGE;
        break;
    case Nominal_Elevation:
        minAttName = MIN_NOMINAL_ELEVATION;
        maxAttName = MAX_NOMINAL_ELEVATION;
        break;
    case Compound:
        //[[fallthrough]];
    default:
        throw UnknownSimpleLayerType{};
    }

    const ::H5::DataSpace minElevDataSpace{};
    const auto minElevAtt = pH5dataSet->createAttribute(minAttName,
        ::H5::PredType::NATIVE_FLOAT, minElevDataSpace);

    constexpr float minElev = 0.f;
    minElevAtt.write(::H5::PredType::NATIVE_FLOAT, &minElev);

    const ::H5::DataSpace maxElevDataSpace{};
    const auto maxElevAtt = pH5dataSet->createAttribute(maxAttName,
        ::H5::PredType::NATIVE_FLOAT, maxElevDataSpace);

    constexpr float maxElev = 0.f;
    maxElevAtt.write(::H5::PredType::NATIVE_FLOAT, &maxElev);

    return pH5dataSet;
}

std::unique_ptr<uint8_t[]> SimpleLayer::readProxy(
    uint32_t rowStart,
    uint32_t columnStart,
    uint32_t rowEnd,
    uint32_t columnEnd) const
{
    //TODO Consider if this is reading 1 row and 1 column .. (RANK == 1)
    const auto rows = (rowEnd - rowStart) + 1;
    const auto columns = (columnEnd - columnStart) + 1;
    const std::array<hsize_t, RANK> count{rows, columns};
    const std::array<hsize_t, RANK> offset{rowStart, columnStart};

    // Query the file for the specified rows and columns.
    const auto h5dataSpace = m_pH5dataSet->getSpace();
    h5dataSpace.selectHyperslab(H5S_SELECT_SET, count.data(), offset.data());

    // Initialize the output buffer.
    const auto bufferSize = this->getDescriptor().getReadBufferSize(rows,
        columns);
    auto buffer = std::make_unique<uint8_t[]>(bufferSize);

    // Prepare the memory space.
    const ::H5::DataSpace h5memSpace{RANK, count.data(), count.data()};

    m_pH5dataSet->read(buffer.get(), H5Dget_type(m_pH5dataSet->getId()),
        h5memSpace, h5dataSpace);

    return buffer;
}

void SimpleLayer::writeProxy(
    uint32_t rowStart,
    uint32_t columnStart,
    uint32_t rowEnd,
    uint32_t columnEnd,
    const uint8_t* buffer) const
{
    //TODO Consider if this is writing 1 row and 1 column .. (RANK == 1)
    const auto rows = (rowEnd - rowStart) + 1;
    const auto columns = (columnEnd - columnStart) + 1;
    const std::array<hsize_t, RANK> count{rows, columns};
    const std::array<hsize_t, RANK> offset{rowStart, columnStart};

    // Query the file for the specified rows and columns.
    const auto h5fileSpace = m_pH5dataSet->getSpace();
    h5fileSpace.selectHyperslab(H5S_SELECT_SET, count.data(), offset.data());

    // Prepare the memory space.
    const ::H5::DataSpace h5memSpace{RANK, count.data(), count.data()};

    m_pH5dataSet->write(buffer, H5Dget_type(m_pH5dataSet->getId()),
        h5memSpace, h5fileSpace);
}

}   //namespace BAG

