
#include "bag_descriptor.h"
#include "bag_exceptions.h"
#include "bag_layer.h"
#include "bag_metadata.h"

#include <algorithm>


namespace BAG {

Descriptor::Descriptor(
    const Metadata& metadata)
    : m_horizontalReferenceSystem(metadata.horizontalReferenceSystemAsWKT())
    , m_verticalReferenceSystem(metadata.verticalReferenceSystemAsWKT())
    , m_dims({metadata.rows(), metadata.columns()})
    , m_projectedCover({metadata.llCornerX(), metadata.llCornerY(),
        metadata.urCornerX(), metadata.urCornerY()})
    , m_origin({metadata.llCornerX(), metadata.llCornerY()})
    , m_gridSpacing({metadata.rowResolution(), metadata.columnResolution()})
{
}

Descriptor& Descriptor::addLayerDescriptor(
    const LayerDescriptor& inDescriptor) &
{
    const auto foundIter = std::find_if(cbegin(m_layerDescriptors),
        cend(m_layerDescriptors),
        [&inDescriptor](const auto& descriptor) {
            return inDescriptor.getInternalPath() == descriptor->getInternalPath();
        });
    if (foundIter != cend(m_layerDescriptors))
        throw LayerExists{};

    m_layerDescriptors.emplace_back(inDescriptor.shared_from_this());

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

std::vector<uint32_t> Descriptor::getLayerIds() const noexcept
{
    std::vector<uint32_t> ids;
    ids.reserve(m_layerDescriptors.size());

    for (const auto& descriptor : m_layerDescriptors)
        ids.emplace_back(descriptor->getId());

    return ids;
}

const LayerDescriptor& Descriptor::getLayerDescriptor(LayerType type) const &
{
    return *m_layerDescriptors.at(type);
}

const std::vector<std::shared_ptr<const LayerDescriptor>>&
Descriptor::getLayerDescriptors() const & noexcept
{
    return m_layerDescriptors;
}

std::vector<LayerType> Descriptor::getLayerTypes() const
{
    std::vector<LayerType> types;
    types.reserve(m_layerDescriptors.size());

    for (const auto& layerDescriptor : m_layerDescriptors)
        types.emplace_back(layerDescriptor->getLayerType());

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

