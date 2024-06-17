#ifndef BAG_VRNODEDESCRIPTOR_H
#define BAG_VRNODEDESCRIPTOR_H

#include "bag_config.h"
#include "bag_fordec.h"
#include "bag_layerdescriptor.h"


namespace BAG {

//! Describe the variable resolution node.
class BAG_API VRNodeDescriptor final : public LayerDescriptor
{
public:
    VRNodeDescriptor(const VRNodeDescriptor&) = delete;
    VRNodeDescriptor(VRNodeDescriptor&&) = delete;

    VRNodeDescriptor& operator=(const VRNodeDescriptor&) = delete;
    VRNodeDescriptor& operator=(VRNodeDescriptor&&) = delete;

    bool operator==(const VRNodeDescriptor &rhs) const noexcept {
        return m_minHypStrength == rhs.m_minHypStrength &&
               m_maxHypStrength == rhs.m_maxHypStrength &&
               m_minNumHypotheses == rhs.m_minNumHypotheses &&
               m_maxNumHypotheses == rhs.m_maxNumHypotheses &&
               m_minNSamples == rhs.m_minNSamples &&
               m_maxNSamples == rhs.m_maxNSamples;
    }

    bool operator!=(const VRNodeDescriptor &rhs) const noexcept {
        return !(rhs == *this);
    }

    std::tuple<float, float> getMinMaxHypStrength() const noexcept;
    std::tuple<uint32_t, uint32_t> getMinMaxNSamples() const noexcept;
    std::tuple<uint32_t, uint32_t> getMinMaxNumHypotheses() const noexcept;

    VRNodeDescriptor& setMinMaxHypStrength(float minHypStrength,
        float maxHypStrength) & noexcept;
    VRNodeDescriptor& setMinMaxNSamples(uint32_t minNSamples,
        uint32_t maxNSamples) & noexcept;
    VRNodeDescriptor& setMinMaxNumHypotheses(uint32_t minNumHypotheses,
        uint32_t maxNumHypotheses) & noexcept;

protected:
    VRNodeDescriptor(uint32_t id, uint32_t rows, uint32_t cols, uint64_t chunkSize,
        int compressionLevel);
    explicit VRNodeDescriptor(const Dataset& dataset, uint32_t rows, uint32_t cols);

    static std::shared_ptr<VRNodeDescriptor> create(const Dataset& dataset,
        uint64_t chunkSize, int compressionLevel);

    static std::shared_ptr<VRNodeDescriptor> open(const Dataset& dataset,
        uint32_t rows, uint32_t cols);

private:
    DataType getDataTypeProxy() const noexcept override;
    uint8_t getElementSizeProxy() const noexcept override;

    //! The minimum hypotheses strength.
    float m_minHypStrength = std::numeric_limits<float>::max();
    //! The maximum hypotheses strength.
    float m_maxHypStrength = std::numeric_limits<float>::lowest();
    //! The minimum number of hypotheses.
    uint32_t m_minNumHypotheses = std::numeric_limits<uint32_t>::max();
    //! The maximum number of hypotheses.
    uint32_t m_maxNumHypotheses = std::numeric_limits<uint32_t>::lowest();
    //! The minimum number of samples..
    uint32_t m_minNSamples = std::numeric_limits<uint32_t>::max();
    //! The maximum number of samples.
    uint32_t m_maxNSamples = std::numeric_limits<uint32_t>::lowest();

    friend Dataset;
    friend VRNode;
};

}  // namespace BAG

#endif  // BAG_VRNODEDESCRIPTOR_H

