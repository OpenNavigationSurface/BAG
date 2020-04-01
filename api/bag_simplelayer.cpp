
#include "bag_attributeinfo.h"
#include "bag_private.h"
#include "bag_simplelayer.h"
#include "bag_simplelayerdescriptor.h"

#include <algorithm>
#include <array>
#include <H5Cpp.h>


namespace BAG {

//! Constructor.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
\param descriptor
    The descriptor of this layer.
\param pH5dataSet
    The HDF5 DataSet that stores this layer.
*/
SimpleLayer::SimpleLayer(
    Dataset& dataset,
    SimpleLayerDescriptor& descriptor,
    std::unique_ptr<::H5::DataSet, DeleteH5dataSet> pH5dataSet)
    : Layer(dataset, descriptor)
    , m_pH5dataSet(std::move(pH5dataSet))
{
}

//! Create a new simple layer.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
\param type
    The type of layer.
\param chunkSize
    The chunk size the HDF5 DataSet will use.
\param compressionLevel
    The compression level the HDF5 DataSet will use.

\return
    The new simple layer.
*/
std::unique_ptr<SimpleLayer> SimpleLayer::create(
    Dataset& dataset,
    LayerType type,
    uint64_t chunkSize,
    int compressionLevel)
{
    auto descriptor = SimpleLayerDescriptor::create(dataset, type, chunkSize,
        compressionLevel);
    auto h5dataSet = SimpleLayer::createH5dataSet(dataset, *descriptor);

    return std::unique_ptr<SimpleLayer>(new SimpleLayer{dataset, *descriptor,
        std::move(h5dataSet)});
}

//! Open an existing simple layer.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
\param descriptor
    The descriptor of this layer.

\return
    The specified simple layer.
*/
std::unique_ptr<SimpleLayer> SimpleLayer::open(
    Dataset& dataset,
    SimpleLayerDescriptor& descriptor)
{
    const auto& h5file = dataset.getH5file();
    auto h5dataSet = std::unique_ptr<::H5::DataSet, DeleteH5dataSet>(
        new ::H5::DataSet{h5file.openDataSet(descriptor.getInternalPath())},
        DeleteH5dataSet{});

    // Read the min/max attribute values.
    const auto possibleMinMax = dataset.getMinMax(descriptor.getLayerType());
    if (std::get<0>(possibleMinMax))
        descriptor.setMinMax(std::get<1>(possibleMinMax),
            std::get<2>(possibleMinMax));

    return std::unique_ptr<SimpleLayer>(new SimpleLayer{dataset, descriptor,
        std::move(h5dataSet)});
}


//! Create the HDF5 DataSet.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
\param descriptor
    The descriptor of this layer.

\return
    The new HDF5 DataSet.
*/
std::unique_ptr<::H5::DataSet, DeleteH5dataSet>
SimpleLayer::createH5dataSet(
    const Dataset& dataset,
    const SimpleLayerDescriptor& descriptor)
{
    uint32_t dim0 = 0, dim1 = 0;
    std::tie(dim0, dim1) = dataset.getDescriptor().getDims();
    const std::array<hsize_t, kRank> fileDims{dim0, dim1};

    ::H5::DataSpace h5dataSpace{kRank, fileDims.data(), fileDims.data()};

    ::H5::FloatType h5dataType;
    h5dataType.copy(::H5::PredType::NATIVE_FLOAT);
    h5dataType.setOrder(H5T_ORDER_LE);

    // Create the creation property list.
    const ::H5::DSetCreatPropList h5createPropList{};
    h5createPropList.setFillTime(H5D_FILL_TIME_ALLOC);

    constexpr float kFillValue = BAG_NULL_ELEVATION;
    h5createPropList.setFillValue(h5dataType, &kFillValue);

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

    // Create the DataSet using the above.
    const auto& h5file = dataset.getH5file();

    auto pH5dataSet = std::unique_ptr<::H5::DataSet, DeleteH5dataSet>(
        new ::H5::DataSet{h5file.createDataSet(descriptor.getInternalPath(),
            h5dataType, h5dataSpace, h5createPropList)},
        DeleteH5dataSet{});

    // Create any attributes.
    const auto attInfo = getAttributeInfo(descriptor.getLayerType());

    const ::H5::DataSpace minElevDataSpace{};
    const auto minElevAtt = pH5dataSet->createAttribute(attInfo.minName,
        attInfo.h5type, minElevDataSpace);

    const ::H5::DataSpace maxElevDataSpace{};
    const auto maxElevAtt = pH5dataSet->createAttribute(attInfo.maxName,
        attInfo.h5type, maxElevDataSpace);

    // Set initial min/max values.
    constexpr float minElev = std::numeric_limits<float>::max();
    minElevAtt.write(attInfo.h5type, &minElev);

    constexpr float maxElev = std::numeric_limits<float>::lowest();
    maxElevAtt.write(attInfo.h5type, &maxElev);

    return pH5dataSet;
}

//! \copydoc Layer::read
UInt8Array SimpleLayer::readProxy(
    uint32_t rowStart,
    uint32_t columnStart,
    uint32_t rowEnd,
    uint32_t columnEnd) const
{
    // Query the file for the specified rows and columns.
    const auto h5fileDataSpace = m_pH5dataSet->getSpace();

    const auto rows = (rowEnd - rowStart) + 1;
    const auto columns = (columnEnd - columnStart) + 1;
    const std::array<hsize_t, kRank> count{rows, columns};
    const std::array<hsize_t, kRank> offset{rowStart, columnStart};

    h5fileDataSpace.selectHyperslab(H5S_SELECT_SET, count.data(), offset.data());

    // Initialize the output buffer.
    const auto bufferSize = this->getDescriptor().getReadBufferSize(rows,
        columns);
    UInt8Array buffer{bufferSize};

    // Prepare the memory space.
    const ::H5::DataSpace h5memSpace{kRank, count.data(), count.data()};

    m_pH5dataSet->read(buffer.data(), H5Dget_type(m_pH5dataSet->getId()),
        h5memSpace, h5fileDataSpace);

    return buffer;
}

//! \copydoc Layer::writeAttributes
void SimpleLayer::writeAttributesProxy() const
{
    const auto& descriptor = this->getDescriptor();
    const auto attInfo = getAttributeInfo(descriptor.getLayerType());

    // Write any attributes, from the layer descriptor.
    // min value
    const auto minMax = descriptor.getMinMax();

    const auto minAtt = m_pH5dataSet->openAttribute(attInfo.minName);
    minAtt.write(attInfo.h5type, &std::get<0>(minMax));

    // max value
    const auto maxAtt = m_pH5dataSet->openAttribute(attInfo.maxName);
    maxAtt.write(attInfo.h5type, &std::get<1>(minMax));
}

//! \copydoc Layer::write
void SimpleLayer::writeProxy(
    uint32_t rowStart,
    uint32_t columnStart,
    uint32_t rowEnd,
    uint32_t columnEnd,
    const uint8_t* buffer)
{
    auto h5fileDataSpace = m_pH5dataSet->getSpace();

    // Make sure the area being written to does not exceed the file dimensions.
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

    m_pH5dataSet->write(buffer, H5Dget_type(m_pH5dataSet->getId()),
        h5memDataSpace, h5fileDataSpace);

    // Update min/max attributes
    auto& descriptor = this->getDescriptor();
    const auto attInfo = getAttributeInfo(descriptor.getLayerType());
    float min = 0.f, max = 0.f;

    if (attInfo.h5type == ::H5::PredType::NATIVE_FLOAT)
    {
        const auto* floatBuffer = reinterpret_cast<const float*>(buffer);

        const auto begin = floatBuffer;
        const auto end = floatBuffer + rows * columns;

        const auto mm = std::minmax_element(begin, end);
        min = *std::get<0>(mm);
        max = *std::get<1>(mm);
    }
    else if (attInfo.h5type == ::H5::PredType::NATIVE_UINT32)
    {
        const auto* uint32Buffer = reinterpret_cast<const uint32_t*>(buffer);

        const auto begin = uint32Buffer;
        const auto end = uint32Buffer + rows * columns;

        const auto mm = std::minmax_element(begin, end);
        min = static_cast<float>(*std::get<0>(mm));
        max = static_cast<float>(*std::get<1>(mm));
    }
    else
        throw UnsupportedAttributeType{};

    const float currentMin = std::get<0>(descriptor.getMinMax());
    const float currentMax = std::get<1>(descriptor.getMinMax());

    descriptor.setMinMax(std::min(currentMin, min), std::max(currentMax, max));
}

}   //namespace BAG

