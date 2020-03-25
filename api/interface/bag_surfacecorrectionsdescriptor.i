%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_surfacecorrectionsdescriptor

%include <std_shared_ptr.i>
%shared_ptr(BAG::SurfaceCorrectionsDescriptor)

%{
#include "bag_surfacecorrectionsdescriptor.h"
%}

#define final

%import "bag_layerdescriptor.i"

namespace BAG {

class BAG_API SurfaceCorrectionsDescriptor final : public LayerDescriptor
{
public:
    static std::shared_ptr<SurfaceCorrectionsDescriptor> create(
        BAG_SURFACE_CORRECTION_TOPOGRAPHY type, uint8_t numCorrections,
        uint64_t chunkSize, unsigned int compressionLevel,
        const Dataset& dataset);

    %rename(openDataset) open(const Dataset&);
    static std::shared_ptr<SurfaceCorrectionsDescriptor> open(
        const Dataset& dataset);

    //TODO Temp, make sure only move operations are used until development is done.
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

    SurfaceCorrectionsDescriptor& setDims(uint32_t numRws,
        uint32_t numColumns) & noexcept;
    SurfaceCorrectionsDescriptor& setVerticalDatum(
        std::string verticalDatums) & noexcept;
    SurfaceCorrectionsDescriptor& setOrigin(double swX,
        double swY) & noexcept;
    SurfaceCorrectionsDescriptor& setSpacing(double xSpacing,
        double ySpacing) & noexcept;
};
}
