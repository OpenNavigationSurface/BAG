%begin %{
#ifdef _MSC_VER
#ifdef SWIGPYTHON
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
#endif
%}

%module bag_vrnode

%{
#include "bag_vrnode.h"
%}

%import "bag_layer.i"

%include <std_shared_ptr.i>
%shared_ptr(BAG::VRNode)

namespace BAG {

class VRNodeDescriptor;

class VRNode final : public Layer
{
public:
    VRNode(const VRNode&) = delete;
    VRNode(VRNode&&) = delete;

    VRNode& operator=(const VRNode&) = delete;
    VRNode& operator=(VRNode&&) = delete;

    bool operator==(const VRNode &rhs) const noexcept;
    bool operator!=(const VRNode &rhs) const noexcept;

    std::shared_ptr<VRNodeDescriptor> getDescriptor() & noexcept;
    std::shared_ptr<const VRNodeDescriptor> getDescriptor() const & noexcept;
};

}  // namespace BAG

