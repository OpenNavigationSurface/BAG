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

TEST_CASE("bagLegacyToWkt handles valid conversions", "[legacy_crs][conversion]") {
    auto vdatums = std::vector<std::string>{"North_American_Vertical_Datum_1988", // wgs84
        "National Geodetic Vertical Datum 1929", // wgs72
        "National Geodetic Vertical Datum 1929", // nad83
        "North_American_Vertical_Datum_1988", // utm
        "North_American_Vertical_Datum_1988", // albers
        "North_American_Vertical_Datum_1988", // azimEq
        "EGM84 height", // bonne
        "EGM84 height", // cassini
        "EGM84 height", // cylindrical
        "EGM84 height", // eckert4
        "EGM84 height", // eckert6
        "EGM84 height", // equiCyl
        "EGM84 height", // gnomic
        "North_American_Vertical_Datum_1988", // lambert
        "EGM84 height", // miller
        "EGM84 height", // mollweide
        "GCS_ITRF_1988", //nzmg
        "North_American_Vertical_Datum_1988", // ortho
        "North_American_Vertical_Datum_1988", // polStereo
        "EGM84 height", // polyconic
        "EGM84 height", // sinusoidal
        "North_American_Vertical_Datum_1988", // stereographic
        "North_American_Vertical_Datum_1988", // transMerc
        "EGM84 height", // vanDerGrinten
        "UNKNOWN"};
    BAG::BagLegacyReferenceSystem wgs84;
    wgs84.coordSys = BAG::CoordinateType::Geodetic;
    wgs84.geoParameters.datum = BAG::BagDatum::wgs84;

    BAG::BagLegacyReferenceSystem wgs72;
    wgs72.coordSys = BAG::CoordinateType::Geodetic;
    wgs72.geoParameters.datum = BAG::BagDatum::wgs72;

    BAG::BagLegacyReferenceSystem nad83;
    nad83.coordSys = BAG::CoordinateType::Geodetic;
    nad83.geoParameters.datum = BAG::BagDatum::nad83;

    BAG::BagLegacyReferenceSystem utm;
    utm.coordSys = BAG::CoordinateType::UTM;
    utm.geoParameters.datum = BAG::BagDatum::wgs84;

    BAG::BagLegacyReferenceSystem albers;
    albers.coordSys = BAG::CoordinateType::Albers_Equal_Area_Conic;
    albers.geoParameters.datum = BAG::BagDatum::nad83;

    BAG::BagLegacyReferenceSystem azimEq;
    azimEq.coordSys = BAG::CoordinateType::Azimuthal_Equidistant;
    azimEq.geoParameters.datum = BAG::BagDatum::wgs84;

    BAG::BagLegacyReferenceSystem bonne;
    bonne.coordSys = BAG::CoordinateType::Bonne;
    bonne.geoParameters.datum = BAG::BagDatum::wgs84;

    BAG::BagLegacyReferenceSystem cassini;
    cassini.coordSys = BAG::CoordinateType::Cassini;
    cassini.geoParameters.datum = BAG::BagDatum::wgs84;

    BAG::BagLegacyReferenceSystem cylindrical;
    cylindrical.coordSys = BAG::CoordinateType::Cylindrical_Equal_Area;
    cylindrical.geoParameters.datum = BAG::BagDatum::wgs84;

    BAG::BagLegacyReferenceSystem eckert4;
    eckert4.coordSys = BAG::CoordinateType::Eckert4;
    eckert4.geoParameters.datum = BAG::BagDatum::unknown;

    BAG::BagLegacyReferenceSystem eckert6;
    eckert6.coordSys = BAG::CoordinateType::Eckert6;
    eckert6.geoParameters.datum = BAG::BagDatum::unknown;

    BAG::BagLegacyReferenceSystem equiCyl;
    equiCyl.coordSys = BAG::CoordinateType::Equidistant_Cylindrical;
    equiCyl.geoParameters.datum = BAG::BagDatum::wgs84;

    BAG::BagLegacyReferenceSystem gnomic;
    gnomic.coordSys = BAG::CoordinateType::Gnomonic;
    gnomic.geoParameters.datum = BAG::BagDatum::wgs84;

    BAG::BagLegacyReferenceSystem lambert;
    lambert.coordSys = BAG::CoordinateType::Lambert_Conformal_Conic;
    lambert.geoParameters.datum = BAG::BagDatum::nad83;

    BAG::BagLegacyReferenceSystem miller;
    miller.coordSys = BAG::CoordinateType::Miller_Cylindrical;
    miller.geoParameters.datum = BAG::BagDatum::wgs84;

    BAG::BagLegacyReferenceSystem mollweide;
    mollweide.coordSys = BAG::CoordinateType::Mollweide;
    mollweide.geoParameters.datum = BAG::BagDatum::wgs84;

    BAG::BagLegacyReferenceSystem nzmg;
    nzmg.coordSys = BAG::CoordinateType::NZMG;
    nzmg.geoParameters.datum = BAG::BagDatum::wgs84;

    BAG::BagLegacyReferenceSystem ortho;
    ortho.coordSys = BAG::CoordinateType::Orthographic;
    ortho.geoParameters.datum = BAG::BagDatum::wgs84;

    BAG::BagLegacyReferenceSystem polStereo;
    polStereo.coordSys = BAG::CoordinateType::Polar_Stereo;
    polStereo.geoParameters.datum = BAG::BagDatum::wgs84;

    BAG::BagLegacyReferenceSystem polyconic;
    polyconic.coordSys = BAG::CoordinateType::Polyconic;
    polyconic.geoParameters.datum = BAG::BagDatum::wgs84;

    BAG::BagLegacyReferenceSystem sinusoidal;
    sinusoidal.coordSys = BAG::CoordinateType::Sinusoidal;
    sinusoidal.geoParameters.datum = BAG::BagDatum::wgs84;

    BAG::BagLegacyReferenceSystem stereographic;
    stereographic.coordSys = BAG::CoordinateType::Stereographic;
    stereographic.geoParameters.datum = BAG::BagDatum::wgs84;

    BAG::BagLegacyReferenceSystem transMerc;
    transMerc.coordSys = BAG::CoordinateType::Transverse_Mercator;
    transMerc.geoParameters.datum = BAG::BagDatum::nad83;

    BAG::BagLegacyReferenceSystem vanDerGrinten;
    vanDerGrinten.coordSys = BAG::CoordinateType::Van_der_Grinten;
    vanDerGrinten.geoParameters.datum = BAG::BagDatum::wgs84;

    BAG::BagLegacyReferenceSystem unknown;
    unknown.coordSys = BAG::CoordinateType::Mercator;
    unknown.geoParameters.datum = BAG::BagDatum::unknown;

    auto systems = std::vector<BAG::BagLegacyReferenceSystem>{wgs84, wgs72, nad83, utm, albers, azimEq, bonne,
        cassini, cylindrical, eckert4, eckert6, equiCyl, gnomic, lambert, miller, mollweide, nzmg, ortho, polStereo,
        polyconic, sinusoidal, stereographic, transMerc, vanDerGrinten, unknown};
    REQUIRE(vdatums.size() == systems.size());

    for (size_t i = 0; i < vdatums.size(); i++) {
        auto system = systems[i];
        auto vdatum = vdatums[i].c_str();
        strncpy(system.geoParameters.vertical_datum, vdatum, strlen(vdatum));
        system.geoParameters.scale_factor = 1.0;
        system.geoParameters.central_meridian = 0.0;
        system.geoParameters.latitude_of_centre = 0.0;

        char hBuffer[256] = {0};
        char vBuffer[256] = {0};

        // Test valid conversions
        BagError err = BAG::bagLegacyToWkt(system, hBuffer, sizeof(hBuffer), vBuffer, sizeof(vBuffer));

        REQUIRE(err == BAG_SUCCESS);
        REQUIRE(strlen(hBuffer) > 0);
        REQUIRE(strlen(vBuffer) > 0);
    }
}
