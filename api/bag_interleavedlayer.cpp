
#include "bag_hdfhelper.h"
#include "bag_interleavedlayer.h"
#include "bag_interleavedlayerdescriptor.h"
#include "bag_private.h"

#include <array>
#include <H5Cpp.h>


namespace BAG {

InterleavedLayer::InterleavedLayer(
    Dataset& dataset,
    InterleavedLayerDescriptor& descriptor,
    std::unique_ptr<::H5::DataSet, DeleteH5dataSet> h5dataSet)
    : Layer(dataset, descriptor)
    , m_pH5dataSet(std::move(h5dataSet))
{
}

std::unique_ptr<InterleavedLayer> InterleavedLayer::open(
    Dataset& dataset,
    InterleavedLayerDescriptor& descriptor)
{
    const auto& h5file = dataset.getH5file();
    const auto& path = descriptor.getInternalPath();
    auto h5dataSet = std::unique_ptr<::H5::DataSet, DeleteH5dataSet>(
        new ::H5::DataSet{h5file.openDataSet(path)},
        DeleteH5dataSet{});

    // Read the min/max attribute values.
    const auto possibleMinMax = dataset.getMinMax(descriptor.getLayerType(), path);
    if (std::get<0>(possibleMinMax))
        descriptor.setMinMax(std::get<1>(possibleMinMax),
            std::get<2>(possibleMinMax));

    return std::unique_ptr<InterleavedLayer>(new InterleavedLayer{dataset,
        descriptor, std::move(h5dataSet)});
}


std::unique_ptr<uint8_t[]> InterleavedLayer::readProxy(
    uint32_t rowStart,
    uint32_t columnStart,
    uint32_t rowEnd,
    uint32_t columnEnd) const
{
    if (!dynamic_cast<const InterleavedLayerDescriptor*>(&this->getDescriptor()))
        throw InvalidDescriptor{};

    const auto rows = (rowEnd - rowStart) + 1;
    const auto columns = (columnEnd - columnStart) + 1;
    const std::array<hsize_t, RANK> count{rows, columns};
    const std::array<hsize_t, RANK> offset{rowStart, columnStart};

    // Query the file for the specified rows and columns.
    const auto h5fileSpace = m_pH5dataSet->getSpace();
    h5fileSpace.selectHyperslab(H5S_SELECT_SET, count.data(), offset.data());

    const auto& descriptor =
        static_cast<const InterleavedLayerDescriptor&>(this->getDescriptor());

    // Initialize the output buffer.
    const auto bufferSize = descriptor.getReadBufferSize(rows, columns);
    auto buffer = std::make_unique<uint8_t[]>(bufferSize);

    // Prepare the memory space.
    const ::H5::DataSpace h5memSpace{RANK, count.data(), count.data()};

    // Set up the type.
    const auto h5dataType = createH5compType(descriptor.getLayerType(),
        descriptor.getGroupType());

    m_pH5dataSet->read(buffer.get(), h5dataType, h5memSpace, h5fileSpace);

    return buffer;
}

void InterleavedLayer::writeProxy(
    uint32_t /*rowStart*/,
    uint32_t /*columnStart*/,
    uint32_t /*rowEnd*/,
    uint32_t /*columnEnd*/,
    const uint8_t* /*buffer*/)
{
    // Writing Interleaved layers not supported.
}

void InterleavedLayer::writeAttributesProxy() const
{
    // Writing Interleaved layers not supported.
}

void InterleavedLayer::DeleteH5dataSet::operator()(::H5::DataSet* ptr) noexcept
{
    delete ptr;
}

}   //namespace BAG

