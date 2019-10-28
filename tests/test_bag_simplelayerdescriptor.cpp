
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
    REQUIRE_NOTHROW(SimpleLayerDescriptor::create(Elevation));

    auto descriptor = SimpleLayerDescriptor::create(Elevation);
    UNSCOPED_INFO("Check that creating a simple layer descriptor returns something.");
    REQUIRE(descriptor);

    UNSCOPED_INFO("Check that the layer descriptor type matches that was created.");
    CHECK(descriptor->getLayerType() == Elevation);

    descriptor = SimpleLayerDescriptor::create(Uncertainty);
    UNSCOPED_INFO("Check that the layer descriptor type matches that was created.");
    CHECK(descriptor->getLayerType() == Uncertainty);
}

//  const std::string& getName() const & noexcept;
//  SimpleLayerDescriptor& setName(std::string inName) & noexcept;
TEST_CASE("test layer descriptor get/set name",
    "[simplelayerdescriptor][getName][setName]")
{
    const auto descriptor = SimpleLayerDescriptor::create(Elevation);

    UNSCOPED_INFO("Verify the default name of Elevation is what it should be.");
    CHECK(descriptor->getName() == kLayerTypeMapString.at(Elevation));

    const std::string kExpectedName{"Expected Name"};
    UNSCOPED_INFO("Verify setting the name of a layer descriptor does not throw.");
    REQUIRE_NOTHROW(descriptor->setName(kExpectedName));

    UNSCOPED_INFO("Verify the new name of Elevation is what it was set to.");
    CHECK(descriptor->getName() == kExpectedName);
}

//  DataType getDataType() const noexcept;
//  SimpleLayerDescriptor& setDataType(DataType inType) & noexcept;
TEST_CASE("test layer descriptor get/set data type",
    "[simplelayerdescriptor][getDataType][setDataType]")
{
    auto descriptor = SimpleLayerDescriptor::create(Elevation);

    UNSCOPED_INFO("Verify the data type of an Elevation layer descriptor is correct.");
    CHECK(descriptor->getDataType() == Layer::getDataType(Elevation));

    descriptor = SimpleLayerDescriptor::create(Num_Hypotheses);

    UNSCOPED_INFO("Verify the data type of an Num_Hypotheses layer descriptor is correct.");
    CHECK(descriptor->getDataType() == Layer::getDataType(Num_Hypotheses));
}

//  LayerType getLayerType() const noexcept;
//  SimpleLayerDescriptor& setLayerType(LayerType inType) & noexcept;
TEST_CASE("test layer descriptor get/set layer type",
    "[simplelayerdescriptor][getLayerType][setLayerType]")
{
    auto descriptor = SimpleLayerDescriptor::create(Elevation);

    UNSCOPED_INFO("Verify the layer type of an Elevation layer descriptor is correct.");
    CHECK(descriptor->getLayerType() == Elevation);

    descriptor = SimpleLayerDescriptor::create(Std_Dev);

    UNSCOPED_INFO("Verify the layer type of an Std_Dev layer descriptor is correct.");
    CHECK(descriptor->getDataType() == Layer::getDataType(Std_Dev));
}

//  std::tuple<double, double> getMinMax() const noexcept;
//  SimpleLayerDescriptor& setMinMax(std::tuple<double, double> inMinMax) & noexcept;
TEST_CASE("test layer descriptor get/set min max",
    "[simplelayerdescriptor][getMinMax][setMinMax]")
{
    auto descriptor = SimpleLayerDescriptor::create(Elevation);
    const std::tuple<float, float> kExpectedMinMax{1.2345f, 9876.54321f};

    UNSCOPED_INFO("Verify setting min max does not throw.");
    REQUIRE_NOTHROW(descriptor->setMinMax(kExpectedMinMax));

    UNSCOPED_INFO("Verify the set min max value was set correctly.");
    REQUIRE_NOTHROW(descriptor->getMinMax());
    const auto minMax = descriptor->getMinMax();
    CHECK(minMax == kExpectedMinMax);
}

