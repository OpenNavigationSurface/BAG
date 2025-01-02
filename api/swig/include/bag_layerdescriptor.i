%begin %{
#ifdef _MSC_VER
#ifdef SWIGPYTHON
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
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
%shared_ptr(BAG::GeorefMetadataLayerDescriptor)
%shared_ptr(BAG::InterleavedLegacyLayerDescriptor)
%shared_ptr(BAG::SimpleLayerDescriptor)
%shared_ptr(BAG::SurfaceCorrectionsDescriptor)
%shared_ptr(BAG::VRMetadataDescriptor)
%shared_ptr(BAG::VRNodeDescriptor)
%shared_ptr(AG::VRRefinementsDescriptor)


namespace BAG {

class LayerDescriptor
{
public:
    LayerDescriptor(const LayerDescriptor&) = delete;
    LayerDescriptor(LayerDescriptor&&) = delete;
    virtual ~LayerDescriptor() = default;

    LayerDescriptor& operator=(const LayerDescriptor&) = delete;
    LayerDescriptor& operator=(LayerDescriptor&&) = delete;

    bool operator==(const LayerDescriptor &rhs) const noexcept;
    bool operator!=(const LayerDescriptor &rhs) const noexcept;

    uint64_t getChunkSize() const noexcept;
    int getCompressionLevel() const noexcept;
    DataType getDataType() const noexcept;
    uint8_t getElementSize() const noexcept;
    uint32_t getId() const noexcept;
    const std::string& getInternalPath() const & noexcept;
    LayerType getLayerType() const noexcept;

    // Converted to std::pair<T, T> below.
    //! Intentionally omit exposing of std::tuple method (unsupported by SWIG),
    //! so it can be exposed with std::pair below.
    //std::tuple<float, float> getMinMax() const noexcept;
    //const std::tuple<uint32_t, uint32_t>& getDims() const & noexcept;

    const std::string& getName() const & noexcept;

    LayerDescriptor& setName(std::string inName) & noexcept;
    LayerDescriptor& setMinMax(float min, float max) & noexcept;
    LayerDescriptor& setDims(uint32_t rows, uint32_t cols) & noexcept;
};

%extend LayerDescriptor
{
    std::pair<float, float> getMinMax() const noexcept
    {
        float min=0.0, max=0.0;
        std::tie(min, max) = self->getMinMax();
        return std::pair<float, float>(min, max);
    }

    std::pair<uint32_t, uint32_t> getDims() const noexcept
    {
        uint32_t rows=0, cols=0;
        std::tie(rows, cols) = self->getDims();
        return std::pair<uint32_t, uint32_t>(rows, cols);
    }
}

}  // namespace BAG

