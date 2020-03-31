/************************************************************************
/      File:    bagcreate.cpp
/
/      Open Navigation Surface Working Group, 2005
/
/      - Initial implementation
/        Mark Paton, 7/22/2005
/
*************************************************************************/

#include <bag_dataset.h>
#include <bag_metadata.h>
#include <bag_simplelayer.h>
#include <bag_surfacecorrections.h>
#include <bag_surfacecorrectionsdescriptor.h>
#include <bag_types.h>

#include <array>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>


namespace {

constexpr uint32_t kGridSize = 100;
constexpr uint32_t kSepSize = 3;

}  // namespace

int main(
    int argc,
    char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage is: bag_create <inputXMLFile> <outputBagFile>\n";
        return EXIT_FAILURE;
    }

    const std::string xmlFileName = argv[1];  // Store the XML fileName
    const std::string outFileName = argv[2];  // Store the BAG fileName to write

    /* Configure the dynamic ranges for the data layers that we're going to write,
     * and set up the separation surface parameters.  We generate the elevation and
     * uncertainty layers one row at a time, and therefore don't have to make the
     * whole thing now.
     */

    // Initial construction from the XML metadata example file provided.

    printf( "Creating the BAG from XML file metadata, " );

    BAG::Metadata metadata;

    try
    {
        metadata.loadFromFile(xmlFileName);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    // Create the dataset.

    std::shared_ptr<BAG::Dataset> dataset;

    try
    {
        uint64_t chunkSize = 100;
        unsigned int compressionLevel = 1;

        dataset = BAG::Dataset::create(outFileName, std::move(metadata),
            chunkSize, compressionLevel);
    }
    catch(const std::exception& e)
    {
        std::cerr << "Error creating BAG file.\n";
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    // Write the elevation layer, constructing bogus data as we do so.

    auto* elevationLayer = dataset->getSimpleLayer(Elevation);

    // Set the min/max values (optional).
    // NOTE: Layer::write() calls update min/max.
    {
        const std::array<float, 2> surfRange{-10.0f,
            -10.0f - ((kGridSize - 1) * (kGridSize - 1) + kGridSize) / 10.0f};

        auto& descriptor = elevationLayer->getDescriptor();
        descriptor.setMinMax(surfRange[0], surfRange[1]);

        elevationLayer->writeAttributes();
    }

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
            return EXIT_FAILURE;
        }
    }

    // Write the uncertainty layer, constructing bogus data as we do so.

    auto* uncertaintyLayer = dataset->getSimpleLayer(Uncertainty);

    // Set the min/max values (optional).
    // NOTE: Layer::write() calls update min/max.
    {
        const std::array<float, 2> uncertRange{1.0f,
            1.0f + ((kGridSize - 1) * (kGridSize - 1) + kGridSize) / 100.0f};

        auto& descriptor = uncertaintyLayer->getDescriptor();
        descriptor.setMinMax(uncertRange[0], uncertRange[1]);

        uncertaintyLayer->writeAttributes();
    }

    // Write the data.
    std::array<float, kGridSize> uncert{};

    for(uint32_t row=0; row<kGridSize; ++row)
    {
        for (uint32_t column=0; column<kGridSize; ++column)
            uncert[column] = ((column * row) % kGridSize) / 1000.0f;

        try
        {
            const auto* buffer = reinterpret_cast<uint8_t*>(uncert.data());
            constexpr uint32_t columnStart = 0;
            constexpr uint32_t columnEnd = kGridSize - 1;

            uncertaintyLayer->write(row, columnStart, row, columnEnd, buffer);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            return EXIT_FAILURE;
        }
    }

	// Add optional nominal elevation dataset

    try
    {
        uint64_t chunkSize = 100;
        unsigned int compressionLevel = 1;

        auto& nominalElevationLayer = dataset->createSimpleLayer(
            Nominal_Elevation, chunkSize, compressionLevel);

        // Set the min/max values (optional).
        // NOTE: Layer::write() calls update min/max.
        {
	        const std::array<float, 2> nominalDepthRange{20.0f,
                20.0f + ((kGridSize - 1) * (kGridSize - 1) + kGridSize) / 20.0f};

            auto& descriptor = uncertaintyLayer->getDescriptor();
            descriptor.setMinMax(nominalDepthRange[0], nominalDepthRange[1]);

            nominalElevationLayer.writeAttributes();
        }

        // Write the data.
        std::array<float, kGridSize> nominalDepth{};

	    for (uint32_t row=0; row<kGridSize; ++row)
        {
            for (uint32_t column=0; column<kGridSize; ++column)
                nominalDepth[column] = ((column * row) % kGridSize) + 1.0f +
                    (column / static_cast<float>(kGridSize));

            auto buffer = reinterpret_cast<uint8_t*>(nominalDepth.data());
            constexpr uint32_t columnStart = 0;
            constexpr uint32_t columnEnd = kGridSize - 1;

            nominalElevationLayer.write(row, columnStart, row, columnEnd, buffer);
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    // Add optional sep elevation dataset.

    try
    {
        uint64_t chunkSize = 0;  //TODO look into this (hopefully turns off chunking)
        unsigned int compressionLevel = 0;
        constexpr uint8_t kNumCorrectors = 2;

        auto& correctionsLayer = dataset->createSurfaceCorrections(
            BAG_SURFACE_IRREGULARLY_SPACED, kNumCorrectors, chunkSize,
            compressionLevel);

        // Set the min/max values (optional).
        // NOTE: Layer::write() calls update min/max.
        auto* descriptor = dynamic_cast<BAG::SurfaceCorrectionsDescriptor*>(
            &correctionsLayer.getDescriptor());
        if (!descriptor)
        {
            std::cerr << "Internal error\n";
            return EXIT_FAILURE;
        }

        const std::array<float, 2> sepDepthRange{0.3333f, 103.333f};

        // Set the vertical datums.
        const std::string verticalDatums{"Test,Unknown"};
        descriptor->setVerticalDatums(verticalDatums);

        // Write the data.
        std::array<std::array<BAG::VerticalDatumCorrections, kSepSize>, kSepSize> sepDepth{};

        for (uint32_t row=0; row<kSepSize; ++row)
            for (uint32_t column=0; column<kSepSize; ++column)
            {
			    sepDepth[row][column].z[0] = -(row + 0.3333f) * (column + 1);
			    sepDepth[row][column].z[1] = (row + 0.55f) * (column + 1);
			    sepDepth[row][column].x = (row + 10.3333f) * (column + 1);
			    sepDepth[row][column].y = (row + 180.3333f) * (column + 1);
            }


        constexpr uint32_t columnStart = 0;
        constexpr uint32_t columnEnd = kSepSize - 1;

	    for(uint32_t row=0; row<kSepSize; ++row)
        {
            const auto* buffer = reinterpret_cast<uint8_t*>(sepDepth[row].data());
            correctionsLayer.write(row, columnStart, row, columnEnd, buffer);
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "BAG created\n";

    return EXIT_SUCCESS;
}

