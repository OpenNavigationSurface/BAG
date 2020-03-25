%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_layerdescriptor

%include <std_shared_ptr.i>
%shared_ptr(BAG::LayerDescriptor)

%{
#include "bag_layerdescriptor.h"
%}

namespace BAG
{
class Dataset;
}

%import "bag_config.h"
%import "bag_types.i"

namespace BAG {

class BAG_API LayerDescriptor 
{
public:
    //TODO Temp, make sure only move operations are used until development is done.
    LayerDescriptor(const LayerDescriptor&) = delete;
    LayerDescriptor(LayerDescriptor&&) = delete;
    LayerDescriptor& operator=(const LayerDescriptor&) = delete;
    LayerDescriptor& operator=(LayerDescriptor&&) = delete;
    virtual ~LayerDescriptor() = default;

    uint64_t getChunkSize() const noexcept;
    unsigned int getCompressionLevel() const noexcept;
    DataType getDataType() const noexcept;
    uint8_t getElementSize() const noexcept;
    uint32_t getId() const noexcept;
    const std::string& getInternalPath() const & noexcept;
    LayerType getLayerType() const noexcept;
    std::tuple<float, float> getMinMax() const noexcept;
    const std::string& getName() const & noexcept;

    LayerDescriptor& setName(std::string inName) & noexcept;
    LayerDescriptor& setMinMax(float min, float max) & noexcept;
};
}