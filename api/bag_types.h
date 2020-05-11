#ifndef BAG_TYPES_H
#define BAG_TYPES_H

#include "bag_c_types.h"

#include <limits>
#include <unordered_map>
#include <vector>


namespace BAG
{

// The rank of simple layers.
constexpr static int kRank = 2;

//! An invalid layer id.
constexpr static uint32_t kInvalidLayerId = std::numeric_limits<uint32_t>::max();

//! The types of data
using DataType = BAG_DATA_TYPE;
//! The types of layers.
using LayerType = BAG_LAYER_TYPE;
//! The group types.
using GroupType = BAG_GROUP_TYPE;
//! The open mode when opening a BAG.
using OpenMode = BAG_OPEN_MODE;

//! The type of item in a tracking list.
using TrackingItem = BagTrackingItem;

using VerticalDatumCorrections = BagVerticalDatumCorrections;
using VerticalDatumCorrectionsGridded = BagVerticalDatumCorrectionsGridded;

//! The type of item in a variable resolution metadata layer.
using VRMetadataItem = BagVRMetadataItem;
//! The type of item in a variable resolution node layer.
using VRNodeItem = BagVRNodeItem;
//! The type of item in a variable resolution refinements layer.
using VRRefinementsItem = BagVRRefinementsItem;
//! The type of item in a variable resolution tracking list.
using VRTrackingItem = BagVRTrackingItem;

//! The type of a record definition (used with Compound Layers).
using RecordDefinition = std::vector<FieldDefinition>;


//! A default layer name for each layer.
const std::unordered_map<LayerType, std::string> kLayerTypeMapString {
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

