
#include "test_utils.h"

#include <fstream>  // std::ifstream, std::ofstream
#include <array>
#include <utility>
#include <memory>
#include <iostream>

#include <catch2/catch.hpp>

#include <bag_dataset.h>
#include <bag_metadata.h>
#include <bag_simplelayer.h>
#include <bag_surfacecorrections.h>
#include <bag_surfacecorrectionsdescriptor.h>


using BAG::Dataset;

namespace TestUtils {

constexpr uint32_t kGridSize = 100;
constexpr uint32_t kSepSize = 3;

//! Helper to copy a source file to a destination file.
void copyFile(
    const std::string& source,
    const std::string& dest)
{
    const std::ifstream in{source, std::ios::binary|std::ios::in};
    std::ofstream out{dest, std::ios::binary|std::ios::out};

    out << in.rdbuf();
}

std::pair<std::shared_ptr<BAG::Dataset>, std::string>
    createBag(const std::string metadataFileName,
              const std::string bagFileName) {
    // Load sample metadata
    BAG::Metadata metadata;
    metadata.loadFromFile(metadataFileName);

    // Create the dataset
    constexpr uint64_t chunkSize = 100;
    constexpr int compressionLevel = 1;
    std::shared_ptr<BAG::Dataset> dataset = BAG::Dataset::create(bagFileName, std::move(metadata),
                                   chunkSize, compressionLevel);

    // Write the elevation layer, constructing bogus data as we do so.
    auto* elevationLayer = dataset->getSimpleLayer(Elevation);

    // Set the min/max values (optional).
    // NOTE: Layer::write() calls update min/max.
    {
        const std::array<float, 2> surfRange{-10.0f,
                                             -10.0f - ((kGridSize - 1) * (kGridSize - 1) + kGridSize) / 10.0f};

        auto pDescriptor = elevationLayer->getDescriptor();
        pDescriptor->setMinMax(surfRange[0], surfRange[1]);

        elevationLayer->writeAttributes();
    }
    std::string elevationLayerName = elevationLayer->getDescriptor()->getName();

    // Write the data.
    std::array<float, kGridSize> surf{};

    for(uint32_t row=0; row<kGridSize; ++row)
    {
        for (uint32_t column=0; column<kGridSize; ++column)
            surf[column] = ((column * row) % kGridSize) +
                           (column / static_cast<float>(kGridSize));

        try
        {
            const auto* buffer = reinterpret_cast<uint8_t*>(surf.data());
            constexpr uint32_t columnStart = 0;
            constexpr uint32_t columnEnd = kGridSize - 1;

            elevationLayer->write(row, columnStart, row, columnEnd, buffer);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            FAIL(e.what());
        }
    }

    // Write the uncertainty layer, constructing bogus data as we do so.
    auto* uncertaintyLayer = dataset->getSimpleLayer(Uncertainty);

    // Set the min/max values (optional).
    // NOTE: Layer::write() calls update min/max.
    {
        const std::array<float, 2> uncertRange{1.0f,
                                               1.0f + ((kGridSize - 1) * (kGridSize - 1) + kGridSize) / 100.0f};

        auto pDescriptor = uncertaintyLayer->getDescriptor();
        pDescriptor->setMinMax(uncertRange[0], uncertRange[1]);

        uncertaintyLayer->writeAttributes();
    }

    // Write the data.
    {
        std::array<float, kGridSize> uncert{};

        for (uint32_t row = 0; row < kGridSize; ++row) {
            for (uint32_t column = 0; column < kGridSize; ++column)
                uncert[column] = ((column * row) % kGridSize) / 1000.0f;

            try {
                const auto *buffer = reinterpret_cast<uint8_t *>(uncert.data());
                constexpr uint32_t columnStart = 0;
                constexpr uint32_t columnEnd = kGridSize - 1;

                uncertaintyLayer->write(row, columnStart, row, columnEnd, buffer);
            }
            catch (const std::exception &e) {
                std::cerr << e.what() << '\n';
                FAIL(e.what());
            }
        }
    }

    // Write the uncertainty layer, constructing bogus data as we do so.
    uncertaintyLayer = dataset->getSimpleLayer(Uncertainty);

    // Set the min/max values (optional).
    // NOTE: Layer::write() calls update min/max.
    {
        const std::array<float, 2> uncertRange{1.0f,
                                               1.0f + ((kGridSize - 1) * (kGridSize - 1) + kGridSize) / 100.0f};

        auto pDescriptor = uncertaintyLayer->getDescriptor();
        pDescriptor->setMinMax(uncertRange[0], uncertRange[1]);

        uncertaintyLayer->writeAttributes();
    }

    // Write the data.
    {
        std::array<float, kGridSize> uncert{};

        for (uint32_t row = 0; row < kGridSize; ++row) {
            for (uint32_t column = 0; column < kGridSize; ++column)
                uncert[column] = ((column * row) % kGridSize) / 1000.0f;

            try {
                const auto *buffer = reinterpret_cast<uint8_t *>(uncert.data());
                constexpr uint32_t columnStart = 0;
                constexpr uint32_t columnEnd = kGridSize - 1;

                uncertaintyLayer->write(row, columnStart, row, columnEnd, buffer);
            }
            catch (const std::exception &e) {
                std::cerr << e.what() << '\n';
                FAIL(e.what());
            }
        }
    }

    return std::pair<std::shared_ptr<BAG::Dataset>, std::string>{dataset, elevationLayerName};
}

}  // namespace TestUtils

