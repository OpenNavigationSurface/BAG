#include <array>
#include <stdlib.h>
#include <iostream>
#include <bag_dataset.h>
#include <bag_simplelayer.h>

using BAG::Dataset;

constexpr uint32_t kGridSize = 100;
constexpr uint32_t kSepSize = 3;

int main(int argc, char** argv) {
    const std::string xmlFileName = argv[1];
    const std::string outFileName = argv[2];

    // Read metadata from XML file
    BAG::Metadata metadata;
    try {
        metadata.loadFromFile(xmlFileName);
    } catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    // Create the dataset.
    std::shared_ptr<BAG::Dataset> dataset;
    try {
        constexpr uint64_t chunkSize = 100;
        constexpr int compressionLevel = 1;
        dataset = BAG::Dataset::create(outFileName, std::move(metadata),
            chunkSize, compressionLevel);
    } catch(const std::exception& e) {
        std::cerr << "Error creating BAG file." << std::endl;
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    // Write the elevation layer, constructing bogus data as we do so.
    auto elevationLayer = dataset->getSimpleLayer(Elevation);

    // Set the min/max values (optional).
    // NOTE: Layer::write() calls update min/max.
    {
        const std::array<float, 2> surfRange{-10.0f,
            -10.0f - ((kGridSize - 1) * (kGridSize - 1) + kGridSize) / 10.0f};
        auto pDescriptor = elevationLayer->getDescriptor();
        std::cout << "Elevation min: " << surfRange[0] << ", max: " << surfRange[1] << std::endl;
        pDescriptor->setMinMax(surfRange[0], surfRange[1]);
        elevationLayer->writeAttributes();
    }

    // Write the data.
    std::array<float, kGridSize> surf{};
    for (uint32_t row=0; row<kGridSize; ++row) {
        for (uint32_t column=0; column<kGridSize; ++column) {
            surf[column] = ((column * row) % kGridSize) +
                (column / static_cast<float>(kGridSize));
        }
        try {
            const auto* buffer = reinterpret_cast<uint8_t*>(surf.data());
            constexpr uint32_t columnStart = 0;
            constexpr uint32_t columnEnd = kGridSize - 1;
            elevationLayer->write(row, columnStart, row, columnEnd, buffer);
        } catch(const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }
    }

    // Write the uncertainty layer, constructing bogus data as we do so.
    auto uncertaintyLayer = dataset->getSimpleLayer(Uncertainty);

    // Set the min/max values (optional).
    // NOTE: Layer::write() calls update min/max.
    {
        const std::array<float, 2> uncertRange{1.0f,
            1.0f + ((kGridSize - 1) * (kGridSize - 1) + kGridSize) / 100.0f};
        auto pDescriptor = uncertaintyLayer->getDescriptor();
        std::cout << "Uncertainty min: " << uncertRange[0] << ", max: " << uncertRange[1] << std::endl;
        pDescriptor->setMinMax(uncertRange[0], uncertRange[1]);
        uncertaintyLayer->writeAttributes();
    }

    // Write the data.
    std::array<float, kGridSize> uncert{};
    for (uint32_t row=0; row<kGridSize; ++row) {
        for (uint32_t column=0; column<kGridSize; ++column)
            uncert[column] = ((column * row) % kGridSize) / 1000.0f;
        try {
            const auto* buffer = reinterpret_cast<uint8_t*>(uncert.data());
            constexpr uint32_t columnStart = 0;
            constexpr uint32_t columnEnd = kGridSize - 1;
            uncertaintyLayer->write(row, columnStart, row, columnEnd, buffer);
        } catch(const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}