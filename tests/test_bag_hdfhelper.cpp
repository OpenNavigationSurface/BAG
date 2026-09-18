#include <catch2/catch_all.hpp>

#include <H5Cpp.h>

#include <bag_hdfhelper.h>
#include <bag_private.h>


TEST_CASE("test HDF helper utility code, createH5compType", "[hdfhelper][createH5compType]")
{
    H5::CompType h;

    // Node group
    h = BAG::createH5compType(Hypothesis_Strength, NODE);
    CHECK(h.getMemberName(0) == "hyp_strength");
    CHECK(h.getMemberArrayType(0) == H5::PredType::NATIVE_FLOAT);
    h = BAG::createH5compType(Num_Hypotheses, NODE);
    CHECK(h.getMemberName(0) == "num_hypotheses");
    CHECK(h.getMemberArrayType(0) == H5::PredType::NATIVE_UINT32);

    REQUIRE_THROWS_AS(BAG::createH5compType(Std_Dev, NODE), BAG::UnsupportedLayerType);

    // Elevation group
    h = BAG::createH5compType(Shoal_Elevation, ELEVATION);
    CHECK(h.getMemberName(0) == "shoal_elevation");
    CHECK(h.getMemberArrayType(0) == H5::PredType::NATIVE_FLOAT);
    h = BAG::createH5compType(Std_Dev, ELEVATION);
    CHECK(h.getMemberName(0) == "stddev");
    CHECK(h.getMemberArrayType(0) == H5::PredType::NATIVE_FLOAT);
    h = BAG::createH5compType(Num_Soundings, ELEVATION);
    CHECK(h.getMemberName(0) == "num_soundings");
    CHECK(h.getMemberArrayType(0) == H5::PredType::NATIVE_INT32);
    REQUIRE_THROWS_AS(BAG::createH5compType(Num_Hypotheses, ELEVATION), BAG::UnsupportedLayerType);

    // Invalid group type
    REQUIRE_THROWS_AS(BAG::createH5compType(Num_Hypotheses, UNKNOWN_GROUP_TYPE), BAG::UnsupportedGroupType);
}

TEST_CASE("test HDF helper utility code, getH5fileType", "[hdfhelper][getH5fileType]")
{
    CHECK(BAG::getH5fileType(DT_UINT32) == H5::PredType::NATIVE_UINT32);
    CHECK(BAG::getH5fileType(DT_FLOAT32) == H5::PredType::NATIVE_FLOAT);
    CHECK(BAG::getH5fileType(DT_UINT8) == H5::PredType::NATIVE_UINT8);
    CHECK(BAG::getH5fileType(DT_UINT16) == H5::PredType::NATIVE_UINT16);
    CHECK(BAG::getH5fileType(DT_UINT64) == H5::PredType::NATIVE_UINT64);
    CHECK(BAG::getH5fileType(DT_BOOLEAN) == H5::PredType::NATIVE_HBOOL);
    CHECK(BAG::getH5fileType(DT_STRING) == H5::StrType{H5::PredType::C_S1, H5T_VARIABLE});
    REQUIRE_THROWS_AS(BAG::getH5fileType(DT_COMPOUND), BAG::UnsupportedDataType);
    REQUIRE_THROWS_AS(BAG::getH5fileType(DT_UNKNOWN_DATA_TYPE), BAG::UnsupportedDataType);
}

TEST_CASE("test HDF helper utility code, getH5memoryType", "[hdfhelper][getH5memoryType]")
{
    CHECK(BAG::getH5memoryType(DT_UINT32) == H5::PredType::NATIVE_UINT32);
    CHECK(BAG::getH5memoryType(DT_FLOAT32) == H5::PredType::NATIVE_FLOAT);
    CHECK(BAG::getH5memoryType(DT_UINT8) == H5::PredType::NATIVE_UINT8);
    CHECK(BAG::getH5memoryType(DT_UINT16) == H5::PredType::NATIVE_UINT16);
    CHECK(BAG::getH5memoryType(DT_UINT64) == H5::PredType::NATIVE_UINT64);
    CHECK(BAG::getH5memoryType(DT_BOOLEAN) == H5::PredType::NATIVE_HBOOL);
    CHECK(BAG::getH5memoryType(DT_STRING) == H5::StrType{H5::PredType::C_S1, H5T_VARIABLE});
    REQUIRE_THROWS_AS(BAG::getH5memoryType(DT_COMPOUND), BAG::UnsupportedDataType);
    REQUIRE_THROWS_AS(BAG::getH5memoryType(DT_UNKNOWN_DATA_TYPE), BAG::UnsupportedDataType);
}

TEST_CASE("test HDF helper utility code, createAttributes", "[hdfhelper][createAttributes]")
{
    // Use a dummy dataset to test the branch where paths are empty, the other branch is tested elsewhere.
    // TODO: poss in a real dataset and make sure that calling createAttributes with an empty path has no side effects.
    H5::DataSet h5dataSet = H5::DataSet();
    REQUIRE_NOTHROW(BAG::createAttributes(h5dataSet, H5::PredType::NATIVE_UINT32,{}));
}

TEST_CASE("test HDF helper utility code, writeAttributes", "[hdfhelper][writeAttributes]")
{
    // Use a dummy dataset to test the branch where paths are empty, the other branch is tested elsewhere.
    // TODO: poss in a real dataset and make sure that calling writeAttributes with an empty path has no side effects.
    H5::DataSet h5dataSet = H5::DataSet();
    REQUIRE_NOTHROW(BAG::writeAttributes(h5dataSet, H5::PredType::NATIVE_UINT32,
        std::numeric_limits<uint32_t>::max(), {}));
}
