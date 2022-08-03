%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_surfacecorrections

%{
#include "bag_surfacecorrections.h"
%}

#define final

%import "bag_layer.i"
%import "bag_layeritems.i"
%import "bag_simplelayer.i"
%import "bag_types.i"
%import "bag_uint8array.i"

%include <stdint.i>


namespace BAG {

class SurfaceCorrections final : public Layer
{
public:
    SurfaceCorrections(const SurfaceCorrections&) = delete;
    SurfaceCorrections(SurfaceCorrections&&) = delete;

    SurfaceCorrections& operator=(const SurfaceCorrections&) = delete;
    SurfaceCorrections& operator=(SurfaceCorrections&&) = delete;

    bool operator==(const SurfaceCorrections &rhs) const noexcept;
    bool operator!=(const SurfaceCorrections &rhs) const noexcept;
};

%extend SurfaceCorrections
{
    LayerItems readCorrected(
        uint32_t rowStart,
        uint32_t columnStart,
        uint32_t rowEnd,
        uint32_t columnEnd,
        uint8_t corrector,
        const SimpleLayer& layer) const
    {
        return BAG::LayerItems{$self->readCorrected(rowStart, columnStart,
            rowEnd, columnEnd, corrector, layer)};
    }

    LayerItems readCorrectedRow(
        uint32_t row,
        uint32_t columnStart,
        uint32_t columnEnd,
        uint8_t corrector,
        const SimpleLayer& layer) const
    {
        return BAG::LayerItems{$self->readCorrectedRow(row, columnStart,
            columnEnd, corrector, layer)};
    }

}

}  // namespace BAG

