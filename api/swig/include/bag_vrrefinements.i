%begin %{
#ifdef _MSC_VER
#ifdef SWIGPYTHON
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
#endif
%}

%module bag_vrrefinements

%{
#include "bag_vrrefinements.h"
%}

%import "bag_layer.i"


namespace BAG {

class VRRefinements final : public Layer
{
public:
    VRRefinements(const VRRefinements&) = delete;
    VRRefinements(VRRefinements&&) = delete;

    VRRefinements& operator=(const VRRefinements&) = delete;
    VRRefinements& operator=(VRRefinements&&) = delete;

    bool operator==(const VRRefinements &rhs) const noexcept;
    bool operator!=(const VRRefinements &rhs) const noexcept;
};

}  // namespace BAG

