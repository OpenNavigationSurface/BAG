
#include "bag_dataset.h"
#include "bag_private.h"
#include "bag_simplelayer.h"
#include "bag_surfacecorrections.h"
#include "bag_surfacecorrectionsdescriptor.h"

#include <array>
#include <cmath>
#include <cstring>  // memset
#include <memory>
#include <H5Cpp.h>


namespace BAG {

//! The maximum length of the list of datums.
constexpr uint16_t kMaxDatumsLength = 256;
//! The radius to use while searching for nearest neighbours.
constexpr int32_t kSearchRadius = 3;

namespace {

//! Get the HDF5 CompType specified by the surface corrections descriptor.
/*!
\param descriptor
    The surface corrections descriptor.

\return
    The HDF5 CompType as specified by the descriptor.
*/
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

    const std::array<hsize_t, kRank> zDims{1, descriptor.getNumCorrectors()};
    ::H5::ArrayType h5zDataType{::H5::PredType::NATIVE_FLOAT, kRank, zDims.data()};

    h5memDataType.insertMember("z", offset, h5zDataType);

    return h5memDataType;
}

}  // namespace

//! Constructor.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
\param descriptor
    The descriptor of this layer.
\param pH5dataSet
    The HDF5 DataSet that stores this interleaved layer.
*/
SurfaceCorrections::SurfaceCorrections(
    Dataset& dataset,
    SurfaceCorrectionsDescriptor& descriptor,
    std::unique_ptr<::H5::DataSet, DeleteH5dataSet> pH5dataSet)
    : Layer(dataset, descriptor)
    , m_pH5dataSet(std::move(pH5dataSet))
{
}

//! Create a surface corrections layer.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
\param type
    The type of surface correction topography.
\param numCorrectors
    The number of correctors provided.
    Valid range is 1-10.
\param chunkSize
    The chunk size the HDF5 DataSet will use.
\param compressionLevel
    The compression level the HDF5 DataSet will use.

\return
    The new surface corrections layer.
*/
std::shared_ptr<SurfaceCorrections> SurfaceCorrections::create(
    Dataset& dataset,
    BAG_SURFACE_CORRECTION_TOPOGRAPHY type,
    uint8_t numCorrectors,
    uint64_t chunkSize,
    int compressionLevel)
{
    auto descriptor = SurfaceCorrectionsDescriptor::create(dataset, type,
        numCorrectors, chunkSize, compressionLevel);

    auto h5dataSet = SurfaceCorrections::createH5dataSet(dataset, *descriptor);

    return std::make_shared<SurfaceCorrections>(dataset,
        *descriptor, std::move(h5dataSet));
}

//! Open an existing surface corrections layer.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
\param descriptor
    The descriptor of this layer.

\return
    The new surface corrections layer.
