%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_layer


%{
#include "../bag_layer.h"
%}

%import "bag_layerdescriptor.i"
%import "bag_types.i"
%import "bag_uint8array.i"
%import "bag_layeritem.i"

%include <downcast_shared_ptr.i>
%include <std_shared_ptr.i>
%include <std_string.i>
%include <stdint.i>

%shared_ptr(BAG::LayerDescriptor)

// define typemap so that returned UInt8Array objects are converted correctly
%typemap(out, optimal="1") BAG::UInt8Array %{
    $result = SWIG_NewPointerObj(($1_ltype*)&$1, $&1_descriptor, 0);
%}
//%typemap(out, optimal="1") BAG::LayerDescriptor& %{
//
//    $result = *SWIG_NewPointerObj(SWIG_as_voidptr($1),
//        SWIGTYPE_p_BAG__LayerDescriptor, 0 |  0 );
//
//%}

%downcast_shared(std::shared_ptr<BAG::LayerDescriptor> BAG::Layer::getDescriptor,
    BAG::CompoundLayerDescriptor, BAG::InterleavedLayerDescriptor,
    BAG::SimpleLayerDescriptor, BAG::SurfaceCorrectionsDescriptor,
    BAG::VRMetadataDescriptor, BAG::VRNodeDescriptor,
    BAG::VRRefinementsDescriptor, BAG::LayerDescriptor)


namespace BAG {

class Layer
{
public:
    Layer(const Layer&) = delete;
    Layer(Layer&&) = delete;
    virtual ~Layer() = default;

    Layer& operator=(const Layer&) = delete;
    Layer& operator=(Layer&&) = delete;

    static DataType getDataType(LayerType layerType) noexcept;
    static uint8_t getElementSize(DataType type);
    static std::string getInternalPath(LayerType layerType,
        GroupType groupType = UNKNOWN_GROUP_TYPE);

    std::shared_ptr<LayerDescriptor> getDescriptor() & noexcept;
    %ignore getDescriptor() const & noexcept;

    //UInt8Array 
    %ignore read(uint32_t rowStart, uint32_t columnStart, uint32_t rowEnd,
        uint32_t columnEnd) const;

    //void 
    %ignore write(uint32_t rowStart, uint32_t columnStart, uint32_t rowEnd,
        uint32_t columnEnd, const uint8_t* buffer);

    void writeAttributes() const;
};

%extend Layer
{
    BAG::LayerItem read2(uint32_t rowStart, uint32_t columnStart, 
        uint32_t rowEnd, uint32_t columnEnd) const
    {
        BAG::UInt8Array readItem = self->read(rowStart, columnStart, rowEnd, columnEnd);
        return BAG::LayerItem(readItem.data()); 
    }

    void write2(uint32_t rowStart, uint32_t columnStart, uint32_t rowEnd,
        uint32_t columnEnd, BAG::LayerItem item)
    {
        //auto surfC = item.getAs<BagVerticalDatumCorrections>();
        //printf("x: %f \n", surfC->x);
        //printf("y: %f \n", surfC->y);
        self->write(rowStart, columnStart, rowEnd, columnEnd, item.data());
    }
}

}