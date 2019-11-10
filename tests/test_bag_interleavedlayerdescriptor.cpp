
#include <bag_interleavedlayer.h>
#include <bag_interleavedlayerdescriptor.h>

#include <catch2/catch.hpp>


using BAG::Layer;
using BAG::InterleavedLayer;
using BAG::InterleavedLayerDescriptor;

//  static std::shared_ptr<InterleavedLayerDescriptor> create(
//      LayerType layerType, GroupType groupType) noexcept;
TEST_CASE("test interleaved layer descriptor creation",
    "[interleavedlayerdescriptor][create]")
{
    UNSCOPED_INFO("Check that creating an interleaved layer descriptor does not throw.");
    constexpr uint64_t chunkSize = 100;
    constexpr unsigned int compressionLevel = 6;
    REQUIRE_NOTHROW(InterleavedLayerDescriptor::create(Hypothesis_Strength,
        NODE, chunkSize, compressionLevel));
}

//  GroupType getGroupType() const noexcept;
TEST_CASE("test interleaved layer descriptor get group type",
    "[interleavedlayerdescriptor][getGroupType]")
{
    constexpr uint64_t chunkSize = 100;
    constexpr unsigned int compressionLevel = 6;

    {
        const BAG::GroupType kExpectedGroup = NODE;
        auto descriptor = InterleavedLayerDescriptor::create(Hypothesis_Strength,
            kExpectedGroup, chunkSize, compressionLevel);

        UNSCOPED_INFO("Verify the get group type matches the constructor.");
        CHECK(descriptor->getGroupType() == kExpectedGroup);
    }
    {
        const BAG::GroupType kExpectedGroup = ELEVATION;
        auto descriptor = InterleavedLayerDescriptor::create(Num_Soundings,
            kExpectedGroup, chunkSize, compressionLevel);

        UNSCOPED_INFO("Verify the get group type matches the constructor.");
        CHECK(descriptor->getGroupType() == kExpectedGroup);
    }
    {
        UNSCOPED_INFO("Verify the interleaved layer creation throws with an "
            "invalid group and layer combination.");
        REQUIRE_THROWS(InterleavedLayerDescriptor::create(Hypothesis_Strength,
            ELEVATION, chunkSize, compressionLevel));
    }
}

//  uint8_t getElementSize() const noexcept override;
TEST_CASE("test interleaved layer descriptor get/set element size",
    "[interleavedlayerdescriptor][getElementSize][setElementSize]")
{
    constexpr uint64_t chunkSize = 100;
    constexpr unsigned int compressionLevel = 6;

    {
        const BAG::LayerType kExpectedLayerType = Num_Hypotheses;
        const BAG::GroupType kExpectedGroupType = NODE;
        auto descriptor = InterleavedLayerDescriptor::create(kExpectedLayerType,
            kExpectedGroupType, chunkSize, compressionLevel);

        UNSCOPED_INFO("Verify getting element size does not throw.");
        REQUIRE_NOTHROW(descriptor->getElementSize());
        UNSCOPED_INFO("Verify NODE size is as expected.");
        CHECK(descriptor->getElementSize() ==
            Layer::getElementSize(Layer::getDataType(kExpectedLayerType)));
    }
    {
        const BAG::LayerType kExpectedLayerType = Std_Dev;
        const BAG::GroupType kExpectedGroupType = ELEVATION;
        auto descriptor = InterleavedLayerDescriptor::create(kExpectedLayerType,
            kExpectedGroupType, chunkSize, compressionLevel);

        UNSCOPED_INFO("Verify getting element size does not throw.");
        REQUIRE_NOTHROW(descriptor->getElementSize());
        UNSCOPED_INFO("Verify NODE size is as expected.");
        CHECK(descriptor->getElementSize() ==
            Layer::getElementSize(Layer::getDataType(kExpectedLayerType)));
    }
}

