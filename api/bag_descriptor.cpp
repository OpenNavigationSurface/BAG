
#include "bag_descriptor.h"
#include "bag_exceptions.h"
#include "bag_layer.h"
#include "bag_metadata.h"

#include <algorithm>
#include <cctype>


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
        [&inDescriptor](const std::weak_ptr<const LayerDescriptor>& desc) {
            if (desc.expired())
                return false;

            auto descriptor = desc.lock();
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

    for (const auto& desc : m_layerDescriptors)
    {
        if (desc.expired())
            continue;

        auto descriptor = desc.lock();
        ids.emplace_back(descriptor->getId());
    }

    return ids;
}

const LayerDescriptor& Descriptor::getLayerDescriptor(
    uint32_t id) const &
{
    const auto& desc = m_layerDescriptors.at(id);
    if (desc.expired())
        throw InvalidLayerDescriptor{};

    return *desc.lock();
}

const LayerDescriptor* Descriptor::getLayerDescriptor(
    LayerType type,
    const std::string& name) const &
{
    if (type == Compound && name.empty())
        throw NameRequired{};

    std::string nameLower{name};
    std::transform(begin(nameLower), end(nameLower), begin(nameLower),
        [](char c) noexcept {
            return static_cast<char>(std::tolower(c));
        });

    const auto& layerDesc = std::find_if(cbegin(m_layerDescriptors),
        cend(m_layerDescriptors),
        [type, &nameLower](const std::weak_ptr<const LayerDescriptor>& d) {
            if (d.expired())
                return false;

            auto desc = d.lock();

            if (desc->getLayerType() != type)
                return false;

            if (nameLower.empty())
                return true;

            std::string foundNameLower{desc->getName()};
            std::transform(begin(foundNameLower), end(foundNameLower), begin(foundNameLower),
                [](char c) noexcept {
                    return static_cast<char>(std::tolower(c));
                });

            return foundNameLower == nameLower;
        });
    if (layerDesc == cend(m_layerDescriptors))
        throw LayerNotFound{};

    return layerDesc->lock().get();
}

const std::vector<std::weak_ptr<const LayerDescriptor>>&
Descriptor::getLayerDescriptors() const & noexcept
{
    return m_layerDescriptors;
}

std::vector<LayerType> Descriptor::getLayerTypes() const
{
    std::vector<LayerType> types;
    types.reserve(m_layerDescriptors.size());

    bool addedCompoundLayer = false;

    for (const auto& desc : m_layerDescriptors)
    {
        if (desc.expired())
            continue;

        auto layerDescriptor = desc.lock();

        const auto type = layerDescriptor->getLayerType();
        if (type == Compound)
        {
            if (!addedCompoundLayer)
                addedCompoundLayer = true;
            else
                continue;
        }

        types.emplace_back(type);
    }

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

Descriptor& Descriptor::setDims(
    uint32_t rows,
    uint32_t columns) & noexcept
{
    m_dims = {rows, columns};
    return *this;
}

Descriptor& Descriptor::setGridSpacing(
    double xSpacing,
    double ySpacing) & noexcept
{
    m_gridSpacing = {xSpacing, ySpacing};
    return *this;
}

Descriptor& Descriptor::setHorizontalReferenceSystem(
    const std::string& horizontalReferenceSystem) & noexcept
{
    m_horizontalReferenceSystem = horizontalReferenceSystem;
    return *this;
}

Descriptor& Descriptor::setOrigin(
    double llX,
    double llY) & noexcept
{
    m_origin = {llX, llY};
    return *this;
}

Descriptor& Descriptor::setProjectedCover(
    double llX,
    double llY,
    double urX,
    double urY) & noexcept
{
    m_projectedCover = {llX, llY, urX, urY};
    return *this;
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

Descriptor& Descriptor::setVerticalReferenceSystem(
    const std::string& verticalReferenceSystem) & noexcept
{
    m_verticalReferenceSystem = verticalReferenceSystem;
    return *this;
}

}  // namespace BAG

