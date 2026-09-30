
#include "bag_uint8array.h"

#include <catch2/catch_all.hpp>


namespace {

    TEST_CASE("test uint8array", "[uint8array]")
    {
        size_t bufferSize = 8;
        BAG::UInt8Array b{bufferSize};
        // Use [] operator
        for (uint8_t i = 0; i < bufferSize; i++) {
            b[i] = i;
            CHECK(b[i] == i);
        }
        // Test out-of-bounds access
        CHECK_THROWS_AS(b[42], std::out_of_range);
        const BAG::UInt8Array& ref = b;
        CHECK_THROWS_AS(ref[42], std::out_of_range);
        // Cover release() method.
        b.release();
    }

}
