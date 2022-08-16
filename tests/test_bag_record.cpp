
#include <string>

#include <catch2/catch_all.hpp>

#include <bag_compounddatatype.h>


using BAG::Record;
using BAG::CompoundDataType;


TEST_CASE("test record creation", "[record][constructor]") {
    UNSCOPED_INFO("Test record creation");
    Record r = BAG::Record{
        BAG::CompoundDataType{std::string{"123.45"}},
        BAG::CompoundDataType{42.229f},
        BAG::CompoundDataType{23u}
    };
    CHECK(r.size() == 3);
    CHECK(std::string{"123.45"} == r[0].asString());
    CHECK(Catch::Approx( 42.229f ) == r[1].asFloat());
    CHECK(23u == r[2].asUInt32());
}
