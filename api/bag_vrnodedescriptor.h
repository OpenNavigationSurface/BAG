#ifndef BAG_VRNODEDESCRIPTOR_H
#define BAG_VRNODEDESCRIPTOR_H

#include "bag_config.h"
#include "bag_fordec.h"
#include "bag_layerdescriptor.h"


namespace BAG {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

class BAG_API VRNodeDescriptor final : public LayerDescriptor
{
public:
    //TODO Temp, make sure only move operations are used until development is done.
    VRNodeDescriptor(const VRNodeDescriptor&) = delete;
    VRNodeDescriptor(VRNodeDescriptor&&) = delete;
    VRNodeDescriptor& operator=(const VRNodeDescriptor&) = delete;
    VRNodeDescriptor& operator=(VRNodeDescriptor&&) = delete;

    std::tuple<float, float> getMinMaxHypStrength() const noexcept;
    void setMinMaxHypStrength(float minDepth, float maxDepth) noexcept;
    std::tuple<uint32_t, uint32_t> getMinMaxNumHypotheses() const noexcept;
    void setMinMaxNumHypotheses(uint32_t minNumHypotheses, uint32_t maxNumHypotheses) noexcept;
    std::tuple<uint32_t, uint32_t> getMinMaxNSamples() const noexcept;
    void setMinMaxNSamples(uint32_t minNSamples, uint32_t maxNSamples) noexcept;

protected:
    VRNodeDescriptor(uint32_t id, uint64_t chunkSize,
        unsigned int compressionLevel);
    explicit VRNodeDescriptor(const Dataset& dataset);

    static std::shared_ptr<VRNodeDescriptor> create(uint64_t chunkSize,
        unsigned int compressionLevel, const Dataset& dataset);

    static std::shared_ptr<VRNodeDescriptor> open(const Dataset& dataset);

private:
    DataType getDataTypeProxy() const noexcept override;
    uint8_t getElementSizeProxy() const noexcept override;

    //! The minimum hyp strength.
    float m_minHypStrength = std::numeric_limits<float>::max();
    //! The maximum hyp strength.
    float m_maxHypStrength = std::numeric_limits<float>::lowest();
    //! The minimum uncertainty.
    uint32_t m_minNumHypotheses = std::numeric_limits<uint32_t>::max();
    //! The maximum uncertainty.
    uint32_t m_maxNumHypotheses = std::numeric_limits<uint32_t>::lowest();
    //! The minimum uncertainty.
    uint32_t m_minNSamples = std::numeric_limits<uint32_t>::max();
    //! The maximum uncertainty.
    uint32_t m_maxNSamples = std::numeric_limits<uint32_t>::lowest();

    friend class Dataset;
    friend class VRNode;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

}  // namespace BAG

#endif  // BAG_VRNODEDESCRIPTOR_H

