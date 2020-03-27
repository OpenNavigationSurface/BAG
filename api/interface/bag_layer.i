%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_layer

%{
#include "bag_layer.h"
%}

%import "bag_config.h"
%import "bag_layerdescriptor.i"

%import "bag_types.i"
%include "std_unique_ptr.i"
wrap_unique_ptr(UintUniquePtr, BAG::UintArray);

namespace BAG {

class BAG_API Layer
{
public:
    Layer(const Layer&) = delete;
    Layer(Layer&&) = delete;
    Layer& operator=(const Layer&) = delete;
    Layer& operator=(Layer&&) = delete;

    virtual ~Layer() = default;

    static DataType getDataType(LayerType layerType) noexcept;
    static uint8_t getElementSize(DataType type);
    static std::string getInternalPath(LayerType layerType,
        GroupType groupType = UNKNOWN_GROUP_TYPE);

    LayerDescriptor& getDescriptor() & noexcept;
    %ignore getDescriptor() const&;

    std::unique_ptr<UintArray> read(uint32_t rowStart,
        uint32_t columnStart, uint32_t rowEnd, uint32_t columnEnd) const;

    void write(uint32_t rowStart, uint32_t columnStart, uint32_t rowEnd,
        uint32_t columnEnd, const uint8_t* buffer);

    void writeAttributes() const;
};
}
