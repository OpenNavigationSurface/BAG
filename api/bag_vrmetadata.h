#ifndef BAG_VRMETADATA_H
#define BAG_VRMETADATA_H

#include "bag_config.h"
#include "bag_deleteh5dataset.h"
#include "bag_fordec.h"
#include "bag_layer.h"

#include <memory>


namespace BAG {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)  // std classes do not have DLL-interface when exporting
#endif

//! The interface for variable resolution metadata.
class BAG_API VRMetadata final : public Layer
{
public:
    VRMetadata(Dataset& dataset,
               VRMetadataDescriptor& descriptor,
               std::unique_ptr<::H5::DataSet, DeleteH5dataSet> pH5dataSet);

    VRMetadata(const VRMetadata&) = delete;
    VRMetadata(VRMetadata&&) = delete;

    VRMetadata& operator=(const VRMetadata&) = delete;
    VRMetadata& operator=(VRMetadata&&) = delete;

    bool operator==(const VRMetadata &rhs) const noexcept {
        return m_pH5dataSet == rhs.m_pH5dataSet;
    }

    bool operator!=(const VRMetadata &rhs) const noexcept {
        return !(rhs == *this);
    }

    std::shared_ptr<VRMetadataDescriptor> getDescriptor() & noexcept;
    std::shared_ptr<const VRMetadataDescriptor> getDescriptor() const & noexcept;

protected:
    static std::shared_ptr<VRMetadata> create(Dataset& dataset,
        uint64_t chunkSize, int compressionLevel);

    static std::shared_ptr<VRMetadata> open(Dataset& dataset,
        VRMetadataDescriptor& descriptor);

private:
    static std::unique_ptr<::H5::DataSet, DeleteH5dataSet>
        createH5dataSet(const Dataset& dataset,
            const VRMetadataDescriptor& descriptor);

    UInt8Array readProxy(uint32_t rowStart,
        uint32_t columnStart, uint32_t rowEnd, uint32_t columnEnd) const override;

    void writeProxy(uint32_t rowStart, uint32_t columnStart, uint32_t rowEnd,
        uint32_t columnEnd, const uint8_t *buffer) override;

    void writeAttributesProxy() const override;

    //! The HDF5 DataSet the metadata wraps.
    std::unique_ptr<::H5::DataSet, DeleteH5dataSet> m_pH5dataSet;

    friend Dataset;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

}  // namespace BAG

#endif  // BAG_VRMETADATA_H

