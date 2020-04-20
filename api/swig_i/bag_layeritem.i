%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_layeritem

%{
    
#include "../bag_layeritem.h"
%}

%import "bag_uint8array.i"
%import "bag_c_types.i"

namespace BAG {

class LayerItem
{
public:
        
    //BagLayerItem(BAG::UInt8Array item/*, BagLayerType lt, BAG::DataType dt*/) : 
    //    m_data(std::move(&&item.data()))
    //{
    //}            
    
    //template <typename T>
    LayerItem(const uint8_t* item);
    LayerItem(const float* item);
    LayerItem(const BagVRMetadataItem& item);
    LayerItem(const BagVRRefinementsItem& item);
    LayerItem(const BagVRNodeItem& item);
    LayerItem(const BagVerticalDatumCorrections& item);
    LayerItem(const BagVerticalDatumCorrectionsGridded& item);

    const uint8_t* data();
    template <typename T> const T* getAs() const;
        
    %template(asUInt32Item) LayerItem::getAs<const uint32_t>;
    %template(asFloatItem) LayerItem::getAs<const float>;
    %template(asVRMetadataItem) LayerItem::getAs<const BagVRMetadataItem>;
    %template(asVRRefinementsItem) LayerItem::getAs<const BagVRRefinementsItem>;
    %template(asVRNodeItem) LayerItem::getAs<const BagVRNodeItem>;
    %template(asBagVerticalDatumCorrections) LayerItem::getAs<const BagVerticalDatumCorrections>;
    %template(asBagVerticalDatumCorrectionsGridded) LayerItem::getAs<const BagVerticalDatumCorrectionsGridded>;
};
}

