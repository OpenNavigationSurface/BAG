%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_vrmetadata

%{
#include "bag_vrmetadata.h"
%}

%import "bag_layer.i"

%include <std_shared_ptr.i>

%shared_ptr(BAG::LayerDescriptor)


namespace BAG {

class BAG_API VRMetadata final : public Layer
{
public:
    VRMetadata(const VRMetadata&) = delete;
    VRMetadata(VRMetadata&&) = delete;

    VRMetadata& operator=(const VRMetadata&) = delete;
    VRMetadata& operator=(VRMetadata&&) = delete;
};

}  // namespace BAG

