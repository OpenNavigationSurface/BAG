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
%import "../bag_config.h"
%import "bag_layer.i"

namespace BAG {
class BAG_API InterleavedLayer final : public Layer
{
public:
    //TODO Temp, make sure only move operations are used until development is done.
    InterleavedLayer(const InterleavedLayer&) = delete;
    InterleavedLayer(InterleavedLayer&&) = delete;
    InterleavedLayer& operator=(const InterleavedLayer&) = delete;
    InterleavedLayer& operator=(InterleavedLayer&&) = delete;
};
}