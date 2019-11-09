
#include "test_utils.h"
#include <bag_dataset.h>  //TODO maybe mock this?
#include <bag_simplelayer.h>
#include <bag_types.h>

#include <array>
#include <catch2/catch.hpp>
#include <cstdlib>  // std::getenv
#include <string>


using BAG::Dataset;
using BAG::Layer;

//  const LayerDescriptor& getDescriptor() const;
TEST_CASE("test simple layer get name", "[simplelayer][getDescriptor]")
{
    const std::string bagFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"};

    const auto dataset = Dataset::open(bagFileName, BAG_OPEN_READONLY);

    REQUIRE(dataset);
    REQUIRE_NOTHROW(dataset->getLayer(Elevation));

    const auto& layer = dataset->getLayer(Elevation);

    REQUIRE_NOTHROW(layer.getDescriptor());

    const auto& descriptor = layer.getDescriptor();

    CHECK(std::string{descriptor.getName()} == BAG::kLayerTypeMapString.at(Elevation));
}

//  virtual std::unique_ptr<uint8_t[]> read(uint32_t rowStart,
//      uint32_t columnStart, uint32_t rowEnd, uint32_t columnEnd) const;
TEST_CASE("test simple layer read", "[simplelayer][read]")
{
    //TODO Redo this with an in-memory/temp file.
    const std::string bagFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/NAVO_data/JD211_public_Release_1-4_UTM.bag"};

    const auto dataset = Dataset::open(bagFileName, BAG_OPEN_READONLY);

    REQUIRE(dataset);

    constexpr BAG::LayerType kLayerType = Elevation;

    REQUIRE_NOTHROW(dataset->getLayer(kLayerType));
    const auto& elevLayer = dataset->getLayer(kLayerType);

    constexpr int32_t rowStart = 288;
    constexpr int32_t rowEnd = 289;
    constexpr int32_t columnStart = 249;
    constexpr int32_t columnEnd = 251;
    auto buffer = elevLayer.read(rowStart, columnStart, rowEnd, columnEnd); // 2x3

    constexpr size_t kExpectedNumNodes = 6;
    constexpr int32_t rows = (rowEnd - rowStart) + 1;
    constexpr int32_t columns = (columnEnd - columnStart) + 1;

    REQUIRE(buffer);
    REQUIRE((kExpectedNumNodes * sizeof(float)) ==
        (Layer::getElementSize(Layer::getDataType(kLayerType)) * rows * columns));

    std::array<float, kExpectedNumNodes> kExpectedBuffer{
        1'000'000.0f, -52.161003f, -52.172005f,
        1'000'000.0f, -52.177002f, -52.174004f};

    const float* floats = reinterpret_cast<const float*>(buffer.get());
    for (size_t i=0; i<kExpectedNumNodes; ++i)
        CHECK(kExpectedBuffer[i] == floats[i]);
}

//  virtual void write(uint32_t rowStart, uint32_t columnStart, uint32_t rowEnd,
//      uint32_t columnEnd, const uint8_t* buffer) const;
TEST_CASE("test simple layer write", "[.][simplelayer][write]")
{
    const std::string bagFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"};

    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(bagFileName, tmpFileName);

    constexpr BAG::LayerType kLayerType = Elevation;
    constexpr size_t kExpectedNumNodes = 12;

    // Scope dataset to force write to be saved to disk.
    {
        const auto dataset = Dataset::open(tmpFileName, BAG_OPEN_READ_WRITE);

        REQUIRE(dataset);

        REQUIRE_NOTHROW(dataset->getLayer(kLayerType));
        auto& elevLayer = dataset->getLayer(kLayerType);

        // Write all 1's.
        {
            std::array<uint8_t, kExpectedNumNodes> buffer;
            buffer.fill(1);

            REQUIRE_NOTHROW(elevLayer.write(1, 2, 3, 5, buffer.data())); // 3x4
        }

        // Read the data back in from memory(?).
        std::array<uint8_t, kExpectedNumNodes> kExpectedBuffer;
        kExpectedBuffer.fill(1);

        auto buffer = elevLayer.read(1, 2, 3, 5); // 3x4
        REQUIRE(buffer);

        for (size_t i=0; i<kExpectedNumNodes; ++i)
            CHECK(buffer[i] == kExpectedBuffer[i]);
    }

    const auto dataset = Dataset::open(tmpFileName, BAG_OPEN_READ_WRITE);

    REQUIRE(dataset);

    REQUIRE_NOTHROW(dataset->getLayer(kLayerType));
    const auto& elevLayer = dataset->getLayer(kLayerType);

    // Read the data back in from disk.
    std::array<uint8_t, kExpectedNumNodes> kExpectedBuffer;
    kExpectedBuffer.fill(1);

    auto buffer = elevLayer.read(1, 2, 3, 5); // 5x5
    REQUIRE(buffer);

    for (size_t i=0; i<kExpectedNumNodes; ++i)
        CHECK(buffer[i] == kExpectedBuffer[i]);
}

