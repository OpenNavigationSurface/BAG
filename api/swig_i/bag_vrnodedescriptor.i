%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_vrnodedescriptor

%{
#include "../bag_vrnodedescriptor.h"
%}

%import "bag_layerdescriptor.i"

%include <std_pair.i>
%include <std_shared_ptr.i>
%include <stdint.i>

%shared_ptr(BAG::VRNodeDescriptor);


namespace BAG {

class VRNodeDescriptor final : public LayerDescriptor
{
public:
    VRNodeDescriptor(const VRNodeDescriptor&) = delete;
    VRNodeDescriptor(VRNodeDescriptor&&) = delete;

    VRNodeDescriptor& operator=(const VRNodeDescriptor&) = delete;
    VRNodeDescriptor& operator=(VRNodeDescriptor&&) = delete;

    // Converted to std::pair<T, T> below.
    //std::tuple<float, float> getMinMaxHypStrength() const noexcept;
    //std::tuple<uint32_t, uint32_t> getMinMaxNSamples() const noexcept;
    //std::tuple<uint32_t, uint32_t> getMinMaxNumHypotheses() const noexcept;

    VRNodeDescriptor& setMinMaxHypStrength(float minHypStrength,
        float maxHypStrength) & noexcept;
    VRNodeDescriptor& setMinMaxNSamples(uint32_t minNSamples,
        uint32_t maxNSamples) & noexcept;
    VRNodeDescriptor& setMinMaxNumHypotheses(uint32_t minNumHypotheses,
        uint32_t maxNumHypotheses) & noexcept;
};

%extend VRNodeDescriptor
{
    std::pair<float, float> getMinMaxHypStrength() const noexcept
    {
        const auto minMax = $self->getMinMaxHypStrength();
        return {std::get<0>(minMax), std::get<1>(minMax)};
    }

    std::pair<uint32_t, uint32_t> getMinMaxNSamples() const noexcept
    {
        const auto minMax = $self->getMinMaxNSamples();
        return {std::get<0>(minMax), std::get<1>(minMax)};
    }

    std::pair<uint32_t, uint32_t> getMinMaxNumHypotheses() const noexcept
    {
        const auto minMax = $self->getMinMaxNumHypotheses();
        return {std::get<0>(minMax), std::get<1>(minMax)};
    }
}

}  // namespace BAG

