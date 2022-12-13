
#include "test_utils.h"

#include <catch2/catch_all.hpp>
#include <cstdlib>
#include <utility>

#include <bag_simplelayer.h>
#include <bag_surfacecorrections.h>
#include <bag_surfacecorrectionsdescriptor.h>

#include <bag_private.h>
#include <H5Cpp.h>

using BAG::Dataset;

namespace {

    TEST_CASE("test dataset with georeferenced metadata layer of unknown metadata profile type",
              "[dataset][create][georefMetadataLayer][unknown]")
    {
        const std::string metadataFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
                                           "/sample.xml"};
        const TestUtils::RandomFileGuard tmpBagFileName;

        // Create BAG with unknown metadata profile
        {
            UNSCOPED_INFO("Create dataset with unknown metadata profile.");
            const auto result = TestUtils::createBag(metadataFileName,
                                                     tmpBagFileName);
            std::shared_ptr<BAG::Dataset> dataset = result.first;
            REQUIRE(dataset);
            std::string elevationLayerName = result.second;

            TestUtils::create_unknown_metadata(elevationLayerName, dataset);
            dataset->close();
        }

        // Open the dataset and read some values back
        {
            UNSCOPED_INFO("Open dataset with unknown metadata profile.");
            const auto datasetRO = Dataset::open(tmpBagFileName, BAG_OPEN_READONLY);
            REQUIRE(datasetRO);
            REQUIRE(datasetRO->getLayers().size() == 3);

            uint32_t numRows = 0;
            uint32_t numColumns = 0;
            std::tie(numRows, numColumns) = datasetRO->getDescriptor().getDims();

            std::string elevationLayerName = "elevation";
            const auto& compoundLayer = datasetRO->getGeorefMetadataLayer(elevationLayerName);
            REQUIRE(compoundLayer);

            const auto& valueTable = compoundLayer->getValueTable();

            {
                // Read region of georeferenced metadata layer raster associated with first index, check values
                auto buff = compoundLayer->read(0, 0, 4,
                                                numColumns - 1);
                auto* buffer = reinterpret_cast<uint16_t*>(buff.data());

                const auto recordIndex = buffer[0];
                const auto &dummy_int = valueTable.getValue(recordIndex,
                                                                  "dummy_int");
                REQUIRE(1u == dummy_int.asUInt32());

                const auto fieldIndex = valueTable.getFieldIndex("dummy_float");
                const auto &dummy_float = valueTable.getValue(recordIndex,
                                                              fieldIndex);
                REQUIRE(123.456f == dummy_float.asFloat());
            }

            {
                // Read region of georeferenced metadata layer raster associated with first index, check values
                auto buff = compoundLayer->read(5, 0, numRows - 1,
                                                numColumns - 1);
                auto* buffer = reinterpret_cast<uint16_t*>(buff.data());

                const auto recordIndex = buffer[0];
                const auto &dummy_int = valueTable.getValue(recordIndex,
                                                             "dummy_int");
                REQUIRE(2u == dummy_int.asUInt32());

                const auto fieldIndex = valueTable.getFieldIndex("dummy_float");
                const auto &dummy_float = valueTable.getValue(recordIndex,
                                                               fieldIndex);
                REQUIRE(456.123f == dummy_float.asFloat());
            }
        }

    }

    TEST_CASE("test dataset NOAA OCS 2022-10 metadata profile creation", "[dataset][create][compoundLayer][NOAA_OCS_2022_10]")
    {
        const std::string metadataFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
                                           "/sample.xml"};
        const TestUtils::RandomFileGuard tmpBagFileName;

        // Create BAG with NOAA NBS 2022-06 metadata profile
        const auto result = TestUtils::createBag(metadataFileName,
                             tmpBagFileName);
        std::shared_ptr<BAG::Dataset> dataset = result.first;
        std::string elevationLayerName = result.second;
        TestUtils::create_NOAA_OCS_Metadata(elevationLayerName, dataset);
        REQUIRE(dataset->getLayers().size() == 3);
        dataset->close();

        // Read BAG from disk
        const auto datasetRO = Dataset::open(tmpBagFileName, BAG_OPEN_READONLY);
        REQUIRE(datasetRO);
        REQUIRE(datasetRO->getLayers().size() == 3);

        // Read NOAA OCS 2022-10 georeferenced metadata layer and make sure metadata profile is properly declared and defined.
        const auto& compoundLayer = datasetRO->getGeorefMetadataLayer(elevationLayerName);
        REQUIRE(compoundLayer);

        const auto& valueTable = compoundLayer->getValueTable();

        // Get the georeferenced metadata layer records specified by the fifth and sixth rows,
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

    TEST_CASE("test dataset with georeferenced metadata layer with undefined metadata profile attribute",
              "[dataset][create][georefMetadataLayer][undefined_metadata_profile]")
    {
        const std::string metadataFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
                                           "/sample.xml"};
        const TestUtils::RandomFileGuard tmpBagFileName;

        // Create BAG with unknown metadata profile
        {
            UNSCOPED_INFO("Create dataset with unknown metadata profile.");
            const auto result = TestUtils::createBag(metadataFileName,
                                                     tmpBagFileName);
            std::shared_ptr<BAG::Dataset> dataset = result.first;
            REQUIRE(dataset);
            std::string elevationLayerName = result.second;

            TestUtils::create_unknown_metadata(elevationLayerName, dataset);
            dataset->close();

            // Go behind the BAG library's back to delete the metadata profile attribute from the HDF5 file
            // so that we can test reading a BAG that doesn't declare the profile attribute in its georeferenced metadata layer.
            {
                ::H5::H5File *m_pH5file = new ::H5::H5File{tmpBagFileName,H5F_ACC_RDONLY};
                const std::string internalPath{GEOREF_METADATA_PATH + elevationLayerName + COMPOUND_KEYS};
                const auto h5dataSet = ::H5::DataSet{m_pH5file->openDataSet(internalPath)};
                h5dataSet.removeAttr(METADATA_PROFILE_TYPE);
                m_pH5file->close();
                delete m_pH5file;
            }
        }

        // Open the dataset to exercise code for handling undefined metadata profile
        {
            UNSCOPED_INFO("Open dataset with undefined metadata profile.");
            const auto datasetRO = Dataset::open(tmpBagFileName, BAG_OPEN_READONLY);
            REQUIRE(datasetRO);
            REQUIRE(datasetRO->getLayers().size() == 3);
        }
    }

    TEST_CASE("test dataset with georeferenced metadata layer with unrecognized metadata profile attribute",
              "[dataset][create][georefMetadataLayer][unrecognized_metadata_profile]")
    {
        const std::string metadataFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
                                           "/sample.xml"};
        const TestUtils::RandomFileGuard tmpBagFileName;

        // Create BAG with unknown metadata profile
        {
            UNSCOPED_INFO("Create dataset with unknown metadata profile.");
            const auto result = TestUtils::createBag(metadataFileName,
                                                     tmpBagFileName);
            std::shared_ptr<BAG::Dataset> dataset = result.first;
            REQUIRE(dataset);
            std::string elevationLayerName = result.second;

            TestUtils::create_unknown_metadata(elevationLayerName, dataset);
            dataset->close();

            // Go behind the BAG library's back to change the metadata profile attribute in the HDF5 file to be an
            // unrecognized value so that we can test reading a BAG with an unrecognized profile attribute in its
            // georeferenced metadata layer.
            {
                ::H5::H5File *m_pH5file = new ::H5::H5File{tmpBagFileName,H5F_ACC_RDONLY};
                const std::string internalPath{GEOREF_METADATA_PATH + elevationLayerName + COMPOUND_KEYS};
                const auto h5dataSet = ::H5::DataSet{m_pH5file->openDataSet(internalPath)};
                const auto metadataProfileAtt = h5dataSet.openAttribute(METADATA_PROFILE_TYPE);
                const auto profileAttType = ::H5::StrType{0, METADATA_PROFILE_LEN};
                metadataProfileAtt.write(profileAttType, "Some unknown profile");
                m_pH5file->close();
                delete m_pH5file;
            }
        }

        // Open the dataset to exercise code for handling unrecognized metadata profile
        {
            UNSCOPED_INFO("Open dataset with undefined metadata profile.");
            const auto datasetRO = Dataset::open(tmpBagFileName, BAG_OPEN_READONLY);
            REQUIRE(datasetRO);
            REQUIRE(datasetRO->getLayers().size() == 3);
        }
    }
}
