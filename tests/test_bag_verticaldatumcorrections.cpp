
#include <bag_dataset.h>
#include <bag_verticaldatumcorrections.h>

#include <catch2/catch.hpp>
#include <cstdlib>  // std::getenv
#include <string>


using BAG::Dataset;
using BAG::VerticalDatumCorrections;

//NOTE .. Testing the methods that forward to std::vector<value_type> is not covered.

//  explicit TrackingList(Dataset& dataset);
TEST_CASE("test vertical datum corrections", "[verticaldatumcorrections][constructor][destructor]")
{
    const std::string bagFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"};

    const auto dataset = Dataset::open(bagFileName, BAG_OPEN_READONLY);
    REQUIRE(dataset);

    // Test constructor.
    VerticalDatumCorrections verticalDatumCorrections(*dataset);
    // Test destructor.
    for (auto&& item : verticalDatumCorrections)
        item;

    SUCCEED();
}

