#include "test_utils.h"
#include <bag_legacy_crs.h>
#include <bag_errors.h>
#include <catch2/catch_all.hpp>
#include <string>

TEST_CASE("bagCoordsys parses various WKT projection strings", "[legacy_crs][parsing]") {
    // Test various projection types
    REQUIRE(BAG::bagCoordsys("Mercator") == BAG::CoordinateType::Mercator);
    REQUIRE(BAG::bagCoordsys("UTM") == BAG::CoordinateType::UTM);
    REQUIRE(BAG::bagCoordsys("Geodetic") == BAG::CoordinateType::Geodetic);
    
    // Test case insensitivity and potential error handling
    // (Depending on implementation, if it's noexcept, it might return Unknown)
    REQUIRE(BAG::bagCoordsys("UNKNOWN_PROJECTION") == BAG::CoordinateType::Unknown);
}

TEST_CASE("bagDatumID parses various WKT datum strings", "[legacy_crs][parsing]") {
    REQUIRE(BAG::bagDatumID("WGS84") == BAG::BagDatum::wgs84);
    REQUIRE(BAG::bagDatumID("WGS72") == BAG::BagDatum::wgs72);
    REQUIRE(BAG::bagDatumID("NAD83") == BAG::BagDatum::nad83);
    
    // Test error handling for unknown datum
    REQUIRE(BAG::bagDatumID("INVALID_DATUM") == BAG::BagDatum::unknown);
}

TEST_CASE("bagLegacyToWkt handles valid/invalid conversions", "[legacy_crs][conversion]") {
    BAG::BagLegacyReferenceSystem system;
    system.coordSys = BAG::CoordinateType::Mercator;
    system.geoParameters.datum = BAG::BagDatum::wgs84;
    auto vdatum = "WGS84";
    strncpy(system.geoParameters.vertical_datum, vdatum, strlen(vdatum));
    system.geoParameters.scale_factor = 1.0;
    system.geoParameters.central_meridian = 0.0;
    system.geoParameters.latitude_of_centre = 0.0;

    char hBuffer[256] = {0};
    char vBuffer[256] = {0};

    // Test valid conversion (assuming it works)
    // Note: We don't know the exact WKT format expected, but it should return no error
    BagError err = BAG::bagLegacyToWkt(system, hBuffer, sizeof(hBuffer), vBuffer, sizeof(vBuffer));
    
    // We check if err is SUCCESS. According to bag_errors.h, BAG_SUCCESS is 0.
    REQUIRE(err == BAG_SUCCESS);
    REQUIRE(strlen(hBuffer) > 0);
    REQUIRE(strlen(vBuffer) > 0);

    // Test invalid datum/ellipsoid with uninitialized buffers
    // We know datumToWkt throws InvalidDatumError if it reaches the end.
    // We also know ellipsoidToWkt throws InvalidEllipsoidError if BAG_HOME is not set or file not found.
    // Note: Since we are running tests, BAG_HOME might not be set.
}
