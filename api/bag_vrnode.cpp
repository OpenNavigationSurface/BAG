
#include "bag_hdfhelper.h"
#include "bag_private.h"
#include "bag_vrnode.h"
#include "bag_vrnodedescriptor.h"

#include <array>
#include <cstring>  //memset
#include <H5Cpp.h>


namespace BAG {

namespace {

//! Create an HDF5 CompType for the variable resolution node.
/*!
\return
    An HDF5 CompType for the variable resolution node.
*/
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

//! Read an attribute from an HDF5 DataSet.
/*!
\param h5file
    The HDF5 file.
\param name
    The name of the attribute.

\return
    The value in the specified attribute.
*/
template<typename T>
T readAttribute(
    const ::H5::H5File& h5file,
    const char* const name)
{
    const auto h5DataSet = h5file.openDataSet(VR_NODE_PATH);
    const auto attribute = h5DataSet.openAttribute(name);

    T value{};
    attribute.read(attribute.getDataType(), &value);

    return value;
}

}  // namespace

//! Constructor.
/*!
\param dataset
    The BAG Dataset that this layer belongs to.
\param descriptor
    The descriptor of this layer.
\param pH5dataSet
    The HDF5 DataSet this class wraps.
*/
VRNode::VRNode(
    Dataset& dataset,
    VRNodeDescriptor& descriptor,
    std::unique_ptr<::H5::DataSet, DeleteH5dataSet> pH5dataSet)
    : Layer(dataset, descriptor)
    , m_pH5dataSet(std::move(pH5dataSet))
{
}

//! Create a variable resolution node.
/*!
\param dataset
    The BAG Dataset that this layer belongs to.
\param chunkSize
    The chunk size in the HDF5 DataSet.
\param compressionLevel
    The compression level in the HDF5 DataSet.

\return
    The new variable resolution node.
*/
std::unique_ptr<VRNode> VRNode::create(
    Dataset& dataset,
    uint64_t chunkSize,
    int compressionLevel)
{
    auto descriptor = VRNodeDescriptor::create(dataset, chunkSize,
        compressionLevel);

    auto h5dataSet = VRNode::createH5dataSet(dataset, *descriptor);

    return std::unique_ptr<VRNode>(new VRNode{dataset,
        *descriptor, std::move(h5dataSet)});
}

//! Open an existing variable resolution node.
/*!
\param dataset
    The BAG Dataset that this layer belongs to.
\param descriptor
    The descriptor of this layer.

\return
    The specified variable resolution node.
*/
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


//! Create the HDF5 DataSet.
/*!
\param dataset
    The BAG Dataset that this layer belongs to.
\param descriptor
    The descriptor of this layer.

\return
    The new HDF5 DataSet.
*/
std::unique_ptr<::H5::DataSet, VRNode::DeleteH5dataSet>
VRNode::createH5dataSet(
    const Dataset& dataset,
    const VRNodeDescriptor& descriptor)
{
    const hsize_t fileLength = 0;
    const hsize_t kMaxFileLength = H5S_UNLIMITED;
    const ::H5::DataSpace h5fileDataSpace{1, &fileLength, &kMaxFileLength};

    // Create the creation property list.
    const ::H5::DSetCreatPropList h5createPropList{};

    // Use chunk size and compression level from the descriptor.
    const hsize_t chunkSize = descriptor.getChunkSize();
    const auto compressionLevel = descriptor.getCompressionLevel();
    if (chunkSize > 0)
    {
        h5createPropList.setChunk(1, &chunkSize);

        if (compressionLevel > 0 && compressionLevel <= kMaxCompressionLevel)
            h5createPropList.setDeflate(compressionLevel);
    }
    else if (compressionLevel > 0)
        throw CompressionNeedsChunkingSet{};

    h5createPropList.setFillTime(H5D_FILL_TIME_ALLOC);

    const auto memDataType = makeDataType();

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

    // Set initial min/max values.
    writeAttribute(h5dataSet, ::H5::PredType::NATIVE_FLOAT,
        std::numeric_limits<float>::max(), VR_NODE_MIN_HYP_STRENGTH);
    writeAttribute(h5dataSet, ::H5::PredType::NATIVE_FLOAT,
        std::numeric_limits<float>::lowest(), VR_NODE_MAX_HYP_STRENGTH);

    BAG::writeAttributes(h5dataSet, ::H5::PredType::NATIVE_UINT32,
        std::numeric_limits<uint32_t>::max(), {VR_NODE_MIN_NUM_HYPOTHESES,
        VR_NODE_MIN_N_SAMPLES});
    BAG::writeAttributes(h5dataSet, ::H5::PredType::NATIVE_UINT32,
        std::numeric_limits<uint32_t>::lowest(), {VR_NODE_MAX_NUM_HYPOTHESES,
        VR_NODE_MAX_N_SAMPLES});

    return std::unique_ptr<::H5::DataSet, DeleteH5dataSet>(
        new ::H5::DataSet{h5dataSet}, DeleteH5dataSet{});
}

//! \copydoc Layer::read
//! The rowStart and rowEnd are ignored since the data is 1 dimensional.
UInt8Array VRNode::readProxy(
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
    UInt8Array buffer{bufferSize};

    const ::H5::DataSpace memDataSpace{1, &columns, &columns};

    const auto memDataType = makeDataType();

    m_pH5dataSet->read(buffer.data(), memDataType, memDataSpace, fileDataSpace);

    return buffer;
}

//! \copydoc Layer::writeAttributes
void VRNode::writeAttributesProxy() const
{
    const auto* descriptor =
        dynamic_cast<const VRNodeDescriptor*>(&this->getDescriptor());
    if (!descriptor)
        throw UnexpectedLayerDescriptorType{};

    // Write the attributes from the layer descriptor.
    // min/max hyp strength
    const auto minMaxHypStrength = descriptor->getMinMaxHypStrength();
    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_FLOAT,
        std::get<0>(minMaxHypStrength), VR_NODE_MIN_HYP_STRENGTH);

    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_FLOAT,
        std::get<1>(minMaxHypStrength), VR_NODE_MAX_HYP_STRENGTH);

    // min/max num hypotheses
    const auto minMaxNumHypotheses = descriptor->getMinMaxNumHypotheses();
    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_UINT32,
        std::get<0>(minMaxNumHypotheses), VR_NODE_MIN_NUM_HYPOTHESES);

    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_UINT32,
        std::get<1>(minMaxNumHypotheses), VR_NODE_MAX_NUM_HYPOTHESES);

    // min/max n samples
    const auto minMaxNSamples = descriptor->getMinMaxNSamples();
    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_UINT32,
        std::get<0>(minMaxNSamples), VR_NODE_MIN_N_SAMPLES);

    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_UINT32,
        std::get<1>(minMaxNSamples), VR_NODE_MAX_N_SAMPLES);
}

