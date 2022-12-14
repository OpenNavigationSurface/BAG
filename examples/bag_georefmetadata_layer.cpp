
#include <bag_metadataprofiles.h>
#include <bag_dataset.h>
#include <bag_metadata.h>
#include <bag_simplelayer.h>
#include <bag_compounddatatype.h>
#include <bag_surfacecorrections.h>
#include <bag_surfacecorrectionsdescriptor.h>
#include <bag_types.h>

#include <array>
#include <cstdlib>
#include <iostream>
#include <memory>


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
        std::cerr << "Usage is: bag_georefmetadata_layer_create <inputXMLFile> <outputBagFile>\n";
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
    std::cout << "Creating the BAG from XML file metadata..." << std::endl;

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
        constexpr uint64_t chunkSize = 100;
        constexpr int compressionLevel = 1;

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
    auto elevationLayer = dataset->getSimpleLayer(Elevation);

    // Set the min/max values (optional).
    // NOTE: Layer::write() calls update min/max.
    {
        const std::array<float, 2> surfRange{-10.0f,
            -10.0f - ((kGridSize - 1) * (kGridSize - 1) + kGridSize) / 10.0f};

        auto pDescriptor = elevationLayer->getDescriptor();
        pDescriptor->setMinMax(surfRange[0], surfRange[1]);

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
    auto uncertaintyLayer = dataset->getSimpleLayer(Uncertainty);

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


    // Add a georeferenced metadata layer (additional metadata) for elevation.

    // Using Figure 4 from the specification as an example
    // see https://github.com/OpenNavigationSurface/BAG/issues/2

    try
    {
        const auto& simpleLayerName = elevationLayer->getDescriptor()->getName();
        constexpr uint64_t chunkSize = 100;
        constexpr unsigned int compressionLevel = 1;
        auto& georefMetaLayer = dataset->createGeorefMetadataLayer(
                BAG::GeorefMetadataProfile::NOAA_OCS_2022_10_METADATA_PROFILE,
                simpleLayerName, chunkSize, compressionLevel);

        // At this point, all entries in the georeferenced metadata layer point to index 0,
        // which is a no data value.

        // Write a couple records.

        using BAG::CompoundDataType;

        BAG::Record record = BAG::CreateRecord_NOAA_OCS_2022_10(
                false,     // significant_features
                true,      // feature_least_depth
                1234.567f,      // feature_size
                765.4321f,   // feature_size_var
                true,            // coverage
                false,       // bathy_coverage
                9.87f,  // horizontal_uncert_fixed
                1.23f,   // horizontal_uncert_var
                std::string{"2019-04-01 00:00:00.0Z"}, // survey_data_start
                std::string{"2019-04-01 12:00:00.0Z"},  // survey_date_end
                std::string("NOAA"),                   // source_institution
                std::string("CD71EB77-5812-4735-B728-0DC1AE2A2F3B"),            // source_survey_id
                42,                                                              // source_survey_index
                std::string("Creative Commons Zero Public Domain Dedication (CC0)"),  // license_name
                std::string("https://creativecommons.org/publicdomain/zero/1.0/")   // license_url
        );

        auto& valueTable = georefMetaLayer.getValueTable();

        // Store the new record in memory and in the BAG.
        const auto firstRecordIndex = valueTable.addRecord(record);

        record = BAG::CreateRecord_NOAA_OCS_2022_10(
                true,         // significant_features
                false,        // feature_least_depth
                987.6f,            // feature_size
                6.789f,         // feature_size_var
                false,              // coverage
                true,           // bathy_coverage
                12345.67f, // horizontal_uncert_fixed
                89.0f,      // horizontal_uncert_var
                std::string{"2019-04-02 00:00:00.0Z"},  // survey_data_start
                std::string{"2019-04-02 12:00:00.0Z"},   // survey_date_end
                std::string("NOAA"),                    // source_institution
                std::string("15B46F99-1D94-4669-92D8-AA86F533B097"),            // source_survey_id
                23,                                                              // source_survey_index
                std::string("Open Data Commons Public Domain Dedication and Licence (PDDL)"), // license_name
                std::string("http://opendatacommons.org/licenses/pddl/1.0/")        // license_url
        );

        // Store the new record in memory and in the BAG.
        const auto secondRecordIndex = valueTable.addRecord(record);

        uint32_t numRows = 0;
        uint32_t numColumns = 0;
        std::tie(numRows, numColumns) = dataset->getDescriptor().getDims();

        // Set up the georeferenced metadata layer to point to the new records.
        // Let's say the first 5 rows of elevation should use the first record
        // index, and the next 3 columns use the second record index.

        // Start at row 0, go to (including) row 4.
        // Use the entire column.
        uint32_t rowStart = 0;
        uint32_t columnStart = 0;
        uint32_t rowEnd = 4;
        uint32_t columnEnd = numColumns - 1;

        // Create the buffer.  The type depends on the indexType used when
        // creating the georeferenced metadata layer.
        // The buffer contains the first record's index covering the first four
        // rows (across all the columns).
        size_t numElements = (rowEnd - rowStart + 1) * numColumns;
        const std::vector<uint16_t> firstBuffer(numElements, firstRecordIndex);

        georefMetaLayer.write(rowStart, columnStart, rowEnd, columnEnd,
            reinterpret_cast<const uint8_t*>(firstBuffer.data()));

        // Start at row 6, go to the last row.
        // Start at column 0, go to (including) column 2.
        rowStart = 5;
        columnStart = 0;
        rowEnd = numRows - 1;
        columnEnd = 2;

        // Create the buffer.  The type depends on the indexType used when
        // creating the georeferenced metadata layer.
        // The buffer contains the second record's index covering the first four
        // rows (across all the columns).
        numElements = (rowEnd - rowStart + 1) * (columnEnd - columnStart + 1);
        const std::vector<uint16_t> secondBuffer(numElements, secondRecordIndex);

        georefMetaLayer.write(rowStart, columnStart, rowEnd, columnEnd,
            reinterpret_cast<const uint8_t*>(secondBuffer.data()));

        // Read the data back.
        // Get the georeferenced metadata layer records specified by the fifth and sixth rows,
        // second and third columns.

        rowStart = 4;  // fifth row
        columnStart = 1;  // second column
        rowEnd = 5;  // sixth row
        columnEnd = 2;  // third column

        auto buff = georefMetaLayer.read(rowStart, columnStart, rowEnd,
            columnEnd);


        // With the indices, look at some values using the value table.
        // Room for 4 indices and initialize them with 0.
        auto* buffer = reinterpret_cast<uint16_t*>(buff.data());
        numElements = (rowEnd - rowStart + 1) * (columnEnd - columnStart + 1);

        const auto& records = valueTable.getRecords();

        for (size_t i=0; i<numElements; ++i)
        {
            const auto recordIndex = buffer[i];

            
            {  // Retrieve values via the ValueTable::getValue().
                // Get survey_date_start by field name
                const auto& surveyDateStart = valueTable.getValue(recordIndex,
                    "survey_date_start");

                std::cout << "survey_date_start is " << surveyDateStart.asString()
                    << " from record index: " << recordIndex << '\n';

                // Get feature_size by field index.
                const auto fieldIndex = valueTable.getFieldIndex("feature_size");
                const auto& featureSize = valueTable.getValue(recordIndex,
                    fieldIndex);

                std::cout << "feature_size is " << featureSize.asFloat()
                    << " from record index: " << recordIndex << '\n';
            }

            // Another way to grab the values using the records directly.
            // This only supports numerical indices.
            {
                // Get survey_date_start.
                auto fieldIndex = valueTable.getFieldIndex("survey_date_start");
                const auto& surveyDateStart = records[recordIndex][fieldIndex];

                std::cout << "survey_date_start is " << surveyDateStart.asString()
                    << " from record index: " << recordIndex << '\n';

                // Get feature_size.
                fieldIndex = valueTable.getFieldIndex("feature_size");
                const auto& featureSize = records[recordIndex][fieldIndex];

                std::cout << "feature_size is " << featureSize.asFloat()
                    << " from record index: " << recordIndex << '\n';
            }

        }

    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "BAG with georeferenced metadata layer created\n";

    return EXIT_SUCCESS;
}

