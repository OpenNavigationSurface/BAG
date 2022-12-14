#ifndef BAG_VRMETADATADESCRIPTOR_H
#define BAG_VRMETADATADESCRIPTOR_H

#include "bag_config.h"
#include "bag_fordec.h"
#include "bag_layerdescriptor.h"


namespace BAG {

//! Describe the variable resolution metadata.
class BAG_API VRMetadataDescriptor final : public LayerDescriptor
{
public:
    VRMetadataDescriptor(const VRMetadataDescriptor&) = delete;
    VRMetadataDescriptor(VRMetadataDescriptor&&) = delete;

    VRMetadataDescriptor& operator=(const VRMetadataDescriptor&) = delete;
    VRMetadataDescriptor& operator=(VRMetadataDescriptor&&) = delete;

    bool operator==(const VRMetadataDescriptor &rhs) const noexcept {
        return m_minDimX == rhs.m_minDimX &&
               m_minDimY == rhs.m_minDimY &&
               m_maxDimX == rhs.m_maxDimX &&
               m_maxDimY == rhs.m_maxDimY &&
               m_minResX == rhs.m_minResX &&
               m_minResY == rhs.m_minResY &&
               m_maxResX == rhs.m_maxResX &&
               m_maxResY == rhs.m_maxResY;
    }

    bool operator!=(const VRMetadataDescriptor &rhs) const noexcept {
        return !(rhs == *this);
    }

    std::tuple<uint32_t, uint32_t> getMaxDimensions() const noexcept;
    std::tuple<float, float> getMaxResolution() const noexcept;
    std::tuple<uint32_t, uint32_t> getMinDimensions() const noexcept;
    std::tuple<float, float> getMinResolution() const noexcept;

    VRMetadataDescriptor& setMaxDimensions(uint32_t maxDimX, uint32_t maxDimY) & noexcept;
    VRMetadataDescriptor& setMaxResolution(float maxResX, float maxResY) & noexcept;
    VRMetadataDescriptor& setMinDimensions(uint32_t minDimX, uint32_t minDimY) & noexcept;
    VRMetadataDescriptor& setMinResolution(float minResX, float minResY) & noexcept;

protected:
    VRMetadataDescriptor(uint32_t id, uint64_t chunkSize,
        int compressionLevel);
    explicit VRMetadataDescriptor(const Dataset& dataset);

    static std::shared_ptr<VRMetadataDescriptor> create(const Dataset& dataset,
        uint64_t chunkSize, int compressionLevel);

    static std::shared_ptr<VRMetadataDescriptor> open(const Dataset& dataset);

private:
    DataType getDataTypeProxy() const noexcept override;
    uint8_t getElementSizeProxy() const noexcept override;

    //! The minimum X dimension.
    uint32_t m_minDimX = std::numeric_limits<uint32_t>::max();
    //! The minimum Y dimension.
    uint32_t m_minDimY = std::numeric_limits<uint32_t>::max();
    //! The maximum X dimension.
    uint32_t m_maxDimX = std::numeric_limits<uint32_t>::lowest();
    //! The maximum Y dimension.
    uint32_t m_maxDimY = std::numeric_limits<uint32_t>::lowest();
    //! The minimum X resolution.
    float m_minResX = std::numeric_limits<float>::max();
    //! The minimum Y resolution.
    float m_minResY = std::numeric_limits<float>::max();
    //! The maximum X resolution.
    float m_maxResX = std::numeric_limits<float>::lowest();
    //! The maximum Y resolution.
    float m_maxResY = std::numeric_limits<float>::lowest();;

    friend Dataset;
    friend VRMetadata;
};

}  // namespace BAG

#endif  // BAG_VRMETADATADESCRIPTOR_H

