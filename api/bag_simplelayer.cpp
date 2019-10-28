
#include "bag_private.h"
#include "bag_simplelayer.h"

#include <array>
#include <h5cpp.h>


namespace BAG {

#if 0 // TODO Needed?
namespace {

const ::H5:PredType& getH5dataType(
    LayerType layerType)
{
    switch (layerType)
    {
    case Elevation: // Uncertainty, Nominal_Elevation
        h5type.insertMember("hyp_strength",
            HOFFSET(BagOptNodeGroup, hyp_strength),
            ::H5::PredType::NATIVE_FLOAT);
        break;
    case Num_Hypotheses:
        h5type.insertMember("num_hypotheses",
            HOFFSET(BagOptNodeGroup, num_hypotheses),
            ::H5::PredType::NATIVE_UINT);
        break;
    default:
        throw 112233;  // Unknown group type.
        break;
    }
}

}  // namespace
#endif

SimpleLayer::SimpleLayer(
    Dataset& dataset,
    const LayerDescriptor& descriptor,
    std::unique_ptr<::H5::DataSet, Dataset::DeleteH5DataSet> h5dataSet)
    : Layer(dataset, descriptor)
    , m_pH5dataSet(std::move(h5dataSet))
{
}

std::unique_ptr<SimpleLayer> SimpleLayer::create(
    Dataset& dataset,
    const LayerDescriptor& descriptor)
{
    auto h5DataSet = dataset.createH5DataSet(descriptor);

    return std::unique_ptr<SimpleLayer>(new SimpleLayer{dataset, descriptor,
        std::move(h5DataSet)});
}

std::unique_ptr<SimpleLayer> SimpleLayer::open(
    Dataset& dataset,
    const LayerDescriptor& descriptor)
{
    auto h5DataSet = dataset.openH5DataSet(descriptor);
    return std::unique_ptr<SimpleLayer>(new SimpleLayer{dataset, descriptor,
        std::move(h5DataSet)});
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
    const auto h5dataSpace = m_pH5dataSet->getSpace();
    h5dataSpace.selectHyperslab(H5S_SELECT_SET, count.data(), offset.data());

    // Prepare the memory space.
    const ::H5::DataSpace h5memSpace{2, count.data(), count.data()};

    m_pH5dataSet->write(buffer, H5Dget_type(m_pH5dataSet->getId()),
        h5memSpace, h5dataSpace);
}

}   //namespace BAG

