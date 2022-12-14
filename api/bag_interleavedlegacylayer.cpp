
#include "bag_hdfhelper.h"
#include "bag_interleavedlegacylayer.h"
#include "bag_interleavedlegacylayerdescriptor.h"
#include "bag_private.h"

#include <array>
#include <H5Cpp.h>


namespace BAG {

//! Constructor
/*
\param dataset
    The BAG Dataset this layer belongs to.
\param descriptor
    The descriptor of this layer.
\param pH5dataSet
    The HDF5 DataSet that stores this interleaved layer.
*/
InterleavedLegacyLayer::InterleavedLegacyLayer(
    Dataset& dataset,
    InterleavedLegacyLayerDescriptor& descriptor,
    std::unique_ptr<::H5::DataSet, DeleteH5dataSet> pH5dataSet)
    : Layer(dataset, descriptor)
    , m_pH5dataSet(std::move(pH5dataSet))
{
}

//! Open an existing interleaved layer.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
\param descriptor
    The descriptor of this layer.
*/
std::shared_ptr<InterleavedLegacyLayer> InterleavedLegacyLayer::open(
    Dataset& dataset,
    InterleavedLegacyLayerDescriptor& descriptor)
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

    return std::make_shared<InterleavedLegacyLayer>(dataset,
        descriptor, std::move(h5dataSet));
}


//! \copydoc Layer::read
UInt8Array InterleavedLegacyLayer::readProxy(
    uint32_t rowStart,
    uint32_t columnStart,
    uint32_t rowEnd,
    uint32_t columnEnd) const
{
    auto pDescriptor =
        std::dynamic_pointer_cast<const InterleavedLegacyLayerDescriptor>(
            this->getDescriptor());
    if (!pDescriptor)
        throw InvalidDescriptor{};

    const auto rows = (rowEnd - rowStart) + 1;
    const auto columns = (columnEnd - columnStart) + 1;
    const std::array<hsize_t, kRank> count{rows, columns};
    const std::array<hsize_t, kRank> offset{rowStart, columnStart};

    // Query the file for the specified rows and columns.
    const auto h5fileSpace = m_pH5dataSet->getSpace();
    h5fileSpace.selectHyperslab(H5S_SELECT_SET, count.data(), offset.data());

    // Initialize the output buffer.
    const auto bufferSize = pDescriptor->getReadBufferSize(rows, columns);
    UInt8Array buffer{bufferSize};

    // Prepare the memory space.
    const ::H5::DataSpace h5memSpace{kRank, count.data(), count.data()};

    // Set up the type.
    const auto h5dataType = createH5compType(pDescriptor->getLayerType(),
        pDescriptor->getGroupType());

    m_pH5dataSet->read(buffer.data(), h5dataType, h5memSpace, h5fileSpace);

    return buffer;
}

//! \copydoc Layer::writeAttributes
void InterleavedLegacyLayer::writeAttributesProxy() const
{
    // Writing Interleaved layers not supported.
}

//! \copydoc Layer::write
void InterleavedLegacyLayer::writeProxy(
    uint32_t /*rowStart*/,
    uint32_t /*columnStart*/,
    uint32_t /*rowEnd*/,
    uint32_t /*columnEnd*/,
    const uint8_t* /*buffer*/)
{
    // Writing Interleaved layers not supported.
}

}   //namespace BAG

