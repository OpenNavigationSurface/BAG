%begin %{
#ifdef _MSC_VER
#ifdef SWIGPYTHON
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
#endif
%}

%module bag_layer


%{
#include <memory>

#include "bag_layer.h"
%}

%import "bag_layerdescriptor.i"
%import "bag_types.i"
%import "bag_uint8array.i"
%import "bag_layeritems.i"

%include <std_string.i>
%include <stdint.i>

%include <std_shared_ptr.i>
%shared_ptr(BAG::Layer)
%shared_ptr(BAG::LayerDescriptor)


namespace BAG {

class Layer
{
public:
    Layer(const Layer&) = delete;
    Layer(Layer&&) = delete;
    virtual ~Layer() = default;

    Layer& operator=(const Layer&) = delete;
    Layer& operator=(Layer&&) = delete;

    bool operator==(const Layer &rhs) const noexcept;
    bool operator!=(const Layer &rhs) const noexcept;

    static DataType getDataType(LayerType layerType) noexcept;
    static uint8_t getElementSize(DataType type);
    static std::string getInternalPath(LayerType layerType,
        GroupType groupType = UNKNOWN_GROUP_TYPE);

    std::shared_ptr<LayerDescriptor> getDescriptor() & noexcept;
    //std::shared_ptr<const LayerDescriptor> getDescriptor() const & noexcept;

    //UInt8Array read(uint32_t rowStart, uint32_t columnStart, uint32_t rowEnd,
    //    uint32_t columnEnd) const;

    //void write(uint32_t rowStart, uint32_t columnStart, uint32_t rowEnd,
    //    uint32_t columnEnd, const uint8_t* buffer);

    void writeAttributes() const;
};

%extend Layer
{
    LayerItems read(
        uint32_t rowStart,
        uint32_t columnStart,
        uint32_t rowEnd,
        uint32_t columnEnd) const
    {
        return BAG::LayerItems{$self->read(rowStart, columnStart, rowEnd,
            columnEnd)};
    }

    void write(
        uint32_t rowStart,
        uint32_t columnStart,
        uint32_t rowEnd,
        uint32_t columnEnd,
        const LayerItems& items)
    {
        $self->write(rowStart, columnStart, rowEnd, columnEnd, items.data());
    }
}

}  // namespace BAG

