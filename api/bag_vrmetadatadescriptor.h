#ifndef BAG_VRMETADATADESCRIPTOR_H
#define BAG_VRMETADATADESCRIPTOR_H

#include "bag_config.h"
#include "bag_fordec.h"
#include "bag_layerdescriptor.h"


namespace BAG {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

class BAG_API VRMetadataDescriptor final : public LayerDescriptor
{
public:
    //TODO Temp, make sure only move operations are used until development is done.
    VRMetadataDescriptor(const VRMetadataDescriptor&) = delete;
    VRMetadataDescriptor(VRMetadataDescriptor&&) = delete;
    VRMetadataDescriptor& operator=(const VRMetadataDescriptor&) = delete;
    VRMetadataDescriptor& operator=(VRMetadataDescriptor&&) = delete;

    std::tuple<uint32_t, uint32_t> getMaxDimensions() const noexcept;
    void setMaxDimensions(uint32_t maxDimX, uint32_t maxDimY) noexcept;
    std::tuple<uint32_t, uint32_t> getMinDimensions() const noexcept;
    void setMinDimensions(uint32_t minDimX, uint32_t minDimY) noexcept;
    std::tuple<float, float> getMaxResolution() const noexcept;
    void setMaxResolution(float maxResX, float maxResY) noexcept;
    std::tuple<float, float> getMinResolution() const noexcept;
    void setMinResolution(float minResX, float minResY) noexcept;

protected:
    VRMetadataDescriptor(uint32_t id, uint64_t chunkSize,
        unsigned int compressionLevel);
    explicit VRMetadataDescriptor(const Dataset& dataset);

    static std::shared_ptr<VRMetadataDescriptor> create(uint64_t chunkSize,
        unsigned int compressionLevel, const Dataset& dataset);

    static std::shared_ptr<VRMetadataDescriptor> open(const Dataset& dataset);

private:
    DataType getDataTypeProxy() const noexcept override;
    uint8_t getElementSizeProxy() const noexcept override;

    //! The minimum X dimension.
    uint32_t m_minDimX = 0;
    //! The minimum Y dimension.
    uint32_t m_minDimY = 0;
    //! The maximum X dimension.
    uint32_t m_maxDimX = 0;
    //! The maximum Y dimension.
    uint32_t m_maxDimY = 0;
    //! The minimum X resolution.
    float m_minResX = 0.f;
    //! The minimum Y resolution.
    float m_minResY = 0.f;
    //! The maximum X resolution.
    float m_maxResX = 0.f;
    //! The maximum Y resolution.
    float m_maxResY = 0.f;

    friend class Dataset;
    friend class VRMetadata;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

}  // namespace BAG

#endif  // BAG_VRMETADATADESCRIPTOR_H

