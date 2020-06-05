
#include "test_utils.h"
#include <bag_dataset.h>
#include <bag_interleavedlegacylayer.h>
#include <bag_types.h>

#include <array>
#include <catch2/catch.hpp>
#include <cstdlib>  // std::getenv
#include <string>


using BAG::Dataset;

//  virtual UInt8Array read(uint32_t rowStart,
//      uint32_t columnStart, uint32_t rowEnd, uint32_t columnEnd) const;
TEST_CASE("test interleaved legacy layer read", "[interleavedlegacylayer][read]")
{
    const std::string bagFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/example_w_qc_layers.bag"};

    const auto dataset = Dataset::open(bagFileName, BAG_OPEN_READONLY);

    REQUIRE(dataset);

    const BAG::LayerType kLayerType = Hypothesis_Strength;

    REQUIRE_NOTHROW(dataset->getLayer(kLayerType));
    const auto& layer = dataset->getLayer(kLayerType);

    const auto buffer = layer.read(247, 338, 248, 340); // 2x3
    REQUIRE(buffer);

    constexpr size_t kExpectedNumNodes = 6;
    const std::array<float, kExpectedNumNodes> kExpectedBuffer{
        1.e6f, 1.e6f, 0.f,
        1.e6f,   0.f, 0.f,};

    const float* floats = reinterpret_cast<const float*>(buffer.data());

    for (size_t i=0; i<kExpectedNumNodes; ++i)
        CHECK(kExpectedBuffer[i] == Approx(floats[i]));
}

