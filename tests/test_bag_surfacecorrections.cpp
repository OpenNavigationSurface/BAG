
#include "test_utils.h"
#include <bag_dataset.h>
#include <bag_surfacecorrections.h>
#include <bag_surfacecorrectionsdescriptor.h>

#include <catch2/catch.hpp>
#include <cstdlib>  // std::getenv
#include <string>


using BAG::Dataset;
using BAG::SurfaceCorrections;
using BAG::SurfaceCorrectionsDescriptor;


//  More of an integration test to see if the surface corrections is read.
TEST_CASE("test surface corrections read irregular",
    "[surfacecorrections][constructor][destructor][BAG_SURFACE_IRREGULARLY_SPACED]")
{
    const std::string bagFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"};

    const auto pDataset = Dataset::open(bagFileName, BAG_OPEN_READONLY);
    UNSCOPED_INFO("Check that the dataset was loaded.");
    REQUIRE(pDataset);

    UNSCOPED_INFO("Check that the surface corrections exist.");
    REQUIRE_NOTHROW(pDataset->getSurfaceCorrections());
    const auto& corrections = pDataset->getSurfaceCorrections();

    UNSCOPED_INFO("Check that the surface corrections descriptor exists.");
    REQUIRE_NOTHROW(corrections.getDescriptor());

    UNSCOPED_INFO("Check that the surface corrections descriptor exists.");
    REQUIRE_NOTHROW(dynamic_cast<const SurfaceCorrectionsDescriptor&>(corrections.getDescriptor()));
    const auto& descriptor =
        static_cast<const SurfaceCorrectionsDescriptor&>(corrections.getDescriptor());

    UNSCOPED_INFO("Check that the surface type being corrected is gridded.");
    CHECK(descriptor.getSurfaceType() == BAG_SURFACE_IRREGULARLY_SPACED);

    UNSCOPED_INFO("Check that the expected number of correctors were loaded.");
    constexpr size_t kExpectedNumCorrectors= 2;
    CHECK(descriptor.getNumCorrectors() == kExpectedNumCorrectors);
}

//  More of an integration test to see if the surface corrections is read.
TEST_CASE("test surface corrections read gridded",
    "[surfacecorrections][constructor][destructor][BAG_SURFACE_GRID_EXTENTS]")
{
    const std::string bagFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/micro151.bag"};

    UNSCOPED_INFO("Check that the dataset was loaded.");
    const auto pDataset = Dataset::open(bagFileName, BAG_OPEN_READONLY);
    REQUIRE(pDataset);

    UNSCOPED_INFO("Check that the surface corrections exist.");
    REQUIRE_NOTHROW(pDataset->getSurfaceCorrections());
    const auto& corrections = pDataset->getSurfaceCorrections();

    UNSCOPED_INFO("Check that the surface corrections descriptor exists.");
    REQUIRE_NOTHROW(corrections.getDescriptor());

    UNSCOPED_INFO("Check that the surface corrections descriptor exists.");
    REQUIRE_NOTHROW(dynamic_cast<const SurfaceCorrectionsDescriptor&>(corrections.getDescriptor()));
    const auto& descriptor =
        static_cast<const SurfaceCorrectionsDescriptor&>(corrections.getDescriptor());

    UNSCOPED_INFO("Check that the surface type being corrected is irregular.");
    CHECK(descriptor.getSurfaceType() == BAG_SURFACE_GRID_EXTENTS);

    UNSCOPED_INFO("Check that the expected number of correctors were loaded.");
    constexpr size_t kExpectedNumCorrectors = 3;
    CHECK(descriptor.getNumCorrectors() == kExpectedNumCorrectors);
}

