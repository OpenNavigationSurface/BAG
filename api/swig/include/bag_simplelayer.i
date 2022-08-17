%begin %{
#ifdef _MSC_VER
#ifdef SWIGPYTHON
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
#endif
%}

%module bag_simplelayer

%{
#include "bag_simplelayer.h"
%}

#define final

%import "bag_layer.i"
%import "bag_types.i"

namespace BAG {
    class SimpleLayer final : public Layer
    {
    public:
        SimpleLayer(const SimpleLayer&) = delete;
        SimpleLayer(SimpleLayer&&) = delete;
        SimpleLayer& operator=(const SimpleLayer&) = delete;
        SimpleLayer& operator=(SimpleLayer&&) = delete;
        bool operator==(const SimpleLayer &rhs) const noexcept;
        bool operator!=(const SimpleLayer &rhs) const noexcept;
    };
}