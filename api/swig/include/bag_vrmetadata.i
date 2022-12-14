%begin %{
#ifdef _MSC_VER
#ifdef SWIGPYTHON
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
#endif
%}

%module bag_vrmetadata

%{
#include "bag_vrmetadata.h"
%}

%import "bag_layer.i"

%include <std_shared_ptr.i>
%shared_ptr(BAG::VRMetadata)


namespace BAG {

class VRMetadataDescriptor;

class BAG_API VRMetadata final : public Layer
{
public:
    VRMetadata(const VRMetadata&) = delete;
    VRMetadata(VRMetadata&&) = delete;

    VRMetadata& operator=(const VRMetadata&) = delete;
    VRMetadata& operator=(VRMetadata&&) = delete;

    bool operator==(const VRMetadata &rhs) const noexcept;
    bool operator!=(const VRMetadata &rhs) const noexcept;

    std::shared_ptr<VRMetadataDescriptor> getDescriptor() & noexcept;
    std::shared_ptr<const VRMetadataDescriptor> getDescriptor() const & noexcept;
};

}  // namespace BAG

