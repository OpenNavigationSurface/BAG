#ifndef BAG_VRREFINEMENTS_H
#define BAG_VRREFINEMENTS_H

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

//! The interface for the variable resolution refinements layer.
class BAG_API VRRefinements final : public Layer
{
public:
    VRRefinements(const VRRefinements&) = delete;
    VRRefinements(VRRefinements&&) = delete;

    VRRefinements& operator=(const VRRefinements&) = delete;
    VRRefinements& operator=(VRRefinements&&) = delete;

    bool operator==(const VRRefinements &rhs) const noexcept {
        return m_pH5dataSet == rhs.m_pH5dataSet;
    }

    bool operator!=(const VRRefinements &rhs) const noexcept {
        return !(rhs == *this);
    }

    std::shared_ptr<VRRefinementsDescriptor> getDescriptor() & noexcept;
    std::shared_ptr<const VRRefinementsDescriptor> getDescriptor() const & noexcept;

protected:
    VRRefinements(Dataset& dataset,
        VRRefinementsDescriptor& descriptor,
        std::unique_ptr<::H5::DataSet, DeleteH5dataSet> h5dataSet);

    static std::unique_ptr<VRRefinements> create(Dataset& dataset,
        uint64_t chunkSize, int compressionLevel);

    static std::unique_ptr<VRRefinements> open(Dataset& dataset,
        VRRefinementsDescriptor& descriptor);

private:
    static std::unique_ptr<::H5::DataSet, DeleteH5dataSet>
        createH5dataSet(const Dataset& dataset,
            const VRRefinementsDescriptor& descriptor);

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

#endif  // BAG_VRREFINEMENTS_H

