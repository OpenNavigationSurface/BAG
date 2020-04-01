
#include "bag_hdfhelper.h"
#include "bag_private.h"
#include "bag_vrmetadata.h"
#include "bag_vrmetadatadescriptor.h"

#include <array>
#include <cstring>  //member
#include <H5Cpp.h>


namespace BAG {

namespace {

//! Generate an HDF5 CompType for the variable resolution metadata.
/*!
\return
    The HDF5 CompType for the variable resolution metadata.
*/
::H5::CompType makeDataType()
{
    const ::H5::CompType memDataType{sizeof(VRMetadataItem)};

    memDataType.insertMember("index", HOFFSET(VRMetadataItem, index),
        ::H5::PredType::NATIVE_UINT32);
    memDataType.insertMember("dimensions_x",
        HOFFSET(VRMetadataItem, dimensions_x), ::H5::PredType::NATIVE_UINT32);
    memDataType.insertMember("dimensions_y",
        HOFFSET(VRMetadataItem, dimensions_y), ::H5::PredType::NATIVE_UINT32);
    memDataType.insertMember("resolution_x",
        HOFFSET(VRMetadataItem, resolution_x), ::H5::PredType::NATIVE_FLOAT);
    memDataType.insertMember("resolution_y",
        HOFFSET(VRMetadataItem, resolution_y), ::H5::PredType::NATIVE_FLOAT);
    memDataType.insertMember("sw_corner_x",
        HOFFSET(VRMetadataItem, sw_corner_x), ::H5::PredType::NATIVE_FLOAT);
    memDataType.insertMember("sw_corner_y",
        HOFFSET(VRMetadataItem, sw_corner_y), ::H5::PredType::NATIVE_FLOAT);

    return memDataType;
}

//! Read an HDF5 attribute.
/*!
\param h5file
    The HDF5 file.
\param name
    The name of the attribute.

\return
    The value in the attribute.
*/
template<typename T>
T readAttribute(
    const ::H5::H5File& h5file,
    const char* const name)
{
    const auto h5DataSet = h5file.openDataSet(VR_METADATA_PATH);
    const auto attribute = h5DataSet.openAttribute(name);

    T value{};
    attribute.read(attribute.getDataType(), &value);

    return value;
}

}  // namespace

//! Constructor
/*!
\param dataset
    The BAG Dataset this layer belongs to.
\param descriptor
    The descriptor of this layer.
\param pH5dataSet
    The HDF5 DataSet this class wraps.
*/
VRMetadata::VRMetadata(
    Dataset& dataset,
    VRMetadataDescriptor& descriptor,
    std::unique_ptr<::H5::DataSet, DeleteH5dataSet> pH5dataSet)
    : Layer(dataset, descriptor)
    , m_pH5dataSet(std::move(pH5dataSet))
{
}

//! Create a new variable resolution metadata layer.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
\param chunkSize
    The chunk size the HDF5 DataSet will use.
\param compressionLevel
    The compression level the HDF5 DataSet will use.

\return
    The new variable resolution metadata.
*/
std::unique_ptr<VRMetadata> VRMetadata::create(
    Dataset& dataset,
    uint64_t chunkSize,
    int compressionLevel)
{
    auto descriptor = VRMetadataDescriptor::create(dataset, chunkSize,
        compressionLevel);

    auto h5dataSet = VRMetadata::createH5dataSet(dataset, *descriptor);

    return std::unique_ptr<VRMetadata>(new VRMetadata{dataset,
        *descriptor, std::move(h5dataSet)});
}

//! Open a variable resolution metadata layer.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
\param descriptor
    The descriptor of this layer.

\return
    The variable resolution metadata.
*/
std::unique_ptr<VRMetadata> VRMetadata::open(
    Dataset& dataset,
    VRMetadataDescriptor& descriptor)
{
    auto& h5file = dataset.getH5file();

    // Read the attribute values from the file and set in the descriptor.
    const auto minDimsX = readAttribute<uint32_t>(h5file, VR_METADATA_MIN_DIMS_X);
    const auto minDimsY = readAttribute<uint32_t>(h5file, VR_METADATA_MIN_DIMS_Y);

    descriptor.setMinDimensions(minDimsX, minDimsY);

    const auto maxDimsX = readAttribute<uint32_t>(h5file, VR_METADATA_MAX_DIMS_X);
    const auto maxDimsY = readAttribute<uint32_t>(h5file, VR_METADATA_MAX_DIMS_Y);

    descriptor.setMaxDimensions(maxDimsX, maxDimsY);

    const auto minResX = readAttribute<float>(h5file, VR_METADATA_MIN_RES_X);
    const auto minResY = readAttribute<float>(h5file, VR_METADATA_MIN_RES_Y);

    descriptor.setMinResolution(minResX, minResY);

    const auto maxResX = readAttribute<float>(h5file, VR_METADATA_MAX_RES_X);
    const auto maxResY = readAttribute<float>(h5file, VR_METADATA_MAX_RES_Y);

    descriptor.setMaxResolution(maxResX, maxResY);

    auto h5dataSet = std::unique_ptr<::H5::DataSet, DeleteH5dataSet>(
        new ::H5::DataSet{h5file.openDataSet(VR_METADATA_PATH)},
            DeleteH5dataSet{});

    return std::unique_ptr<VRMetadata>(new VRMetadata{dataset,
        descriptor, std::move(h5dataSet)});
}


//! Create the HDF5 DataSet.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
\param descriptor
    The descriptor of this layer.

\return
    The HDF5 Dataset for this variable resolution metadata layer.
*/
std::unique_ptr<::H5::DataSet, VRMetadata::DeleteH5dataSet>
VRMetadata::createH5dataSet(
    const Dataset& dataset,
    const VRMetadataDescriptor& descriptor)
{
    std::array<hsize_t, kRank> fileDims{0, 0};
    const std::array<hsize_t, kRank> kMaxFileDims{H5S_UNLIMITED, H5S_UNLIMITED};
    const ::H5::DataSpace h5fileDataSpace{kRank, fileDims.data(), kMaxFileDims.data()};

    // Create the creation property list.
    const ::H5::DSetCreatPropList h5createPropList{};

    // Use chunk size and compression level from the descriptor.
    const auto compressionLevel = descriptor.getCompressionLevel();
    const auto chunkSize = descriptor.getChunkSize();
    if (chunkSize > 0)
    {
    	std::array<hsize_t, kRank> chunkDims{chunkSize, chunkSize};
        h5createPropList.setChunk(kRank, chunkDims.data());

        if (compressionLevel > 0 && compressionLevel <= kMaxCompressionLevel)
            h5createPropList.setDeflate(compressionLevel);
    }
    else if (compressionLevel > 0)
        throw CompressionNeedsChunkingSet{};

    h5createPropList.setFillTime(H5D_FILL_TIME_ALLOC);

    const auto memDataType = makeDataType();

    // Create the DataSet using the above.
    const auto& h5file = dataset.getH5file();

    const auto h5dataSet = h5file.createDataSet(VR_METADATA_PATH,
        memDataType, h5fileDataSpace, h5createPropList);

    // Create attributes.
    createAttributes(h5dataSet, ::H5::PredType::NATIVE_UINT32,
        {VR_METADATA_MIN_DIMS_X, VR_METADATA_MIN_DIMS_Y, VR_METADATA_MAX_DIMS_X,
        VR_METADATA_MAX_DIMS_Y});
    createAttributes(h5dataSet, ::H5::PredType::NATIVE_FLOAT,
        {VR_METADATA_MIN_RES_X, VR_METADATA_MIN_RES_Y, VR_METADATA_MAX_RES_X,
        VR_METADATA_MAX_RES_Y});

    // Set initial min/max values.
    BAG::writeAttributes(h5dataSet, ::H5::PredType::NATIVE_UINT32,
        std::numeric_limits<uint32_t>::max(), {VR_METADATA_MIN_DIMS_X,
        VR_METADATA_MIN_DIMS_Y});

    BAG::writeAttributes(h5dataSet, ::H5::PredType::NATIVE_UINT32,
        std::numeric_limits<uint32_t>::lowest(), {VR_METADATA_MAX_DIMS_X,
        VR_METADATA_MAX_DIMS_Y});

    BAG::writeAttributes(h5dataSet, ::H5::PredType::NATIVE_FLOAT,
        std::numeric_limits<float>::max(), {VR_METADATA_MIN_RES_X,
        VR_METADATA_MIN_RES_Y});

    BAG::writeAttributes(h5dataSet, ::H5::PredType::NATIVE_FLOAT,
        std::numeric_limits<float>::lowest(), {VR_METADATA_MAX_RES_X,
        VR_METADATA_MAX_RES_Y});

    return std::unique_ptr<::H5::DataSet, DeleteH5dataSet>(
        new ::H5::DataSet{h5dataSet}, DeleteH5dataSet{});
}

//! \copydoc Layer::read
UInt8Array VRMetadata::readProxy(
    uint32_t rowStart,
    uint32_t columnStart,
    uint32_t rowEnd,
    uint32_t columnEnd) const
{
    const auto* descriptor = dynamic_cast<const VRMetadataDescriptor*>(
        &this->getDescriptor());
    if (!descriptor)
        throw UnexpectedLayerDescriptorType{};

    // Query the file for the specified rows and columns.
    const auto rows = (rowEnd - rowStart) + 1;
    const auto columns = (columnEnd - columnStart) + 1;
    const std::array<hsize_t, kRank> count{rows, columns};
    const std::array<hsize_t, kRank> offset{rowStart, columnStart};

    const auto fileDataSpace = m_pH5dataSet->getSpace();
    fileDataSpace.selectHyperslab(H5S_SELECT_SET, count.data(), offset.data());

    const auto bufferSize = descriptor->getReadBufferSize(rows, columns);
    UInt8Array buffer{bufferSize};

    const ::H5::DataSpace memDataSpace{kRank, count.data(), count.data()};

    const auto memDataType = makeDataType();

    m_pH5dataSet->read(buffer.data(), memDataType, memDataSpace, fileDataSpace);

    return buffer;
}

//! \copydoc Layer::writeAttributes
void VRMetadata::writeAttributesProxy() const
{
    auto* descriptor = dynamic_cast<const VRMetadataDescriptor*>(
        &this->getDescriptor());
    if (!descriptor)
        throw UnexpectedLayerDescriptorType{};

    // Write the attributes from the layer descriptor.
    // min X,Y dimensions
    const auto minDims = descriptor->getMinDimensions();
    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_UINT32,
        std::get<0>(minDims), VR_METADATA_MIN_DIMS_X);

    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_UINT32,
        std::get<1>(minDims), VR_METADATA_MIN_DIMS_Y);

    // max X,Y dimensions
    const auto maxDims = descriptor->getMaxDimensions();
    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_UINT32,
        std::get<0>(maxDims), VR_METADATA_MAX_DIMS_X);

    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_UINT32,
        std::get<1>(maxDims), VR_METADATA_MAX_DIMS_Y);

    // min X,Y resolution
    const auto minRes = descriptor->getMinResolution();
    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_FLOAT,
        std::get<0>(minRes), VR_METADATA_MIN_RES_X);

    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_FLOAT,
        std::get<1>(minRes), VR_METADATA_MIN_RES_Y);

    // max X,Y resolution
    const auto maxRes = descriptor->getMaxResolution();
    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_FLOAT,
        std::get<0>(maxRes), VR_METADATA_MAX_RES_X);

    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_FLOAT,
        std::get<1>(maxRes), VR_METADATA_MAX_RES_Y);
}

