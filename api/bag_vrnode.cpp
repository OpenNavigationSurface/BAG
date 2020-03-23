
#include "bag_hdfhelper.h"
#include "bag_private.h"
#include "bag_vrnode.h"
#include "bag_vrnodedescriptor.h"

#include <array>
#include <H5Cpp.h>


namespace BAG {

namespace {

::H5::CompType makeDataType()
{
    const ::H5::CompType memDataType{sizeof(BagVRNodeItem)};

    memDataType.insertMember("hyp_strength",
        HOFFSET(BagVRNodeItem, hyp_strength), ::H5::PredType::NATIVE_FLOAT);
    memDataType.insertMember("num_hypotheses",
        HOFFSET(BagVRNodeItem, num_hypotheses), ::H5::PredType::NATIVE_UINT32);
    memDataType.insertMember("n_samples", HOFFSET(BagVRNodeItem, n_samples),
        ::H5::PredType::NATIVE_UINT32);

    return memDataType;
}

template<typename T>
T readAttribute(
    const ::H5::H5File& h5file,
    const char* const attributeName)
{
    const auto h5DataSet = h5file.openDataSet(VR_NODE_PATH);
    const auto attribute = h5DataSet.openAttribute(attributeName);

    T value{};
    attribute.read(attribute.getDataType(), &value);

    return value;
}

}  // namespace

VRNode::VRNode(
    Dataset& dataset,
    VRNodeDescriptor& descriptor,
    std::unique_ptr<::H5::DataSet, DeleteH5dataSet> h5dataSet)
    : Layer(dataset, descriptor)
    , m_pH5dataSet(std::move(h5dataSet))
{
}

std::unique_ptr<VRNode> VRNode::create(
    Dataset& dataset,
    uint64_t chunkSize,
    unsigned int compressionLevel)
{
    auto descriptor = VRNodeDescriptor::create(dataset, chunkSize,
        compressionLevel);

    auto h5dataSet = VRNode::createH5dataSet(dataset, *descriptor);

    return std::unique_ptr<VRNode>(new VRNode{dataset,
        *descriptor, std::move(h5dataSet)});
}

std::unique_ptr<VRNode> VRNode::open(
    Dataset& dataset,
    VRNodeDescriptor& descriptor)
{
    auto& h5file = dataset.getH5file();

    // Read the attribute values from the file and set in the descriptor.
    // min/max hyp strength
    const auto minHypStrength = readAttribute<float>(h5file,
        VR_NODE_MIN_HYP_STRENGTH);
    const auto maxHypStrength = readAttribute<float>(h5file,
        VR_NODE_MAX_HYP_STRENGTH);

    descriptor.setMinMaxHypStrength(minHypStrength, maxHypStrength);

    // min/max num hypotheses
    const auto minNumHypotheses = readAttribute<uint32_t>(h5file,
        VR_NODE_MIN_NUM_HYPOTHESES);
    const auto maxNumHypotheses = readAttribute<uint32_t>(h5file,
        VR_NODE_MAX_NUM_HYPOTHESES);

    descriptor.setMinMaxNumHypotheses(minNumHypotheses, maxNumHypotheses);

    // min/max n samples
    const auto minNSamples = readAttribute<uint32_t>(h5file,
        VR_NODE_MIN_N_SAMPLES);
    const auto maxNSamples = readAttribute<uint32_t>(h5file,
        VR_NODE_MAX_N_SAMPLES);

    descriptor.setMinMaxNSamples(minNSamples, maxNSamples);

    auto h5dataSet = std::unique_ptr<::H5::DataSet, DeleteH5dataSet>(
        new ::H5::DataSet{h5file.openDataSet(VR_REFINEMENT_PATH)},
            DeleteH5dataSet{});

    return std::unique_ptr<VRNode>(new VRNode{dataset,
        descriptor, std::move(h5dataSet)});
}


std::unique_ptr<::H5::DataSet, VRNode::DeleteH5dataSet>
VRNode::createH5dataSet(
    const Dataset& dataset,
    const VRNodeDescriptor& descriptor)
{
    const hsize_t fileLength = 0;
    const hsize_t kMaxFileLength = H5S_UNLIMITED;
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

    auto zeroData = std::make_unique<UInt8Array>(descriptor.getElementSize());
    memset(zeroData->get(), 0, descriptor.getElementSize());
    h5createPropList.setFillValue(memDataType, zeroData.get());

    // Create the DataSet using the above.
    const auto& h5file = dataset.getH5file();

    const auto h5dataSet = h5file.createDataSet(VR_NODE_PATH,
        memDataType, h5fileDataSpace, h5createPropList);

    // Create attributes.
    createAttributes(h5dataSet, ::H5::PredType::NATIVE_FLOAT,
        {VR_NODE_MIN_HYP_STRENGTH, VR_NODE_MAX_HYP_STRENGTH});

    createAttributes(h5dataSet, ::H5::PredType::NATIVE_UINT32,
        {VR_NODE_MIN_NUM_HYPOTHESES, VR_NODE_MAX_NUM_HYPOTHESES,
        VR_NODE_MIN_N_SAMPLES, VR_NODE_MAX_N_SAMPLES});

    return std::unique_ptr<::H5::DataSet, DeleteH5dataSet>(
        new ::H5::DataSet{h5dataSet}, DeleteH5dataSet{});
}

//! Ignore rows since the data is 1 dimensional.
std::unique_ptr<UInt8Array> VRNode::readProxy(
    uint32_t /*rowStart*/,
    uint32_t columnStart,
    uint32_t /*rowEnd*/,
    uint32_t columnEnd) const
{
    if (!dynamic_cast<const VRNodeDescriptor*>(&this->getDescriptor()))
        throw UnexpectedLayerDescriptorType{};

    // Query the file for the specified rows and columns.
    const hsize_t columns = (columnEnd - columnStart) + 1;
    const hsize_t offset = columnStart;

    const auto fileDataSpace = m_pH5dataSet->getSpace();
    fileDataSpace.selectHyperslab(H5S_SELECT_SET, &columns, &offset);

    const auto& descriptor =
        dynamic_cast<const VRNodeDescriptor&>(this->getDescriptor());

    const auto bufferSize = descriptor.getReadBufferSize(1,
        static_cast<uint32_t>(columns));
    auto buffer = std::make_unique<UInt8Array>(bufferSize);

    const ::H5::DataSpace memDataSpace{1, &columns, &columns};

    const auto memDataType = makeDataType();

    m_pH5dataSet->read(buffer->get(), memDataType, memDataSpace, fileDataSpace);

    return buffer;
}

//! Ignore rows since the data is 1 dimensional.
void VRNode::writeProxy(
    uint32_t /*rowStart*/,
    uint32_t columnStart,
    uint32_t /*rowEnd*/,
    uint32_t columnEnd,
    const uint8_t* buffer)
{
    if (!dynamic_cast<VRNodeDescriptor*>(&this->getDescriptor()))
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
}

void VRNode::writeAttributesProxy() const
{
    if (!dynamic_cast<const VRNodeDescriptor*>(&this->getDescriptor()))
        throw UnexpectedLayerDescriptorType{};

    const auto& descriptor =
        dynamic_cast<const VRNodeDescriptor&>(this->getDescriptor());

    // Write the attributes from the layer descriptor.
    // min/max hyp strength
    const auto minMaxHypStrength = descriptor.getMinMaxHypStrength();
    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_FLOAT,
        std::get<0>(minMaxHypStrength), VR_NODE_MIN_HYP_STRENGTH);

    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_FLOAT,
        std::get<1>(minMaxHypStrength), VR_NODE_MAX_HYP_STRENGTH);

    // min/max num hypotheses
    const auto minMaxNumHypotheses = descriptor.getMinMaxNumHypotheses();
    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_UINT32,
        std::get<0>(minMaxNumHypotheses), VR_NODE_MIN_NUM_HYPOTHESES);

    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_UINT32,
        std::get<1>(minMaxNumHypotheses), VR_NODE_MAX_NUM_HYPOTHESES);

    // min/max n samples
    const auto minMaxNSamples = descriptor.getMinMaxNSamples();
    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_UINT32,
        std::get<0>(minMaxNSamples), VR_NODE_MIN_N_SAMPLES);

    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_UINT32,
        std::get<1>(minMaxNSamples), VR_NODE_MAX_N_SAMPLES);
}

void VRNode::DeleteH5dataSet::operator()(::H5::DataSet* ptr) noexcept
{
    delete ptr;
}

}   //namespace BAG

