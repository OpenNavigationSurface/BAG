#ifndef BAG_INTERLEAVEDLAYER
#define BAG_INTERLEAVEDLAYER

#include "bag_config.h"
#include "bag_deleteh5dataset.h"
#include "bag_fordec.h"
#include "bag_layer.h"
#include "bag_types.h"

#include <memory>


namespace H5 {

class DataSet;

}  // namespace H5

namespace BAG {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)  // std classes do not have DLL-interface when exporting
#endif

//! The interface for an interleaved layer.
/*!
    This class is only here to support older (pre 2.0) BAGs that have an optional
    layer of NODE or ELEVATION group type.
    In 2.0, these "interleaved" layers are split up into layers of each type in
    the group.
    The NODE group is made of Hypothesis_Strength and Num_Hypotheses.
    The ELEVATION group is made of Shoal_Elevation, Std_Dev and Num_Soundings.
*/
class BAG_API InterleavedLegacyLayer final : public Layer
{
public:
    InterleavedLegacyLayer(Dataset& dataset,
                           InterleavedLegacyLayerDescriptor& descriptor,
                           std::unique_ptr<::H5::DataSet, DeleteH5dataSet> h5dataSet);

    InterleavedLegacyLayer(const InterleavedLegacyLayer&) = delete;
    InterleavedLegacyLayer(InterleavedLegacyLayer&&) = delete;

    InterleavedLegacyLayer& operator=(const InterleavedLegacyLayer&) = delete;
    InterleavedLegacyLayer& operator=(InterleavedLegacyLayer&&) = delete;

    bool operator==(const InterleavedLegacyLayer &rhs) const noexcept {
        return m_pH5dataSet == rhs.m_pH5dataSet;
    }

    bool operator!=(const InterleavedLegacyLayer &rhs) const noexcept {
        return !(rhs == *this);
    }

protected:
    static std::shared_ptr<InterleavedLegacyLayer> open(Dataset& dataset,
        InterleavedLegacyLayerDescriptor& descriptor);

private:
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

#endif  // BAG_INTERLEAVEDLAYER

