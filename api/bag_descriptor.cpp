#include "bag_descriptor.h"
#include "bag_exceptions.h"
#include "bag_layer.h"


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

namespace BAG {

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

std::vector<LayerType> Descriptor::getLayerTypes() const
{
    std::vector<LayerType> types;
    types.reserve(m_layerDescriptors.size());

    for (auto&& layerDescriptor : m_layerDescriptors)
        types.push_back(layerDescriptor.first);

    return types;
}

const std::string& Descriptor::getVersion() const & noexcept
{
    return m_version;
}

bool Descriptor::isReadOnly() const noexcept
{
    return m_isReadOnly;
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

const std::unordered_map<LayerType, std::shared_ptr<const LayerDescriptor>>&
Descriptor::getLayerDescriptors() const & noexcept
{
    return m_layerDescriptors;
}

const LayerDescriptor& Descriptor::getLayerDescriptor(LayerType type) const &
{
    return *m_layerDescriptors.at(type);
}

}  // namespace BAG

#ifdef _MSC_VER
#pragma warning(pop)
#endif

