
#include "bag_dataset.h"
#include "bag_errors.h"
#include "bag_private.h"
#include "bag_surfacecorrections.h"
#include "bag_surfacecorrectionsdescriptor.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

#include <array>
#include <h5cpp.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif


namespace BAG {

constexpr uint8_t kMaxDatumsLength = 255;

namespace {

::H5::CompType getCompoundType(
    const SurfaceCorrectionsDescriptor& descriptor)
{
    const ::H5::CompType h5memDataType{
        static_cast<size_t>(descriptor.getElementSize())};

    size_t offset = 0;

    if (descriptor.getSurfaceType() == BAG_SURFACE_IRREGULARLY_SPACED)
    {
        h5memDataType.insertMember("x", offset, ::H5::PredType::NATIVE_DOUBLE);
        offset += sizeof(double);

        h5memDataType.insertMember("y", offset, ::H5::PredType::NATIVE_DOUBLE);
        offset += sizeof(double);
    }

    const std::array<hsize_t, RANK> zDims{1, descriptor.getNumCorrectors()};
    ::H5::ArrayType h5zDataType{::H5::PredType::NATIVE_FLOAT, RANK, zDims.data()};

    h5memDataType.insertMember("z", offset, h5zDataType);

    return h5memDataType;
}

}  // namespace

SurfaceCorrections::SurfaceCorrections(
    Dataset& dataset,
    SurfaceCorrectionsDescriptor& descriptor,
    std::unique_ptr<::H5::DataSet, DeleteH5dataSet> pH5dataSet)
    : Layer(dataset, descriptor)
    , m_pH5dataSet(std::move(pH5dataSet))
{
}

std::unique_ptr<SurfaceCorrections> SurfaceCorrections::create(
    Dataset& dataset,
    BAG_SURFACE_CORRECTION_TOPOGRAPHY type,
    uint8_t numCorrectors,
    uint64_t chunkSize,
    unsigned int compressionLevel)
{
    auto descriptor = SurfaceCorrectionsDescriptor::create(type, numCorrectors,
        chunkSize, compressionLevel);

    auto h5dataSet = SurfaceCorrections::createH5dataSet(dataset, *descriptor);

    return std::unique_ptr<SurfaceCorrections>(new SurfaceCorrections{dataset,
        *descriptor, std::move(h5dataSet)});
}

std::unique_ptr<SurfaceCorrections> SurfaceCorrections::open(
    Dataset& dataset,
    SurfaceCorrectionsDescriptor& descriptor)
{
    const auto& h5file = dataset.getH5file();
    auto h5dataSet = std::unique_ptr<::H5::DataSet, DeleteH5dataSet>(
        new ::H5::DataSet{h5file.openDataSet(descriptor.getInternalPath())},
        DeleteH5dataSet{});

    return std::unique_ptr<SurfaceCorrections>(new SurfaceCorrections{dataset,
        descriptor, std::move(h5dataSet)});
}


std::unique_ptr<::H5::DataSet, SurfaceCorrections::DeleteH5dataSet>
SurfaceCorrections::createH5dataSet(
    const Dataset& inDataSet,
    const SurfaceCorrectionsDescriptor& descriptor)
{
    // Use the dimensions from the descriptor.
    const auto dims = descriptor.getDims();
    const std::array<hsize_t, RANK> fileDims{std::get<0>(dims), std::get<1>(dims)};
    const std::array<uint64_t, RANK> kMaxFileDims{H5S_UNLIMITED, H5S_UNLIMITED};
    const ::H5::DataSpace h5fileDataSpace{RANK, fileDims.data(), kMaxFileDims.data()};

    // Use chunk size and compression level from the descriptor.
    const auto chunkSize = descriptor.getChunkSize();
	std::array<hsize_t, RANK> chunkDims{chunkSize, chunkSize};

    auto compressionLevel = descriptor.getCompressionLevel();

    // Create the creation property list.
    const ::H5::DSetCreatPropList h5createPropList{};

    if (compressionLevel > 0 && compressionLevel < 10)
    {
        h5createPropList.setLayout(H5D_CHUNKED);
        h5createPropList.setChunk(RANK, chunkDims.data());
        h5createPropList.setDeflate(compressionLevel);
    }

    h5createPropList.setFillTime(H5D_FILL_TIME_ALLOC);

    const auto h5memDataType = getCompoundType(descriptor);

    auto zeroData = std::make_unique<uint8_t[]>(descriptor.getElementSize());
    memset(zeroData.get(), 0, descriptor.getElementSize());
    h5createPropList.setFillValue(h5memDataType, zeroData.get());

    // Create the DataSet using the above.
    const auto& h5file = inDataSet.getH5file();

    const auto h5dataSet = h5file.createDataSet(VERT_DATUM_CORR_PATH,
        h5memDataType, h5fileDataSpace, h5createPropList);

    // Create any attributes.
    const ::H5::DataSpace kScalarDataSpace{};
    auto att = h5dataSet.createAttribute(VERT_DATUM_CORR_SURFACE_TYPE,
        ::H5::PredType::NATIVE_UINT8, kScalarDataSpace);

    const auto surfaceType = descriptor.getSurfaceType();
    const auto tmpSurfaceType = static_cast<uint8_t>(surfaceType);
    att.write(::H5::PredType::NATIVE_UINT8, &tmpSurfaceType);

    constexpr hsize_t xmlLength = 0;
    constexpr hsize_t kMaxSize = kMaxDatumsLength;
    const ::H5::DataSpace kVerticalDatumDataSpace{1, &xmlLength, &kMaxSize};

    att = h5dataSet.createAttribute(VERT_DATUM_CORR_VERTICAL_DATUM,
        ::H5::PredType::C_S1, kVerticalDatumDataSpace);

    // Create any optional attributes.
    if (surfaceType == BAG_SURFACE_GRID_EXTENTS)
    {
        constexpr double junk = 0.;

        att = h5dataSet.createAttribute(VERT_DATUM_CORR_SWX,
            ::H5::PredType::NATIVE_DOUBLE, kScalarDataSpace);
        att.write(::H5::PredType::NATIVE_DOUBLE, &junk);

        att = h5dataSet.createAttribute(VERT_DATUM_CORR_SWY,
            ::H5::PredType::NATIVE_DOUBLE, kScalarDataSpace);
        att.write(::H5::PredType::NATIVE_DOUBLE, &junk);

        att = h5dataSet.createAttribute(VERT_DATUM_CORR_NSX,
            ::H5::PredType::NATIVE_DOUBLE, kScalarDataSpace);
        att.write(::H5::PredType::NATIVE_DOUBLE, &junk);

        att = h5dataSet.createAttribute(VERT_DATUM_CORR_NSY,
            ::H5::PredType::NATIVE_DOUBLE, kScalarDataSpace);
        att.write(::H5::PredType::NATIVE_DOUBLE, &junk);
    }

    return std::unique_ptr<::H5::DataSet, DeleteH5dataSet>(
        new ::H5::DataSet{h5dataSet}, DeleteH5dataSet{});
}

const ::H5::DataSet& SurfaceCorrections::getH5dataSet() const & noexcept
{
    return *m_pH5dataSet;
}

std::unique_ptr<uint8_t[]> SurfaceCorrections::readProxy(
    uint32_t rowStart,
    uint32_t columnStart,
    uint32_t rowEnd,
    uint32_t columnEnd) const
{
    const auto rows = (rowEnd - rowStart) + 1;
    const auto columns = (columnEnd - columnStart) + 1;
    const std::array<hsize_t, RANK> count{rows, columns};
    const std::array<hsize_t, RANK> offset{rowStart, columnStart};

    // Query the file for the specified rows and columns.
    const auto h5fileSpace = m_pH5dataSet->getSpace();
    h5fileSpace.selectHyperslab(H5S_SELECT_SET, count.data(), offset.data());

    if (!dynamic_cast<const SurfaceCorrectionsDescriptor*>(&this->getDescriptor()))
        throw UnexpectedLayerDescriptorType{};

    const auto& descriptor =
        static_cast<const SurfaceCorrectionsDescriptor&>(this->getDescriptor());

    const auto bufferSize = descriptor.getReadBufferSize(rows, columns);
    auto buffer = std::make_unique<uint8_t[]>(bufferSize);

    const ::H5::DataSpace h5memSpace{RANK, count.data(), count.data()};

    const auto h5memDataType = getCompoundType(descriptor);

    m_pH5dataSet->read(buffer.get(), h5memDataType, h5memSpace, h5fileSpace);

    return buffer;
}

void SurfaceCorrections::writeProxy(
    uint32_t rowStart,
    uint32_t columnStart,
    uint32_t rowEnd,
    uint32_t columnEnd,
    const uint8_t* buffer)
{
    if (!dynamic_cast<SurfaceCorrectionsDescriptor*>(&this->getDescriptor()))
        throw UnexpectedLayerDescriptorType{};

    auto& descriptor =
        static_cast<SurfaceCorrectionsDescriptor&>(this->getDescriptor());

    const auto rows = (rowEnd - rowStart) + 1;
    const auto columns = (columnEnd - columnStart) + 1;
    const std::array<hsize_t, RANK> count{rows, columns};
    const std::array<hsize_t, RANK> offset{rowStart, columnStart};
    const ::H5::DataSpace h5memDataSpace{RANK, count.data(), count.data()};

    ::H5::DataSpace h5fileDataSpace = m_pH5dataSet->getSpace();

    // Expand the file data space if needed.
    std::array<hsize_t, RANK> fileDims{};
    std::array<hsize_t, RANK> maxFileDims{};
    h5fileDataSpace.getSimpleExtentDims(fileDims.data(), maxFileDims.data());

    if ((fileDims[0] < (rowEnd + 1)) ||
        (fileDims[1] < (columnEnd + 1)))
    {
        const std::array<hsize_t, RANK> newDims{
            std::max<hsize_t>(fileDims[0], rowEnd + 1),
            std::max<hsize_t>(fileDims[1], columnEnd + 1)};

        m_pH5dataSet->extend(newDims.data());

        h5fileDataSpace = m_pH5dataSet->getSpace();

        // Update the dataset's dimensions.
        descriptor.setDims(static_cast<uint32_t>(newDims[0]),
            static_cast<uint32_t>(newDims[1]));
    }

    h5fileDataSpace.selectHyperslab(H5S_SELECT_SET, count.data(), offset.data());

    const auto h5memDataType = getCompoundType(descriptor);

    m_pH5dataSet->write(buffer, h5memDataType, h5memDataSpace, h5fileDataSpace);
}

void SurfaceCorrections::writeAttributesProxy() const
{
    if (!dynamic_cast<const SurfaceCorrectionsDescriptor*>(&this->getDescriptor()))
        throw UnexpectedLayerDescriptorType{};

    const auto& descriptor =
        static_cast<const SurfaceCorrectionsDescriptor&>(this->getDescriptor());

    // Write any attributes, from the layer descriptor.
    // surface type
    auto att = m_pH5dataSet->openAttribute(VERT_DATUM_CORR_SURFACE_TYPE);
    const auto surfaceType = descriptor.getSurfaceType();
    const auto tmpSurfaceType = static_cast<uint8_t>(surfaceType);
    att.write(::H5::PredType::NATIVE_UINT8, &tmpSurfaceType);

    // vertical datums
    auto tmpDatums = descriptor.getVerticalDatums();
    if (tmpDatums.size() > kMaxDatumsLength)
        tmpDatums.resize(kMaxDatumsLength);

    att = m_pH5dataSet->openAttribute(VERT_DATUM_CORR_VERTICAL_DATUM);
    att.write(::H5::PredType::C_S1, tmpDatums);

    // Write any optional attributes.
    if (surfaceType == BAG_SURFACE_GRID_EXTENTS)
    {
        // sw corner x
        att = m_pH5dataSet->openAttribute(VERT_DATUM_CORR_SWX);
        const auto origin = descriptor.getOrigin();
        att.write(::H5::PredType::NATIVE_DOUBLE, &std::get<0>(origin));

        // sw corner y
        att = m_pH5dataSet->openAttribute(VERT_DATUM_CORR_SWY);
        att.write(::H5::PredType::NATIVE_DOUBLE, &std::get<1>(origin));

        // node spacing x
        const auto spacing = descriptor.getSpacing();
        att = m_pH5dataSet->openAttribute(VERT_DATUM_CORR_NSX);
        att.write(::H5::PredType::NATIVE_DOUBLE, &std::get<0>(spacing));

        // node spacing y
        att = m_pH5dataSet->openAttribute(VERT_DATUM_CORR_NSY);
        att.write(::H5::PredType::NATIVE_DOUBLE, &std::get<1>(spacing));
    }
}

void SurfaceCorrections::DeleteH5dataSet::operator()(::H5::DataSet* ptr) noexcept
{
    delete ptr;
}

}   //namespace BAG

