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

class LayerItems
{
public:
    explicit LayerItems(const UInt8Array& items);
    template <typename T>
    explicit LayerItems(const std::vector<T>& items);

    const uint8_t* data() const &;
    bool empty() const noexcept;
    template <typename T>
    std::vector<T> getAs() const;
    size_t size() const noexcept;
};

%template(FloatLayerItems) LayerItems::LayerItems<float>;
%template(UInt32LayerItems) LayerItems::LayerItems<uint32_t>;
%template(SurfaceCorrectionsLayerItems) LayerItems::LayerItems<BagVerticalDatumCorrections>;
%template(SurfaceCorrectionsGriddedLayerItems) LayerItems::LayerItems<BagVerticalDatumCorrectionsGridded>;
%template(VRMetadataLayerItems) LayerItems::LayerItems<BagVRMetadataItem>;
%template(VRNodeLayerItems) LayerItems::LayerItems<BagVRNodeItem>;
%template(VRRefinementsLayerItems) LayerItems::LayerItems<BagVRRefinementsItem>;
%template(VerticalDatumCorrectionsLayerItems) LayerItems::LayerItems<BagVerticalDatumCorrections>;
%template(VerticalDatumCorrectionsGriddedLayerItems) LayerItems::LayerItems<BagVerticalDatumCorrectionsGridded>;

%template(asUInt32Items) LayerItems::getAs<uint32_t>;
%template(asFloatItems) LayerItems::getAs<float>;
%template(asVerticalDatumCorrections) LayerItems::getAs<BagVerticalDatumCorrections>;
%template(asVerticalDatumCorrectionsGridded) LayerItems::getAs<BagVerticalDatumCorrectionsGridded>;
%template(asVRMetadataItems) LayerItems::getAs<BagVRMetadataItem>;
%template(asVRRefinementsItems) LayerItems::getAs<BagVRRefinementsItem>;
%template(asVRNodeItems) LayerItems::getAs<BagVRNodeItem>;

}  // namespace BAG

