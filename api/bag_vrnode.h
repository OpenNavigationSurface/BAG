#ifndef BAG_VRNODE_H
#define BAG_VRNODE_H

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

//! Interface for the variable resolution node layer.
class BAG_API VRNode final : public Layer
{
public:
    VRNode(Dataset& dataset,
           VRNodeDescriptor& descriptor,
           std::unique_ptr<::H5::DataSet, DeleteH5dataSet> pH5dataSet);

    VRNode(const VRNode&) = delete;
    VRNode(VRNode&&) = delete;

    VRNode& operator=(const VRNode&) = delete;
    VRNode& operator=(VRNode&&) = delete;

    bool operator==(const VRNode &rhs) const noexcept {
        return m_pH5dataSet == rhs.m_pH5dataSet;
    }

    bool operator!=(const VRNode &rhs) const noexcept {
        return !(rhs == *this);
    }

protected:
    static std::shared_ptr<VRNode> create(Dataset& dataset,
        uint64_t chunkSize, int compressionLevel);

    static std::shared_ptr<VRNode> open(Dataset& dataset,
        VRNodeDescriptor& descriptor);

private:
    static std::unique_ptr<::H5::DataSet, DeleteH5dataSet>
        createH5dataSet(const Dataset& dataset,
            const VRNodeDescriptor& descriptor);

    UInt8Array readProxy(uint32_t rowStart,
        uint32_t columnStart, uint32_t rowEnd, uint32_t columnEnd) const override;

    void writeProxy(uint32_t rowStart, uint32_t columnStart, uint32_t rowEnd,
        uint32_t columnEnd, const uint8_t *buffer) override;

    void writeAttributesProxy() const override;

    //! The HDF5 DataSet this layer wraps.
    std::unique_ptr<H5::DataSet, DeleteH5dataSet> m_pH5dataSet;

    friend Dataset;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

}  // namespace BAG

#endif  // BAG_VRNODE_H