//  const std::string& getInternalPath() const & noexcept;
//  SimpleLayerDescriptor& setInternalPath(std::string inPath) & noexcept;
TEST_CASE("test layer descriptor get/set internal path",
    "[simplelayerdescriptor][getInternalPath][setInternalPath]")
{
    auto descriptor = SimpleLayerDescriptor::create(Elevation);

    UNSCOPED_INFO("Verify Elevation internal path is as expected.");
    REQUIRE_NOTHROW(descriptor->getInternalPath());
    CHECK(descriptor->getInternalPath() == Layer::getInternalPath(Elevation));

    const std::string kExpectedInternalPath{"/some_random_path"};

    UNSCOPED_INFO("Verify setting an internal path does not throw.");
    REQUIRE_NOTHROW(descriptor->setInternalPath(kExpectedInternalPath));

    UNSCOPED_INFO("Verify getting the internal path does not throw.");
    REQUIRE_NOTHROW(descriptor->getInternalPath());

    UNSCOPED_INFO("Verify the internal path is properly set after setting it.");
    CHECK(descriptor->getInternalPath() == kExpectedInternalPath);
}

//  uint8_t getElementSize() const noexcept override;
//  SimpleLayerDescriptor& setElementSize(uint8_t inSize) & noexcept override;
TEST_CASE("test layer descriptor get/set element size",
    "[simplelayerdescriptor][getElementSize][setElementSize]")
{
    auto descriptor = SimpleLayerDescriptor::create(Elevation);

    UNSCOPED_INFO("Verify Elevation element size is as expected.");
    REQUIRE_NOTHROW(descriptor->getElementSize());
    CHECK(descriptor->getElementSize() == Layer::getElementSize(Layer::getDataType(Elevation)));

    const auto kExpectedElementSize = Layer::getElementSize(UINT32);

    UNSCOPED_INFO("Verify setting an element size does not throw.");
    REQUIRE_NOTHROW(descriptor->setElementSize(kExpectedElementSize));

    UNSCOPED_INFO("Verify the element size is properly set after setting it.");
    CHECK(descriptor->getElementSize() == kExpectedElementSize);
}

//  uint64_t getChunkSize() const noexcept;
//  Descriptor& setChunkSize(uint64_t inChunkSize) & noexcept;
TEST_CASE("test descriptor get/set chunk size",
    "[simplelayerdescriptor][getChunkSize][setChunkSize]")
{
    auto descriptor = SimpleLayerDescriptor::create(Elevation);

    constexpr uint64_t kExpectedChunkSize{1024};

    UNSCOPED_INFO("Verify setting the chunk size does not throw.");
    REQUIRE_NOTHROW(descriptor->setChunkSize(kExpectedChunkSize));

    UNSCOPED_INFO("Verify getting the chunk size does not throw.");
    REQUIRE_NOTHROW(descriptor->getChunkSize());

    UNSCOPED_INFO("Verify getting the chunk size matches the expected and does not narrow.");
    const uint64_t chunkSize{descriptor->getChunkSize()};
    CHECK(chunkSize == kExpectedChunkSize);
}

//  unsigned int getCompressionLevel() const noexcept;
//  Descriptor& setCompressionLevel(unsigned int inCompressionLevel) & noexcept;
TEST_CASE("test descriptor get/set compression level",
    "[simplelayerdescriptor][getCompressionLevel][setCompressionLevel]")
{
    auto descriptor = SimpleLayerDescriptor::create(Elevation);

    constexpr unsigned int kExpectedCompressionLevel{4};

    UNSCOPED_INFO("Verify setting the compression level does not throw.");
    REQUIRE_NOTHROW(descriptor->setCompressionLevel(kExpectedCompressionLevel));

    UNSCOPED_INFO("Verify getting the compression level does not throw.");
    REQUIRE_NOTHROW(descriptor->getCompressionLevel());

    UNSCOPED_INFO("Verify getting the compression level matches the expected and does not narrow.");
    const unsigned int compressionLevel{descriptor->getCompressionLevel()};
    CHECK(compressionLevel == kExpectedCompressionLevel);
}

//  std::tuple<uint64_t, uint64_t> getDims() const noexcept;
//  Descriptor& setDims(const std::tuple<uint64_t, uint64_t>& inDims) & noexcept;
TEST_CASE("test descriptor get/set dims", "[simplelayerdescriptor][getDims][setDims]")
{
    auto descriptor = SimpleLayerDescriptor::create(Elevation);

    const std::tuple<uint32_t, uint32_t> kExpectedDims{1001, 9982};

    UNSCOPED_INFO("Verify setting the dims does not throw.");
    REQUIRE_NOTHROW(descriptor->setDims(kExpectedDims));

    UNSCOPED_INFO("Verify getting the dims does not throw.");
    REQUIRE_NOTHROW(descriptor->getDims());

    UNSCOPED_INFO("Verify getting the dims matches the expected.");
    const std::tuple<uint64_t, uint64_t> dims{descriptor->getDims()};
    CHECK(dims == kExpectedDims);
}


