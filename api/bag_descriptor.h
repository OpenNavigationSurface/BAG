#ifndef BAG_DESCRIPTOR_H
#define BAG_DESCRIPTOR_H

#include "bag_config.h"
#include "bag_fordec.h"
#include "bag_layerdescriptor.h"
#include "bag_types.h"

#include <exception>
#include <memory>
#include <unordered_map>


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif


namespace BAG {

class BAG_API Descriptor
{
public:
    Descriptor() = default;
    //TODO Temp, make sure only move operations are used until development is done.
    Descriptor(const Descriptor&) = delete;
    Descriptor(Descriptor&&) = delete;
    Descriptor& operator=(const Descriptor&) = delete;
    Descriptor& operator=(Descriptor&&) = default;

    std::vector<LayerType> getLayerTypes() const;

    bool isReadOnly() const noexcept;
    Descriptor& setReadOnly(bool readOnly) & noexcept;

    const std::string& getVersion() const & noexcept;
    Descriptor& setVersion(std::string inVersion) & noexcept;

    const std::unordered_map<LayerType, std::shared_ptr<const LayerDescriptor>>&
        getLayerDescriptors() const & noexcept;

    const LayerDescriptor& getLayerDescriptor(LayerType id) const &;
    Descriptor& addLayerDescriptor(const LayerDescriptor& inDescriptor) &;

private:

    std::string m_version;
    bool m_isReadOnly = true;
    std::unordered_map<LayerType, std::shared_ptr<const LayerDescriptor>> m_layerDescriptors;

};

}  // namespace BAG

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif  // BAG_DESCRIPTOR_H

