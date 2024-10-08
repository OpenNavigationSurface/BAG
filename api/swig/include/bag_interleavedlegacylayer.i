%begin %{
#ifdef _MSC_VER
#ifdef SWIGPYTHON
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
#endif
%}

%module bag_interleavedlegacylayer

%{
#include "bag_interleavedlegacylayer.h"
%}

#define final

%import "bag_layer.i"
%import "bag_types.i"

%include <std_shared_ptr.i>
%shared_ptr(BAG::InterleavedLegacyLayer)

namespace BAG {
    class InterleavedLegacyLayer final : public Layer
    {
    public:
        InterleavedLegacyLayer(const InterleavedLegacyLayer&) = delete;
        InterleavedLegacyLayer(InterleavedLegacyLayer&&) = delete;

        InterleavedLegacyLayer& operator=(const InterleavedLegacyLayer&) = delete;
        InterleavedLegacyLayer& operator=(InterleavedLegacyLayer&&) = delete;

        bool operator==(const InterleavedLegacyLayer &rhs) const noexcept;
        bool operator!=(const InterleavedLegacyLayer &rhs) const noexcept;
    };
}