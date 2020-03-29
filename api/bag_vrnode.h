#ifndef BAG_VRNODE_H
#define BAG_VRNODE_H

#include "bag_config.h"
#include "bag_fordec.h"
#include "bag_layer.h"

#include <memory>


namespace BAG {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)  // std classes do not have DLL-interface when exporting
#endif

class BAG_API VRNode final : public Layer
{
public:
    //TODO Temp, make sure only move operations are used until development is done.
    VRNode(const VRNode&) = delete;
    VRNode(VRNode&&) = delete;
    VRNode& operator=(const VRNode&) = delete;
    VRNode& operator=(VRNode&&) = delete;

protected:
    //! Custom deleter to avoid needing a definition for ::H5::DataSet::~DataSet().
    struct BAG_API DeleteH5dataSet final
    {
        void operator()(::H5::DataSet* ptr) noexcept;
    };

    VRNode(Dataset& dataset,
        VRNodeDescriptor& descriptor,
        std::unique_ptr<::H5::DataSet, DeleteH5dataSet> h5dataSet);

    static std::unique_ptr<VRNode> create(Dataset& dataset,
        uint64_t chunkSize, unsigned int compressionLevel);

    static std::unique_ptr<VRNode> open(Dataset& dataset,
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

    //! The HDF5 DataSet.
    std::unique_ptr<H5::DataSet, DeleteH5dataSet> m_pH5dataSet;

    friend Dataset;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

}  // namespace BAG

#endif  // BAG_VRNODE_H

