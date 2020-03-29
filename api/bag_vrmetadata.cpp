
#include "bag_hdfhelper.h"
#include "bag_private.h"
#include "bag_vrmetadata.h"
#include "bag_vrmetadatadescriptor.h"

#include <array>
#include <H5Cpp.h>


namespace BAG {

namespace {

::H5::CompType makeDataType()
{
    const ::H5::CompType memDataType{sizeof(BagVRMetadataItem)};

    memDataType.insertMember("index", HOFFSET(BagVRMetadataItem, index),
        ::H5::PredType::NATIVE_UINT32);
    memDataType.insertMember("dimensions_x",
        HOFFSET(BagVRMetadataItem, dimensions_x), ::H5::PredType::NATIVE_UINT32);
    memDataType.insertMember("dimensions_y",
        HOFFSET(BagVRMetadataItem, dimensions_y), ::H5::PredType::NATIVE_UINT32);
    memDataType.insertMember("resolution_x",
        HOFFSET(BagVRMetadataItem, resolution_x), ::H5::PredType::NATIVE_FLOAT);
    memDataType.insertMember("resolution_y",
        HOFFSET(BagVRMetadataItem, resolution_y), ::H5::PredType::NATIVE_FLOAT);
    memDataType.insertMember("sw_corner_x",
        HOFFSET(BagVRMetadataItem, sw_corner_x), ::H5::PredType::NATIVE_FLOAT);
    memDataType.insertMember("sw_corner_y",
        HOFFSET(BagVRMetadataItem, sw_corner_y), ::H5::PredType::NATIVE_FLOAT);

    return memDataType;
}

template<typename T>
T readAttribute(
    const ::H5::H5File& h5file,
    const char* const attributeName)
{
    const auto h5DataSet = h5file.openDataSet(VR_METADATA_PATH);
    const auto attribute = h5DataSet.openAttribute(attributeName);

    T value{};
    attribute.read(attribute.getDataType(), &value);

    return value;
}

}  // namespace

VRMetadata::VRMetadata(
    Dataset& dataset,
    VRMetadataDescriptor& descriptor,
    std::unique_ptr<::H5::DataSet, DeleteH5dataSet> h5dataSet)
    : Layer(dataset, descriptor)
    , m_pH5dataSet(std::move(h5dataSet))
{
}

std::unique_ptr<VRMetadata> VRMetadata::create(
    Dataset& dataset,
    uint64_t chunkSize,
    unsigned int compressionLevel)
{
    auto descriptor = VRMetadataDescriptor::create(dataset, chunkSize,
        compressionLevel);

    auto h5dataSet = VRMetadata::createH5dataSet(dataset, *descriptor);

    return std::unique_ptr<VRMetadata>(new VRMetadata{dataset,
        *descriptor, std::move(h5dataSet)});
}

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


std::unique_ptr<::H5::DataSet, VRMetadata::DeleteH5dataSet>
VRMetadata::createH5dataSet(
    const Dataset& dataset,
    const VRMetadataDescriptor& descriptor)
{
    std::array<hsize_t, RANK> fileDims{0, 0};
    const std::array<uint64_t, RANK> kMaxFileDims{H5S_UNLIMITED, H5S_UNLIMITED};
    const ::H5::DataSpace h5fileDataSpace{RANK, fileDims.data(), kMaxFileDims.data()};

    // Use chunk size and compression level from the descriptor.
    const auto chunkSize = descriptor.getChunkSize();
	std::array<hsize_t, RANK> chunkDims{chunkSize, chunkSize};

    const auto compressionLevel = descriptor.getCompressionLevel();

    // Create the creation property list.
    const ::H5::DSetCreatPropList h5createPropList{};

    if (compressionLevel <= kMaxCompressionLevel)
    {
        h5createPropList.setLayout(H5D_CHUNKED);
        h5createPropList.setChunk(RANK, chunkDims.data());
        h5createPropList.setDeflate(compressionLevel);
    }

    h5createPropList.setFillTime(H5D_FILL_TIME_ALLOC);

    const auto memDataType = makeDataType();

    auto zeroData = std::make_unique<uint8_t[]>(descriptor.getElementSize());
    memset(zeroData.get(), 0, descriptor.getElementSize());
    h5createPropList.setFillValue(memDataType, zeroData.get());

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

    return std::unique_ptr<::H5::DataSet, DeleteH5dataSet>(
        new ::H5::DataSet{h5dataSet}, DeleteH5dataSet{});
}

UInt8Array VRMetadata::readProxy(
    uint32_t rowStart,
    uint32_t columnStart,
    uint32_t rowEnd,
    uint32_t columnEnd) const
{
    if (!dynamic_cast<const VRMetadataDescriptor*>(&this->getDescriptor()))
        throw UnexpectedLayerDescriptorType{};

    // Query the file for the specified rows and columns.
    const auto rows = (rowEnd - rowStart) + 1;
    const auto columns = (columnEnd - columnStart) + 1;
    const std::array<hsize_t, RANK> count{rows, columns};
    const std::array<hsize_t, RANK> offset{rowStart, columnStart};

    const auto fileDataSpace = m_pH5dataSet->getSpace();
    fileDataSpace.selectHyperslab(H5S_SELECT_SET, count.data(), offset.data());

    const auto& descriptor =
        dynamic_cast<const VRMetadataDescriptor&>(this->getDescriptor());

    const auto bufferSize = descriptor.getReadBufferSize(rows, columns);
    UInt8Array buffer{bufferSize};

    const ::H5::DataSpace memDataSpace{RANK, count.data(), count.data()};

    const auto memDataType = makeDataType();

    m_pH5dataSet->read(buffer.get(), memDataType, memDataSpace, fileDataSpace);

    return buffer;
}

void VRMetadata::writeProxy(
    uint32_t rowStart,
    uint32_t columnStart,
    uint32_t rowEnd,
    uint32_t columnEnd,
    const uint8_t* buffer)
{
    if (!dynamic_cast<VRMetadataDescriptor*>(&this->getDescriptor()))
        throw UnexpectedLayerDescriptorType{};

    const auto rows = (rowEnd - rowStart) + 1;
    const auto columns = (columnEnd - columnStart) + 1;
    const std::array<hsize_t, RANK> count{rows, columns};
    const std::array<hsize_t, RANK> offset{rowStart, columnStart};
    const ::H5::DataSpace memDataSpace{RANK, count.data(), count.data()};

    ::H5::DataSpace fileDataSpace = m_pH5dataSet->getSpace();

    // Expand the file data space if needed.
    std::array<hsize_t, RANK> fileDims{};
    std::array<hsize_t, RANK> maxFileDims{};
    fileDataSpace.getSimpleExtentDims(fileDims.data(), maxFileDims.data());

    if ((fileDims[0] < (rowEnd + 1)) ||
        (fileDims[1] < (columnEnd + 1)))
    {
        const std::array<hsize_t, RANK> newDims{
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
}

void VRMetadata::writeAttributesProxy() const
{
    if (!dynamic_cast<const VRMetadataDescriptor*>(&this->getDescriptor()))
        throw UnexpectedLayerDescriptorType{};

    const auto& descriptor =
        dynamic_cast<const VRMetadataDescriptor&>(this->getDescriptor());

    // Write the attributes from the layer descriptor.
    // min X,Y dimensions
    const auto minDims = descriptor.getMinDimensions();
    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_UINT32,
        std::get<0>(minDims), VR_METADATA_MIN_DIMS_X);

    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_UINT32,
        std::get<1>(minDims), VR_METADATA_MIN_DIMS_Y);

    // max X,Y dimensions
    const auto maxDims = descriptor.getMaxDimensions();
    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_UINT32,
        std::get<0>(maxDims), VR_METADATA_MAX_DIMS_X);

    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_UINT32,
        std::get<1>(maxDims), VR_METADATA_MAX_DIMS_Y);

    // min X,Y resolution
    const auto minRes = descriptor.getMinResolution();
    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_FLOAT,
        std::get<0>(minRes), VR_METADATA_MIN_RES_X);

    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_FLOAT,
        std::get<1>(minRes), VR_METADATA_MIN_RES_Y);

    // max X,Y resolution
    const auto maxRes = descriptor.getMaxResolution();
    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_FLOAT,
        std::get<0>(maxRes), VR_METADATA_MAX_RES_X);

    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_FLOAT,
        std::get<1>(maxRes), VR_METADATA_MAX_RES_Y);
}

void VRMetadata::DeleteH5dataSet::operator()(::H5::DataSet* ptr) noexcept
{
    delete ptr;
}

}   //namespace BAG

