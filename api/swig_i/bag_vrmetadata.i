%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_vrmetadata

%{
#include "../bag_vrmetadata.h"
%}

#define final

%import "../bag_config.h"
%import "bag_layer.i"

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