//! \copydoc Layer::write
void VRMetadata::writeProxy(
    uint32_t rowStart,
    uint32_t columnStart,
    uint32_t rowEnd,
    uint32_t columnEnd,
    const uint8_t* buffer)
{
    auto* descriptor = dynamic_cast<VRMetadataDescriptor*>(
        &this->getDescriptor());
    if (!descriptor)
        throw UnexpectedLayerDescriptorType{};

    const auto rows = (rowEnd - rowStart) + 1;
    const auto columns = (columnEnd - columnStart) + 1;
    const std::array<hsize_t, kRank> count{rows, columns};
    const std::array<hsize_t, kRank> offset{rowStart, columnStart};
    const ::H5::DataSpace memDataSpace{kRank, count.data(), count.data()};

    ::H5::DataSpace fileDataSpace = m_pH5dataSet->getSpace();

    // Expand the file data space if needed.
    std::array<hsize_t, kRank> fileDims{};
    std::array<hsize_t, kRank> maxFileDims{};
    fileDataSpace.getSimpleExtentDims(fileDims.data(), maxFileDims.data());

    if ((fileDims[0] < (rowEnd + 1)) ||
        (fileDims[1] < (columnEnd + 1)))
    {
        const std::array<hsize_t, kRank> newDims{
            std::max<hsize_t>(fileDims[0], rowEnd + 1),
            std::max<hsize_t>(fileDims[1], columnEnd + 1)};

        m_pH5dataSet->extend(newDims.data());

        fileDataSpace = m_pH5dataSet->getSpace();

        // Update the dataset's dimensions.
        if (this->getDataset().expired())
            throw DatasetNotFound{};

        auto pDataset = this->getDataset().lock();
        pDataset->getDescriptor().setDims(static_cast<uint32_t>(newDims[0]),
            static_cast<uint32_t>(newDims[1]));
    }

    fileDataSpace.selectHyperslab(H5S_SELECT_SET, count.data(), offset.data());

    const auto memDataType = makeDataType();

    m_pH5dataSet->write(buffer, memDataType, memDataSpace, fileDataSpace);

    // Update any attributes that are affected by the data being written.
    // Get the current min/max from descriptor.
    uint32_t minDimX = 0, minDimY = 0;
    std::tie(minDimX, minDimY) = descriptor->getMinDimensions();

    uint32_t maxDimX = 0, maxDimY = 0;
    std::tie(maxDimX, maxDimY) = descriptor->getMaxDimensions();

    float minResX = 0.f, minResY = 0.f;
    std::tie(minResX, minResY) = descriptor->getMinResolution();

    float maxResX = 0.f, maxResY = 0.f;
    std::tie(maxResX, maxResY) = descriptor->getMaxResolution();

    // Update the min/max from new data.
    const auto* items = reinterpret_cast<const VRMetadataItem*>(buffer);

    auto* item = items;
    const auto end = items + rows * columns;

    for (; item != end; ++item)
    {
        minDimX = item->dimensions_x < minDimX ? item->dimensions_x : minDimX;
        minDimY = item->dimensions_y < minDimY ? item->dimensions_y : minDimY;

        maxDimX = item->dimensions_x > maxDimX ? item->dimensions_x : maxDimX;
        maxDimY = item->dimensions_y > maxDimY ? item->dimensions_y : maxDimY;

        minResX = item->resolution_x < minResX ? item->resolution_x : minResX;
        minResY = item->resolution_y < minResY ? item->resolution_y : minResY;

        maxResX = item->resolution_x > minResX ? item->resolution_x : maxResX;
        maxResY = item->resolution_y > minResY ? item->resolution_y : maxResY;
    }

    descriptor->setMinDimensions(minDimX, minDimY);
    descriptor->setMaxDimensions(maxDimX, maxDimY);
    descriptor->setMinResolution(minResX, minResY);
    descriptor->setMaxResolution(maxResX, maxResY);
}

void VRMetadata::DeleteH5dataSet::operator()(::H5::DataSet* ptr) noexcept
{
    delete ptr;
}

}   //namespace BAG

