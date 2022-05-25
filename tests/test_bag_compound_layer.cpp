
#include "test_utils.h"

#include <catch2/catch.hpp>
#include <cstdlib>
#include <array>
#include <utility>

#include <bag_dataset.h>
#include <bag_metadata.h>
#include <bag_simplelayer.h>
#include <bag_surfacecorrections.h>
#include <bag_surfacecorrectionsdescriptor.h>



using BAG::Dataset;

namespace {

    TEST_CASE("test dataset S101 metadata profile creation", "[dataset][create][S101][compoundLayer]")
    {
        const std::string metadataFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
                                           "/sample.xml"};
        const TestUtils::RandomFileGuard tmpBagFileName;

        const auto result = TestUtils::createBag(metadataFileName,
                             tmpBagFileName);

        std::shared_ptr<BAG::Dataset> dataset = result.first;
        std::string elevationLayerName = result.second;

        REQUIRE( dataset->getLayers().size() == 2 );

        // TODO: Create compound layers of various metadata profiles

        // TODO: Read BAG from disk

        // TODO: Read compound layers and make sure metadata profile is properly declared and defined.
    }
}