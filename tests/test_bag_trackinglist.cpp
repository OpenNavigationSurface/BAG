
#include <bag_dataset.h>
#include <bag_trackinglist.h>

#include <catch2/catch.hpp>
#include <cstdlib>  // std::getenv
#include <string>


using BAG::Dataset;
using BAG::TrackingList;

//NOTE .. Testing the methods that forward to std::vector<value_type> is not covered.

//  explicit TrackingList(Dataset& dataset);
TEST_CASE("test tracking list", "[trackinglist][constructor][destructor]")
{
//    const std::string bagFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
//        "/sample.bag"};
    const std::string bagFileName{"D:/Data/BAG/02000002.bag"};

    const auto dataset = Dataset::open(bagFileName, BAG_OPEN_READONLY);
    REQUIRE(dataset);

    // Test constructor.
    TrackingList trackingList(*dataset);
    // Test destructor.
    for (auto&& item : trackingList)
        item;

    SUCCEED();
}

