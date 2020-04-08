%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_surfacecorrections

%{
#include "../bag_surfacecorrections.h"
%}

#define final

%import "../bag_config.h"
%import "bag_layer.i"
%import "bag_types.i"
%import "bag_uint8array.i"

namespace BAG {

    class BAG_API SurfaceCorrections final : public Layer
    {
    public:
        SurfaceCorrections(const SurfaceCorrections&) = delete;
        SurfaceCorrections(SurfaceCorrections&&) = delete;
        SurfaceCorrections& operator=(const SurfaceCorrections&) = delete;
        SurfaceCorrections& operator=(SurfaceCorrections&&) = delete;
    
        UInt8Array readCorrected(uint32_t rowStart,
            uint32_t columnStart, uint32_t rowEnd, uint32_t columnEnd,
            uint8_t corrector, const SimpleLayer& layer) const;
        UInt8Array readCorrectedRow(uint32_t row,
            uint32_t columnStart, uint32_t columnEnd, uint8_t corrector,
            const SimpleLayer& layer) const;
    };
}