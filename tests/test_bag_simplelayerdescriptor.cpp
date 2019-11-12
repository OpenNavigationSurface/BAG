
#include <bag_descriptor.h>
#include <bag_layer.h>
#include <bag_simplelayerdescriptor.h>
#include <bag_types.h>

#include <catch2/catch.hpp>
#include <string>


using BAG::Layer;
using BAG::SimpleLayerDescriptor;
using BAG::kLayerTypeMapString;

// NOTE The base class is also tested here.

//  static std::shared_ptr<SimpleLayerDescriptor> create(LayerType type) noexcept;
TEST_CASE("test layer descriptor creation",
    "[simplelayerdescriptor][create][getLayerType]")
{
    UNSCOPED_INFO("Check that creating a simple layer descriptor returns something.");
    constexpr uint64_t kExpectedChunkSize = 100;
    constexpr unsigned int kExpectedCompressionLevel = 6;
    auto pDescriptor = SimpleLayerDescriptor::create(Elevation,
        kExpectedChunkSize, kExpectedCompressionLevel);
    REQUIRE(pDescriptor);

    UNSCOPED_INFO("Check that the layer descriptor type matches that was created.");
    CHECK(pDescriptor->getLayerType() == Elevation);

    UNSCOPED_INFO("Check the chunk size is read properly.");
    CHECK(pDescriptor->getChunkSize() == kExpectedChunkSize);

    UNSCOPED_INFO("Check the compression level is read properly.");
    CHECK(pDescriptor->getCompressionLevel() == kExpectedCompressionLevel);
}

//  const std::string& getName() const & noexcept;
//  SimpleLayerDescriptor& setName(std::string inName) & noexcept;
TEST_CASE("test layer descriptor get/set name",
    "[simplelayerdescriptor][getName][setName]")
{
    constexpr uint64_t kExpectedChunkSize = 100;
    constexpr unsigned int kExpectedCompressionLevel = 6;
    const auto pDescriptor = SimpleLayerDescriptor::create(Elevation,
        kExpectedChunkSize, kExpectedCompressionLevel);

    const std::string kExpectedName{"Expected Name"};
    UNSCOPED_INFO("Verify setting the name of a layer descriptor does not throw.");
    REQUIRE_NOTHROW(pDescriptor->setName(kExpectedName));

    UNSCOPED_INFO("Verify the new name of Elevation is what it was set to.");
    CHECK(pDescriptor->getName() == kExpectedName);
}

//  DataType getDataType() const noexcept;
TEST_CASE("test layer descriptor get data type",
    "[simplelayerdescriptor][getDataType]")
{
    constexpr uint64_t kExpectedChunkSize = 100;
    constexpr unsigned int kExpectedCompressionLevel = 6;
    auto pDescriptor = SimpleLayerDescriptor::create(Elevation,
        kExpectedChunkSize, kExpectedCompressionLevel);

    UNSCOPED_INFO("Verify the data type of an Elevation layer descriptor is correct.");
    CHECK(pDescriptor->getDataType() == Layer::getDataType(Elevation));

    pDescriptor = SimpleLayerDescriptor::create(Num_Hypotheses, kExpectedChunkSize,
        kExpectedCompressionLevel);

    UNSCOPED_INFO("Verify the data type of an Num_Hypotheses layer descriptor is correct.");
    CHECK(pDescriptor->getDataType() == Layer::getDataType(Num_Hypotheses));
}

//  LayerType getLayerType() const noexcept;
TEST_CASE("test layer descriptor get layer type",
    "[simplelayerdescriptor][getLayerType]")
{
    constexpr uint64_t kExpectedChunkSize = 100;
    constexpr unsigned int kExpectedCompressionLevel = 6;
    auto pDescriptor = SimpleLayerDescriptor::create(Elevation,
        kExpectedChunkSize, kExpectedCompressionLevel);

    UNSCOPED_INFO("Verify the layer type of an Elevation layer descriptor is correct.");
    CHECK(pDescriptor->getLayerType() == Elevation);

    pDescriptor = SimpleLayerDescriptor::create(Std_Dev, kExpectedChunkSize,
        kExpectedCompressionLevel);

    UNSCOPED_INFO("Verify the layer type of an Std_Dev layer descriptor is correct.");
    CHECK(pDescriptor->getDataType() == Layer::getDataType(Std_Dev));
}

//  std::tuple<double, double> getMinMax() const noexcept;
//  SimpleLayerDescriptor& setMinMax(std::tuple<double, double> inMinMax) & noexcept;
TEST_CASE("test layer descriptor get/set min max",
    "[simplelayerdescriptor][getMinMax][setMinMax]")
{
    constexpr uint64_t kExpectedChunkSize = 100;
    constexpr unsigned int kExpectedCompressionLevel = 6;
    auto pDescriptor = SimpleLayerDescriptor::create(Elevation,
        kExpectedChunkSize, kExpectedCompressionLevel);

    UNSCOPED_INFO("Verify setting min max does not throw.");
    const float kExpectedMin = 1.2345f;
    const float kExpectedMax = 9876.54321f;
    REQUIRE_NOTHROW(pDescriptor->setMinMax(kExpectedMin, kExpectedMax));

    UNSCOPED_INFO("Verify the set min max value was set correctly.");
    const auto actualMinMax = pDescriptor->getMinMax();
    CHECK(actualMinMax == std::make_tuple(kExpectedMin, kExpectedMax));
}

