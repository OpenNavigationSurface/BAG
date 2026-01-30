#include <bag_dataset.h>
#include <bag_vrrefinements.h>
#include <bag_vrrefinementsdescriptor.h>
#include <bag_vrmetadata.h>
#include <bag_vrmetadatadescriptor.h>

#include <catch2/catch_all.hpp>


using BAG::Dataset;
using BAG::VRRefinements;
using BAG::VRRefinementsDescriptor;
using BAG::VRMetadata;
using BAG::VRMetadataDescriptor;


// Test basic reading of an existing VR BAG from https://github.com/OSGeo/gdal/blob/master/autotest/gdrivers/data/bag/test_vr.bag
TEST_CASE("test VR BAG reading GDAL", "[dataset][open][VR][GDAL]")
{
    const std::string bagFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
                                  "/test_vr.bag"};

    const size_t kNumExpectedLayers = 4;
    const auto dataset = Dataset::open(bagFileName, BAG_OPEN_READONLY);
    REQUIRE(dataset);

    CHECK(dataset->getLayerTypes().size() == kNumExpectedLayers);

    const uint32_t kExpectedRows = 4;
    const uint32_t kExpectedCols = 6;
    CHECK(dataset->getDescriptor().getVersion() == "1.6.2");
    auto dims = dataset->getDescriptor().getDims();
    CHECK(std::get<0>(dims) == kExpectedRows);
    CHECK(std::get<1>(dims) == kExpectedCols);

    auto vrMeta = dataset->getVRMetadata();
    REQUIRE(vrMeta);
    const auto vrMetaDesc = vrMeta->getDescriptor();
    auto vrMetaDescDims = vrMetaDesc->getDims();
    // VR metadata descriptor dims should be the same as BAG dataset dims...
    CHECK(std::get<0>(vrMetaDescDims) == kExpectedRows);
    CHECK(std::get<1>(vrMetaDescDims) == kExpectedCols);

    auto vrRef = dataset->getVRRefinements();
    REQUIRE(vrRef);
    const auto vrRefDesc = vrRef->getDescriptor();
    auto vrRefDescDims = vrRefDesc->getDims();
    CHECK(std::get<0>(vrRefDescDims) == 1);
    CHECK(std::get<1>(vrRefDescDims) == 556);
}

// Test basic reading of an existing VR BAG from the National Bathymetric Source archive (https://www.nauticalcharts.noaa.gov/learn/nbs.html)
TEST_CASE("test VR BAG reading NBS", "[dataset][open][VR][NBS]")
{
    const std::string bagFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
                                  "/Sample_VR_BAG-gzip.bag"};

    const size_t kNumExpectedLayers = 4;
    const auto dataset = Dataset::open(bagFileName, BAG_OPEN_READONLY);
    REQUIRE(dataset);

    CHECK(dataset->getLayerTypes().size() == kNumExpectedLayers);

    const uint32_t kExpectedRows = 4;
    const uint32_t kExpectedCols = 4;
    CHECK(dataset->getDescriptor().getVersion() == "1.6.0");
    auto dims = dataset->getDescriptor().getDims();
    CHECK(std::get<0>(dims) == kExpectedRows);
    CHECK(std::get<1>(dims) == kExpectedCols);

    auto vrMeta = dataset->getVRMetadata();
    REQUIRE(vrMeta);
    const auto vrMetaDesc = vrMeta->getDescriptor();
    auto vrMetaDescDims = vrMetaDesc->getDims();
    // VR metadata descriptor dims should be the same as BAG dataset dims...
    CHECK(std::get<0>(vrMetaDescDims) == kExpectedRows);
    CHECK(std::get<1>(vrMetaDescDims) == kExpectedCols);

    auto vrRef = dataset->getVRRefinements();
    REQUIRE(vrRef);
    const auto vrRefDesc = vrRef->getDescriptor();
    auto vrRefDescDims = vrRefDesc->getDims();
    CHECK(std::get<0>(vrRefDescDims) == 1);
    CHECK(std::get<1>(vrRefDescDims) == 3750);
}

TEST_CASE("test VR BAG reading libbag 1.6.3", "[dataset][open][VR][1.6.3]")
{
    const std::string bagFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
                                  "/bag_163_vr.bag"};

    const size_t kNumExpectedLayers = 4;  // Elevation, Uncertainty, varres_metadata, varres_refinements

    SECTION("open read only")
    {
        const auto dataset = Dataset::open(bagFileName, BAG_OPEN_READONLY);
        REQUIRE(dataset);

        CHECK(dataset->getLayerTypes().size() == kNumExpectedLayers);

        const uint32_t kExpectedRows = 529;
        const uint32_t kExpectedCols = 579;
        CHECK(dataset->getDescriptor().getVersion() == "1.6.3");
        auto dims = dataset->getDescriptor().getDims();
        CHECK(std::get<0>(dims) == kExpectedRows);
        CHECK(std::get<1>(dims) == kExpectedCols);

        auto vrMeta = dataset->getVRMetadata();
        REQUIRE(vrMeta);
        const auto vrMetaDesc = vrMeta->getDescriptor();
        auto vrMetaDescDims = vrMetaDesc->getDims();
        // VR metadata descriptor dims should be the same as BAG dataset dims...
        CHECK(std::get<0>(vrMetaDescDims) == kExpectedRows);
        CHECK(std::get<1>(vrMetaDescDims) == kExpectedCols);

        // Verify varres_refinements layer dimensions...
        auto vrRef = dataset->getVRRefinements();
        REQUIRE(vrRef);
        const auto vrRefDesc = vrRef->getDescriptor();
        auto vrRefDescDims = vrRefDesc->getDims();
        CHECK(std::get<0>(vrRefDescDims) == 1);
        CHECK(std::get<1>(vrRefDescDims) == 2957372);
    }
}