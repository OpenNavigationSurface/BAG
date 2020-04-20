#ifndef BAG_LAYER_ITEM_H
#define BAG_LAYER_ITEM_H

#include "bag_c_types.h"

namespace BAG
{

class LayerItem
{
public:
        
    //BagLayerItem(BAG::UInt8Array item/*, BagLayerType lt, BAG::DataType dt*/) : 
    //    m_data(std::move(&&item.data()))
    //{
    //}

    LayerItem(const uint8_t* item) :
        m_data(std::move(item))
    {
    }
            
    
    //template <typename T>
    LayerItem(const float* item) : //m_itype(BAG_SIMPLE_INT_ITEM), 
        m_data(reinterpret_cast<const uint8_t*>(item))
    {
    }
    LayerItem(const BagVRMetadataItem& item) : //m_itype(BAG_VR_METADATA_ITEM), 
        m_data(reinterpret_cast<const uint8_t*>(&item))
    {
    }
    LayerItem(const BagVRRefinementsItem& item) : //m_itype(BAG_VR_REFINEMENTS_ITEM), 
        m_data(reinterpret_cast<const uint8_t*>(&item))
    {
    }
    LayerItem(const BagVRNodeItem& item) : //m_itype(BAG_VR_NODE_ITEM), 
        m_data(reinterpret_cast<const uint8_t*>(&item))
    {
    }
    LayerItem(const BagVerticalDatumCorrections& item) :// m_itype(BAG_VERTICAL_DATUM_CORRECTIONS), 
        m_data(reinterpret_cast<const uint8_t*>(&item))
    {
    }
    LayerItem(const BagVerticalDatumCorrectionsGridded& item) : //m_itype(BAG_VERTICAL_DATUM_CORRECTIONS_GRIDDED), 
        m_data(reinterpret_cast<const uint8_t*>(&item))
    {
    }

    template <typename T>
    const T* getAs() const
    {
        return reinterpret_cast<const T*>(m_data);
    }

    /*
    const uint32_t* asUInt32() const
    {
        return reinterpret_cast<const uint32_t*>(m_data);
    }
    const float* asFloat() const
    {
        return reinterpret_cast<const float*>(m_data);
    }
    const BagVRMetadataItem* asVRMetadataItem() const
    {
        return reinterpret_cast<const BagVRMetadataItem*>(m_data);
    }
    const BagVRRefinementsItem* asVRRefinementsItem() const
    {
        return reinterpret_cast<const BagVRRefinementsItem*>(m_data);
    }
    const BagVRNodeItem* asVRNodeItem() const
    {
        return reinterpret_cast<const BagVRNodeItem*>(m_data);
    }
    const BagVerticalDatumCorrections* asBagVerticalDatumCorrections() const
    {
        return reinterpret_cast<const BagVerticalDatumCorrections*>(m_data);
    }
    const BagVerticalDatumCorrectionsGridded* asBagVerticalDatumCorrectionsGridded() const
    {
        return reinterpret_cast<const BagVerticalDatumCorrectionsGridded*>(m_data);
    }*/

 

    const uint8_t* data() { return m_data; }
    //BAG_LAYER_ITEM_TYPE getType() { return m_itype; }

private:
    //BAG_LAYER_ITEM_TYPE m_itype;
    const uint8_t* m_data;
};


}


#endif  //BAG_LAYER_ITEM_H






 
    //! The type of item.
    //enum BAG_LAYER_ITEM_TYPE
    //{
    //    BAG_SIMPLE_INT_ITEM = 0,
    //    BAG_SIMPLE_FLOAT_ITEM,
    //    //BAG_TRACKING_ITEM,
    //    //BAG_VR_TRACKING_ITEM,
    //    BAG_VR_METADATA_ITEM,
    //    BAG_VR_REFINEMENTS_ITEM,
    //    BAG_VR_NODE_ITEM,
    //    BAG_VERTICAL_DATUM_CORRECTIONS,
    //    BAG_VERTICAL_DATUM_CORRECTIONS_GRIDDED
    //};


        
            //if(lt == Surface_Correction)
            //{
            //    // how to distinguish gridded vs not?
            //    m_itype = BAG_VERTICAL_DATUM_CORRECTIONS;
            //    //or BAG_VERTICAL_DATUM_CORRECTIONS_GRIDDED
            //}
            //else if(lt == VarRes_Metadata)
            //{
            //    m_itype = BAG_VR_METADATA_ITEM;
            //}
            //else if(lt == VarRes_Refinement)
            //{
            //    m_itype = BAG_VR_REFINEMENTS_ITEM;
            //}
            //else if(lt == VarRes_Node)
            //{
            //    m_itype = BAG_VR_NODE_ITEM;
            //}            
            ////else if(lt == Compound)
            ////{
            ////    //m_itype = ?;
            ////}
            //else
            //{
            //    if (dt == DT_COMPOUND)
            //    {
            //        //m_itype = ??compound??;
            //    }
            //    else if (dt == DT_BOOLEAN || dt == DT_STRING)
            //    {
            //        //??
            //    }
            //    else if (dt == DT_FLOAT32)
            //    {
            //        m_itype = BAG_SIMPLE_FLOAT_ITEM;
            //    }
            //    else
            //    {
            //        m_itype = BAG_SIMPLE_INT_ITEM;
            //    }
            //    // simple or interleaved or compound?
            //}

