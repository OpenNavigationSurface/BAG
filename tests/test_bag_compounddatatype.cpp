
#include <bag_compounddatatype.h>

#include <catch2/catch_all.hpp>
#include <string>


using BAG::CompoundDataType;

//  CompoundDataType() = default
//  DataType getType() const noexcept
TEST_CASE("test compound data type default creation",
    "[compounddatatype][constructor][getType]")
{
    UNSCOPED_INFO("Check the default type is unknown.");
    const CompoundDataType cdt;
    CHECK(cdt.getType() == DT_UNKNOWN_DATA_TYPE);
}

//  CompoundDataType(float value) noexcept
//  DataType getType() const noexcept
//  template <typename T>
//  T BAG::get(const CompondDataType& cdt)
TEST_CASE("test compound data type float creation",
    "[compounddatatype][constructor][getType][asFloat][get]")
{
    UNSCOPED_INFO("Check the type when constructed with a float is expected.");
    constexpr float kExpectedValue = 42.3f;
    constexpr auto kExpectedType = DT_FLOAT32;
    const CompoundDataType cdt{kExpectedValue};
    CHECK(cdt.getType() == kExpectedType);

    UNSCOPED_INFO("Check getting the float value via asFloat() works.");
    CHECK(cdt.asFloat() == kExpectedValue);

    UNSCOPED_INFO("Check getting the float value via BAG::get<float>() works.");
    CHECK(BAG::get<float>(cdt) == kExpectedValue);
}

//  CompoundDataType(uint32_t value) noexcept
//  DataType getType() const noexcept
//  template <typename T>
//  T BAG::get(const CompondDataType& cdt)
TEST_CASE("test compound data type uint32_t creation",
    "[compounddatatype][constructor][getType][asUInt32][get]")
{
    // Default constructor
    UNSCOPED_INFO("Check the default type is unknown.");
    constexpr uint32_t kExpectedValue = 42;
    constexpr auto kExpectedType = DT_UINT32;
    const CompoundDataType cdt{kExpectedValue};
    CHECK(cdt.getType() == kExpectedType);

    UNSCOPED_INFO("Check getting the uint32_t value via asUInt32() works.");
    CHECK(cdt.asUInt32() == kExpectedValue);

    UNSCOPED_INFO("Check getting the uint32_t value via BAG::get<uint32_t>() works.");
    CHECK(BAG::get<uint32_t>(cdt) == kExpectedValue);
}

