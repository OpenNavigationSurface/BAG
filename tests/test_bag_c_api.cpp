
#include <cstring>

#include <bag.h>
#include <bag_c_types.h>
#include <bag_errors.h>

#include <catch2/catch_all.hpp>

/**
 * @file test_bag_.cpp
 * @brief Test suite for the C API of the BAG library using Catch2 v3.
 *
 * Each test case is designed to be granular, with unique and descriptive tags
 * to allow for efficient filtering and execution.
 */

// --- FILE LIFECYCLE ---

TEST_CASE("FileOpen - Happy path", "[bag_c_api][lifecycle][file_open][happy]") {
    const std::string bagFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"};
    BagHandle* handle = nullptr;
    CHECK(bagFileOpen(&handle, BAG_OPEN_MODE::BAG_OPEN_READONLY, bagFileName.c_str()) == BAG_SUCCESS);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("FileOpen - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][lifecycle][file_open][errors]") {
    REQUIRE(bagFileOpen(nullptr, BAG_OPEN_MODE::BAG_OPEN_READONLY, "test.bag") == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("FileOpen - NULL filename returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][lifecycle][file_open][errors]") {
    BagHandle* handle = nullptr;
    REQUIRE(bagFileOpen(&handle, BAG_OPEN_MODE::BAG_OPEN_READONLY, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
}

TEST_CASE("FileOpen - Invalid filename returns error code", "[bag_c_api][lifecycle][file_open][errors]") {
    BagHandle* handle = nullptr;
    BagError err = bagFileOpen(&handle, BAG_OPEN_MODE::BAG_OPEN_READONLY, "non_existent_bag_file_12345.bag");
    REQUIRE(err == BAG_NO_FILE_FOUND);
    if (handle) bagFileClose(handle);
}

TEST_CASE("FileClose - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][lifecycle][file_close][errors]") {
    REQUIRE(bagFileClose(nullptr) == BAG_INVALID_BAG_HANDLE);
}

// --- CREATION ---

TEST_CASE("BagCreateFromBuffer - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][creation][buffer][errors]") {
    uint8_t dummy_metadata[] = {0xDE, 0xAD, 0xBE, 0xEF};
    REQUIRE(bagCreateFromBuffer(nullptr, "test.bag", dummy_metadata, sizeof(dummy_metadata)) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("BagCreateFromBuffer - NULL filename returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][creation][buffer][errors]") {
    BagHandle* handle = nullptr;
    uint8_t dummy_metadata[] = {0xDE, 0xAD, 0xBE, 0xEF};
    REQUIRE(bagCreateFromBuffer(&handle, nullptr, dummy_metadata, sizeof(dummy_metadata)) == BAG_INVALID_FUNCTION_ARGUMENT);
}

TEST_CASE("BagCreateFromBuffer - NULL buffer returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][creation][buffer][errors]") {
    BagHandle* handle = nullptr;
    REQUIRE(bagCreateFromBuffer(&handle, "test.bag", nullptr, 10) == BAG_INVALID_FUNCTION_ARGUMENT);
}

// --- GRID & SPATIAL INFO ---

TEST_CASE("GridDimensions - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][grid][errors]") {
    BagHandle* handle = nullptr;
    uint32_t rows, cols;
    REQUIRE(bagGetGridDimensions(handle, &rows, &cols) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("GridDimensions - NULL output pointers return BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][grid][errors]") {
    const std::string bagFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} + "/sample.bag"};

    BagHandle* handle = nullptr;
    uint32_t rows, cols;
    double r_space, c_space;
    double llx, lly, urx, ury;

    CHECK(bagGetGridDimensions(handle, nullptr, &cols) == BAG_INVALID_BAG_HANDLE);
    CHECK(bagGetSpacing(handle, &r_space, nullptr) == BAG_INVALID_BAG_HANDLE);
    CHECK(bagGetGeoCover(handle, &llx, &lly, &urx, &ury) == BAG_INVALID_BAG_HANDLE);
    // Now make the handle valid by opening a BAG and test invalid function argument invocations...
    REQUIRE(bagFileOpen(&handle, BAG_OPEN_MODE::BAG_OPEN_READONLY, bagFileName.c_str()) == BAG_SUCCESS);
    CHECK(bagGetGridDimensions(handle, nullptr, &cols) == BAG_INVALID_FUNCTION_ARGUMENT);
    CHECK(bagGetSpacing(handle, &r_space, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    CHECK(bagGetGeoCover(handle, &llx, &lly, nullptr, &ury) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GridSpacing - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][grid][errors]") {
    BagHandle* handle = nullptr;
    double r_space, c_space;
    REQUIRE(bagGetSpacing(handle, &r_space, &c_space) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("GeoCover - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][grid][errors]") {
    BagHandle* handle = nullptr;
    double llx, lly, urx, ury;
    REQUIRE(bagGetGeoCover(handle, &llx, &lly, &urx, &ury) == BAG_INVALID_BAG_HANDLE);
}

// --- LAYERS ---

TEST_CASE("NumLayers - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][layers][errors]") {
    BagHandle* handle = nullptr;
    uint32_t num_layers;
    REQUIRE(bagGetNumLayers(handle, &num_layers) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("ContainsLayer - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][layers][errors]") {
    BagHandle* handle = nullptr;
    BagError err;
    bool contains = bagContainsLayer(handle, static_cast<BAG_LAYER_TYPE>(0), "layer", &err);
    CHECK_FALSE(contains);
    REQUIRE(err == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("SimpleLayer MinMax - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][layers][errors]") {
    BagHandle* handle = nullptr;
    float min_v, max_v;
    REQUIRE(bagGetMinMaxSimple(handle, static_cast<BAG_LAYER_TYPE>(0), &min_v, &max_v) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("SimpleLayer MinMax - NULL handle on set returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][layers][errors]") {
    BagHandle* handle = nullptr;
    REQUIRE(bagSetMinMaxSimple(handle, static_cast<BAG_LAYER_TYPE>(0), 0.0f, 1.0f) == BAG_INVALID_BAG_HANDLE);
}

// --- MEMORY MANAGEMENT ---

TEST_CASE("BagAllocate - standard allocation works", "[bag_c_api][memory]") {
    const uint32_t size = 1024;
    uint8_t* buffer = bagAllocate(size);
    REQUIRE(buffer != nullptr);
    std::memset(buffer, 0xAA, size);
    for (uint32_t i = 0; i < size; ++i) {
        CHECK(buffer[i] == 0xAA);
    }
    bagFree(buffer);
}

TEST_CASE("BagAllocateBuffer - NULL handle returns nullptr", "[bag_c_api][memory][errors]") {
    BagHandle* handle = nullptr;
    BagError err;
    uint8_t* buffer = bagAllocateBuffer(handle, 10, 10, static_cast<BAG_LAYER_TYPE>(0), "layer", &err);
    CHECK(buffer == nullptr);
}

// --- ERRORS ---

TEST_CASE("BagGetErrorString - success case", "[bag_c_api][errors][string]") {
    uint8_t* error_str = nullptr;
    REQUIRE(bagGetErrorString(BAG_SUCCESS, &error_str) == BAG_SUCCESS);
    if (error_str) {
        std::string msg(reinterpret_cast<char*>(error_str));
        CHECK(msg == "Bag returned a successful completion");
    }
}

TEST_CASE("BagGetErrorString - error code case", "[bag_c_api][errors][string]") {
    uint8_t* error_str = nullptr;
    REQUIRE(bagGetErrorString(BAG_INVALID_BAG_HANDLE, &error_str) == BAG_SUCCESS);
    if (error_str) {
        std::string msg(reinterpret_cast<char*>(error_str));
        CHECK_FALSE(msg.empty());
    }
}

TEST_CASE("BagGetErrorString - NULL pointer returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][errors][string]") {
    uint8_t* error_str = nullptr;
    REQUIRE(bagGetErrorString(BAG_INVALID_BAG_HANDLE, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
}

// --- COORDINATES ---

TEST_CASE("ComputePosition - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][coordinates][errors]") {
    BagHandle* handle = nullptr;
    double x, y;
    REQUIRE(bagComputePostion(handle, 0, 0, &x, &y) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("ComputeIndex - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][coordinates][errors]") {
    BagHandle* handle = nullptr;
    uint32_t row, col;
    REQUIRE(bagComputeIndex(handle, 1.0, 1.0, &row, &col) == BAG_INVALID_BAG_HANDLE);
}

// --- VERTICAL CORRECTIONS ---

TEST_CASE("VerticalCorrections - NULL handle on read returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][vertical_corrections][errors]") {
    BagHandle* handle = nullptr;
    uint8_t datum[256];
    std::memset(datum, 0, 256);
    REQUIRE(bagReadCorrectorVerticalDatum(handle, 1, datum) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("VerticalCorrections - NULL handle on write returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][vertical_corrections][errors]") {
    BagHandle* handle = nullptr;
    uint8_t datum[256];
    std::memset(datum, 0, 256);
    REQUIRE(bagWriteCorrectorVerticalDatum(handle, 1, datum) == BAG_INVALID_BAG_HANDLE);
}
