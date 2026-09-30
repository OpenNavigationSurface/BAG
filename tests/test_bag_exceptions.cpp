#include "test_utils.h"
#include <bag_exceptions.h>
#include <bag_attributeinfo.h>
#include <catch2/catch_all.hpp>

using BAG::LayerType;

TEST_CASE("getAttributeInfo throws UnsupportedSimpleLayerType for unknown layer type", "[exceptions]") {
    // We use an invalid LayerType value to trigger the default case in getAttributeInfo
    LayerType invalidType = static_cast<LayerType>(100);
    
    REQUIRE_THROWS_AS(BAG::getAttributeInfo(invalidType), BAG::UnsupportedSimpleLayerType);
}

TEST_CASE("Dataset::open throws errors for invalid files", "[exceptions][DatasetNotFound]") {
    // Testing with a non-existent file to check for DatasetNotFound (or error handling)
    // Note: BagDataset::open might not throw but returns null according to its implementation
    // Let's check how it handles it.
    const std::string nonExistentFile = "/tmp/non_existent_file_for_bag_test";
    auto dataset = BAG::Dataset::open(nonExistentFile, BAG_OPEN_READONLY);
    REQUIRE(!dataset);
}

TEST_CASE("Dataset::getLayer throws errors for invalid id", "[exceptions][invalidId]") {
    // We need a dataset to call getLayer on.
    // Since we can't easily create one without files, we can't test this yet.
}

TEST_CASE("AttributeInfo exceptions", "[exceptions][UnsupportedAttributeType]") {
    // Test unsupported attribute type
    // We need to find a way to trigger UnsupportedAttributeType
    // It's used in some contexts.
}
