
#include "test_utils.h"
#include <bag_dataset.h>

#include <catch2/catch.hpp>
#include <cstdlib>  // std::getenv
#include <string>


using BAG::Dataset;

//  static std::shared_ptr<Dataset> open(const std::string &fileName,
//      OpenMode openMode);
TEST_CASE("test dataset reading", "[.][dataset][open][getLayerTypes][createLayer]")
{
    const std::string bagFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"};

    SECTION("open read only")
    {
        const auto dataset = Dataset::open(bagFileName, BAG_OPEN_READONLY);
        REQUIRE(dataset);

        CHECK(dataset->getLayerTypes().size() == 2);
    }
    SECTION("open read write")
    {
        // Copy sample.bag and modify it.
        const TestUtils::RandomFileGuard tmpFileName;
        TestUtils::copyFile(bagFileName, tmpFileName);
        size_t numLayers = 0;

        {   // Add a new layer to the dataset.
            // Scoped so the dataset is destroyed (saves at that point).
            const auto dataset = Dataset::open(tmpFileName, BAG_OPEN_READ_WRITE);
            REQUIRE(dataset);

            const auto numLayerTypes = dataset->getLayerTypes().size();
            CHECK(numLayerTypes == 2);

            /*auto& layer = */ dataset->createLayer(Average_Elevation);

            numLayers = dataset->getLayerTypes().size();
            REQUIRE(numLayers == (numLayerTypes + 1));
        }

        // Read the saved BAG.
        const auto dataset = Dataset::open(tmpFileName, BAG_OPEN_READONLY);
        REQUIRE(dataset);

        REQUIRE(dataset->getLayerTypes().size() == numLayers);
    }
}

//  static std::shared_ptr<Dataset> create(const std::string &fileName,
//      const Metadata& metadata);
TEST_CASE("test dataset creation", "[.][dataset][create][getLayerTypes][open]")
{
    const TestUtils::RandomFileGuard tmpFileName;

    {
        const BAG::Metadata metadata;
        const auto dataset = Dataset::create(tmpFileName, metadata);

        REQUIRE(dataset);
        REQUIRE(dataset->getLayerTypes().size() == 0);
    }

    const auto dataset = Dataset::open(tmpFileName, BAG_OPEN_READONLY);

    REQUIRE(dataset);
    REQUIRE(dataset->getLayerTypes().size() == 0);
}

//  std::vector<LayerType> getLayerTypes() const;
TEST_CASE("test get layer types", "[dataset][open][getLayerTypes]")
{
    const std::string bagFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/NAVO_data/JD211_Public_Release_1-5.bag"};

    const auto dataset = Dataset::open(bagFileName, BAG_OPEN_READONLY);
    REQUIRE(dataset);

    CHECK(dataset->getLayerTypes().size() == 2);
}

//  Layer& createLayer(LayerType type);
TEST_CASE("test add layer", "[.][dataset][open][createLayer][getLayerTypes]")
{
    const std::string bagFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"};

    // Copy sample.bag and modify it.
    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(bagFileName, tmpFileName);
    size_t numLayers = 0;

    {   // Add a new layer to the dataset.
        // Scoped so the dataset is destroyed (saves at that point).
        const auto dataset = Dataset::open(tmpFileName, BAG_OPEN_READ_WRITE);
        REQUIRE(dataset);

        const auto numLayerTypes = dataset->getLayerTypes().size();
        CHECK(numLayerTypes == 2);

        /*auto& layer = */ dataset->createLayer(Average_Elevation);

        numLayers = dataset->getLayerTypes().size();
        REQUIRE(numLayers == (numLayerTypes + 1));
    }

    // Read the saved BAG.
    const auto dataset = Dataset::open(tmpFileName, BAG_OPEN_READ_WRITE);
    REQUIRE(dataset);

    REQUIRE(dataset->getLayerTypes().size() == numLayers);
}

//  Layer& getLayer(LayerType type);
//  const Layer& getLayer(LayerType type) const;
TEST_CASE("test get layer", "[dataset][open][getLayerTypes][getLayer]")
{
    const std::string bagFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"};

    // Test the non-const method.
    const auto dataset = Dataset::open(bagFileName, BAG_OPEN_READ_WRITE);
    REQUIRE(dataset);

    for (auto layerType : dataset->getLayerTypes())
        REQUIRE_NOTHROW(dataset->getLayer(layerType));

    // Test the const method.
    const std::shared_ptr<const Dataset> constDataset{dataset};
    REQUIRE(constDataset);

    for (auto layerType : constDataset->getLayerTypes())
        REQUIRE_NOTHROW(constDataset->getLayer(layerType));
}

//  TrackingList& getTrackingList();
//  const TrackingList& getTrackingList() const;
TEST_CASE("test get tracking list", "[dataset][open][getTrackingList]")
{
    const std::string bagFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"};

    // Test the non-const method.
    const auto dataset = Dataset::open(bagFileName, BAG_OPEN_READ_WRITE);
    REQUIRE(dataset);

    {
        REQUIRE_NOTHROW(dataset->getTrackingList());
        auto& trackingList = dataset->getTrackingList();
        trackingList;  //TODO Do something.
        //...
    }

    // Test the const method.
    const std::shared_ptr<const Dataset> constDataset{dataset};
    REQUIRE(constDataset);

    {
        REQUIRE_NOTHROW(constDataset->getTrackingList());
        const auto& trackingList = constDataset->getTrackingList();
        trackingList;  //TODO Do something.
        //...
    }
}

//  const Metadata& getMetadata() const;
TEST_CASE("test get metadata", "[dataset][open][getMetadata]")
{
    const std::string bagFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"};

    // Test the non-const method.
    const auto dataset = Dataset::open(bagFileName, BAG_OPEN_READ_WRITE);
    REQUIRE(dataset);

    REQUIRE_NOTHROW(dataset->getMetadata());
    const auto& metadata = dataset->getMetadata();
    metadata;  //TODO Do something.
    //...
}

//  std::tuple<double, double> gridToGeo(uint32_t row, uint32_t column) const;
TEST_CASE("test grid to geo", "[dataset][open][gridToGeo]")
{
    const std::string bagFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"};

    // Test the non-const method.
    const auto dataset = Dataset::open(bagFileName, BAG_OPEN_READ_WRITE);
    REQUIRE(dataset);

    double x = 0.;
    double y = 0.;

    std::tie(x, y) = dataset->gridToGeo(0, 0);

    CHECK(x == Approx(687910.0));
    CHECK(y == Approx(5554620.0));
}

//  std::tuple<uint32_t, uint32_t> geoToGrid(double x, double y) const;
TEST_CASE("test geo to grid", "[dataset][open][geoToGrid]")
{
    const std::string bagFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"};

    // Test the non-const method.
    const auto dataset = Dataset::open(bagFileName, BAG_OPEN_READ_WRITE);
    REQUIRE(dataset);

    uint64_t row = 123;
    uint64_t column = 456;

    std::tie(row, column) = dataset->geoToGrid(687910.0, 5554620.0);

    CHECK(row == 0);
    CHECK(column == 0);
}

//  const Descriptor& getDescriptor() const noexcept;
TEST_CASE("test get descriptor", "[dataset][getDescriptor]")
{
    const std::string bagFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"};

    // Test the non-const method.
    const auto dataset = Dataset::open(bagFileName, BAG_OPEN_READ_WRITE);
    REQUIRE(dataset);

    const auto& descriptor = dataset->getDescriptor();

    CHECK(descriptor.isReadOnly() == false);
}
