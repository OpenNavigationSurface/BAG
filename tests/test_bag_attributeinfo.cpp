#include "test_utils.h"

#include <bag_attributeinfo.h>
#include <bag_c_types.h>

#include <catch2/catch_all.hpp>

using LayerType = BAG_LAYER_TYPE;

TEST_CASE("getAttributeInfo returns correct info for valid layer types", "[attributeInfo][valid]") {
    BAG::AttributeInfo info;
    // Test Elevation
    info = BAG::getAttributeInfo(Elevation);
    REQUIRE(info.minName != nullptr);
    REQUIRE(info.maxName != nullptr);
    REQUIRE(info.path != nullptr);

    // Test Uncertainty
    info = BAG::getAttributeInfo(Uncertainty);
    REQUIRE(info.minName != nullptr);
    REQUIRE(info.maxName != nullptr);
    REQUIRE(info.path != nullptr);

    // Test Hypothesis_Strength
    info = BAG::getAttributeInfo(Hypothesis_Strength);
    REQUIRE(info.minName != nullptr);
    REQUIRE(info.maxName != nullptr);
    REQUIRE(info.path != nullptr);

    // Test Num_Hypotheses
    info = BAG::getAttributeInfo(Num_Hypotheses);
    REQUIRE(info.minName != nullptr);
    REQUIRE(info.maxName != nullptr);
    REQUIRE(info.path != nullptr);

    // Test Shoal_Elevation
    info = BAG::getAttributeInfo(Shoal_Elevation);
    REQUIRE(info.minName != nullptr);
    REQUIRE(info.maxName != nullptr);
    REQUIRE(info.path != nullptr);

    // Test Std_Dev
    info = BAG::getAttributeInfo(Std_Dev);
    REQUIRE(info.minName != nullptr);
    REQUIRE(info.maxName != nullptr);
    REQUIRE(info.path != nullptr);

    // Test Num_Soundings
    info = BAG::getAttributeInfo(Num_Soundings);
    REQUIRE(info.minName != nullptr);
    REQUIRE(info.maxName != nullptr);
    REQUIRE(info.path != nullptr);

    // Test Average_Elevation
    info = BAG::getAttributeInfo(Average_Elevation);
    REQUIRE(info.minName != nullptr);
    REQUIRE(info.maxName != nullptr);
    REQUIRE(info.path != nullptr);

    // Test Nominal_Elevation
    info = BAG::getAttributeInfo(Nominal_Elevation);
    REQUIRE(info.minName != nullptr);
    REQUIRE(info.maxName != nullptr);
    REQUIRE(info.path != nullptr);
}

TEST_CASE("getAttributeInfo throws UnsupportedSimpleLayerType for unknown layer type", "[attributeInfo][UnsupportedLayerType]") {
    // We use an invalid LayerType value to trigger the default case in getAttributeInfo
    auto invalidType = static_cast<LayerType>(100);
    
    REQUIRE_THROWS_AS(BAG::getAttributeInfo(invalidType), BAG::UnsupportedSimpleLayerType);
}