TEST_CASE("test surface corrections create empty irregular",
    "[surfacecorrections][constructor][destructor][create][BAG_SURFACE_IRREGULARLY_SPACED]")
{
    const TestUtils::RandomFileGuard tmpFileName;

    UNSCOPED_INFO("Check dataset was created successfully.");
    auto pDataset = Dataset::create(tmpFileName, BAG::Metadata{});
    REQUIRE(pDataset);

    constexpr BAG_SURFACE_CORRECTION_TOPOGRAPHY kExpectedSurfaceType = BAG_SURFACE_IRREGULARLY_SPACED;
    constexpr uint8_t kExpectedNumCorrectors = 4;
    const auto& corrections = pDataset->createSurfaceCorrections(
        kExpectedSurfaceType, kExpectedNumCorrectors);

    UNSCOPED_INFO("Check surface corrections descriptor is the default descriptor.");
    REQUIRE_NOTHROW(dynamic_cast<const SurfaceCorrectionsDescriptor&>(corrections.getDescriptor()));

    const auto& descriptor = static_cast<const SurfaceCorrectionsDescriptor&>(corrections.getDescriptor());

    UNSCOPED_INFO("Check surface corrections num correctors.");
    CHECK(descriptor.getNumCorrectors() == kExpectedNumCorrectors);
    UNSCOPED_INFO("Check surface corrections surface type.");
    CHECK(descriptor.getSurfaceType() == kExpectedSurfaceType);
    UNSCOPED_INFO("Check surface corrections default datums.");
    CHECK(descriptor.getVerticalDatums() == "");
    UNSCOPED_INFO("Check surface corrections default southwest X, Y.");
    CHECK(descriptor.getOrigin() == std::make_tuple(0., 0.));
    UNSCOPED_INFO("Check surface corrections default X and Y spacing.");
    CHECK(descriptor.getSpacing() == std::make_tuple(0., 0.));
}

TEST_CASE("test surface corrections create empty gridded",
    "[surfacecorrections][constructor][destructor][create][BAG_SURFACE_GRID_EXTENTS]")
{
    const TestUtils::RandomFileGuard tmpFileName;

    UNSCOPED_INFO("Check dataset was created successfully.");
    auto pDataset = Dataset::create(tmpFileName, BAG::Metadata{});
    REQUIRE(pDataset);

    constexpr BAG_SURFACE_CORRECTION_TOPOGRAPHY kExpectedSurfaceType = BAG_SURFACE_GRID_EXTENTS;
    constexpr uint8_t kExpectedNumCorrectors = 2;
    const auto& corrections = pDataset->createSurfaceCorrections(
        kExpectedSurfaceType, kExpectedNumCorrectors);

    UNSCOPED_INFO("Check surface corrections descriptor is the default descriptor.");
    REQUIRE_NOTHROW(dynamic_cast<const SurfaceCorrectionsDescriptor&>(corrections.getDescriptor()));

    const auto& descriptor = static_cast<const SurfaceCorrectionsDescriptor&>(corrections.getDescriptor());

    UNSCOPED_INFO("Check surface corrections num correctors.");
    CHECK(descriptor.getNumCorrectors() == kExpectedNumCorrectors);
    UNSCOPED_INFO("Check surface corrections surface type.");
    CHECK(descriptor.getSurfaceType() == kExpectedSurfaceType);
    UNSCOPED_INFO("Check surface corrections default datums.");
    CHECK(descriptor.getVerticalDatums() == "");
    UNSCOPED_INFO("Check surface corrections default southwest X, Y.");
    CHECK(descriptor.getOrigin() == std::make_tuple(0., 0.));
    UNSCOPED_INFO("Check surface corrections default X and Y spacing.");
    CHECK(descriptor.getSpacing() == std::make_tuple(0., 0.));
}