//! \copydoc Layer::write
//! Ignore rows since the data is 1 dimensional.
void VRNode::writeProxy(
    uint32_t /*rowStart*/,
    uint32_t columnStart,
    uint32_t /*rowEnd*/,
    uint32_t columnEnd,
    const uint8_t* buffer)
{
    auto* descriptor = dynamic_cast<VRNodeDescriptor*>(&this->getDescriptor());
    if (!descriptor)
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
    // Get the current min/max from descriptor.
    float minHypStr = 0.f, maxHypStr = 0.f;
    std::tie(minHypStr, maxHypStr) = descriptor->getMinMaxHypStrength();

    uint32_t minNumHyp = 0, maxNumHyp = 0;
    std::tie(minNumHyp, maxNumHyp) = descriptor->getMinMaxNumHypotheses();

    uint32_t minNSamples = 0, maxNSamples = 0;
    std::tie(minNSamples, maxNSamples) = descriptor->getMinMaxNSamples();

    // Update the min/max from new data.
    const auto* items = reinterpret_cast<const BagVRNodeItem*>(buffer);

    auto* item = items;
    const auto end = items + columns;

    for (; item != end; ++item)
    {
        minHypStr = item->hyp_strength < minHypStr ? item->hyp_strength : minHypStr;
        maxHypStr = item->hyp_strength > maxHypStr ? item->hyp_strength : maxHypStr;

        minNumHyp = item->num_hypotheses < minNumHyp ? item->num_hypotheses : minNumHyp;
        maxNumHyp = item->num_hypotheses > maxNumHyp ? item->num_hypotheses : maxNumHyp;

        minNSamples = item->n_samples < minNSamples ? item->n_samples : minNSamples;
        maxNSamples = item->n_samples > maxNSamples ? item->n_samples : maxNSamples;
    }

    descriptor->setMinMaxHypStrength(minHypStr, maxHypStr);
    descriptor->setMinMaxNumHypotheses(minNumHyp, maxNumHyp);
    descriptor->setMinMaxNSamples(minNSamples, maxNSamples);
}

void VRNode::DeleteH5dataSet::operator()(::H5::DataSet* ptr) noexcept
{
    delete ptr;
}

}   //namespace BAG

