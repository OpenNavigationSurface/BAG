%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_layer

#pragma SWIG nowarn=475 // ignore warnings about "optimal attribute usage in the out typemap."

%{
#include "../bag_layer.h"
%}

#define final

%import "../bag_config.h"
%import "bag_types.i"
%import "bag_uint8array.i"
%import "bag_layerdescriptor.i"
%include <std_string.i>

// define typemap so that returned UInt8Array objects are converted correctly 
%typemap(out, optimal="1") BAG::UInt8Array %{
    $result = SWIG_NewPointerObj(($1_ltype*)&$1, $&1_descriptor, 0);
%}

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

    UInt8Array read(uint32_t rowStart,
        uint32_t columnStart, uint32_t rowEnd, uint32_t columnEnd) const;

    void write(uint32_t rowStart, uint32_t columnStart, uint32_t rowEnd,
        uint32_t columnEnd, const uint8_t* buffer);

    void writeAttributes() const;
};
}
