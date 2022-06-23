%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_layerdescriptor

%{
#include "bag_layerdescriptor.h"
%}

%import "bag_types.i"

%include <std_shared_ptr.i>
%include <std_string.i>

%shared_ptr(BAG::LayerDescriptor)


namespace BAG {

class LayerDescriptor
{
public:
    LayerDescriptor(const LayerDescriptor&) = delete;
    LayerDescriptor(LayerDescriptor&&) = delete;
    virtual ~LayerDescriptor() = default;

    LayerDescriptor& operator=(const LayerDescriptor&) = delete;
    LayerDescriptor& operator=(LayerDescriptor&&) = delete;

    uint64_t getChunkSize() const noexcept;
    int getCompressionLevel() const noexcept;
    DataType getDataType() const noexcept;
    uint8_t getElementSize() const noexcept;
    uint32_t getId() const noexcept;
    const std::string& getInternalPath() const & noexcept;
    LayerType getLayerType() const noexcept;

#if 0
    //! Intentionally omit exposing of std::tuple method (unsupported by SWIG),
    //! so it can be exposed with std::pair below.
    std::tuple<float, float> getMinMax() const noexcept;
#endif

    const std::string& getName() const & noexcept;

    LayerDescriptor& setName(std::string inName) & noexcept;
    LayerDescriptor& setMinMax(float min, float max) & noexcept;
};

%extend LayerDescriptor
{
    std::pair<float, float> getMinMax() const noexcept
    {
        float min=0.0, max=0.0;
        std::tie(min, max) = self->getMinMax();
        return std::pair<float, float>(min, max);
    }
}

}  // namespace BAG

