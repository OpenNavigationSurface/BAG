#include "test_utils.h"
#include <bag_dataset.h>

#include <catch2/catch_all.hpp>
#include <cstdlib>  // std::getenv


using Catch::Approx;
using BAG::Dataset;

TEST_CASE("test reading BAG 1.6.3 VR dataset", "[dataset][compat][1.6.3][vr][open]")
{
	const std::string bagFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
	                              "/bag_163_vr.bag"};

	const size_t kNumExpectedLayers = 4;  // Elevation, Uncertainty, varres_metadata, varres_refinements

	SECTION("open read only")
	{
		const auto dataset = Dataset::open(bagFileName, BAG_OPEN_READONLY);
		REQUIRE(dataset);

		CHECK(dataset->getLayerTypes().size() == kNumExpectedLayers);

		// TODO: Verify varres_refinements layer dimensions and other VR aspects...
	}
}
