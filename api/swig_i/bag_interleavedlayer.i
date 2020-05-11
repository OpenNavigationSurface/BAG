%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_interleavedlayer

%{
#include "../bag_interleavedlayer.h"
%}

#define final

%import "bag_layer.i"
%import "bag_types.i"

namespace BAG {
    class InterleavedLayer final : public Layer
    {
    public:
        InterleavedLayer(const InterleavedLayer&) = delete;
        InterleavedLayer(InterleavedLayer&&) = delete;
        InterleavedLayer& operator=(const InterleavedLayer&) = delete;
        InterleavedLayer& operator=(InterleavedLayer&&) = delete;
    };
}