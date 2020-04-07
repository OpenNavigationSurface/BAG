%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_vrmetadatadescriptor

%{
#include "../bag_vrmetadatadescriptor.h"
%}

#define final

%include <std_shared_ptr.i>

%shared_ptr(BAG::VRMetadataDescriptor)

%include <stdint.i>

%import "bag_layerdescriptor.i"
%import "bag_types.i"


namespace BAG {

class BAG_API VRMetadataDescriptor final : public LayerDescriptor
{
public:
    VRMetadataDescriptor(const VRMetadataDescriptor&) = delete;
    VRMetadataDescriptor(VRMetadataDescriptor&&) = delete;

    VRMetadataDescriptor& operator=(const VRMetadataDescriptor&) = delete;
    VRMetadataDescriptor& operator=(VRMetadataDescriptor&&) = delete;

    // Converted to std::pair<T, T> below.
    //std::tuple<uint32_t, uint32_t> getMaxDimensions() const noexcept;
    //std::tuple<float, float> getMaxResolution() const noexcept;
    //std::tuple<uint32_t, uint32_t> getMinDimensions() const noexcept;
    //std::tuple<float, float> getMinResolution() const noexcept;

    VRMetadataDescriptor& setMaxDimensions(uint32_t maxDimX, uint32_t maxDimY) & noexcept;
    VRMetadataDescriptor& setMaxResolution(float maxResX, float maxResY) & noexcept;

    VRMetadataDescriptor& setMinDimensions(uint32_t minDimX, uint32_t minDimY) & noexcept;
    VRMetadataDescriptor& setMinResolution(float minResX, float minResY) & noexcept;
};

%extend VRMetadataDescriptor
{

    std::pair<uint32_t, uint32_t> getMaxDimensions() const noexcept
    {
        const auto dims = $self->getMaxDimensions();
        return {std::get<0>(dims), std::get<1>(dims)};
    }

    std::pair<float, float> getMaxResolution() const noexcept
    {
        const auto res = $self->getMaxResolution();
        return {std::get<0>(res), std::get<1>(res)};
    }

    std::pair<uint32_t, uint32_t> getMinDimensions() const noexcept
    {
        const auto dims = $self->getMinDimensions();
        return {std::get<0>(dims), std::get<1>(dims)};
    }

    std::pair<float, float> getMinResolution() const noexcept
    {
        const auto res = $self->getMinResolution();
        return {std::get<0>(res), std::get<1>(res)};
    }
}

}  // namespace BAG

