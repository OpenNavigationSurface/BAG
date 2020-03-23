#ifndef BAG_TYPES_H
#define BAG_TYPES_H

#include "bag_c_types.h"

#include <unordered_map>


namespace BAG
{

constexpr int RANK = 2;

using LayerType = BAG_LAYER_TYPE;
using DataType = BAG_DATA_TYPE;
using OpenMode = BAG_OPEN_MODE;
using GroupType = BAG_GROUP_TYPE;

using TrackingItem = BagTrackingItem;
using VRTrackingItem = BagVRTrackingItem;

const std::unordered_map<LayerType, const char*> kLayerTypeMapString {
    {Elevation, "Elevation"},
    {Uncertainty, "Uncertainty"},
    {Hypothesis_Strength, "Hypothesis_Strength"},
    {Num_Hypotheses, "Num_Hypotheses"},
    {Shoal_Elevation, "Shoal_Elevation"},
    {Std_Dev, "Std_Dev"},
    {Num_Soundings, "Num_Soundings"},
    {Average_Elevation, "Average_Elevation"},
    {Nominal_Elevation, "Nominal_Elevation"},
    {Surface_Correction, "Surface_Correction"},
    //{Compound, "Compound"},
    {VarRes_Metadata, "Variable_Resolution_Metadata"},
    {VarRes_Refinement, "Variable_Resolution_Refinement"},
    {VarRes_Node, "Variable_Resolution_Node"},
};

}   //namespace BAG

#endif  //BAG_TYPES_H

