
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
    std::unique_ptr<::H5::DataSet, DeleteH5dataSet> h5dataSet)
    : Layer(dataset, descriptor)
    , m_pH5dataSet(std::move(h5dataSet))
{
}

std::unique_ptr<SimpleLayer> SimpleLayer::create(
    Dataset& dataset,
    LayerType type,
    uint64_t chunkSize,
    unsigned int compressionLevel)
{
    auto descriptor = SimpleLayerDescriptor::create(type, chunkSize,
        compressionLevel);
    auto h5dataSet = SimpleLayer::createH5dataSet(dataset, *descriptor);

    return std::unique_ptr<SimpleLayer>(new SimpleLayer{dataset, *descriptor,
        std::move(h5dataSet)});
}

std::unique_ptr<SimpleLayer> SimpleLayer::open(
    Dataset& dataset,
    LayerDescriptor& descriptor)
{
    const auto& h5file = dataset.getH5file();
    auto h5dataSet = std::unique_ptr<::H5::DataSet, DeleteH5dataSet>(
        new ::H5::DataSet{h5file.openDataSet(descriptor.getInternalPath())},
        DeleteH5dataSet{});

    // Read the min/max attribute values.
    const auto possibleMinMax = dataset.getMinMax(descriptor.getLayerType());
    if (std::get<0>(possibleMinMax))
        descriptor.setMinMax({std::get<1>(possibleMinMax),
            std::get<2>(possibleMinMax)});

    return std::unique_ptr<SimpleLayer>(new SimpleLayer{dataset, descriptor,
        std::move(h5dataSet)});
}


std::unique_ptr<::H5::DataSet, SimpleLayer::DeleteH5dataSet>
SimpleLayer::createH5dataSet(
    const Dataset& dataset,
    const LayerDescriptor& descriptor)
{
    // Use the dimensions from the descriptor.
    const auto dims = descriptor.getDims();
    const std::array<hsize_t, RANK> fileDims{std::get<0>(dims), std::get<1>(dims)};
    const std::array<uint64_t, RANK> kMaxFileDims{H5S_UNLIMITED, H5S_UNLIMITED};
    ::H5::DataSpace h5dataSpace{RANK, fileDims.data(), kMaxFileDims.data()};

    ::H5::AtomType h5dataType{::H5::PredType::NATIVE_FLOAT};
    h5dataType.setOrder(H5T_ORDER_LE);

    // Create the creation property list.
    const ::H5::DSetCreatPropList h5createPropList{};
    h5createPropList.setFillTime(H5D_FILL_TIME_ALLOC);

    constexpr float kFillValue = BAG_NULL_ELEVATION;
    h5createPropList.setFillValue(h5dataType, &kFillValue);

    // Use chunk size and compression level from the descriptor.
    const int compressionLevel = descriptor.getCompressionLevel();
    if (compressionLevel > 0 && compressionLevel <= 9)
    {
        h5createPropList.setLayout(H5D_CHUNKED);

        const std::array<uint64_t, 2> chunkSize{descriptor.getChunkSize(),
            descriptor.getChunkSize()};
        h5createPropList.setChunk(RANK, chunkSize.data());
        h5createPropList.setDeflate(compressionLevel);
    }

    // Create the DataSet using the above.
    const auto& h5file = dataset.getH5file();

    auto pH5dataSet = std::unique_ptr<::H5::DataSet, DeleteH5dataSet>(
            new ::H5::DataSet{h5file.createDataSet(descriptor.getInternalPath(),
                h5dataType, h5dataSpace, h5createPropList)},
            DeleteH5dataSet{});

    // Create any attributes.
    const auto attInfo = Layer::getAttributeInfo(descriptor.getLayerType());

    const ::H5::DataSpace minElevDataSpace{};
    const auto minElevAtt = pH5dataSet->createAttribute(attInfo.minName,
        attInfo.h5type, minElevDataSpace);

    constexpr float minElev = std::numeric_limits<float>::lowest();
    minElevAtt.write(attInfo.h5type, &minElev);

    const ::H5::DataSpace maxElevDataSpace{};
    const auto maxElevAtt = pH5dataSet->createAttribute(attInfo.maxName,
        attInfo.h5type, maxElevDataSpace);

    constexpr float maxElev = std::numeric_limits<float>::max();
    maxElevAtt.write(attInfo.h5type, &maxElev);

    return pH5dataSet;
}