//  CompoundDataType() = default
//  DataType getType() const noexcept
//  CompoundDataType& operator=(float rhs) noexcept
//  CompoundDataType& operator=(uint32_t rhs) noexcept
//  CompoundDataType& operator=(bool rhs) noexcept
//  CompoundDataType& operator=(std::string rhs) noexcept
//  CompoundDataType& operator=(CompoundDataType&& rhs)
//  template <typename T>
//  T BAG::get(const CompondDataType& cdt)
TEST_CASE("test compound data type assignment operators",
    "[compounddatatype][constructor][getType][get]")
{
    UNSCOPED_INFO("Check the default type is unknown.");
    CompoundDataType cdt;
    CHECK(cdt.getType() == DT_UNKNOWN_DATA_TYPE);

    {
        UNSCOPED_INFO("Check using the assignment operator with a uint32_t sets the type as expected");
        constexpr float kExpectedValue = 123.456f;
        cdt = kExpectedValue;
        CHECK(cdt.getType() == DT_FLOAT32);

        UNSCOPED_INFO("Check getting the float value via BAG::get() works.");
        CHECK(BAG::get<float>(cdt) == kExpectedValue);

        // Cover assigning to self
        auto *cdt_p = &cdt;
        cdt = *cdt_p;
        CHECK(BAG::get<float>(cdt) == kExpectedValue);
        CHECK(cdt.getType() == DT_FLOAT32);

        // Cover move assignment
        CompoundDataType dst;
        CompoundDataType src{kExpectedValue};
        dst = std::move(src);
        CHECK(dst.asFloat() == kExpectedValue);
    }

    {
        UNSCOPED_INFO("Check using the assignment operator with a float sets the type as expected");
        constexpr uint32_t kExpectedValue = 101;
        cdt = kExpectedValue;
        CHECK(cdt.getType() == DT_UINT32);

        UNSCOPED_INFO("Check getting the float value via BAG::get() works.");
        CHECK(BAG::get<uint32_t>(cdt) == kExpectedValue);

        // Cover move assignment
        CompoundDataType dst;
        CompoundDataType src{kExpectedValue};
        dst = std::move(src);
        CHECK(dst.asUInt32() == kExpectedValue);
    }

    {
        UNSCOPED_INFO("Check using the assignment operator with a bool sets the type as expected");
        constexpr bool kExpectedValue = false;
        cdt = kExpectedValue;
        CHECK(cdt.getType() == DT_BOOLEAN);

        UNSCOPED_INFO("Check getting the bool value via BAG::get() works.");
        CHECK(BAG::get<bool>(cdt) == kExpectedValue);

        // Cover move assignment
        CompoundDataType dst;
        CompoundDataType src{kExpectedValue};
        dst = std::move(src);
        CHECK(dst.asBool() == kExpectedValue);
    }

    {
        UNSCOPED_INFO("Check using the assignment operator with a std::string sets the type as expected");
        const std::string kExpectedValue{"once upon a time"};
        cdt = kExpectedValue;
        CHECK(cdt.getType() == DT_STRING);

        UNSCOPED_INFO("Check getting the string value via BAG::get() works.");
        CHECK(BAG::get<std::string>(cdt) == kExpectedValue);

        // Cover move assignment
        CompoundDataType dst;
        CompoundDataType src{kExpectedValue};
        dst = std::move(src);
        CHECK(dst.asString() == kExpectedValue);
    }

    {
        UNSCOPED_INFO("Check using the equality operator on two different types");
        const CompoundDataType cdtf{42.23f};
        const CompoundDataType cdtu{42u};
        CHECK(cdtf != cdtu);
    }
}

TEST_CASE("test compound data type invalid cases",
    "[compounddatatype][constructor][getType][get]")
{
    // Default constructor
    UNSCOPED_INFO("Check trying to get a float from an unknown type.");
    CompoundDataType cdt;
    REQUIRE_THROWS(BAG::get<float>(cdt));

    UNSCOPED_INFO("Check trying to get a uint32_t from an unknown type.");
    REQUIRE_THROWS(BAG::get<uint32_t>(cdt));

    UNSCOPED_INFO("Check trying to get a bool from an unknown type.");
    REQUIRE_THROWS(BAG::get<bool>(cdt));

    UNSCOPED_INFO("Check trying to get an std::string from an unknown type.");
    REQUIRE_THROWS(BAG::get<std::string>(cdt));

    {
        UNSCOPED_INFO("Check trying to get a uint32_t from a float.");
        constexpr float kExpectedValue = 123.456f;
        cdt = kExpectedValue;
        REQUIRE_THROWS(BAG::get<uint32_t>(cdt));
    }

    {
        UNSCOPED_INFO("Check trying to get a bool from a float.");
        constexpr float kExpectedValue = 123.456f;
        cdt = kExpectedValue;
        REQUIRE_THROWS(BAG::get<bool>(cdt));
        REQUIRE_THROWS_AS(cdt.asString(), BAG::InvalidType);
    }

    {
        UNSCOPED_INFO("Check trying to get a float from a uint32_t.");
        constexpr uint32_t kExpectedValue = 101;
        cdt = kExpectedValue;
        REQUIRE_THROWS(BAG::get<float>(cdt) == kExpectedValue);
        REQUIRE_THROWS_AS(BAG::get<std::string>(cdt), BAG::InvalidType);
    }
}
