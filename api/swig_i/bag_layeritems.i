%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_layeritems

%{
#include "../bag_exceptions.h"
#include "../bag_layeritems.h"
%}

%import "bag_c_types.i"
%import "bag_uint8array.i"

%include "std_vector.i"
%include "stdint.i"

//%template(FloatItems) std::vector<float>;
//%template(UInt32Items) std::vector<uint32_t>;

%template(BagVerticalDatumCorrectionsItems) std::vector<BagVerticalDatumCorrections>;
%template(BagVerticalDatumCorrectionsGriddedItems) std::vector<BagVerticalDatumCorrectionsGridded>;

%template(BagVRMetadataItems) std::vector<BagVRMetadataItem>;
%template(BagVRNodeItems) std::vector<BagVRNodeItem>;
%template(BagVRRefinementsItems) std::vector<BagVRRefinementsItem>;


namespace BAG {

class LayerItem
{
public:
    explicit LayerItem(const UInt8Array& items);
    template <typename T>
    explicit LayerItem(const std::vector<T>& items);

    const uint8_t* data() const &;
    bool empty() const noexcept;
    template <typename T>
    std::vector<T> getAs() const;
    size_t size() const noexcept;
};

%template(FloatLayerItems) LayerItem::LayerItem<float>;
%template(UInt32LayerItems) LayerItem::LayerItem<uint32_t>;
%template(SurfaceCorrectionsLayerItems) LayerItem::LayerItem<BagVerticalDatumCorrections>;
%template(SurfaceCorrectionsGriddedLayerItems) LayerItem::LayerItem<BagVerticalDatumCorrectionsGridded>;
%template(VRMetadataLayerItems) LayerItem::LayerItem<BagVRMetadataItem>;
%template(VRNodeLayerItems) LayerItem::LayerItem<BagVRNodeItem>;
%template(VRRefinementsLayerItems) LayerItem::LayerItem<BagVRRefinementsItem>;
%template(VerticalDatumCorrectionsLayerItems) LayerItem::LayerItem<BagVerticalDatumCorrections>;
%template(VerticalDatumCorrectionsGriddedLayerItems) LayerItem::LayerItem<BagVerticalDatumCorrectionsGridded>;

%template(asUInt32Items) LayerItem::getAs<uint32_t>;
%template(asFloatItems) LayerItem::getAs<float>;
%template(asVerticalDatumCorrections) LayerItem::getAs<BagVerticalDatumCorrections>;
%template(asVerticalDatumCorrectionsGridded) LayerItem::getAs<BagVerticalDatumCorrectionsGridded>;
%template(asVRMetadataItems) LayerItem::getAs<BagVRMetadataItem>;
%template(asVRRefinementsItems) LayerItem::getAs<BagVRRefinementsItem>;
%template(asVRNodeItems) LayerItem::getAs<BagVRNodeItem>;

}  // namespace BAG

