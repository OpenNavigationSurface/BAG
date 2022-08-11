%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_layeritems

%{
#include "bag_exceptions.h"
#include "bag_layeritems.h"
%}

%import "bag_c_types.i"
%import "bag_uint8array.i"

%include <std_vector.i>
%include <stdint.i>

%template(BagVerticalDatumCorrectionsItems) std::vector<BagVerticalDatumCorrections>;
%template(BagVerticalDatumCorrectionsGriddedItems) std::vector<BagVerticalDatumCorrectionsGridded>;

%template(BagVRMetadataItems) std::vector<BagVRMetadataItem>;
%template(BagVRNodeItems) std::vector<BagVRNodeItem>;
%template(BagVRRefinementsItems) std::vector<BagVRRefinementsItem>;


namespace BAG {

class LayerItems
{
public:
    explicit LayerItems(const UInt8Array& items);
    template <typename T>
    explicit LayerItems(const std::vector<T>& items);

    template <typename OldType, typename NewType>
    LayerItems convert() const;

    const uint8_t* data() const &;
    bool empty() const noexcept;
    template <typename T>
    std::vector<T> getAs() const;
    size_t size() const noexcept;

    bool operator==(const LayerItems &rhs) const noexcept;
    bool operator!=(const LayerItems &rhs) const noexcept;
};

%extend LayerItems {
    /**
     * Overload len() in Python
     */
    size_t __len__() {
        return $self->size();
    }

    /**
     * Overload [] (read) in Python
     */
    uint8_t __getitem__(size_t pos) {
        return $self->data()[pos];
    }

    %pythoncode %{

    def __iter__(self):
        """
          Implement iterator using yield
        """
        for i in range(len(self)):
            yield self[i]
    %}
};

%template(FloatLayerItems) LayerItems::LayerItems<float>;
%template(SurfaceCorrectionsLayerItems) LayerItems::LayerItems<BagVerticalDatumCorrections>;
%template(SurfaceCorrectionsGriddedLayerItems) LayerItems::LayerItems<BagVerticalDatumCorrectionsGridded>;

%template(UInt16LayerItems) LayerItems::LayerItems<uint16_t>;
%template(UInt32LayerItems) LayerItems::LayerItems<uint32_t>;
%template(VRMetadataLayerItems) LayerItems::LayerItems<BagVRMetadataItem>;
%template(VRNodeLayerItems) LayerItems::LayerItems<BagVRNodeItem>;
%template(VRRefinementsLayerItems) LayerItems::LayerItems<BagVRRefinementsItem>;
%template(VerticalDatumCorrectionsLayerItems) LayerItems::LayerItems<BagVerticalDatumCorrections>;
%template(VerticalDatumCorrectionsGriddedLayerItems) LayerItems::LayerItems<BagVerticalDatumCorrectionsGridded>;

%template(asFloatItems) LayerItems::getAs<float>;
%template(asUInt32Items) LayerItems::getAs<uint32_t>;
%template(asVerticalDatumCorrections) LayerItems::getAs<BagVerticalDatumCorrections>;
%template(asVerticalDatumCorrectionsGridded) LayerItems::getAs<BagVerticalDatumCorrectionsGridded>;
%template(asVRMetadataItems) LayerItems::getAs<BagVRMetadataItem>;
%template(asVRRefinementsItems) LayerItems::getAs<BagVRRefinementsItem>;
%template(asVRNodeItems) LayerItems::getAs<BagVRNodeItem>;

%template(fromUInt16ToUInt32) LayerItems::convert<uint16_t, uint32_t>;
%template(fromUInt32ToUInt16) LayerItems::convert<uint32_t, uint16_t>;
%template(fromUInt32ToUInt8) LayerItems::convert<uint32_t, uint8_t>;
%template(fromUInt8ToUInt32) LayerItems::convert<uint8_t, uint32_t>;

}  // namespace BAG

