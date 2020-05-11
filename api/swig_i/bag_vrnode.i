%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_vrnode

%{
#include "../bag_vrnode.h"
%}

%import "bag_layer.i"


namespace BAG {

class VRNode final : public Layer
{
public:
    VRNode(const VRNode&) = delete;
    VRNode(VRNode&&) = delete;

    VRNode& operator=(const VRNode&) = delete;
    VRNode& operator=(VRNode&&) = delete;
};

}  // namespace BAG

