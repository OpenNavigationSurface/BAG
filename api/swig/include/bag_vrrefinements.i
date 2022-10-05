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

#define final

%import "bag_layer.i"

%include <std_shared_ptr.i>
%shared_ptr(BAG::VRRefinements)

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

