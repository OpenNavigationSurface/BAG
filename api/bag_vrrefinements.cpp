
#include "bag_hdfhelper.h"
#include "bag_private.h"
#include "bag_vrrefinements.h"
#include "bag_vrrefinementsdescriptor.h"

#include <iostream>
#include <array>
#include <cstring>  //memset
#include <H5Cpp.h>

namespace BAG {

namespace {

//! Create an HDF5 CompType for the variable resolution refinements layer.
/*!
\return
    The HDF5 CompType for the variable resolution refinements layer.
*/
::H5::CompType makeDataType()
{
    const ::H5::CompType memDataType{sizeof(BagVRRefinementsItem)};

    memDataType.insertMember("depth", HOFFSET(BagVRRefinementsItem, depth),
        ::H5::PredType::NATIVE_FLOAT);
    memDataType.insertMember("depth_uncrt", HOFFSET(BagVRRefinementsItem, depth_uncrt),
        ::H5::PredType::NATIVE_FLOAT);

    return memDataType;
}

//! Read an attribute from an HDF5 DataSet.
/*!
\param h5file
    The HDF5 file.
\param name
    The attribute name.
*/
template<typename T>
T readAttribute(
    const ::H5::H5File& h5file,
    const char* const name)
{
    const auto h5DataSet = h5file.openDataSet(VR_REFINEMENT_PATH);
    const auto attribute = h5DataSet.openAttribute(name);

    T value{};
    attribute.read(attribute.getDataType(), &value);

    return value;
}

}  // namespace

//! Retrieve the layer's descriptor. Note: this shadows BAG::Layer.getDescriptor()
/*!
\return
    The layer's descriptor.
    Will never be nullptr.
*/
std::shared_ptr<VRRefinementsDescriptor> VRRefinements::getDescriptor() & noexcept
{
    return std::dynamic_pointer_cast<VRRefinementsDescriptor>(Layer::getDescriptor());
}

//! Retrieve the layer's descriptor. Note: this shadows BAG::Layer.getDescriptor()
/*!
\return
    The layer's descriptor.
    Will never be nullptr.
*/
std::shared_ptr<const VRRefinementsDescriptor> VRRefinements::getDescriptor() const & noexcept
{
    return std::dynamic_pointer_cast<const VRRefinementsDescriptor>(Layer::getDescriptor());
}

//! Constructor.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
\param descriptor
    The descriptor of this layer.
\param h5dataSet
    The HDF5 DataSet that stores this layer.
*/
VRRefinements::VRRefinements(
    Dataset& dataset,
    VRRefinementsDescriptor& descriptor,
    std::unique_ptr<::H5::DataSet, DeleteH5dataSet> h5dataSet)
    : Layer(dataset, descriptor)
    , m_pH5dataSet(std::move(h5dataSet))
{
}

//! Create a new variable resolution refinements layer.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
\param chunkSize
    The chunk size the HDF5 DataSet will use.
\param compressionLevel
    The compression level the HDF5 DataSet will use.

\return
    The new variable resolution refinements layer.
*/
std::unique_ptr<VRRefinements> VRRefinements::create(
    Dataset& dataset,
    uint64_t chunkSize,
    int compressionLevel)
{
    auto descriptor = VRRefinementsDescriptor::create(dataset, chunkSize,
        compressionLevel);

    auto h5dataSet = VRRefinements::createH5dataSet(dataset, *descriptor);

    return std::unique_ptr<VRRefinements>(new VRRefinements{dataset,
        *descriptor, std::move(h5dataSet)});
}

//! Open an existing variable resolution refinements layer.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
\param descriptor
    The descriptor of this layer.

\return
    The existing variable resolution refinements layer.
*/
std::unique_ptr<VRRefinements> VRRefinements::open(
    Dataset& dataset,
    VRRefinementsDescriptor& descriptor)
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