TEST_CASE("test surface corrections create and write irregular",
    "[surfacecorrections][constructor][destructor][create][read][write][BAG_SURFACE_IRREGULARLY_SPACED]")
{
    const TestUtils::RandomFileGuard tmpFileName;

    UNSCOPED_INFO("Check dataset was created successfully.");
    auto pDataset = Dataset::create(tmpFileName, BAG::Metadata{});
    REQUIRE(pDataset);

    //TODO Need to provide file dims, ...
    constexpr BAG_SURFACE_CORRECTION_TOPOGRAPHY kExpectedSurfaceType = BAG_SURFACE_IRREGULARLY_SPACED;
    constexpr uint8_t kExpectedNumCorrectors = 2;
    auto& corrections = pDataset->createSurfaceCorrections(kExpectedSurfaceType,
        kExpectedNumCorrectors);

    UNSCOPED_INFO("Check surface corrections descriptor is the default descriptor.");
    REQUIRE_NOTHROW(dynamic_cast<const SurfaceCorrectionsDescriptor&>(corrections.getDescriptor()));

    constexpr BagVerticalDatumCorrections kExpectedItem0{1.2, 2.1, {3.4f, 4.5f}};

    UNSCOPED_INFO("Write one record.");
    const uint8_t* buffer = reinterpret_cast<const uint8_t*>(&kExpectedItem0);
    constexpr uint32_t kRowStart = 0;
    constexpr uint32_t kColumnStart = 0;
    constexpr uint32_t kRowEnd = 0;
    constexpr uint32_t kColumnEnd = 0;

    REQUIRE_NOTHROW(corrections.write(kRowStart, kColumnStart, kRowEnd, kColumnEnd, buffer));

    UNSCOPED_INFO("Read the record back.");
    auto result = corrections.read(kRowStart, kColumnStart, kRowEnd, kColumnEnd);

    const auto* res = reinterpret_cast<const BagVerticalDatumCorrections*>(result.get());
    CHECK(res->x == kExpectedItem0.x);
    CHECK(res->y == kExpectedItem0.y);
    CHECK(res->z[0] == kExpectedItem0.z[0]);
    CHECK(res->z[1] == kExpectedItem0.z[1]);
}

TEST_CASE("test surface corrections create and write gridded",
    "[surfacecorrections][constructor][destructor][create][read][write][BAG_SURFACE_GRID_EXTENTS]")
{
    const TestUtils::RandomFileGuard tmpFileName;

    UNSCOPED_INFO("Check dataset was created successfully.");
    auto pDataset = Dataset::create(tmpFileName, BAG::Metadata{});
    REQUIRE(pDataset);

    //TODO Need to provide file dims, ...
    constexpr BAG_SURFACE_CORRECTION_TOPOGRAPHY kExpectedSurfaceType = BAG_SURFACE_GRID_EXTENTS;
    constexpr uint8_t kExpectedNumCorrectors = 3;
    auto& corrections = pDataset->createSurfaceCorrections(kExpectedSurfaceType,
        kExpectedNumCorrectors);

    UNSCOPED_INFO("Check surface corrections descriptor is the default descriptor.");
    REQUIRE_NOTHROW(dynamic_cast<const SurfaceCorrectionsDescriptor&>(corrections.getDescriptor()));

    constexpr BagVerticalDatumCorrectionsGridded kExpectedItem0{9.87f, 6.543f, 2.109876f};

    UNSCOPED_INFO("Write one record.");
    const uint8_t* buffer = reinterpret_cast<const uint8_t*>(&kExpectedItem0);
    constexpr uint32_t kRowStart = 0;
    constexpr uint32_t kColumnStart = 0;
    constexpr uint32_t kRowEnd = 0;
    constexpr uint32_t kColumnEnd = 0;

    REQUIRE_NOTHROW(corrections.write(kRowStart, kColumnStart, kRowEnd, kColumnEnd, buffer));

    UNSCOPED_INFO("Read the record back.");
    auto result = corrections.read(kRowStart, kColumnStart, kRowEnd, kColumnEnd);

    const auto* res = reinterpret_cast<const BagVerticalDatumCorrectionsGridded*>(result.get());
    CHECK(res->z[0] == kExpectedItem0.z[0]);
    CHECK(res->z[1] == kExpectedItem0.z[1]);
    CHECK(res->z[2] == kExpectedItem0.z[2]);
}

