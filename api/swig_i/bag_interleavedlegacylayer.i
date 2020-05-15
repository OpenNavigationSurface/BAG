%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_interleavedlegacylayer

%{
#include "../bag_interleavedlegacylayer.h"
%}

#define final

%import "bag_layer.i"
%import "bag_types.i"

namespace BAG {
    class InterleavedLegacyLayer final : public Layer
    {
    public:
        InterleavedLegacyLayer(const InterleavedLegacyLayer&) = delete;
        InterleavedLegacyLayer(InterleavedLegacyLayer&&) = delete;

        InterleavedLegacyLayer& operator=(const InterleavedLegacyLayer&) = delete;
        InterleavedLegacyLayer& operator=(InterleavedLegacyLayer&&) = delete;
    };
}