    // We need to know the dimensions of the array on file so that we can update the
    // descriptor for the layer.
    hsize_t dims[2];
    int ndims = h5dataSet->getSpace().getSimpleExtentDims(dims, nullptr);
    if (ndims != 2) {
        // Should be 1D according to BAG spec, but some implementations use a 2D array,
        // so for compatibility's sake, use 2D.
        throw InvalidVRRefinementDimensions{};
    }
    descriptor.setDims(dims[0], dims[1]);
    return std::unique_ptr<VRRefinements>(new VRRefinements{dataset,
        descriptor, std::move(h5dataSet)});
}


//! Create a new HDF5 DataSet for this variable resolution refinements layer.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
\param descriptor
    The descriptor of this layer.

\return
    A new HDF5 DataSet.
*/
std::unique_ptr<::H5::DataSet, DeleteH5dataSet>
VRRefinements::createH5dataSet(
    const Dataset& dataset,
    const VRRefinementsDescriptor& descriptor)
{
    std::array<hsize_t, kRank> fileDims{0, 0};
    const std::array<hsize_t, kRank> kMaxFileDims{H5S_UNLIMITED, H5S_UNLIMITED};
    const ::H5::DataSpace h5fileDataSpace{kRank, fileDims.data(), kMaxFileDims.data()};

    // Create the creation property list.
    const ::H5::DSetCreatPropList h5createPropList{};

    // Use chunk size and compression level from the descriptor.
    const hsize_t chunkSize = descriptor.getChunkSize();
    const auto compressionLevel = descriptor.getCompressionLevel();
    if (chunkSize > 0)
    {
        const std::array<hsize_t, kRank> chunkDims{chunkSize, chunkSize};
        h5createPropList.setChunk(kRank, chunkDims.data());

        if (compressionLevel > 0 && compressionLevel <= kMaxCompressionLevel)
            h5createPropList.setDeflate(compressionLevel);
    }
    else if (compressionLevel > 0)
        throw CompressionNeedsChunkingSet{};
    else
        throw LayerRequiresChunkingSet{};

    h5createPropList.setFillTime(H5D_FILL_TIME_ALLOC);

    const auto memDataType = makeDataType();

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

//! \copydoc Layer::read
//! Ignore rows since the data is 1 dimensional.
UInt8Array VRRefinements::readProxy(
    uint32_t /*rowStart*/,
    uint32_t columnStart,
    uint32_t /*rowEnd*/,
    uint32_t columnEnd) const
{
    auto pDescriptor = std::dynamic_pointer_cast<const VRRefinementsDescriptor>(
        this->getDescriptor());
    if (!pDescriptor)
        throw InvalidLayerDescriptor{};

    // Query the file for the specified rows and columns.
    const hsize_t columns = (columnEnd - columnStart) + 1;
    const hsize_t offset = columnStart;

    const std::array<hsize_t, kRank> sizes{1, columns};
    const std::array<hsize_t, kRank> offsets{0, offset};

    const auto h5fileDataSpace = m_pH5dataSet->getSpace();
    h5fileDataSpace.selectHyperslab(H5S_SELECT_SET, sizes.data(), offsets.data());

    const auto bufferSize = pDescriptor->getReadBufferSize(1, columns);
    UInt8Array buffer{bufferSize};
    
    const ::H5::DataSpace memDataSpace{kRank, sizes.data(), sizes.data()};

    const auto memDataType = makeDataType();

    m_pH5dataSet->read(buffer.data(), memDataType, memDataSpace, h5fileDataSpace);

    return buffer;
}

//! \copydoc Layer::writeAttributes
void VRRefinements::writeAttributesProxy() const
{
    auto pDescriptor =
        std::dynamic_pointer_cast<const VRRefinementsDescriptor>(
            this->getDescriptor());
    if (!pDescriptor)
        throw InvalidLayerDescriptor{};

    // Write the attributes from the layer descriptor.
    // min/max depth
    const auto minMaxDepth = pDescriptor->getMinMaxDepth();
    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_FLOAT,
        std::get<0>(minMaxDepth), VR_REFINEMENT_MIN_DEPTH);

    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_FLOAT,
        std::get<1>(minMaxDepth), VR_REFINEMENT_MAX_DEPTH);

    // min/max uncertainty
    const auto minMaxUncertainty = pDescriptor->getMinMaxUncertainty();
    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_FLOAT,
        std::get<0>(minMaxUncertainty), VR_REFINEMENT_MIN_UNCERTAINTY);

    writeAttribute(*m_pH5dataSet, ::H5::PredType::NATIVE_FLOAT,
        std::get<1>(minMaxUncertainty), VR_REFINEMENT_MAX_UNCERTAINTY);
}

