
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

        // Create BAG with S101 metadata profile
        const auto result = TestUtils::createBag(metadataFileName,
                             tmpBagFileName);
        std::shared_ptr<BAG::Dataset> dataset = result.first;
        std::string elevationLayerName = result.second;
        TestUtils::createS101Metadata(elevationLayerName, dataset);
        REQUIRE(dataset->getLayers().size() == 3);
        dataset->close();

        // Read BAG from disk
        const auto datasetRO = Dataset::open(tmpBagFileName, BAG_OPEN_READONLY);
        REQUIRE(datasetRO);
        REQUIRE(datasetRO->getLayers().size() == 3);

        // Read S101 compound layer and make sure metadata profile is properly declared and defined.
        const auto& compoundLayer = datasetRO->getCompoundLayer(elevationLayerName);
        REQUIRE(compoundLayer);

        const auto& valueTable = compoundLayer->getValueTable();

        // Get the compound layer records specified by the fifth and sixth rows,
        // second and third columns.
        uint32_t rowStart = 4;  // fifth row
        uint32_t columnStart = 1;  // second column
        uint32_t rowEnd = 5;  // sixth row
        uint32_t columnEnd = 2;  // third column

        auto buff = compoundLayer->read(rowStart, columnStart, rowEnd,
                                       columnEnd);

        // With the indices, look at some values using the value table.
        auto* buffer = reinterpret_cast<uint16_t*>(buff.data());

        {
            const auto recordIndex = buffer[0];
            const auto &surveyDateStart = valueTable.getValue(recordIndex,
                                                              "survey_date_start");
            REQUIRE("2019-04-01 00:00:00.0Z" == surveyDateStart.asString());

            const auto fieldIndex = valueTable.getFieldIndex("feature_size");
            const auto &featureSize = valueTable.getValue(recordIndex,
                                                          fieldIndex);
            REQUIRE(1234.56702f == featureSize.asFloat());
        }

        {
            const auto recordIndex = buffer[1];
            const auto &surveyDateStart = valueTable.getValue(recordIndex,
                                                              "survey_date_start");
            REQUIRE("2019-04-01 00:00:00.0Z" == surveyDateStart.asString());

            const auto fieldIndex = valueTable.getFieldIndex("feature_size");
            const auto &featureSize = valueTable.getValue(recordIndex,
                                                          fieldIndex);
            REQUIRE(1234.56702f == featureSize.asFloat());
        }

        {
            const auto recordIndex = buffer[2];
            const auto &surveyDateStart = valueTable.getValue(recordIndex,
                                                              "survey_date_start");
            REQUIRE("2019-04-02 00:00:00.0Z" == surveyDateStart.asString());

            const auto fieldIndex = valueTable.getFieldIndex("feature_size");
            const auto &featureSize = valueTable.getValue(recordIndex,
                                                          fieldIndex);
            REQUIRE(987.59998f == featureSize.asFloat());
        }

        {
            const auto recordIndex = buffer[3];
            const auto &surveyDateStart = valueTable.getValue(recordIndex,
                                                              "survey_date_start");
            REQUIRE("2019-04-02 00:00:00.0Z" == surveyDateStart.asString());

            const auto fieldIndex = valueTable.getFieldIndex("feature_size");
            const auto &featureSize = valueTable.getValue(recordIndex,
                                                          fieldIndex);
            REQUIRE(987.59998f == featureSize.asFloat());
        }

    }
}