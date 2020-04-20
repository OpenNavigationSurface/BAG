%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_c_types

%{
#include "../bag_c_types.h"
%}

//%include "bag_uint8array.i"
%include "../bag_c_types.h"


//! SWIG-style constructor for BagTrackingItem
%extend BagTrackingItem
{
    BagTrackingItem(uint32_t inRow, uint32_t inCol, float inDepth, 
        float inUncertainty, uint8_t inTrack_code, uint16_t inList_series)
    {
        return new BagTrackingItem{inRow, inCol, inDepth, 
            inUncertainty, inTrack_code, inList_series};
    }
}

// Add a constructor for BagVRMetadataItem.
%extend BagVRMetadataItem
{
    BagVRMetadataItem(uint32_t inIndex, uint32_t inDimX, uint32_t inDimY,
        float inResX, float inResY, float inSWx, float inSWy)
    {
        return new BagVRMetadataItem{inIndex, inDimX, inDimY, inResX, inResY,
            inSWx, inSWy};
    }
};

// Add a constructor for BagVRNodeItem.
%extend BagVRNodeItem
{
    BagVRNodeItem(float inHypStr, uint32_t inNumHyp, uint32_t inNSam)
    {
        return new BagVRNodeItem{inHypStr, inNumHyp, inNSam};
    }
};

// Add a constructor for BagVRRefinementsItem.
%extend BagVRRefinementsItem
{
    BagVRRefinementsItem(float inDepth, float inUncertaintyDepth)
    {
        return new BagVRRefinementsItem{inDepth, inUncertaintyDepth};
    }
};

// Add a constructor for BagVRTrackingItem.
%extend BagVRTrackingItem
{
    BagVRTrackingItem(uint32_t inRow, uint32_t inCol, uint32_t inSubRow, uint32_t inSubCol,
        float inDepth, float inUncertainty, uint8_t inTrackCode, uint16_t inListSeries)
    {
        return new BagVRTrackingItem{inRow, inCol, inSubRow, inSubCol, inDepth,
            inUncertainty, inTrackCode, inListSeries};
    }
};


//! SWIG-style constructor for BagVerticalDatumCorrections
%extend BagVerticalDatumCorrections
{
    //BagVerticalDatumCorrections(double _x, double _y, std::initializer_list<float> _z) 
    //    : BaseLayerItem(BAG_VERTICAL_DATUM_CORRECTIONS), x(_x), y(_y)/*, z(_z)*/
    //{
    //    size_t i=0;
    //    for (float zi : _z)
    //        z[i++]=zi;
    //    //for(size_t i =0; i < _z.size(); ++i)
    //      //  z[i] = _z[i];

    //}
    BagVerticalDatumCorrections(double inX, double inY, std::vector<float> inZ)
    {
        printf("constructor\n");
        BagVerticalDatumCorrections* item = new BagVerticalDatumCorrections();
        printf("  x : %f \n", inX);
        printf("  y : %f \n", inY);
        item->x = inX;
        item->y = inY;
        for(size_t i = 0; i < inZ.size(); ++i)
        {
            printf("  z%zd : %f \n", i, inZ[i]);
            item->z[i] = inZ[i];
        }
        return item;
    }

    //BagVerticalDatumCorrections(BAG::UInt8Array& in)
    //{
    //    //printf("constructor from uint: %d\n", in.data());
    //    
    //    const BagVerticalDatumCorrections* item = reinterpret_cast<const BagVerticalDatumCorrections*>(in.data());

    //    BagVerticalDatumCorrections* result = new BagVerticalDatumCorrections();
    //    result->x = item->x;
    //    result->y = item->y;
    //    printf("  x : %f \n", result->x);
    //    printf("  y : %f \n", result->y);
    //    
    //    for(size_t i = 0; i < BAG_SURFACE_CORRECTOR_LIMIT; ++i)
    //    {
    //        result->z[i] = item->z[i];
    //        printf("  z%zd : %f \n", i, result->z[i]);
    //        //item->z[i] = _z[i];
    //    }
    //    //BagVerticalDatumCorrections* out = const_cast< BagVerticalDatumCorrections*>(item);
    //    return result;
    //}

    std::vector<float> getZValues(void)
    {
        
        printf("get z vals\n");
        std::vector<float> zvals;
        for(size_t i = 0; i < BAG_SURFACE_CORRECTOR_LIMIT; ++i)
        {
            printf("  z%zd : %f \n", i, $self->z[i]);
            zvals.push_back($self->z[i]);
        }
        return zvals;
    }

  /*  const uint8_t* toUInt8(void)
    {
        return reinterpret_cast<const uint8_t*>(&self);
    }*/
    
 

    //static const BagVerticalDatumCorrections* fromUInt8(BAG::UInt8Array& in)
    //{
    //    //printf("from uint: %d\n", in.data());
    //    const BagVerticalDatumCorrections* item = reinterpret_cast<const BagVerticalDatumCorrections*>(in.data());
    //    if (!item)
    //    {
    //        printf("cast failed\n");
    //        return item;
    //    }
    //    printf("  x : %f \n", item->x);
    //    printf("  y : %f \n", item->y);
    //    for(size_t i = 0; i < BAG_SURFACE_CORRECTOR_LIMIT; ++i)
    //    {
    //        printf("  z%zd : %f \n", i, item->z[i]);
    //    }
    //    return item;
    //}
}

//BagVerticalDatumCorrectionsGridded(std::initializer_list<float> _z) 
    //    : BaseLayerItem(BAG_VERTICAL_DATUM_CORRECTIONS_GRIDDED)
    //{
    //    size_t i=0;
    //    for (float zi : _z)
    //        z[i++]=zi;
    //}


