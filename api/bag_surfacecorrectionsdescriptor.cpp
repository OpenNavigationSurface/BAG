
#include "bag_dataset.h"
#include "bag_private.h"
#include "bag_surfacecorrections.h"
#include "bag_surfacecorrectionsdescriptor.h"

#include <array>
#include <H5Cpp.h>


namespace BAG {

namespace {

//! Retrieve the size of a node of the specified topography.
/*!
\param type
    The topography type.
\param numCorrectors
    The number of correctors.
    Valid values are 1-10.

\return
    The size of a node in the specified topography.
*/
uint8_t getElementSize(
    BAG_SURFACE_CORRECTION_TOPOGRAPHY type) noexcept
{
    return (type == BAG_SURFACE_IRREGULARLY_SPACED)
        ? sizeof(BAG::VerticalDatumCorrections)
        : sizeof(BAG::VerticalDatumCorrectionsGridded);
}

}  // namespace

//! Constructor.
/*!
\param id
    The unique layer id.
\param type
    The type of surface corrections.
\param numCorrectors
    The number of correctors.
\param chunkSize
    The chunk size the HDF5 DataSet will use.
\param compressionLevel
    The compression level the HDF5 DataSet will use.
*/
SurfaceCorrectionsDescriptor::SurfaceCorrectionsDescriptor(
    uint32_t id,
    BAG_SURFACE_CORRECTION_TOPOGRAPHY type,
    uint8_t numCorrectors,
    uint64_t chunkSize,
    int compressionLevel)
    : LayerDescriptor(id, Layer::getInternalPath(Surface_Correction),
        kLayerTypeMapString.at(Surface_Correction), Surface_Correction,
        chunkSize, compressionLevel)
    , m_surfaceType(type)
    , m_elementSize(BAG::getElementSize(type))
    , m_numCorrectors(numCorrectors)
{
}

//! Constructor.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
*/
SurfaceCorrectionsDescriptor::SurfaceCorrectionsDescriptor(
    const Dataset& dataset)
    : LayerDescriptor(dataset, Surface_Correction)
{
    const auto h5dataSet = dataset.getH5file().openDataSet(
        Layer::getInternalPath(Surface_Correction));

    // Read the attributes.
    // surface type
    uint8_t surfaceType = 0;

    {
        const auto att = h5dataSet.openAttribute(VERT_DATUM_CORR_SURFACE_TYPE);
        att.read(att.getDataType(), &surfaceType);
    }

    m_surfaceType = static_cast<BAG_SURFACE_CORRECTION_TOPOGRAPHY>(surfaceType);

    if (m_surfaceType != BAG_SURFACE_GRID_EXTENTS &&
        m_surfaceType != BAG_SURFACE_IRREGULARLY_SPACED)
        throw UnknownSurfaceType{};

    // vertical datums
    {
        const auto att = h5dataSet.openAttribute(VERT_DATUM_CORR_VERTICAL_DATUM);
        att.read(att.getDataType(), m_verticalDatums);
    }

    // Read extra attributes if the surface type provides them.
    if (m_surfaceType == BAG_SURFACE_GRID_EXTENTS)
    {
        auto att = h5dataSet.openAttribute(VERT_DATUM_CORR_SWX);
        att.read(att.getDataType(), &m_swX);

        att = h5dataSet.openAttribute(VERT_DATUM_CORR_SWY);
        att.read(att.getDataType(), &m_swY);


        att = h5dataSet.openAttribute(VERT_DATUM_CORR_NSX);
        att.read(att.getDataType(), &m_xSpacing);

        att = h5dataSet.openAttribute(VERT_DATUM_CORR_NSY);
        att.read(att.getDataType(), &m_ySpacing);
    }

    // Number of corrections.
    const auto h5fileCompType = h5dataSet.getCompType();
    const int memberNum = h5fileCompType.getMemberIndex("z");
    const auto h5zDataType = h5fileCompType.getMemberArrayType(memberNum);
    std::array<hsize_t, H5S_MAX_RANK> zDims{};

    // Read the number of Z corrections.
    const int zRank = h5zDataType.getArrayDims(zDims.data());
    if (zRank == 2)
        if (zDims[1] > BAG_SURFACE_CORRECTOR_LIMIT)
            throw TooManyCorrections{};
        else
            m_numCorrectors = static_cast<uint8_t>(zDims[1]);
    else
        throw CannotReadNumCorrections{};

    // Element size.
    m_elementSize = BAG::getElementSize(m_surfaceType);

    // Number of rows & columns.
    const auto h5Space = h5dataSet.getSpace();

    std::array<hsize_t, kRank> dims{};
    h5Space.getSimpleExtentDims(dims.data());

    this->setDims(static_cast<uint32_t>(dims[0]),
        static_cast<uint32_t>(dims[1]));
}

//! Create a surface corrections layer.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
\param type
    The type of surface corrections.
\param numCorrectors
    The number of correctors.
\param chunkSize
    The chunk size the HDF5 DataSet will use.
\param compressionLevel
    The compression level the HDF5 DataSet will use.
*/
std::shared_ptr<SurfaceCorrectionsDescriptor>
SurfaceCorrectionsDescriptor::create(
    const Dataset& dataset,
    BAG_SURFACE_CORRECTION_TOPOGRAPHY type,
    uint8_t numCorrectors,
    uint64_t chunkSize,
    int compressionLevel)
{
    if (type != BAG_SURFACE_GRID_EXTENTS &&
        type != BAG_SURFACE_IRREGULARLY_SPACED)
        throw UnknownSurfaceType{};

    if (numCorrectors > BAG_SURFACE_CORRECTOR_LIMIT)
        throw TooManyCorrections{};

    return std::shared_ptr<SurfaceCorrectionsDescriptor>(
        new SurfaceCorrectionsDescriptor{dataset.getNextId(), type,
            numCorrectors, chunkSize, compressionLevel});
}

//! Open an existing surface corrections layer.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
*/
std::shared_ptr<SurfaceCorrectionsDescriptor>
SurfaceCorrectionsDescriptor::open(
    const Dataset& dataset)
{
    return std::shared_ptr<SurfaceCorrectionsDescriptor>(
        new SurfaceCorrectionsDescriptor{dataset});
}


//! \copydoc LayerDescriptor::getDataType
DataType SurfaceCorrectionsDescriptor::getDataTypeProxy() const noexcept
{
    return Layer::getDataType(Surface_Correction);
}

//! Retrieve the dimensions of the surface corrections layer.
/*!
\return
    The dimensions of the surface corrections layer.
*/
std::tuple<uint32_t, uint32_t> SurfaceCorrectionsDescriptor::getDims() const noexcept
{
    return {m_numRows, m_numColumns};
}

//! \copydoc LayerDescriptor::getElementSize
uint8_t SurfaceCorrectionsDescriptor::getElementSizeProxy() const noexcept
{
    return m_elementSize;
}

//! Retrieve the number of correctors.
/*!
\return
    The number of correctors.
*/
uint8_t SurfaceCorrectionsDescriptor::getNumCorrectors() const noexcept
{
    return m_numCorrectors;
}

//! Retrieve the south west corner of the surface corrections layer.
/*!
\return
    The south west corner of the surface corrections layer.
*/
std::tuple<double, double>
SurfaceCorrectionsDescriptor::getOrigin() const noexcept
{
    return {m_swX, m_swY};
}

//! Retrieve the X and Y spacing/resolution of the surface corrections layer.
/*!
\return
    The X and Y spacing/resolution of the surface corrections layer.
*/
std::tuple<double, double>
SurfaceCorrectionsDescriptor::getSpacing() const noexcept
{
    return {m_xSpacing, m_ySpacing};
}

//! Retrieve surface correction topography.
/*!
\return
    The surface correction topography.
*/
BAG_SURFACE_CORRECTION_TOPOGRAPHY
SurfaceCorrectionsDescriptor::getSurfaceType() const noexcept
{
    return m_surfaceType;
}

//! Retrieve the vertical datums.
/*!
\return
    The vertical datums.
*/
const std::string&
SurfaceCorrectionsDescriptor::getVerticalDatums() const & noexcept
{
    return m_verticalDatums;
}

//! Set the grid dimensions.
/*!
\param numRows
    The new number of rows.
\param numColumns
    The new number of columns.

\return
    The surface corrections descriptor.
*/
SurfaceCorrectionsDescriptor& SurfaceCorrectionsDescriptor::setDims(
    uint32_t numRows,
    uint32_t numColumns) & noexcept
{
    m_numRows = numRows;
    m_numColumns = numColumns;
    return *this;
}

//! Set the south west corner.
/*!
\param swX
    The X value of the south west corner.
    Geographic location.
\param swY
    The Y value of the south west corner.
    Geographic location.

\return
    The surface corrections descriptor.
*/
SurfaceCorrectionsDescriptor& SurfaceCorrectionsDescriptor::setOrigin(
    double swX,
    double swY) & noexcept
{
    m_swX = swX;
    m_swY = swY;
    return *this;
}

//! Set the X and Y spacing/resolution.
/*!
\param xSpacing
    The X spacing/resolution.
\param ySpacing
    The Y spacing/resolution.

\return
    The surface corrections descriptor.
*/
SurfaceCorrectionsDescriptor& SurfaceCorrectionsDescriptor::setSpacing(
    double xSpacing,
    double ySpacing) & noexcept
{
    m_xSpacing = xSpacing;
    m_ySpacing = ySpacing;
    return *this;
}

//! Set the vertical datums.
/*!
\param verticalDatums
    The vertical datums.

\return
    The surface corrections descriptor.
*/
SurfaceCorrectionsDescriptor& SurfaceCorrectionsDescriptor::setVerticalDatums(
    std::string verticalDatums) & noexcept
{
    m_verticalDatums = std::move(verticalDatums);
    return *this;
}

}  // namespace BAG

