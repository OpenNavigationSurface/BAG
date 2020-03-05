#ifndef BAG_VRREFINEMENTDESCRIPTOR_H
#define BAG_VRREFINEMENTDESCRIPTOR_H

#include "bag_config.h"
#include "bag_fordec.h"
#include "bag_layerdescriptor.h"


namespace BAG {

class BAG_API VRRefinementDescriptor final : public LayerDescriptor
{
public:
    //TODO Temp, make sure only move operations are used until development is done.
    VRRefinementDescriptor(const VRRefinementDescriptor&) = delete;
    VRRefinementDescriptor(VRRefinementDescriptor&&) = delete;
    VRRefinementDescriptor& operator=(const VRRefinementDescriptor&) = delete;
    VRRefinementDescriptor& operator=(VRRefinementDescriptor&&) = delete;

    std::tuple<float, float> getMinMaxDepth() const noexcept;
    std::tuple<float, float> getMinMaxUncertainty() const noexcept;

    void setMinMaxDepth(float minDepth, float maxDepth) noexcept;
    void setMinMaxUncertainty(float minUncertainty, float maxUncertainty) noexcept;

protected:
    VRRefinementDescriptor(uint32_t id, uint64_t chunkSize,
        unsigned int compressionLevel);
    explicit VRRefinementDescriptor(const Dataset& dataset);

    static std::shared_ptr<VRRefinementDescriptor> create(const Dataset& dataset,
        uint64_t chunkSize, unsigned int compressionLevel);

    static std::shared_ptr<VRRefinementDescriptor> open(const Dataset& dataset);

private:
    DataType getDataTypeProxy() const noexcept override;
    uint8_t getElementSizeProxy() const noexcept override;

    //! The minimum depth.
    float m_minDepth = std::numeric_limits<float>::max();
    //! The maximum depth.
    float m_maxDepth = std::numeric_limits<float>::lowest();
    //! The minimum uncertainty.
    float m_minUncertainty = std::numeric_limits<float>::max();
    //! The maximum uncertainty.
    float m_maxUncertainty = std::numeric_limits<float>::lowest();

    friend class Dataset;
    friend class VRRefinement;
};

}  // namespace BAG

#endif  // BAG_VRREFINEMENTDESCRIPTOR_H