//! \copydoc Layer::write
void VRRefinements::writeProxy(
    uint32_t rowStart,
    uint32_t columnStart,
    uint32_t rowEnd,
    uint32_t columnEnd,
    const uint8_t* buffer)
{
    auto pDescriptor = std::dynamic_pointer_cast<VRRefinementsDescriptor>(
        this->getDescriptor());
    if (!pDescriptor)
        throw InvalidLayerDescriptor{};

    const auto rows = (rowEnd - rowStart) + 1;
    const auto columns = (columnEnd - columnStart) + 1;
    const std::array<hsize_t, kRank> count{rows, columns};
    const std::array<hsize_t, kRank> offset{rowStart, columnStart};
    const ::H5::DataSpace memDataSpace{kRank, count.data(), count.data()};

    ::H5::DataSpace fileDataSpace = m_pH5dataSet->getSpace();

    // Expand the file data space if needed.
    std::array<hsize_t, kRank> fileDims{};
    std::array<hsize_t, kRank> maxFileDims{};

    const int numDims = fileDataSpace.getSimpleExtentDims(fileDims.data(),
                                                          maxFileDims.data());
    if (numDims != kRank) {
        throw InvalidVRRefinementDimensions{};
    }

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

//        auto pDataset = this->getDataset().lock();
        // TODO: Confirm that this is what we want --- this resets the dimensions of the
        // overall BAG, rather than the layer, which means that it's going to set the
        // metadata size of the mandatory layers to 1xN ... which is odd.
//        pDataset->getDescriptor().setDims(1, static_cast<uint32_t>(newMaxLength));
        // So that the read() call checks correctly against the size of the array, rather
        // than the dimensions of the mandatory layer, we need to keep track of the size
        // of the layer in the layer-specific descriptor.
        pDescriptor->setDims(static_cast<uint32_t>(newDims[0]),
                             static_cast<uint32_t>(newDims[1]));
    }

    fileDataSpace.selectHyperslab(H5S_SELECT_SET, count.data(), offset.data());

    const auto memDataType = makeDataType();

    m_pH5dataSet->write(buffer, memDataType, memDataSpace, fileDataSpace);

    // Update min/max attributes
    // Get the current min/max from descriptor.
    float minDepth = 0.f, maxDepth = 0.f;
    std::tie(minDepth, maxDepth) = pDescriptor->getMinMaxDepth();

    float minUncert = 0.f, maxUncert = 0.f;
    std::tie(minUncert, maxUncert) = pDescriptor->getMinMaxUncertainty();

    // Update the min/max from new data.
    const auto* items = reinterpret_cast<const BagVRRefinementsItem*>(buffer);

    auto* item = items;
    const auto end = items + columns;

    for (; item != end; ++item)
    {
        minDepth = item->depth < minDepth ? item->depth : minDepth;
        maxDepth = item->depth > maxDepth ? item->depth : maxDepth;

        minUncert = item->depth_uncrt < minUncert ? item->depth_uncrt : minUncert;
        maxUncert = item->depth_uncrt > maxUncert ? item->depth_uncrt : maxUncert;
    }

    pDescriptor->setMinMaxDepth(minDepth, maxDepth);
    pDescriptor->setMinMaxUncertainty(minUncert, maxUncert);
}

}  // namespace BAG
