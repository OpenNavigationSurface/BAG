
#include "test_utils.h"
#include <bag_dataset.h>
#include <bag_interleavedlayer.h>
#include <bag_types.h>

#include <array>
#include <catch2/catch.hpp>
#include <cstdlib>  // std::getenv
#include <string>


using BAG::Dataset;
using BAG::Layer;

//  virtual std::unique_ptr<uint8_t[]> read(uint32_t rowStart,
//      uint32_t columnStart, uint32_t rowEnd, uint32_t columnEnd) const;
TEST_CASE("test interleaved layer read", "[.][interleavedlayer][read]")
{
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

//  virtual void write(uint32_t rowStart, uint32_t columnStart, uint32_t rowEnd,
//      uint32_t columnEnd, const uint8_t* buffer) const;
TEST_CASE("test interleaved layer write", "[.][interleavedlayer][write]")
{
    const std::string bagFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/micro151.bag"};

    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(bagFileName, tmpFileName);

    const BAG::LayerType kLayerType = Num_Hypotheses;

    constexpr size_t kExpectedNumNodes = 6;
    std::array<uint32_t, kExpectedNumNodes> kExpectedBuffer;
    kExpectedBuffer.fill(1234567);

    // Scope dataset to force write to be saved to disk.
    {
        const auto dataset = Dataset::open(tmpFileName, BAG_OPEN_READ_WRITE);

        REQUIRE(dataset);

        REQUIRE_NOTHROW(dataset->getLayer(kLayerType));
        const auto& layer = dataset->getLayer(kLayerType);

        // Write expected data.
        //TODO fix cast
        REQUIRE_NOTHROW(layer.write(1, 2, 2, 4, (uint8_t*)kExpectedBuffer.data())); // 2x3

        // Read the data back in from ?memory.
        auto buffer = layer.read(1, 2, 2, 4); // 2x3
        REQUIRE(buffer);

        uint32_t* uint32s = reinterpret_cast<uint32_t*>(buffer.get());
        for (size_t i=0; i<kExpectedNumNodes; ++i)
            CHECK(kExpectedBuffer[i] == uint32s[i]);
    }

    const auto dataset = Dataset::open(tmpFileName, BAG_OPEN_READ_WRITE);

    REQUIRE(dataset);

    REQUIRE_NOTHROW(dataset->getLayer(kLayerType));
    const auto& layer = dataset->getLayer(kLayerType);

    // Read the data back in from disk.
    auto buffer = layer.read(1, 2, 2, 4); // 2x3
    REQUIRE(buffer);

    uint32_t* uint32s = reinterpret_cast<uint32_t*>(buffer.get());
    for (size_t i=0; i<kExpectedNumNodes; ++i)
        CHECK(kExpectedBuffer[i] == uint32s[i]);
}

