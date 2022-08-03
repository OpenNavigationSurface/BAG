#ifndef BAG_TYPES_H
#define BAG_TYPES_H

#include "bag_c_types.h"

#include <memory>
#include <limits>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstring>


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
inline bool operator==(const VRTrackingItem &lhs, const VRTrackingItem &rhs) noexcept {
    return lhs.row == rhs.row &&
        lhs.col == rhs.col &&
        lhs.sub_row == rhs.sub_row &&
        lhs.sub_col == rhs.sub_col &&
        lhs.depth == rhs.depth &&
        lhs.uncertainty == rhs.uncertainty &&
        lhs.track_code == rhs.track_code &&
        lhs.list_series == rhs.list_series;
}

//! The type of a record definition (used with Compound Layers).
inline bool operator==(const FieldDefinition &lhs, const FieldDefinition &rhs) noexcept {
    return lhs.type == rhs.type && (std::strcmp(lhs.name, rhs.name) == 0);
}
inline bool operator!=(const FieldDefinition &lhs, const FieldDefinition &rhs) noexcept {
    return !(lhs == rhs);
}
using RecordDefinition = std::vector<FieldDefinition>;
inline bool operator==(const RecordDefinition &lhs, const RecordDefinition &rhs) {
    auto size = lhs.size();
    bool areEqual = size == rhs.size();
    if (!areEqual) return areEqual;

    for (size_t i = 0; i < size; i++) {
        if (lhs[i] != rhs[i]) return false;
    }

    return areEqual;
}


using GeorefMetadataProfile = GEOREF_METADATA_PROFILE;

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

template <typename T, typename U>
inline bool weak_ptr_equals(const std::weak_ptr<T> &t, const std::weak_ptr<U> &u) {
    return !t.owner_before(u) && !u.owner_before(t);
}

}   //namespace BAG

#endif  //BAG_TYPES_H

