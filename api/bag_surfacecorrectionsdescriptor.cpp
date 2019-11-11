
#include "bag_dataset.h"
#include "bag_private.h"
#include "bag_surfacecorrections.h"
#include "bag_surfacecorrectionsdescriptor.h"

#include <array>
#include <h5cpp.h>


namespace BAG {

namespace {
uint8_t getElementSize(
    BAG_SURFACE_CORRECTION_TOPOGRAPHY type,
    uint8_t numCorrectors) noexcept
{
    return (type == BAG_SURFACE_IRREGULARLY_SPACED ? 2 * sizeof(double) : 0)
        + numCorrectors * sizeof(float);
}

}  // namespace

SurfaceCorrectionsDescriptor::SurfaceCorrectionsDescriptor(
    BAG_SURFACE_CORRECTION_TOPOGRAPHY type,
    uint8_t numCorrectors)
    : LayerDescriptor(Surface_Correction)
    , m_surfaceType(type)
    , m_elementSize(BAG::getElementSize(type, numCorrectors))
    , m_numCorrectors(numCorrectors)
{
}

SurfaceCorrectionsDescriptor::SurfaceCorrectionsDescriptor(
    const Dataset& dataset)
    : LayerDescriptor(Surface_Correction, dataset)
{
    const auto h5dataSet = dataset.getH5file().openDataSet(
        Layer::getInternalPath(Surface_Correction));

    // Read the attributes.
    // Surface type.
    uint8_t surfaceType = 0;

    {
        const auto att = h5dataSet.openAttribute(VERT_DATUM_CORR_SURFACE_TYPE);
        att.read(att.getDataType(), &surfaceType);
    }

    m_surfaceType = static_cast<BAG_SURFACE_CORRECTION_TOPOGRAPHY>(surfaceType);

    if (m_surfaceType != BAG_SURFACE_GRID_EXTENTS &&
        m_surfaceType != BAG_SURFACE_IRREGULARLY_SPACED)
        throw UnknownSurfaceType{};

    // Vertical datums.
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
        if (zDims[1] > 10)
            throw TooManyCorrections{};
        else
            m_numCorrectors = static_cast<uint8_t>(zDims[1]);
    else
        throw CannotReadNumCorrections{};

    // Element size.
    m_elementSize = BAG::getElementSize(m_surfaceType, m_numCorrectors);
}

std::shared_ptr<SurfaceCorrectionsDescriptor>
SurfaceCorrectionsDescriptor::create(
    BAG_SURFACE_CORRECTION_TOPOGRAPHY type,
    uint8_t numCorrectors)
{
    if (type != BAG_SURFACE_GRID_EXTENTS &&
        type != BAG_SURFACE_IRREGULARLY_SPACED)
        throw UnknownSurfaceType{};

    if (numCorrectors > 10)
        throw TooManyCorrections{};

    return std::shared_ptr<SurfaceCorrectionsDescriptor>(
        new SurfaceCorrectionsDescriptor{type, numCorrectors});
}

std::shared_ptr<SurfaceCorrectionsDescriptor>
SurfaceCorrectionsDescriptor::create(
    const Dataset& dataset)
{
    return std::shared_ptr<SurfaceCorrectionsDescriptor>(
        new SurfaceCorrectionsDescriptor{dataset});
}


uint8_t SurfaceCorrectionsDescriptor::getElementSizeProxy() const noexcept
{
    return m_elementSize;
}

uint8_t SurfaceCorrectionsDescriptor::getNumCorrectors() const noexcept
{
    return m_numCorrectors;
}

std::tuple<double, double>
SurfaceCorrectionsDescriptor::getOrigin() const noexcept
{
    return std::make_tuple(m_swX, m_swY);
}

std::tuple<double, double>
SurfaceCorrectionsDescriptor::getSpacing() const noexcept
{
    return std::make_tuple(m_xSpacing, m_ySpacing);
}

BAG_SURFACE_CORRECTION_TOPOGRAPHY
SurfaceCorrectionsDescriptor::getSurfaceType() const noexcept
{
    return m_surfaceType;
}

const std::string&
SurfaceCorrectionsDescriptor::getVerticalDatums() const & noexcept
{
    return m_verticalDatums;
}

SurfaceCorrectionsDescriptor& SurfaceCorrectionsDescriptor::setElementSizeProxy(
    uint8_t inSize) & noexcept
{
    m_elementSize = inSize;
    return *this;
}

SurfaceCorrectionsDescriptor& SurfaceCorrectionsDescriptor::setNumCorrectors(
    uint8_t numCorrectors) & noexcept
{
    m_numCorrectors = numCorrectors;
    return *this;
}

SurfaceCorrectionsDescriptor& SurfaceCorrectionsDescriptor::setSurfaceType(
    BAG_SURFACE_CORRECTION_TOPOGRAPHY surfaceType) & noexcept
{
    m_surfaceType = surfaceType;
    return *this;
}

SurfaceCorrectionsDescriptor& SurfaceCorrectionsDescriptor::setVerticalDatum(
    std::string verticalDatums) & noexcept
{
    m_verticalDatums = verticalDatums;
    return *this;
}

SurfaceCorrectionsDescriptor& SurfaceCorrectionsDescriptor::setOrigin(
    double swX,
    double swY) noexcept
{
    m_swX = swX;
    m_swY = swY;
    return *this;
}

SurfaceCorrectionsDescriptor& SurfaceCorrectionsDescriptor::setSpacing(
    double xSpacing,
    double ySpacing) noexcept
{
    m_xSpacing = xSpacing;
    m_ySpacing = ySpacing;
    return *this;
}

}  // namespace BAG

