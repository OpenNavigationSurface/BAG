
#include "test_utils.h"
#include <bag_dataset.h>
#include <bag_interleavedlayer.h>
#include <bag_types.h>

#include <array>
#include <catch2/catch.hpp>
#include <cstdlib>  // std::getenv
#include <string>


using BAG::Dataset;

//  virtual std::unique_ptr<Uint8Array> read(uint32_t rowStart,
//      uint32_t columnStart, uint32_t rowEnd, uint32_t columnEnd) const;
TEST_CASE("test interleaved layer read", "[.][interleavedlayer][read]")  //TODO Hidden because micro151 is not in git.
{
    //TODO Rewrite this to create a temporary BAG.
    //TODO ^ cannot; only support reading interleaved layers
    const std::string bagFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/micro151.bag"};

    const auto dataset = Dataset::open(bagFileName, BAG_OPEN_READONLY);

    REQUIRE(dataset);

    const BAG::LayerType kLayerType = Shoal_Elevation;

    REQUIRE_NOTHROW(dataset->getLayer(kLayerType));
    const auto& layer = dataset->getLayer(kLayerType);

    auto buffer = layer.read(1, 2, 2, 4); // 2x3
    REQUIRE(buffer);

    constexpr size_t kExpectedNumNodes = 6;
    const std::array<float, kExpectedNumNodes> kExpectedBuffer{
        -32.40f, -32.38f, -32.38f,
        -32.33f, -31.89f, -31.98f};

    const float* floats = reinterpret_cast<const float*>(buffer.get());

    for (size_t i=0; i<kExpectedNumNodes; ++i)
        CHECK(kExpectedBuffer[i] == Approx(floats[i]));
}

