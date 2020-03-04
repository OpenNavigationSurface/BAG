
#include "bag_hdfhelper.h"
#include "bag_private.h"
#include "bag_vrrefinement.h"
#include "bag_vrrefinementdescriptor.h"

#include <array>
#include <H5Cpp.h>


namespace BAG {

namespace {

::H5::CompType makeDataType()
{
    const ::H5::CompType memDataType{sizeof(BagVRRefinementItem)};

    memDataType.insertMember("depth", HOFFSET(BagVRRefinementItem, depth),
        ::H5::PredType::NATIVE_FLOAT);
    memDataType.insertMember("depth_uncrt", HOFFSET(BagVRRefinementItem, depth_uncrt),
        ::H5::PredType::NATIVE_FLOAT);

    return memDataType;
}

template<typename T>
T readAttribute(
    const ::H5::H5File& h5file,
    const char* const attributeName)
{
    const auto h5DataSet = h5file.openDataSet(VR_REFINEMENT_PATH);
    const auto attribute = h5DataSet.openAttribute(attributeName);

    T value{};
    attribute.read(attribute.getDataType(), &value);

    return value;
}

}  // namespace

VRRefinement::VRRefinement(
    Dataset& dataset,
    VRRefinementDescriptor& descriptor,
    std::unique_ptr<::H5::DataSet, DeleteH5dataSet> h5dataSet)
    : Layer(dataset, descriptor)
    , m_pH5dataSet(std::move(h5dataSet))
{
}

std::unique_ptr<VRRefinement> VRRefinement::create(
    Dataset& dataset,
    uint64_t chunkSize,
    unsigned int compressionLevel)
{
    auto descriptor = VRRefinementDescriptor::create(dataset, chunkSize,
        compressionLevel);

    auto h5dataSet = VRRefinement::createH5dataSet(dataset, *descriptor);

    return std::unique_ptr<VRRefinement>(new VRRefinement{dataset,
        *descriptor, std::move(h5dataSet)});
}

std::unique_ptr<VRRefinement> VRRefinement::open(
    Dataset& dataset,
    VRRefinementDescriptor& descriptor)
{
    auto& h5file = dataset.getH5file();

    // Read the attribute values from the file and set in the descriptor.
    const auto minDepth = readAttribute<float>(h5file, VR_REFINEMENT_MIN_DEPTH);
    const auto maxDepth = readAttribute<float>(h5file, VR_REFINEMENT_MAX_DEPTH);

    descriptor.setMinMaxDepth(minDepth, maxDepth);

    const auto minUncertainty = readAttribute<float>(h5file,
        VR_REFINEMENT_MIN_UNCERTAINTY);
    const auto maxUncertainty = readAttribute<float>(h5file,
        VR_REFINEMENT_MAX_UNCERTAINTY);

    descriptor.setMinMaxUncertainty(minUncertainty, maxUncertainty);

    auto h5dataSet = std::unique_ptr<::H5::DataSet, DeleteH5dataSet>(
        new ::H5::DataSet{h5file.openDataSet(VR_REFINEMENT_PATH)},
            DeleteH5dataSet{});

    return std::unique_ptr<VRRefinement>(new VRRefinement{dataset,
        descriptor, std::move(h5dataSet)});
}


std::unique_ptr<::H5::DataSet, VRRefinement::DeleteH5dataSet>
VRRefinement::createH5dataSet(
    const Dataset& dataset,
    const VRRefinementDescriptor& descriptor)
{
    constexpr hsize_t fileLength = 0;
    constexpr hsize_t kMaxFileLength = H5S_UNLIMITED;
    const ::H5::DataSpace h5fileDataSpace{1, &fileLength, &kMaxFileLength};

    // Use chunk size and compression level from the descriptor.
    const auto chunkSize = descriptor.getChunkSize();
    const auto compressionLevel = descriptor.getCompressionLevel();

    // Create the creation property list.
    const ::H5::DSetCreatPropList h5createPropList{};

    if (compressionLevel <= kMaxCompressionLevel)
    {
        h5createPropList.setLayout(H5D_CHUNKED);
        h5createPropList.setChunk(1, &chunkSize);
        h5createPropList.setDeflate(compressionLevel);
    }

    h5createPropList.setFillTime(H5D_FILL_TIME_ALLOC);

    const auto memDataType = makeDataType();

    auto zeroData = std::make_unique<uint8_t[]>(descriptor.getElementSize());
    memset(zeroData.get(), 0, descriptor.getElementSize());
    h5createPropList.setFillValue(memDataType, zeroData.get());

    // Create the DataSet using the above.
    const auto& h5file = dataset.getH5file();

    const auto h5dataSet = h5file.createDataSet(VR_REFINEMENT_PATH,
        memDataType, h5fileDataSpace, h5createPropList);

    // Create attributes.
    createAttributes(h5dataSet, ::H5::PredType::NATIVE_FLOAT,
        {VR_REFINEMENT_MIN_DEPTH, VR_REFINEMENT_MAX_DEPTH,
        VR_REFINEMENT_MIN_UNCERTAINTY, VR_REFINEMENT_MAX_UNCERTAINTY});

    // Set initial min/max values.
    BAG::writeAttributes(h5dataSet, ::H5::PredType::NATIVE_FLOAT,
        std::numeric_limits<float>::max(), {VR_REFINEMENT_MIN_DEPTH,
        VR_REFINEMENT_MIN_UNCERTAINTY});
    BAG::writeAttributes(h5dataSet, ::H5::PredType::NATIVE_FLOAT,
        std::numeric_limits<float>::lowest(), {VR_REFINEMENT_MAX_DEPTH,
        VR_REFINEMENT_MAX_UNCERTAINTY});

    return std::unique_ptr<::H5::DataSet, DeleteH5dataSet>(
        new ::H5::DataSet{h5dataSet}, DeleteH5dataSet{});
}

//! Ignore rows since the data is 1 dimensional.
std::unique_ptr<uint8_t[]> VRRefinement::readProxy(
    uint32_t /*rowStart*/,
    uint32_t columnStart,
    uint32_t /*rowEnd*/,
    uint32_t columnEnd) const
{
    if (!dynamic_cast<const VRRefinementDescriptor*>(&this->getDescriptor()))
        throw UnexpectedLayerDescriptorType{};

    // Query the file for the specified rows and columns.
    const hsize_t columns = (columnEnd - columnStart) + 1;
    const hsize_t offset = columnStart;

    const auto fileDataSpace = m_pH5dataSet->getSpace();
    fileDataSpace.selectHyperslab(H5S_SELECT_SET, &columns, &offset);

    const auto& descriptor =
        dynamic_cast<const VRRefinementDescriptor&>(this->getDescriptor());

    const auto bufferSize = descriptor.getReadBufferSize(1,
        static_cast<uint32_t>(columns));
    auto buffer = std::make_unique<uint8_t[]>(bufferSize);

    const ::H5::DataSpace memDataSpace{1, &columns, &columns};

    const auto memDataType = makeDataType();

    m_pH5dataSet->read(buffer.get(), memDataType, memDataSpace, fileDataSpace);

    return buffer;
}

//! Ignore rows since the data is 1 dimensional.
void VRRefinement::writeProxy(
    uint32_t /*rowStart*/,
    uint32_t columnStart,
    uint32_t /*rowEnd*/,
    uint32_t columnEnd,
    const uint8_t* buffer)
{
    if (!dynamic_cast<VRRefinementDescriptor*>(&this->getDescriptor()))
        throw UnexpectedLayerDescriptorType{};

    const hsize_t columns = (columnEnd - columnStart) + 1;
    const hsize_t offset = columnStart;
    const ::H5::DataSpace memDataSpace{1, &columns, &columns};

    // Expand the file data space if needed.
    std::array<hsize_t, H5S_MAX_RANK> fileLength{};
    std::array<hsize_t, H5S_MAX_RANK> maxFileLength{};

    ::H5::DataSpace fileDataSpace = m_pH5dataSet->getSpace();
    const int numDims = fileDataSpace.getSimpleExtentDims(fileLength.data(),
        maxFileLength.data());
    if (numDims != 1)
        throw InvalidVRRefinementDimensions{};

    if (fileLength[0] < (columnEnd + 1))
    {
        const auto newMaxLength = std::max<hsize_t>(fileLength[0], columnEnd + 1);

        m_pH5dataSet->extend(&newMaxLength);

        fileDataSpace = m_pH5dataSet->getSpace();

        // Update the dataset's dimensions.
        if (this->getDataset().expired())
            throw DatasetNotFound{};

        auto pDataset = this->getDataset().lock();
        pDataset->getDescriptor().setDims(1, static_cast<uint32_t>(newMaxLength));
    }

    fileDataSpace.selectHyperslab(H5S_SELECT_SET, &columns, &offset);

    const auto memDataType = makeDataType();

    m_pH5dataSet->write(buffer, memDataType, memDataSpace, fileDataSpace);

    // Update min/max attributes
    auto& descriptor =
        dynamic_cast<VRRefinementDescriptor&>(this->getDescriptor());

    // Get the current min/max from descriptor.
    float minDepth = 0.f, maxDepth = 0.f;
    std::tie(minDepth, maxDepth) = descriptor.getMinMaxDepth();

    float minUncert = 0.f, maxUncert = 0.f;
    std::tie(minUncert, maxUncert) = descriptor.getMinMaxUncertainty();

    // Update the min/max from new data.
    const auto* items = reinterpret_cast<const BagVRRefinementItem*>(buffer);

    auto* item = items;
    const auto end = items + columns;

    for (; item != end; ++item)
    {
        minDepth = item->depth < minDepth ? item->depth : minDepth;
        maxDepth = item->depth > maxDepth ? item->depth : maxDepth;

        minUncert = item->depth_uncrt < minUncert ? item->depth_uncrt : minUncert;
        maxUncert = item->depth_uncrt > maxUncert ? item->depth_uncrt : maxUncert;
    }

    descriptor.setMinMaxDepth(minDepth, maxDepth);
    descriptor.setMinMaxUncertainty(minUncert, maxUncert);
}

void VRRefinement::writeAttributesProxy() const
{
    if (!dynamic_cast<const VRRefinementDescriptor*>(&this->getDescriptor()))
        throw UnexpectedLayerDescriptorType{};

    const auto& descriptor =
        dynamic_cast<const VRRefinementDescriptor&>(this->getDescriptor());

    // Write the attributes from the layer descriptor.
    // min/max depth
    const auto minMaxDepth = descriptor.getMinMaxDepth();
    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_FLOAT,
        std::get<0>(minMaxDepth), VR_REFINEMENT_MIN_DEPTH);

    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_FLOAT,
        std::get<1>(minMaxDepth), VR_REFINEMENT_MAX_DEPTH);

    // min/max uncertainty
    const auto minMaxUncertainty = descriptor.getMinMaxUncertainty();
    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_FLOAT,
        std::get<0>(minMaxUncertainty), VR_REFINEMENT_MIN_UNCERTAINTY);

    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_FLOAT,
        std::get<1>(minMaxUncertainty), VR_REFINEMENT_MAX_UNCERTAINTY);
}

void VRRefinement::DeleteH5dataSet::operator()(::H5::DataSet* ptr) noexcept
{
    delete ptr;
}

}  // namespace BAG

