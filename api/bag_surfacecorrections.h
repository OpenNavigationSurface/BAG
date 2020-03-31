#ifndef BAG_SURFACECORRECTIONS_H
#define BAG_SURFACECORRECTIONS_H

#include "bag_config.h"
#include "bag_deleteh5dataset.h"
#include "bag_fordec.h"
#include "bag_layer.h"
#include "bag_types.h"

#include <memory>


namespace BAG {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)  // std classes do not have DLL-interface when exporting
#endif

//! The interface for the surface corrections layer.
class BAG_API SurfaceCorrections final : public Layer
{
public:
    SurfaceCorrections(const SurfaceCorrections&) = delete;
    SurfaceCorrections(SurfaceCorrections&&) = delete;

    SurfaceCorrections& operator=(const SurfaceCorrections&) = delete;
    SurfaceCorrections& operator=(SurfaceCorrections&&) = delete;

    UInt8Array readCorrected(uint32_t rowStart,
        uint32_t rowEnd, uint32_t columnStart, uint32_t columnEnd,
        uint8_t corrector, const SimpleLayer& layer) const;
    UInt8Array readCorrectedRow(uint32_t row,
        uint32_t columnStart, uint32_t columnEnd, uint8_t corrector,
        const SimpleLayer& layer) const;

protected:
    SurfaceCorrections(Dataset& dataset,
        SurfaceCorrectionsDescriptor& descriptor,
        std::unique_ptr<::H5::DataSet, DeleteH5dataSet> pH5dataSet);

    static std::unique_ptr<SurfaceCorrections> create(Dataset& dataset,
        BAG_SURFACE_CORRECTION_TOPOGRAPHY type, uint8_t numCorrectors,
        uint64_t chunkSize, int compressionLevel);

    static std::unique_ptr<SurfaceCorrections> open(Dataset& dataset,
        SurfaceCorrectionsDescriptor& descriptor);

private:
    static std::unique_ptr<::H5::DataSet, DeleteH5dataSet>
        createH5dataSet(const Dataset& dataset,
            const SurfaceCorrectionsDescriptor& descriptor);

    const ::H5::DataSet& getH5dataSet() const & noexcept;

    UInt8Array readProxy(uint32_t rowStart,
        uint32_t columnStart, uint32_t rowEnd, uint32_t columnEnd) const override;

    void writeProxy(uint32_t rowStart, uint32_t columnStart,
        uint32_t rowEnd, uint32_t columnEnd, const uint8_t* buffer) override;

    void writeAttributesProxy() const override;

    //! The HDF5 DataSet this class relates to.
    std::unique_ptr<::H5::DataSet, DeleteH5dataSet> m_pH5dataSet;

    friend Dataset;
    friend SurfaceCorrectionsDescriptor;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

}  // namespace BAG

#endif  // BAG_SURFACECORRECTIONS_H

