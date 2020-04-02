%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_simplelayer

%{
#include "../bag_simplelayer.h"
%}

#define final

%import "../bag_config.h"
%import "bag_layer.i"
%import "bag_types.i"

namespace BAG {
    class BAG_API SimpleLayer final : public Layer
    {
    public:
        //TODO Temp, make sure only move operations are used until development is done.
        SimpleLayer(const SimpleLayer&) = delete;
        SimpleLayer(SimpleLayer&&) = delete;
        SimpleLayer& operator=(const SimpleLayer&) = delete;
        SimpleLayer& operator=(SimpleLayer&&) = delete;
    };
}