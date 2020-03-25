%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_layer

%{
#include "../bag_layer.h"
%}

#define final
%import "../bag_config.h"
%import "bag_types.i"
%import "bag_uint8array.i"
%import "bag_layerdescriptor.i"

%include "std_unique_ptr.i"
wrap_unique_ptr(UInt8UniquePtr, BAG::UInt8Array);

namespace BAG {

class BAG_API Layer
{
public:
    //TODO Temp, make sure only move operations are used until development is done.
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

    std::unique_ptr<UInt8Array> read(uint32_t rowStart,
        uint32_t columnStart, uint32_t rowEnd, uint32_t columnEnd) const;

    void write(uint32_t rowStart, uint32_t columnStart, uint32_t rowEnd,
        uint32_t columnEnd, const uint8_t* buffer);

    void writeAttributes() const;
};
}
