
#include "bag_descriptor.h"
#include "bag_exceptions.h"
#include "bag_layer.h"
#include "bag_metadata.h"

#include <algorithm>
#include <cctype>


namespace BAG {

namespace {

//! Find a descriptor by layer type and case-insensitive name.
/*!
\param inDescriptors
    The descriptors to search.
\param inType
    The layer type to match.
\param inName
    The case-insensitive layer name to match.

\return
    The found descriptor.
    nullptr if not found.
*/
const LayerDescriptor* getLayerDescriptor(
    const std::vector<std::weak_ptr<const LayerDescriptor>>& inDescriptors,
    LayerType inType,
    const std::string& inName)
{
    if (inType == Compound && inName.empty())
        throw NameRequired{};

    std::string nameLower{inName};
    std::transform(begin(nameLower), end(nameLower), begin(nameLower),
        [](char c) noexcept {
            return static_cast<char>(std::tolower(c));
        });

    const auto& layerDesc = std::find_if(cbegin(inDescriptors),
        cend(inDescriptors),
        [inType, &nameLower](const std::weak_ptr<const LayerDescriptor>& d) {
            if (d.expired())
                return false;

            auto desc = d.lock();

            if (desc->getLayerType() != inType)
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
    if (layerDesc == cend(inDescriptors))
        return {};

    return layerDesc->lock().get();
}

}  // namespace

//! Constructor.
/*!
\param metadata
    The metadata.
*/
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

//! Add a layer descriptor to this descriptor.
/*
\param inDescriptor
    The layer descriptor.

\return
    The updated descriptor.
*/
Descriptor& Descriptor::addLayerDescriptor(
    const LayerDescriptor& inDescriptor) &
{
    // Check if the layer descriptor already exists.
    if (BAG::getLayerDescriptor(m_layerDescriptors, inDescriptor.getLayerType(),
        inDescriptor.getName()))
        throw LayerExists{};

    m_layerDescriptors.emplace_back(inDescriptor.shared_from_this());

    return *this;
}

//! Retrieve BAG grid dimensions.
/*!
\return
    The rows and columns of the BAG.
*/
const std::tuple<uint32_t, uint32_t>& Descriptor::getDims() const & noexcept
{
    return m_dims;
}

//! Retrieve the row and column spacing/resolution of the grid.
/*
\return
    The row and column spacing/resolution of the grid.
*/
const std::tuple<double, double>& Descriptor::getGridSpacing() const & noexcept
{
    return m_gridSpacing;
}

//! Retrieve the horizontal reference system.
/*!
\return
    The horizontal reference system.
*/
const std::string& Descriptor::getHorizontalReferenceSystem() const & noexcept
{
    return m_horizontalReferenceSystem;
}

//! Retrieve a list of layer ids.
/*!
\return
    A list of layer ids.
*/
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

//! Retrieve the layer descriptor by id.
/*!
\param id
    The layer id of the desired layer descriptor.

\return
    The specified layer descriptor.
    An exception will be thrown if the layer id does not exist.
*/
const LayerDescriptor& Descriptor::getLayerDescriptor(
    uint32_t id) const &
{
    const auto& desc = m_layerDescriptors.at(id);
    if (desc.expired())
        throw InvalidLayerDescriptor{};

    return *desc.lock();
}

//! Retrieve the specified layer descriptor by type and name.
/*!
\param type
    The descriptor of the layer specified by type.
\param name
    The case-insensitive name of the layer.
    Optional for all but compound layers.
\return
    The specified layer descriptor.
*/
const LayerDescriptor* Descriptor::getLayerDescriptor(
    LayerType type,
    const std::string& name) const &
{
    return BAG::getLayerDescriptor(m_layerDescriptors, type, name);
}

//! Retrieve all the layer descriptors.
/*!
\return
    All the layer descriptors.
*/
const std::vector<std::weak_ptr<const LayerDescriptor>>&
Descriptor::getLayerDescriptors() const & noexcept
{
    return m_layerDescriptors;
}

//! Retrieve all the layer types the descriptor is aware of.
/*!
\return
    All the unique layer types the descriptor is aware of.
    If multiple compound layers are present, the type is only provided once.
*/
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

//! Retrieve the BAG origin.
/*!
\return
    The geographic position of the south west corner of the BAG.
*/
const std::tuple<double, double>& Descriptor::getOrigin() const & noexcept
{
    return m_origin;
}

//! Retrieve the projected cover.
/*!
\return
    The projected cover of the BAG.
    The values returned in the tuple are:
    Lower left corner X, lower left corner Y, upper right corner X, upper right
    corner Y.
*/
const std::tuple<double, double, double, double>&
Descriptor::getProjectedCover() const & noexcept
{
    return m_projectedCover;
}

//! Retrieve the BAG version as a string.
/*!
\return
    The BAG version as a string.
*/
const std::string& Descriptor::getVersion() const & noexcept
{
    return m_version;
}

//! Retrieve the vertical reference system.
/*!
\return
    The vertical reference system.
*/
const std::string& Descriptor::getVerticalReferenceSystem() const & noexcept
{
    return m_verticalReferenceSystem;
}

//! Retrieve the read only flag value.
/*!
\return
    \e true if the BAG is read only.
    \e false otherwise.
*/
bool Descriptor::isReadOnly() const noexcept
{
    return m_isReadOnly;
}

//! Set the BAG grid size.
/*!
\param rows
    The number of rows in the BAG.
\param columns
    The number of columns in the BAG.

\return
    The modified descriptor.
*/
Descriptor& Descriptor::setDims(
    uint32_t rows,
    uint32_t columns) & noexcept
{
    m_dims = {rows, columns};
    return *this;
}

//! Set the BAG grid spacing/resolution.
/*!
\param xSpacing
    The X spacing/resolution of the BAG grid.
\param ySpacing
    The Y spacing/resolution of the BAG grid.

\return
    The modified descriptor.
*/
Descriptor& Descriptor::setGridSpacing(
    double xSpacing,
    double ySpacing) & noexcept
{
    m_gridSpacing = {xSpacing, ySpacing};
    return *this;
}

//! Set the BAG horizontal reference system.
/*!
\param horizontalReferenceSystem
    The new horizontal reference system as WKT.

\return
    The modified descriptor.
*/
Descriptor& Descriptor::setHorizontalReferenceSystem(
    const std::string& horizontalReferenceSystem) & noexcept
{
    m_horizontalReferenceSystem = horizontalReferenceSystem;
    return *this;
}

//! Set the BAG's origin.
/*!
\param llX
    Lower left x value.
\param llY
    Lower left y value.

\return
    The modified descriptor.
*/
Descriptor& Descriptor::setOrigin(
    double llX,
    double llY) & noexcept
{
    m_origin = {llX, llY};
    return *this;
}

//! Set the BAG's projected cover.
/*!
\param llX
    Lower left x value.
\param llY
    Lower left y value.
\param urX
    Upper right x value.
\param urY
    Upper right y value.

\return
    The modified descriptor.
*/
Descriptor& Descriptor::setProjectedCover(
    double llX,
    double llY,
    double urX,
    double urY) & noexcept
{
    m_projectedCover = {llX, llY, urX, urY};
    return *this;
}

//! Set the BAG's projected cover.
/*!
\param inReadOnly
    The new read only flag.
    \e true enables reading only.
    \e false allows writing.

\return
    The modified descriptor.
*/
Descriptor& Descriptor::setReadOnly(bool inReadOnly) & noexcept
{
    m_isReadOnly = inReadOnly;
    return *this;
}

//! Set the BAG's version as a string.
/*!
\param inVersion
    The new version as a string.

\return
    The modified descriptor.
*/
Descriptor& Descriptor::setVersion(std::string inVersion) & noexcept
{
    m_version = std::move(inVersion);
    return *this;
}

//! Set the BAG's vertical reference system.
/*!
\param verticalReferenceSystem
    The new vertical reference system as WKT.

\return
    The modified descriptor.
*/
Descriptor& Descriptor::setVerticalReferenceSystem(
    const std::string& verticalReferenceSystem) & noexcept
{
    m_verticalReferenceSystem = verticalReferenceSystem;
    return *this;
}

}  // namespace BAG

