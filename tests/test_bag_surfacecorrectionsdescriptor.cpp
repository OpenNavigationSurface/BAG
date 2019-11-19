
#include <bag_dataset.h>
#include <bag_surfacecorrectionsdescriptor.h>

#include <catch2/catch.hpp>
#include <cstdlib>  // std::getenv
#include <string>


using BAG::Dataset;
using BAG::SurfaceCorrectionsDescriptor;

//    static std::shared_ptr<SurfaceCorrectionsDescriptor> create(
//        BAG_SURFACE_CORRECTION_TOPOGRAPHY type, uint8_t numCorrections,
//        uint64_t chunkSize, unsigned int compressionLevel);
TEST_CASE("test surface corrections descriptor creation using type, num correctors, etc",
    "[surfacecorrectionsdescriptor][create]")
{
    constexpr auto kExpectedSurfaceType = BAG_SURFACE_GRID_EXTENTS;
    constexpr uint8_t kExpectedNumCorrectors = 5;
    constexpr uint64_t kExpectedChunkSize = 256;
    constexpr unsigned int kExpectedCompressionLevel = 9;

    const auto pDescriptor = SurfaceCorrectionsDescriptor::create(
        kExpectedSurfaceType, kExpectedNumCorrectors, kExpectedChunkSize,
        kExpectedCompressionLevel);
    UNSCOPED_INFO("Test creation was successful.");
    REQUIRE(pDescriptor);

    UNSCOPED_INFO("Check the surface type is read properly.");
    CHECK(pDescriptor->getSurfaceType() == kExpectedSurfaceType);

    UNSCOPED_INFO("Check the num correctors is read properly.");
    CHECK(pDescriptor->getNumCorrectors() == kExpectedNumCorrectors);

    UNSCOPED_INFO("Check the chunk size is read properly.");
    CHECK(pDescriptor->getChunkSize() == kExpectedChunkSize);

    UNSCOPED_INFO("Check the compression level is read properly.");
    CHECK(pDescriptor->getCompressionLevel() == kExpectedCompressionLevel);
}

//    static std::shared_ptr<SurfaceCorrectionsDescriptor> create(
//        const Dataset& dataset);
TEST_CASE("test surface corrections descriptor read irregular",
    "[surfacecorrectionsdescriptor][create][BAG_SURFACE_GRID_EXTENTS]")
{
    const std::string bagFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"};

    UNSCOPED_INFO("Check that the dataset was loaded.");
    const auto pDataset = Dataset::open(bagFileName, BAG_OPEN_READONLY);
    REQUIRE(pDataset);

    UNSCOPED_INFO("Check that the descriptor is created successfully.");
    const auto pDescriptor = SurfaceCorrectionsDescriptor::create(*pDataset);
    REQUIRE(pDescriptor);

    UNSCOPED_INFO("Check that the surface type being corrected is irregular.");
    constexpr auto kExpectedSurfaceType = BAG_SURFACE_IRREGULARLY_SPACED;
    CHECK(pDescriptor->getSurfaceType() == kExpectedSurfaceType);

    UNSCOPED_INFO("Check that the expected number of correctors were loaded.");
    constexpr uint8_t kExpectedNumCorrectors = 2;
    CHECK(pDescriptor->getNumCorrectors() == kExpectedNumCorrectors);

    UNSCOPED_INFO("Check the vertical datum is read properly.");
    const std::string kExpectedVerticalDatum{};
    CHECK(pDescriptor->getVerticalDatums() == kExpectedVerticalDatum);
}

TEST_CASE("test surface corrections descriptor create empty gridded",
    "[surfacecorrectionsdescriptor][create][setVerticalDatum][setOrigin][setSpacing]")
{
    constexpr auto kExpectedSurfaceType = BAG_SURFACE_GRID_EXTENTS;
    constexpr uint8_t kExpectedNumCorrectors = 5;
    constexpr uint64_t kExpectedChunkSize = 256;
    constexpr unsigned int kExpectedCompressionLevel = 9;

    const auto pDescriptor = SurfaceCorrectionsDescriptor::create(
        kExpectedSurfaceType, kExpectedNumCorrectors, kExpectedChunkSize,
        kExpectedCompressionLevel);
    UNSCOPED_INFO("Test creation was successful.");
    REQUIRE(pDescriptor);

    // Set the values.
    UNSCOPED_INFO("Test setting the vertical datums does not throw.");
    const std::string kExpectedVerticalDatum("datum 1,datum 2,datum 3");
    REQUIRE_NOTHROW(pDescriptor->setVerticalDatum(kExpectedVerticalDatum));

    UNSCOPED_INFO("Test setting the origin does not throw.");
    constexpr double kExpectedOriginX = 123.456;
    constexpr double kExpectedOriginY = 9876.54321;
    REQUIRE_NOTHROW(pDescriptor->setOrigin(kExpectedOriginX, kExpectedOriginY));

    UNSCOPED_INFO("Test setting the spacing does not throw.");
    constexpr double kExpectedSpacingX = 42.8901;
    constexpr double kExpectedSpacingY = 100.247;
    REQUIRE_NOTHROW(pDescriptor->setSpacing(kExpectedSpacingX, kExpectedSpacingY));

    // Get the values back.
    UNSCOPED_INFO("Test getting the set vertical datums.");
    CHECK(pDescriptor->getVerticalDatums() == kExpectedVerticalDatum);

    UNSCOPED_INFO("Test getting the set origin.");
    const auto actualOrigin = pDescriptor->getOrigin();
    CHECK(std::get<0>(actualOrigin) == kExpectedOriginX);
    CHECK(std::get<1>(actualOrigin) == kExpectedOriginY);

    UNSCOPED_INFO("Test getting the set spacing.");
    const auto actualSpacing = pDescriptor->getSpacing();
    CHECK(std::get<0>(actualSpacing) == kExpectedSpacingX);
    CHECK(std::get<1>(actualSpacing) == kExpectedSpacingY);
}