std::unique_ptr<uint8_t[]> SimpleLayer::readProxy(
    uint32_t rowStart,
    uint32_t columnStart,
    uint32_t rowEnd,
    uint32_t columnEnd) const
{
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
    const uint8_t* buffer)
{
    const auto rows = (rowEnd - rowStart) + 1;
    const auto columns = (columnEnd - columnStart) + 1;
    const std::array<hsize_t, RANK> count{rows, columns};
    const std::array<hsize_t, RANK> offset{rowStart, columnStart};

    auto h5fileDataSpace = m_pH5dataSet->getSpace();

    // Expand the file data space if needed.
    std::array<hsize_t, RANK> fileDims{};
    std::array<hsize_t, RANK> maxFileDims{};
    h5fileDataSpace.getSimpleExtentDims(fileDims.data(), maxFileDims.data());

    if ((fileDims[0] < (rowEnd + 1)) ||
        (fileDims[1] < (columnEnd + 1)))
    {
        const std::array<hsize_t, RANK> newDims{
            std::max<hsize_t>(fileDims[0], rowEnd + 1),
            std::max<hsize_t>(fileDims[1], columnEnd + 1)};

        m_pH5dataSet->extend(newDims.data());

        h5fileDataSpace = m_pH5dataSet->getSpace();

        // Update the layer descriptor's dimensions.
        this->getDescriptor().setDims({static_cast<uint32_t>(newDims[0]),
            static_cast<uint32_t>(newDims[1])});
    }

    h5fileDataSpace.selectHyperslab(H5S_SELECT_SET, count.data(), offset.data());

    // Prepare the memory space.
    const ::H5::DataSpace h5memDataSpace{RANK, count.data(), count.data()};

    m_pH5dataSet->write(buffer, H5Dget_type(m_pH5dataSet->getId()),
        h5memDataSpace, h5fileDataSpace);

    // Update min/max attributes
    auto& descriptor = this->getDescriptor();
    const auto attInfo = Layer::getAttributeInfo(descriptor.getLayerType());
    std::tuple<float, float> minMax{0.f, 0.f};

    if (attInfo.h5type == ::H5::PredType::NATIVE_FLOAT)
    {
        const auto* floatBuffer = reinterpret_cast<const float*>(buffer);

        const auto begin = floatBuffer;
        const auto end = floatBuffer + rows * columns;

        const auto mm = std::minmax_element(begin, end);
        minMax = std::make_tuple(*std::get<0>(mm), *std::get<1>(mm));
    }
    else if (attInfo.h5type == ::H5::PredType::NATIVE_UINT32)
    {
        const auto* uint32Buffer = reinterpret_cast<const uint32_t*>(buffer);

        const auto begin = uint32Buffer;
        const auto end = uint32Buffer + rows * columns;

        const auto mm = std::minmax_element(begin, end);
        minMax = std::make_tuple(static_cast<float>(*std::get<0>(mm)),
            static_cast<float>(*std::get<1>(mm)));
    }
    else
        throw UnsupportedAttributeType{};

    auto currentMinMax = descriptor.getMinMax();

    std::get<0>(currentMinMax) = std::min(std::get<0>(minMax), std::get<0>(currentMinMax));
    std::get<1>(currentMinMax) = std::max(std::get<1>(minMax), std::get<1>(currentMinMax));

    descriptor.setMinMax(currentMinMax);
}

void SimpleLayer::DeleteH5dataSet::operator()(::H5::DataSet* ptr) noexcept
{
    delete ptr;
}

}   //namespace BAG

