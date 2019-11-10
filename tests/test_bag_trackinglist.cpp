
#include "test_utils.h"
#include <bag_dataset.h>
#include <bag_metadata.h>
#include <bag_trackinglist.h>

#include <catch2/catch.hpp>
#include <cstdlib>  // std::getenv
#include <string>


using BAG::Dataset;
using BAG::TrackingList;

//NOTE .. Testing the methods that forward to std::vector<TrackingList::value_type> is not covered.

//  explicit TrackingList(Dataset& dataset);
TEST_CASE("test tracking list constructor (open)", "[trackinglist][constructor][destructor]")
{
//    const std::string bagFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
//        "/sample.bag"};
    const std::string bagFileName{"D:/Data/BAG/02000002.bag"};
    constexpr size_t kExpectedNumItems = 72;

    const auto pDataset = Dataset::open(bagFileName, BAG_OPEN_READONLY);

    UNSCOPED_INFO("Check dataset was read successfully.");
    REQUIRE(pDataset);

    const auto& trackingList = pDataset->getTrackingList();

    UNSCOPED_INFO("Check the expected number of items in the tracking list are there.");
    REQUIRE(trackingList.size() == kExpectedNumItems);
}

//  TrackingList(const Dataset& dataset, int compressionLevel);
TEST_CASE("test tracking list construction (create)", "[trackinglist][constructor][destructor]")
{
    const TestUtils::RandomFileGuard tmpFileName;

    constexpr uint64_t chunkSize = 100;
    constexpr unsigned int compressionLevel = 6;
    const auto pDataset = Dataset::create(tmpFileName, BAG::Metadata{},
        chunkSize, compressionLevel);

    UNSCOPED_INFO("Check dataset was created successfully.");
    REQUIRE(pDataset);

    TrackingList::value_type kExpectedItem0{1, 2, 3.4f, 5.6f, 7, 8};

    {
        auto& trackingList = pDataset->getTrackingList();

        UNSCOPED_INFO("Check no items are in the tracking list after creation.");
        CHECK(trackingList.size() == 0);

        // Add 1 record and save.  Is there one record with expected values?
        trackingList.emplace_back(kExpectedItem0);

        UNSCOPED_INFO("Check one item was stored.");
        CHECK(trackingList.size() == 1);

        UNSCOPED_INFO("Check writing to the HDF5 does not throw.");
        REQUIRE_NOTHROW(trackingList.write());
    }
    {
        // Read the expected record.
        const auto& trackingList = pDataset->getTrackingList();

        UNSCOPED_INFO("Check one item was read.");
        CHECK(trackingList.size() == 1);

        UNSCOPED_INFO("Check the expected values are in the item.");
        //CHECK(kExpectedItem0 == trackingList.front());

        const auto& item0 = trackingList.front();

        CHECK(kExpectedItem0.row == item0.row);
        CHECK(kExpectedItem0.col == item0.col);
        CHECK(kExpectedItem0.depth == item0.depth);
        CHECK(kExpectedItem0.uncertainty == item0.uncertainty);
        CHECK(kExpectedItem0.track_code == item0.track_code);
        CHECK(kExpectedItem0.list_series == item0.list_series);
    }

    TrackingList::value_type kExpectedItem1{9, 8, 7.6f, 5.4f, 3, 2};

    {
        // Add a second record to an existing TrackingList.
        auto& trackingList = pDataset->getTrackingList();

        UNSCOPED_INFO("Check one item was read.");
        CHECK(trackingList.size() == 1);

        trackingList.emplace_back(kExpectedItem1);
        CHECK(trackingList.size() == 2);

        UNSCOPED_INFO("Check writing to the HDF5 does not throw.");
        REQUIRE_NOTHROW(trackingList.write());
    }

    const auto& trackingList = pDataset->getTrackingList();

    UNSCOPED_INFO("Check the expected values are in the item.");
    CHECK(trackingList.size() == 2);

    const auto& item0 = trackingList[0];

    CHECK(kExpectedItem0.row == item0.row);
    CHECK(kExpectedItem0.col == item0.col);
    CHECK(kExpectedItem0.depth == item0.depth);
    CHECK(kExpectedItem0.uncertainty == item0.uncertainty);
    CHECK(kExpectedItem0.track_code == item0.track_code);
    CHECK(kExpectedItem0.list_series == item0.list_series);

    const auto& item1 = trackingList[1];

    CHECK(kExpectedItem1.row == item1.row);
    CHECK(kExpectedItem1.col == item1.col);
    CHECK(kExpectedItem1.depth == item1.depth);
    CHECK(kExpectedItem1.uncertainty == item1.uncertainty);
    CHECK(kExpectedItem1.track_code == item1.track_code);
    CHECK(kExpectedItem1.list_series == item1.list_series);
}