//  const std::string& getInternalPath() const & noexcept;
TEST_CASE("test layer descriptor get internal path",
    "[simplelayerdescriptor][getInternalPath]")
{
    constexpr uint64_t kExpectedChunkSize = 100;
    constexpr unsigned int kExpectedCompressionLevel = 6;
    auto pDescriptor = SimpleLayerDescriptor::create(Elevation,
        kExpectedChunkSize, kExpectedCompressionLevel);

    UNSCOPED_INFO("Verify Elevation internal path is as expected.");
    REQUIRE_NOTHROW(pDescriptor->getInternalPath());
    CHECK(pDescriptor->getInternalPath() == Layer::getInternalPath(Elevation));

    pDescriptor = SimpleLayerDescriptor::create(Uncertainty,
        kExpectedChunkSize, kExpectedCompressionLevel);
    UNSCOPED_INFO("Verify Uncertainty internal path is as expected.");
    REQUIRE_NOTHROW(pDescriptor->getInternalPath());
    CHECK(pDescriptor->getInternalPath() == Layer::getInternalPath(Uncertainty));
}

//  uint8_t getElementSize() const noexcept override;
TEST_CASE("test layer descriptor get element size",
    "[simplelayerdescriptor][getElementSize]")
{
    constexpr uint64_t kExpectedChunkSize = 100;
    constexpr unsigned int kExpectedCompressionLevel = 6;
    auto pDescriptor = SimpleLayerDescriptor::create(Elevation,
        kExpectedChunkSize, kExpectedCompressionLevel);

    UNSCOPED_INFO("Verify Elevation element size is as expected.");
    REQUIRE_NOTHROW(pDescriptor->getElementSize());
    CHECK(pDescriptor->getElementSize() == Layer::getElementSize(Layer::getDataType(Elevation)));
}

//  uint64_t getChunkSize() const noexcept;
TEST_CASE("test descriptor get chunk size",
    "[simplelayerdescriptor][getChunkSize]")
{
    constexpr uint64_t kExpectedChunkSize = 100;
    constexpr unsigned int kExpectedCompressionLevel = 6;
    auto pDescriptor = SimpleLayerDescriptor::create(Elevation,
        kExpectedChunkSize, kExpectedCompressionLevel);

    UNSCOPED_INFO("Verify getting the chunk size does not throw.");
    REQUIRE_NOTHROW(pDescriptor->getChunkSize());

    UNSCOPED_INFO("Verify getting the chunk size matches the expected.");
    CHECK(pDescriptor->getChunkSize() == kExpectedChunkSize);
}

//  unsigned int getCompressionLevel() const noexcept;
TEST_CASE("test descriptor get compression level",
    "[simplelayerdescriptor][getCompressionLevel]")
{
    constexpr uint64_t kExpectedChunkSize = 100;
    constexpr unsigned int kExpectedCompressionLevel = 6;
    auto pDescriptor = SimpleLayerDescriptor::create(Elevation,
        kExpectedChunkSize, kExpectedCompressionLevel);

    UNSCOPED_INFO("Verify getting the compression level does not throw.");
    REQUIRE_NOTHROW(pDescriptor->getCompressionLevel());

    UNSCOPED_INFO("Verify getting the compression level matches the expected.");
    CHECK(pDescriptor->getCompressionLevel() == kExpectedCompressionLevel);
}

//  std::tuple<uint64_t, uint64_t> getDims() const noexcept;
//  Descriptor& setDims(const std::tuple<uint64_t, uint64_t>& inDims) & noexcept;
TEST_CASE("test descriptor get/set dims", "[simplelayerdescriptor][getDims][setDims]")
{
    constexpr uint64_t kExpectedChunkSize = 100;
    constexpr unsigned int kExpectedCompressionLevel = 6;
    auto pDescriptor = SimpleLayerDescriptor::create(Elevation,
        kExpectedChunkSize, kExpectedCompressionLevel);

    const std::tuple<uint32_t, uint32_t> kExpectedDims{1001, 9982};

    UNSCOPED_INFO("Verify setting the dims does not throw.");
    REQUIRE_NOTHROW(pDescriptor->setDims(kExpectedDims));

    UNSCOPED_INFO("Verify getting the dims does not throw.");
    REQUIRE_NOTHROW(pDescriptor->getDims());

    UNSCOPED_INFO("Verify getting the dims matches the expected.");
    const auto actualDims = pDescriptor->getDims();
    CHECK(actualDims == kExpectedDims);
}

