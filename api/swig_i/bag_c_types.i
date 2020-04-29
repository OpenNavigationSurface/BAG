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

%include <std_vector.i>


%extend FieldDefinition
{
    bool __eq__(const FieldDefinition& o) const
    {
        return ($self->name == o.name) && ($self->type == o.type);
    }
}

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
    BagVerticalDatumCorrections(
        double inX,
        double inY,
        const std::vector<float>& inZ)
    {
        auto* item = new BagVerticalDatumCorrections{};

        item->x = inX;
        item->y = inY;

        std::memcpy(item->z, inZ.data(), sizeof(float) * inZ.size());

        return item;
    }

    std::vector<float> zValues() const
    {
        std::vector<float> result(10);

        std::memcpy(result.data(), $self->z,
            BAG_SURFACE_CORRECTOR_LIMIT * sizeof(float));

        return result;
    }
}

%extend BagVerticalDatumCorrectionsGridded
{
    BagVerticalDatumCorrectionsGridded(
        const std::vector<float>& inZ)
    {
        auto* item = new BagVerticalDatumCorrectionsGridded{};

        std::memcpy(item->z, inZ.data(), sizeof(float) * inZ.size());

        return item;
    }

    std::vector<float> zValues() const
    {
        std::vector<float> result(10);

        std::memcpy(result.data(), $self->z,
            BAG_SURFACE_CORRECTOR_LIMIT * sizeof(float));

        return result;
    }
}