*/
std::shared_ptr<SurfaceCorrections> SurfaceCorrections::open(
    Dataset& dataset,
    SurfaceCorrectionsDescriptor& descriptor)
{
    const auto& h5file = dataset.getH5file();
    auto h5dataSet = std::unique_ptr<::H5::DataSet, DeleteH5dataSet>(
        new ::H5::DataSet{h5file.openDataSet(descriptor.getInternalPath())},
        DeleteH5dataSet{});

    return std::make_shared<SurfaceCorrections>(dataset,
        descriptor, std::move(h5dataSet));
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
SurfaceCorrections::createH5dataSet(
    const Dataset& dataset,
    const SurfaceCorrectionsDescriptor& descriptor)
{
    std::array<hsize_t, kRank> fileDims{0, 0};
    const std::array<hsize_t, kRank> kMaxFileDims{H5S_UNLIMITED, H5S_UNLIMITED};
    const ::H5::DataSpace h5fileDataSpace{kRank, fileDims.data(), kMaxFileDims.data()};

    // Use chunk size and compression level from the descriptor.
    const auto compressionLevel = descriptor.getCompressionLevel();

    // Create the creation property list.
    const ::H5::DSetCreatPropList h5createPropList{};

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
    else
        throw LayerRequiresChunkingSet{};

    h5createPropList.setFillTime(H5D_FILL_TIME_ALLOC);

    const auto h5memDataType = getCompoundType(descriptor);

    // Create the DataSet using the above.
    const auto& h5file = dataset.getH5file();

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

//! Retrieve the HDF5 DataSet.
const ::H5::DataSet& SurfaceCorrections::getH5dataSet() const & noexcept
{
    return *m_pH5dataSet;
}

//! Read a corrected region from a simple layer using the specified corrector.
/*!
\param rowStart
    The starting row.
\param columnStart
    The starting column.
\param rowEnd
    The ending row (inclusive).
\param columnEnd
    The ending column (inclusive).
\param corrector
    The corrector to use when applying a correction.
    Valid values are 1-10.
\param layer
    The simple layer to correct.

\return
    The corrected date from the simple layer using the specified corrector.
*/
UInt8Array SurfaceCorrections::readCorrected(
    uint32_t rowStart,
    uint32_t columnStart,
    uint32_t rowEnd,
    uint32_t columnEnd,
    uint8_t corrector,
    const SimpleLayer& layer) const
{
    auto pDescriptor =
        std::dynamic_pointer_cast<const SurfaceCorrectionsDescriptor>(
            this->getDescriptor());
    if (!pDescriptor)
        throw InvalidDescriptor{};

    auto weakDataset = this->getDataset();
    if (weakDataset.expired())
        throw DatasetNotFound{};

    auto dataset = weakDataset.lock();

    uint32_t ncols = 0, nrows = 0;
    std::tie(ncols, nrows) = dataset->getDescriptor().getDims();

    if (columnEnd >= ncols || rowEnd >= nrows || rowStart > rowEnd
        || columnStart > columnEnd)
        throw InvalidReadSize{};

    UInt8Array data{(columnEnd - columnStart + 1) * (rowEnd - rowStart + 1)
        * sizeof(float)};

    for (uint32_t f=0, i=0; i<nrows; ++i)
    {
        if (i >= rowStart && i <= rowEnd)
        {
            //! By row at a time, fill the whole buffer
            auto sepData = this->readCorrectedRow(i, columnStart, columnEnd,
                corrector, layer);

            //! For each column in the transfer set
            for (uint32_t j=0; j<columnEnd-columnStart + 1; ++j)
                data[(columnEnd - columnStart + 1) * f + j] = sepData[j];

            f++;
        }
    }

    return data;
}

//! Read a corrected row from a simple layer using the specified corrector.
/*!
\param row
    The row.
\param columnStart
    The starting column.
\param columnEnd
    The ending column (inclusive).
\param corrector
    The corrector to use when applying a correction.
    Valid values are 1-10.
\param layer
    The simple layer to correct.

\return
    The corrected date from the simple layer using the specified corrector.
*/
UInt8Array SurfaceCorrections::readCorrectedRow(
    uint32_t row,
    uint32_t columnStart,
    uint32_t columnEnd,
    uint8_t corrector,
    const SimpleLayer& layer) const
{
    auto pDescriptor =
        std::dynamic_pointer_cast<const SurfaceCorrectionsDescriptor>(
            this->getDescriptor());
    if (!pDescriptor)
        throw InvalidDescriptor{};

    const auto surfaceType = pDescriptor->getSurfaceType();  // aka topography

    if (surfaceType != BAG_SURFACE_GRID_EXTENTS)
        throw UnsupportedSurfaceType{};

    if (corrector < 1 || corrector > pDescriptor->getNumCorrectors())
        throw InvalidCorrector{};

    --corrector;  // This is 0 based when used.

    auto originalRow = layer.read(row, row, columnStart, columnEnd);
    auto* data = reinterpret_cast<float*>(originalRow.data());

    // Obtain cell resolution and SW origin (0,1,1,0).
    double swCornerX = 0., swCornerY = 0.;
    std::tie(swCornerX, swCornerY) = pDescriptor->getOrigin();

    double nodeSpacingX = 0., nodeSpacingY = 0.;
    std::tie(nodeSpacingX, nodeSpacingY) = pDescriptor->getSpacing();

    const auto resratio = nodeSpacingX / nodeSpacingY;

    uint32_t ncols = 0, nrows = 0;
    std::tie(nrows, ncols) = pDescriptor->getDims();

    std::array<int32_t, 2> lastP{-1, -1};

    auto weakDataset = this->getDataset();
    if (weakDataset.expired())
        throw DatasetNotFound{};

    auto dataset = weakDataset.lock();

    double swCornerXsimple = 0., swCornerYsimple = 0.;
    std::tie(swCornerXsimple, swCornerYsimple) =
        dataset->getDescriptor().getOrigin();

    double nodeSpacingXsimple = 0., nodeSpacingYsimple = 0.;
    std::tie(nodeSpacingXsimple, nodeSpacingYsimple) =
        dataset->getDescriptor().getGridSpacing();

    using std::floor;  using std::fabs;  using std::ceil;

    // Compute an SEP for each cell in the row.
    for (auto j=columnStart; j<=columnEnd; ++j)
    {
        const uint32_t rowIndex = j - columnStart;

        if (data[rowIndex] == BAG_NULL_GENERIC ||
            data[rowIndex] == BAG_NULL_ELEVATION ||
            data[rowIndex] == BAG_NULL_UNCERTAINTY)
            continue;

        std::array<uint32_t, 2> rowRange{0, 0};
        std::array<uint32_t, 2> colRange{0, 0};

        //! Determine the X and Y values of given cell.
        const double nodeX = swCornerXsimple + j * nodeSpacingXsimple;
        const double nodeY = swCornerYsimple + row * nodeSpacingYsimple;

#if 0  // Dead code (see logic above); saved from C code for now.
        if (surfaceType == BAG_SURFACE_IRREGULARLY_SPACED)
        {
            if (lastP[0] == -1 || lastP[1] == -1)
            {
                colRange[0] = 0;
                colRange[1] = ncols -1;
                rowRange[0] = 0;
                rowRange[1] = nrows -1;
            }
            else
            {
                colRange[0] = lastP[0] - kSearchRadius;
                colRange[1] = lastP[0] + kSearchRadius;
                rowRange[0] = lastP[1] - kSearchRadius;
                rowRange[1] = lastP[1] + kSearchRadius;
            }
        }
        else
#endif
        if (surfaceType == BAG_SURFACE_GRID_EXTENTS)
        {
            //! A simple calculation for 4 nearest corrector nodes.
            colRange[0] = static_cast<uint32_t>(fabs(floor((swCornerX - nodeX) / nodeSpacingX)));
            colRange[1] = static_cast<uint32_t>(fabs(ceil((swCornerX - nodeX) / nodeSpacingX)));
            rowRange[0] = static_cast<uint32_t>(fabs(floor((swCornerY - nodeY) / nodeSpacingY)));
            rowRange[1] = static_cast<uint32_t>(fabs(ceil((swCornerY - nodeY) / nodeSpacingY)));
        }

        //! Enforce dataset limits.
        if (colRange[0] > colRange[1])
            std::swap(colRange[0], colRange[1]);

        if (colRange[0] >= ncols)
            colRange[0] = ncols -1;

        if (colRange[1] >= ncols)
            colRange[1] = ncols -1;

        if (rowRange[0] > rowRange[1])
            std::swap(rowRange[0], rowRange[1]);

        if (rowRange[0] >= nrows)
            rowRange[0] = nrows -1;

        if (rowRange[1] >= nrows)
            rowRange[1] = nrows -1;

        if (colRange[1] == colRange[0])
        {
            if (colRange[0] > 0)
                --colRange[0];

            if ((colRange[1] + 1) < ncols)
                ++colRange[1];
        }
        if (rowRange[1] == rowRange[0])
        {

            if (rowRange[0] > 0)
                --rowRange[0];

            if ((rowRange[1] + 1) < nrows)
                ++rowRange[1];
        }

        //std::cerr << "INDX: " << rowIndex << " Row: " << row << " RC:  " << rowRange[0] << "  / "<< rowRange[1] << '\n';

        bool isZeroDistance = false;
        double sum_sep = 0.0;
        double sum = 0.0;
        double leastDistSq = std::numeric_limits<double>::max();

        // Look through the SEPs and calculate the weighted average between them and this position.
        for (auto q=rowRange[0]; !isZeroDistance && q <= rowRange[1]; ++q)
        {
            // The SEP should be accessed, up to entire row of all columns of Surface_Correction.
            const auto buffer = this->read(q, q, colRange[0], colRange[1]);
            const auto* readbuf = reinterpret_cast<const BagVerticalDatumCorrectionsGridded*>(buffer.data());
            const auto y1 = swCornerY + q * nodeSpacingY;

            for (auto u=colRange[0]; u<=colRange[1]; ++u)
            {
                const auto* vertCorr = readbuf + (u - colRange[0]);

                const auto z1 = vertCorr->z[corrector];
                const auto x1 = swCornerX + u * nodeSpacingX;

                double distSq = 0.;

                if (nodeX ==  x1 && nodeY == y1)
                {
                    isZeroDistance = true;
                    distSq = 1.0;
                    data[rowIndex] += z1;

                    break;
                }

                // Calculate distance weight between nodeX/nodeY and y1/x1
                distSq = std::pow(fabs(static_cast<double>(nodeX - x1)), 2.0) +
                    std::pow(resratio * fabs(static_cast<double>(nodeY - y1)), 2.0);


                if (leastDistSq > distSq)
                {
                    leastDistSq = distSq;
                    lastP[0] = u;
                    lastP[1] = q;
                }

                // Inverse distance calculation
                sum_sep += z1 / distSq;
                sum += 1.0 / distSq;
            }
        }

        //std::cerr << "sum sum " << sum_sep << " / " << sum << " =  " << sum_sep / sum << " : " << data[rowIndex] << '\n';

        if (!isZeroDistance)
        {
            // is not a constant SEP with one point?
            if (sum_sep != 0.0 && sum != 0.0)
                data[rowIndex] += static_cast<float>(sum_sep / sum);
            else
                data[rowIndex] = BAG_NULL_GENERIC;
        }

    }

    return originalRow;
}

//! \copydoc Layer::read
UInt8Array SurfaceCorrections::readProxy(
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

    auto pDescriptor =
        std::dynamic_pointer_cast<const SurfaceCorrectionsDescriptor>(
            this->getDescriptor());
    if (!pDescriptor)
        throw InvalidLayerDescriptor{};

    const auto bufferSize = pDescriptor->getReadBufferSize(rows, columns);
    UInt8Array buffer{bufferSize};

    const ::H5::DataSpace h5memSpace{kRank, count.data(), count.data()};

    const auto h5memDataType = getCompoundType(*pDescriptor);

    m_pH5dataSet->read(buffer.data(), h5memDataType, h5memSpace, h5fileDataSpace);

    return buffer;
}

//! \copydoc Layer::writeAttributes
void SurfaceCorrections::writeAttributesProxy() const
{
    auto pDescriptor =
        std::dynamic_pointer_cast<const SurfaceCorrectionsDescriptor>(
            this->getDescriptor());
    if (!pDescriptor)
        throw InvalidDescriptor{};

    // Write any attributes, from the layer descriptor.
    // surface type
    auto att = m_pH5dataSet->openAttribute(VERT_DATUM_CORR_SURFACE_TYPE);
    const auto surfaceType = pDescriptor->getSurfaceType();
    const auto tmpSurfaceType = static_cast<uint8_t>(surfaceType);
    att.write(::H5::PredType::NATIVE_UINT8, &tmpSurfaceType);

    // vertical datums
    auto tmpDatums = pDescriptor->getVerticalDatums();
    if (tmpDatums.size() > kMaxDatumsLength)
        tmpDatums.resize(kMaxDatumsLength);

    att = m_pH5dataSet->openAttribute(VERT_DATUM_CORR_VERTICAL_DATUM);
    att.write(::H5::PredType::C_S1, tmpDatums);

    // Write any optional attributes.
    if (surfaceType == BAG_SURFACE_GRID_EXTENTS)
    {
        // sw corner x
        att = m_pH5dataSet->openAttribute(VERT_DATUM_CORR_SWX);
        const auto origin = pDescriptor->getOrigin();
        att.write(::H5::PredType::NATIVE_DOUBLE, &std::get<0>(origin));

        // sw corner y
        att = m_pH5dataSet->openAttribute(VERT_DATUM_CORR_SWY);
        att.write(::H5::PredType::NATIVE_DOUBLE, &std::get<1>(origin));

        // node spacing x
        const auto spacing = pDescriptor->getSpacing();
        att = m_pH5dataSet->openAttribute(VERT_DATUM_CORR_NSX);
        att.write(::H5::PredType::NATIVE_DOUBLE, &std::get<0>(spacing));

        // node spacing y
        att = m_pH5dataSet->openAttribute(VERT_DATUM_CORR_NSY);
        att.write(::H5::PredType::NATIVE_DOUBLE, &std::get<1>(spacing));
    }
}

//! \copydoc Layer::write
void SurfaceCorrections::writeProxy(
    uint32_t rowStart,
    uint32_t columnStart,
    uint32_t rowEnd,
    uint32_t columnEnd,
    const uint8_t* buffer)
{
    auto pDescriptor = std::dynamic_pointer_cast<SurfaceCorrectionsDescriptor>(
        this->getDescriptor());
    if (!pDescriptor)
        throw InvalidDescriptor{};

    const auto rows = (rowEnd - rowStart) + 1;
    const auto columns = (columnEnd - columnStart) + 1;
    const std::array<hsize_t, kRank> count{rows, columns};
    const std::array<hsize_t, kRank> offset{rowStart, columnStart};
    const ::H5::DataSpace h5memDataSpace{kRank, count.data(), count.data()};

    ::H5::DataSpace h5fileDataSpace = m_pH5dataSet->getSpace();

    // Expand the file data space if needed.
    std::array<hsize_t, kRank> fileDims{};
    std::array<hsize_t, kRank> maxFileDims{};
    h5fileDataSpace.getSimpleExtentDims(fileDims.data(), maxFileDims.data());

    if ((fileDims[0] < (rowEnd + 1)) ||
        (fileDims[1] < (columnEnd + 1)))
    {
        const std::array<hsize_t, kRank> newDims{
            std::max<hsize_t>(fileDims[0], rowEnd + 1),
            std::max<hsize_t>(fileDims[1], columnEnd + 1)};

        m_pH5dataSet->extend(newDims.data());

        h5fileDataSpace = m_pH5dataSet->getSpace();

        // Update the dataset's dimensions.
        if (this->getDataset().expired())
            throw DatasetNotFound{};

        auto pDataset = this->getDataset().lock();
        pDataset->getDescriptor().setDims(static_cast<uint32_t>(newDims[0]),
            static_cast<uint32_t>(newDims[1]));
    }

    h5fileDataSpace.selectHyperslab(H5S_SELECT_SET, count.data(), offset.data());

    const auto h5memDataType = getCompoundType(*pDescriptor);

    m_pH5dataSet->write(buffer, h5memDataType, h5memDataSpace, h5fileDataSpace);

    // Update descriptor.
    const auto h5Space = m_pH5dataSet->getSpace();

    std::array<hsize_t, kRank> dims{};
    h5Space.getSimpleExtentDims(dims.data());

    pDescriptor->setDims(static_cast<uint32_t>(dims[0]),
        static_cast<uint32_t>(dims[1]));
}

}   //namespace BAG

