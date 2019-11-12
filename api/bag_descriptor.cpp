#include "bag_descriptor.h"
#include "bag_exceptions.h"
#include "bag_layer.h"
#include "bag_metadata.h"


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

namespace BAG {

Descriptor::Descriptor(
    const Metadata& metadata)
    : m_horizontalReferenceSystem(metadata.horizontalReferenceSystemAsWKT())
    , m_verticalReferenceSystem(metadata.verticalReferenceSystemAsWKT())
    , m_dims({metadata.rows(), metadata.columns()})
    , m_projectedCover({metadata.llCornerX(), metadata.llCornerY(), metadata.urCornerX(), metadata.urCornerY()})
    , m_origin({metadata.llCornerX(), metadata.llCornerY()})
    , m_gridSpacing({metadata.rowResolution(), metadata.columnResolution()})
{
}

Descriptor& Descriptor::addLayerDescriptor(
    const LayerDescriptor& inDescriptor) &
{
    if (m_layerDescriptors.find(inDescriptor.getLayerType()) !=
        cend(m_layerDescriptors))
        throw LayerExists{};

    m_layerDescriptors.emplace(inDescriptor.getLayerType(),
        inDescriptor.shared_from_this());

    return *this;
}

const std::tuple<uint32_t, uint32_t>& Descriptor::getDims() const & noexcept
{
    return m_dims;
}

const std::tuple<double, double>& Descriptor::getGridSpacing() const & noexcept
{
    return m_gridSpacing;
}

const std::string& Descriptor::getHorizontalReferenceSystem() const & noexcept
{
    return m_horizontalReferenceSystem;
}

const LayerDescriptor& Descriptor::getLayerDescriptor(LayerType type) const &
{
    return *m_layerDescriptors.at(type);
}

const std::unordered_map<LayerType, std::shared_ptr<const LayerDescriptor>>&
Descriptor::getLayerDescriptors() const & noexcept
{
    return m_layerDescriptors;
}

std::vector<LayerType> Descriptor::getLayerTypes() const
{
    std::vector<LayerType> types;
    types.reserve(m_layerDescriptors.size());

    for (auto&& layerDescriptor : m_layerDescriptors)
        types.push_back(layerDescriptor.first);

    return types;
}

const std::tuple<double, double>& Descriptor::getOrigin() const & noexcept
{
    return m_origin;
}

const std::tuple<double, double, double, double>&
Descriptor::getProjectedCover() const & noexcept
{
    return m_projectedCover;
}

const std::string& Descriptor::getVersion() const & noexcept
{
    return m_version;
}

const std::string& Descriptor::getVerticalReferenceSystem() const & noexcept
{
    return m_verticalReferenceSystem;
}

bool Descriptor::isReadOnly() const noexcept
{
    return m_isReadOnly;
}

void Descriptor::setDims(
    uint32_t rows,
    uint32_t columns) & noexcept
{
    m_dims = {rows, columns};
}

void Descriptor::setGridSpacing(
    double xSpacing,
    double ySpacing) & noexcept
{
    m_gridSpacing = {xSpacing, ySpacing};
}

void Descriptor::setHorizontalReferenceSystem(
    const std::string& horizontalReferenceSystem) & noexcept
{
    m_horizontalReferenceSystem = horizontalReferenceSystem;
}

void Descriptor::setOrigin(
    double llX,
    double llY) & noexcept
{
    m_origin = {llX, llY};
}

void Descriptor::setProjectedCover(
    double llX,
    double llY,
    double urX,
    double urY) & noexcept
{
    m_projectedCover = {llX, llY, urX, urY};
}

Descriptor& Descriptor::setReadOnly(bool inReadOnly) & noexcept
{
    m_isReadOnly = inReadOnly;
    return *this;
}

Descriptor& Descriptor::setVersion(std::string inVersion) & noexcept
{
    m_version = std::move(inVersion);
    return *this;
}

void Descriptor::setVerticalReferenceSystem(
    const std::string& verticalReferenceSystem) & noexcept
{
    m_verticalReferenceSystem = verticalReferenceSystem;
}

}  // namespace BAG

#ifdef _MSC_VER
#pragma warning(pop)
#endif

