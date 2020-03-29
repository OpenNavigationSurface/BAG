#ifndef BAG_SURFACECORRECTIONSDESCRIPTOR_H
#define BAG_SURFACECORRECTIONSDESCRIPTOR_H

#include "bag_config.h"
#include "bag_fordec.h"
#include "bag_layerdescriptor.h"
#include "bag_types.h"

#include <memory>


namespace BAG {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)  // std classes do not have DLL-interface when exporting
#endif

class BAG_API SurfaceCorrectionsDescriptor final : public LayerDescriptor
{
public:
    static std::shared_ptr<SurfaceCorrectionsDescriptor> create(
        const Dataset& dataset, BAG_SURFACE_CORRECTION_TOPOGRAPHY type,
        uint8_t numCorrections, uint64_t chunkSize, int compressionLevel);

    static std::shared_ptr<SurfaceCorrectionsDescriptor> open(
        const Dataset& dataset);

    SurfaceCorrectionsDescriptor(const SurfaceCorrectionsDescriptor&) = delete;
    SurfaceCorrectionsDescriptor(SurfaceCorrectionsDescriptor&&) = delete;

    SurfaceCorrectionsDescriptor& operator=(const SurfaceCorrectionsDescriptor&) = delete;
    SurfaceCorrectionsDescriptor& operator=(SurfaceCorrectionsDescriptor&&) = delete;

    std::tuple<uint32_t, uint32_t> getDims() const noexcept;
    uint8_t getNumCorrectors() const noexcept;
    std::tuple<double, double> getOrigin() const noexcept;
    std::tuple<double, double> getSpacing() const noexcept;
    BAG_SURFACE_CORRECTION_TOPOGRAPHY getSurfaceType() const noexcept;
    const std::string& getVerticalDatums() const & noexcept;

    SurfaceCorrectionsDescriptor& setDims(uint32_t numRows,
        uint32_t numColumns) & noexcept;
    SurfaceCorrectionsDescriptor& setVerticalDatums(
        std::string verticalDatums) & noexcept;
    SurfaceCorrectionsDescriptor& setOrigin(double swX,
        double swY) & noexcept;
    SurfaceCorrectionsDescriptor& setSpacing(double xSpacing,
        double ySpacing) & noexcept;

protected:
    SurfaceCorrectionsDescriptor(uint32_t id,
        BAG_SURFACE_CORRECTION_TOPOGRAPHY type, uint8_t numCorrectors,
        uint64_t chunkSize, int compressionLevel);
    explicit SurfaceCorrectionsDescriptor(const Dataset& dataset);

private:
    DataType getDataTypeProxy() const noexcept override;
    uint8_t getElementSizeProxy() const noexcept override;

    //! The type of surface this correction applies to.
    BAG_SURFACE_CORRECTION_TOPOGRAPHY m_surfaceType = BAG_SURFACE_UNKNOWN;
    //! The size of a single "element" in the hdf5 file.
    uint8_t m_elementSize = 0;
    //! Number of correction values.
    uint8_t m_numCorrectors = 0;
    //! The name of the vertical datum(s).
    std::string m_verticalDatums;
    //! South-West corner X's value.
    double m_swX = 0.;
    //! South-West corner Y's value.
    double m_swY = 0.;
    //! Node spacing X.
    double m_xSpacing = 0.;
    //! Node spacing Y.
    double m_ySpacing = 0.;
    //! Number of rows.
    uint32_t m_numRows = 0;
    //! Number of columns.
    uint32_t m_numColumns = 0;

};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

}  // namespace BAG

#endif  // BAG_SURFACECORRECTIONSDESCRIPTOR_H

