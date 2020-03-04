#ifndef BAG_DESCRIPTOR_H
#define BAG_DESCRIPTOR_H

#include "bag_config.h"
#include "bag_fordec.h"
#include "bag_layerdescriptor.h"
#include "bag_types.h"

#include <exception>
#include <memory>
#include <vector>


namespace BAG {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)  // std classes do not have DLL-interface when exporting
#endif

class BAG_API Descriptor final
{
public:
    Descriptor() = default;
    Descriptor(const Metadata& metadata);

    //TODO Temp, make sure only move operations are used until development is done.
    Descriptor(const Descriptor&) = delete;
    Descriptor(Descriptor&&) = delete;
    Descriptor& operator=(const Descriptor&) = delete;
    Descriptor& operator=(Descriptor&&) = default;

    std::vector<LayerType> getLayerTypes() const;
    bool isReadOnly() const noexcept;
    std::vector<uint32_t> getLayerIds() const noexcept;
    const std::vector<std::shared_ptr<const LayerDescriptor>>&
        getLayerDescriptors() const & noexcept;
    const LayerDescriptor& getLayerDescriptor(LayerType id) const &;
    const std::string& getVersion() const & noexcept;
    const std::string& getHorizontalReferenceSystem() const & noexcept;
    void setHorizontalReferenceSystem(
        const std::string& horizontalReferenceSystem) & noexcept;
    const std::string& getVerticalReferenceSystem() const & noexcept;
    const std::tuple<uint32_t, uint32_t>& getDims() const & noexcept;
    const std::tuple<double, double, double, double>&
        getProjectedCover() const & noexcept;
    const std::tuple<double, double>& getOrigin() const & noexcept;
    const std::tuple<double, double>& getGridSpacing() const & noexcept;

    Descriptor& addLayerDescriptor(const LayerDescriptor& inDescriptor) &;
    void setVerticalReferenceSystem(
        const std::string& verticalReferenceSystem) & noexcept;
    void setDims(uint32_t rows, uint32_t columns) & noexcept;
    void setProjectedCover(double llX, double llY, double urX,
        double urY) & noexcept;
    void setOrigin(double llX, double llY) & noexcept;
    void setGridSpacing(double xSpacing, double ySpacing) & noexcept;
    Descriptor& setReadOnly(bool readOnly) & noexcept;
    Descriptor& setVersion(std::string inVersion) & noexcept;

private:
    //! The version of the BAG.
    std::string m_version;
    //! True if the BAG is read only.
    bool m_isReadOnly = true;
    //! Layer descriptors (owned by each Layer).
    std::vector<std::shared_ptr<const LayerDescriptor>> m_layerDescriptors;
    //! The name of the horizontal reference system.
    std::string m_horizontalReferenceSystem;
    //! The name of the vertical reference system.
    std::string m_verticalReferenceSystem;
    //! The dimensions of the bag.
    std::tuple<uint32_t, uint32_t> m_dims{};
    //! The projected cover of the bag.
    std::tuple<double, double, double, double> m_projectedCover{};
    //! The origin of the bag.
    std::tuple<double, double> m_origin{};
    //! The grid spacing of the bag.
    std::tuple<double, double> m_gridSpacing{};

};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

}  // namespace BAG

#endif  // BAG_DESCRIPTOR_H

