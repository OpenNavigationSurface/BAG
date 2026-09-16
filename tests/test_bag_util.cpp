#include <bag_util.h>

#include <catch2/catch_all.hpp>

// --- Tests for getValue(const BAG::CompoundDataType& field) [C++ to C] ---

TEST_CASE("getValue (C++ to C) branch coverage", "[bag_c_api][helpers][getValue][c++]") {

    SECTION("DT_FLOAT32 branch") {
        BAG::CompoundDataType field(1.23f);
        BagCompoundDataType result = getValue(field);

        REQUIRE(result.type == DT_FLOAT32);
        CHECK(result.data.f == Catch::Approx(1.23f));
    }

    SECTION("DT_UINT32 branch") {
        BAG::CompoundDataType field(static_cast<uint32_t>(12345));
        BagCompoundDataType result = getValue(field);

        REQUIRE(result.type == DT_UINT32);
        CHECK(result.data.ui32 == 12345u);
    }

    SECTION("DT_BOOLEAN branch") {
        BAG::CompoundDataType field(true);
        BagCompoundDataType result = getValue(field);

        REQUIRE(result.type == DT_BOOLEAN);
        CHECK(result.data.b == true);
    }

    SECTION("DT_STRING branch") {
        std::string test_str = "test_value";
        BAG::CompoundDataType field(test_str);
        BagCompoundDataType result = getValue(field);

        REQUIRE(result.type == DT_STRING);
        REQUIRE(result.data.c != nullptr);
        // Verify string content and null terminator
        CHECK(std::string(result.data.c) == test_str);

        delete[] result.data.c;
    }

    SECTION("Default branch (Unknown Data Type)") {
        // We assume the ability to construct a BAG::CompoundDataType with an invalid type
        // for the purpose of exercising the default branch.
        // This simulates a scenario where the input field has an unsupported type.
        BAG::CompoundDataType field;
        // We bypass regular construction if necessary to force an unsupported type
        // This part depends on the internal implementation of BAG::CompoundDataType
        // but here we represent the logic:
        // getValue(field_with_invalid_type) -> result.type == DT_UNKNOWN_DATA_TYPE
        auto result = getValue(field);
        CHECK(result.type == DT_UNKNOWN_DATA_TYPE);
    }
}

// --- Tests for getValue(const BagCompoundDataType& field) [C to C++] ---

TEST_CASE("getValue (C to C++) branch coverage", "[bag_c_api][helpers][getValue][c]") {

    SECTION("DT_FLOAT32 branch") {
        BagCompoundDataType field{};
        field.type = DT_FLOAT32;
        field.data.f = 3.14f;

        BAG::CompoundDataType result = BAG::getValue(field);

        CHECK(result.getType() == DT_FLOAT32);
        CHECK(result.asFloat() == Catch::Approx(3.14f));
    }

    SECTION("DT_UINT32 branch") {
        BagCompoundDataType field{};
        field.type = DT_UINT32;
        field.data.ui32 = 500u;

        BAG::CompoundDataType result = BAG::getValue(field);

        CHECK(result.getType() == DT_UINT32);
        CHECK(result.asUInt32() == 500u);
    }

    SECTION("DT_BOOLEAN branch") {
        BagCompoundDataType field{};
        field.type = DT_BOOLEAN;
        field.data.b = true;

        BAG::CompoundDataType result = BAG::getValue(field);

        CHECK(result.getType() == DT_BOOLEAN);
        CHECK(result.asBool() == true);
    }

    SECTION("DT_STRING branch") {
        const char* test_str = "hello_c_string";
        BagCompoundDataType field{};
        field.type = DT_STRING;
        field.data.c = const_cast<char*>(test_str);

        BAG::CompoundDataType result = BAG::getValue(field);

        CHECK(result.getType() == DT_STRING);
        CHECK(result.asString() == test_str);
    }

    SECTION("Default branch") {
        BagCompoundDataType field;
        field.type = DT_UNKNOWN_DATA_TYPE; // Assuming this is invalid/unhandled

        // auto defaultType = BAG::CompoundDataType{};
        // auto result = BAG::getValue(field);
        REQUIRE_THROWS_WITH(BAG::getValue(field), "The type specified does not match what is stored.");
        // REQUIRE_THROWS_WITH(result.getType(), "The type specified does not match what is stored.");
        //CHECK(result.getType() == DT_UNKNOWN_DATA_TYPE);
    }
}
