%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_c_types

%{
#include "../bag_c_types.h"
%}


%include "../bag_c_types.h"

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

