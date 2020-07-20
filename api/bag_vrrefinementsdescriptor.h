#ifndef BAG_VRREFINEMENTSDESCRIPTOR_H
#define BAG_VRREFINEMENTSDESCRIPTOR_H

#include "bag_config.h"
#include "bag_fordec.h"
#include "bag_layerdescriptor.h"


namespace BAG {

//! Describe the variable resolution refinements layer.
class BAG_API VRRefinementsDescriptor final : public LayerDescriptor
{
public:
    VRRefinementsDescriptor(const VRRefinementsDescriptor&) = delete;
    VRRefinementsDescriptor(VRRefinementsDescriptor&&) = delete;

    VRRefinementsDescriptor& operator=(const VRRefinementsDescriptor&) = delete;
    VRRefinementsDescriptor& operator=(VRRefinementsDescriptor&&) = delete;

    std::tuple<float, float> getMinMaxDepth() const noexcept;
    std::tuple<float, float> getMinMaxUncertainty() const noexcept;

    VRRefinementsDescriptor& setMinMaxDepth(float minDepth,
        float maxDepth) & noexcept;
    VRRefinementsDescriptor& setMinMaxUncertainty(float minUncertainty,
        float maxUncertainty) & noexcept;

protected:
    VRRefinementsDescriptor(uint32_t id, uint64_t chunkSize,
        int compressionLevel);
    explicit VRRefinementsDescriptor(const Dataset& dataset);

    static std::shared_ptr<VRRefinementsDescriptor> create(const Dataset& dataset,
        uint64_t chunkSize, int compressionLevel);

    static std::shared_ptr<VRRefinementsDescriptor> open(const Dataset& dataset);

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

    friend Dataset;
    friend VRRefinements;
};

}  // namespace BAG

#endif  // BAG_VRREFINEMENTSDESCRIPTOR_H

