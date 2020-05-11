%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_vrrefinementsdescriptor

%{
#include "../bag_vrrefinementsdescriptor.h"
%}

%import "bag_layerdescriptor.i"

%include <std_pair.i>
%include <std_shared_ptr.i>

%shared_ptr(BAG::VRRefinementsDescriptor);


namespace BAG {

class VRRefinementsDescriptor final : public LayerDescriptor
{
public:
    VRRefinementsDescriptor(const VRRefinementsDescriptor&) = delete;
    VRRefinementsDescriptor(VRRefinementsDescriptor&&) = delete;

    VRRefinementsDescriptor& operator=(const VRRefinementsDescriptor&) = delete;
    VRRefinementsDescriptor& operator=(VRRefinementsDescriptor&&) = delete;

    // Converted to std::pair<T, T> below.
    //std::tuple<float, float> getMinMaxDepth() const noexcept;
    //std::tuple<float, float> getMinMaxUncertainty() const noexcept;

    VRRefinementsDescriptor& setMinMaxDepth(float minDepth,
        float maxDepth) & noexcept;
    VRRefinementsDescriptor& setMinMaxUncertainty(float minUncertainty,
        float maxUncertainty) & noexcept;
};

%extend VRRefinementsDescriptor
{
    std::pair<float, float> getMinMaxDepth() const noexcept
    {
        const auto minMax = $self->getMinMaxDepth();
        return {std::get<0>(minMax), std::get<1>(minMax)};
    }

    std::pair<float, float> getMinMaxUncertainty() const noexcept
    {
        const auto minMax = $self->getMinMaxUncertainty();
        return {std::get<0>(minMax), std::get<1>(minMax)};
    }
}

}  // namespace BAG

