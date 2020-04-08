%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_surfacecorrectionsdescriptor

%{
#include "../bag_surfacecorrectionsdescriptor.h"
%}

#define final

%include <std_string.i>
%include <std_shared_ptr.i>
%shared_ptr(BAG::SurfaceCorrectionsDescriptor)

%import "bag_layerdescriptor.i"
%import "bag_types.i"

namespace BAG {

class BAG_API SurfaceCorrectionsDescriptor final : public LayerDescriptor
{
public:
    static std::shared_ptr<SurfaceCorrectionsDescriptor> create(
        const Dataset& dataset, BAG_SURFACE_CORRECTION_TOPOGRAPHY type,
        uint8_t numCorrections, uint64_t chunkSize,
        unsigned int compressionLevel);

    %rename(openDataset) open(const Dataset&);
    static std::shared_ptr<SurfaceCorrectionsDescriptor> open(
        const Dataset& dataset);

    SurfaceCorrectionsDescriptor(const SurfaceCorrectionsDescriptor&) = delete;
    SurfaceCorrectionsDescriptor(SurfaceCorrectionsDescriptor&&) = delete;

    SurfaceCorrectionsDescriptor& operator=(const SurfaceCorrectionsDescriptor&) = delete;
    SurfaceCorrectionsDescriptor& operator=(SurfaceCorrectionsDescriptor&&) = delete;

    uint8_t getNumCorrectors() const noexcept;

#if 0
    //! Intentionally omit exposing of std::tuple method (unsupported by SWIG), 
    //! so it can be exposed with std::pair below.
    std::tuple<uint32_t, uint32_t> getDims() const noexcept;
    std::tuple<double, double> getOrigin() const noexcept;
    std::tuple<double, double> getSpacing() const noexcept;
#endif

    BAG_SURFACE_CORRECTION_TOPOGRAPHY getSurfaceType() const noexcept;
    const std::string& getVerticalDatums() const & noexcept;

    SurfaceCorrectionsDescriptor& setDims(uint32_t numRws,
        uint32_t numColumns) & noexcept;
    SurfaceCorrectionsDescriptor& setOrigin(double swX, double swY) & noexcept;
    SurfaceCorrectionsDescriptor& setSpacing(double xSpacing,
        double ySpacing) & noexcept;
    SurfaceCorrectionsDescriptor& setVerticalDatums(
        std::string verticalDatums) & noexcept;
};
}

%extend BAG::SurfaceCorrectionsDescriptor
{
    const std::pair<uint32_t, uint32_t> getDims() const & noexcept
    {
        uint32_t row=0.0, column=0.0;
        std::tie(row, column) = self->getDims();
        return std::pair<uint32_t, uint32_t>(row, column);
    }

    const std::pair<double, double> getOrigin() const & noexcept
    {
        double x=0.0, y=0.0;
        std::tie(x, y) = self->getOrigin();
        return std::pair<double, double>(x, y);
    }

    const std::pair<double, double> getSpacing() const & noexcept
    {
        double x=0.0, y=0.0;
        std::tie(x, y) = self->getSpacing();
        return std::pair<double, double>(x, y);
    }
}