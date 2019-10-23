
#include <bag_descriptor.h>
#include <bag_simplelayerdescriptor.h>
#include <bag_types.h>

#include <catch2/catch.hpp>
#include <string>


using BAG::Descriptor;
using BAG::SimpleLayerDescriptor;

//  static std::shared_ptr<Dataset> open(const std::string &fileName,
//      OpenMode openMode);
TEST_CASE("test descriptor constructors and destructor",
    "[descriptor][constructor][destructor][move assignment][setVersion][getVersion]")
{
    UNSCOPED_INFO("Verify the Descriptor default constructor and destructor do not throw.");
    REQUIRE_NOTHROW(Descriptor{});

    Descriptor descriptor;

    const char kExpectedVersion[] = "1.5.0";

    UNSCOPED_INFO(R"(Verify setting the version to "1.5.0" does not throw.)");
    REQUIRE_NOTHROW(descriptor.setVersion(kExpectedVersion));

    UNSCOPED_INFO(R"(Verify getting the version is "1.5.0".)");
    REQUIRE(descriptor.getVersion() == kExpectedVersion);

    {  // Move assignment
        Descriptor descriptor2;

        UNSCOPED_INFO(R"(Verify a default constructed Descriptor has a version of "".)");
        CHECK(descriptor2.getVersion() == "");

        descriptor2 = std::move(descriptor);

        UNSCOPED_INFO("Verify move assignment works.");
        CHECK(descriptor2.getVersion() == kExpectedVersion);
    }
}

//  std::vector<LayerType> getLayerTypes() const;
TEST_CASE("test descriptor get layer types",
    "[descriptor][getLayerTypes][addLayerDescriptor]")
{
    Descriptor descriptor;

    UNSCOPED_INFO("Verify getting layer types when there are none does not throw.");
    REQUIRE_NOTHROW(descriptor.getLayerTypes());
    UNSCOPED_INFO("Verify there are no layer types by default.");
    CHECK(descriptor.getLayerTypes().size() == 0);

    UNSCOPED_INFO("Verify adding a layer descriptor does not throw.");
    for (auto type : {Elevation, Uncertainty})
        REQUIRE_NOTHROW(descriptor.addLayerDescriptor(
            *SimpleLayerDescriptor::create(type)));

    UNSCOPED_INFO("Verify getting layer types when there are some does not throw.");
    REQUIRE_NOTHROW(descriptor.getLayerTypes());

    const auto layerTypes = descriptor.getLayerTypes();
    UNSCOPED_INFO("Verify two layer types are found.");
    CHECK(layerTypes.size() == 2);
    UNSCOPED_INFO("Verify the Elevation layer type is found.");
    CHECK(std::find(begin(layerTypes), end(layerTypes), Elevation) != end(layerTypes));
    UNSCOPED_INFO("Verify the Uncertainty layer type is found.");
    CHECK(std::find(begin(layerTypes), end(layerTypes), Uncertainty) != end(layerTypes));
}

//  bool isReadOnly() const noexcept;
//  Descriptor& setReadOnly(bool readOnly) & noexcept;
TEST_CASE("test descriptor is/set read only",
    "[descriptor][isReadOnly][setReadOnly]")
{
    Descriptor descriptor;

    UNSCOPED_INFO("Check that reading the read only flag without setting it does not throw.");
    REQUIRE_NOTHROW(descriptor.isReadOnly());

    UNSCOPED_INFO("Check that the default read only flag is true.");
    CHECK(descriptor.isReadOnly() == true);

    UNSCOPED_INFO("Verify setting read only to true does not throw.");
    REQUIRE_NOTHROW(descriptor.setReadOnly(true));

    UNSCOPED_INFO("Verify setting read only to false does not throw.");
    REQUIRE_NOTHROW(descriptor.setReadOnly(false));

    UNSCOPED_INFO("Verify reading the read only flag does not throw.");
    REQUIRE_NOTHROW(descriptor.isReadOnly());

    UNSCOPED_INFO("Verify the read only flag is false after it was set to false.");
    CHECK(descriptor.isReadOnly() == false);
}

//  const std::vector<LayerDescriptor>& getLayerDescriptors() const & noexcept;
TEST_CASE("test descriptor get layer descriptors",
    "[descriptor][addLayerDescriptor][getLayerDescriptors]")
{
    Descriptor descriptor;

    UNSCOPED_INFO("Verify adding a layer descriptor does not throw.");
    for (auto type : {Elevation, Uncertainty})
        REQUIRE_NOTHROW(descriptor.addLayerDescriptor(
            *SimpleLayerDescriptor::create(type)));

    UNSCOPED_INFO("Verify getting layer descriptors when there are some does not throw.");
    REQUIRE_NOTHROW(descriptor.getLayerDescriptors());

    const auto& layerDescriptors = descriptor.getLayerDescriptors();
    CHECK(layerDescriptors.size() == 2);
}

//  const LayerDescriptor& getLayerDescriptor(size_t id) const &;
TEST_CASE("test descriptor get layer descriptor",
    "[descriptor][addLayerDescriptor][getLayerDescriptor]")
{
    Descriptor descriptor;

    UNSCOPED_INFO("Verify looking for a non-existing layer descriptor throws.");
    REQUIRE_THROWS(descriptor.getLayerDescriptor(Elevation));

    UNSCOPED_INFO("Verify adding Elevation and Uncertainty layer descriptors does not throw.");
    for (auto type : {Elevation, Uncertainty})
        REQUIRE_NOTHROW(descriptor.addLayerDescriptor(
            *SimpleLayerDescriptor::create(type)));

    UNSCOPED_INFO("Verify layer descriptor Elevation exists.");
    REQUIRE_NOTHROW(descriptor.getLayerDescriptor(Elevation));
    UNSCOPED_INFO("Verify layer descriptor Uncertainty exists.");
    REQUIRE_NOTHROW(descriptor.getLayerDescriptor(Uncertainty));
}


