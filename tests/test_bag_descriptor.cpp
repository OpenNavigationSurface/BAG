
#include "test_utils.h"

#include <bag_dataset.h>
#include <bag_descriptor.h>
#include <bag_metadata.h>
#include <bag_simplelayerdescriptor.h>
#include <bag_types.h>

#include <catch2/catch.hpp>
#include <string>


using BAG::Dataset;
using BAG::Descriptor;
using BAG::Metadata;
using BAG::SimpleLayerDescriptor;

namespace {

//TODO Find valid v1 metadata
//const std::string kXMLv1MetadataBuffer;

const std::string kXMLv2MetadataBuffer{R"(<?xml version="1.0" encoding="UTF-8"?>
<gmi:MI_Metadata xmlns:gmi="http://www.isotc211.org/2005/gmi" xmlns:gmd="http://www.isotc211.org/2005/gmd" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:gml="http://www.opengis.net/gml/3.2" xmlns:gco="http://www.isotc211.org/2005/gco" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:bag="http://www.opennavsurf.org/schema/bag" xsi:schemaLocation="http://www.opennavsurf.org/schema/bag http://www.opennavsurf.org/schema/bag/bag.xsd">
	<gmd:fileIdentifier>
		<gco:CharacterString>Unique Identifier</gco:CharacterString>
	</gmd:fileIdentifier>
	<gmd:language>
		<gmd:LanguageCode codeList="http://www.loc.gov/standards/iso639-2/" codeListValue="eng">eng</gmd:LanguageCode>
	</gmd:language>
	<gmd:characterSet>
		<gmd:MD_CharacterSetCode codeList="http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#MD_CharacterSetCode" codeListValue="utf8">utf8</gmd:MD_CharacterSetCode>
	</gmd:characterSet>
	<gmd:hierarchyLevel>
		<gmd:MD_ScopeCode codeList="http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#MD_ScopeCode" codeListValue="dataset">dataset</gmd:MD_ScopeCode>
	</gmd:hierarchyLevel>
	<gmd:contact>
		<gmd:CI_ResponsibleParty>
			<gmd:individualName>
				<gco:CharacterString>Name of individual responsible for the BAG</gco:CharacterString>
			</gmd:individualName>
			<gmd:role>
				<gmd:CI_RoleCode codeList="http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#CI_RoleCode" codeListValue="pointOfContact">pointOfContact</gmd:CI_RoleCode>
			</gmd:role>
		</gmd:CI_ResponsibleParty>
	</gmd:contact>
	<gmd:dateStamp>
		<gco:Date>2012-01-27</gco:Date>
	</gmd:dateStamp>
	<gmd:metadataStandardName>
		<gco:CharacterString>ISO 19115</gco:CharacterString>
	</gmd:metadataStandardName>
	<gmd:metadataStandardVersion>
		<gco:CharacterString>2003/Cor.1:2006</gco:CharacterString>
	</gmd:metadataStandardVersion>
	<gmd:spatialRepresentationInfo>
		<gmd:MD_Georectified>
			<gmd:numberOfDimensions>
				<gco:Integer>2</gco:Integer>
			</gmd:numberOfDimensions>
			<gmd:axisDimensionProperties>
				<gmd:MD_Dimension>
					<gmd:dimensionName>
						<gmd:MD_DimensionNameTypeCode codeList="http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#MD_DimensionNameTypeCode" codeListValue="row">row</gmd:MD_DimensionNameTypeCode>
					</gmd:dimensionName>
					<gmd:dimensionSize>
						<gco:Integer>100</gco:Integer>
					</gmd:dimensionSize>
					<gmd:resolution>
						<gco:Measure uom="Metres">10</gco:Measure>
					</gmd:resolution>
				</gmd:MD_Dimension>
			</gmd:axisDimensionProperties>
			<gmd:axisDimensionProperties>
				<gmd:MD_Dimension>
					<gmd:dimensionName>
						<gmd:MD_DimensionNameTypeCode codeList="http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#MD_DimensionNameTypeCode" codeListValue="column">column</gmd:MD_DimensionNameTypeCode>
					</gmd:dimensionName>
					<gmd:dimensionSize>
						<gco:Integer>100</gco:Integer>
					</gmd:dimensionSize>
					<gmd:resolution>
						<gco:Measure uom="Metres">10</gco:Measure>
					</gmd:resolution>
				</gmd:MD_Dimension>
			</gmd:axisDimensionProperties>
			<gmd:cellGeometry>
				<gmd:MD_CellGeometryCode codeList="http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#MD_CellGeometryCode" codeListValue="point">point</gmd:MD_CellGeometryCode>
			</gmd:cellGeometry>
			<gmd:transformationParameterAvailability>
				<gco:Boolean>1</gco:Boolean>
			</gmd:transformationParameterAvailability>
			<gmd:checkPointAvailability>
				<gco:Boolean>0</gco:Boolean>
			</gmd:checkPointAvailability>
			<gmd:cornerPoints>
				<gml:Point gml:id="id1">
					<gml:coordinates decimal="." cs="," ts=" ">687910.000000,5554620.000000 691590.000000,5562100.000000</gml:coordinates>
				</gml:Point>
			</gmd:cornerPoints>
			<gmd:pointInPixel>
				<gmd:MD_PixelOrientationCode>center</gmd:MD_PixelOrientationCode>
			</gmd:pointInPixel>
		</gmd:MD_Georectified>
	</gmd:spatialRepresentationInfo>
	<gmd:referenceSystemInfo>
		<gmd:MD_ReferenceSystem>
			<gmd:referenceSystemIdentifier>
				<gmd:RS_Identifier>
					<gmd:code>
						<gco:CharacterString>PROJCS["UTM-19N-Nad83",
    GEOGCS["unnamed",
        DATUM["North_American_Datum_1983",
            SPHEROID["North_American_Datum_1983",6378137,298.2572201434276],
            TOWGS84[0,0,0,0,0,0,0]],
        PRIMEM["Greenwich",0],
        UNIT["degree",0.0174532925199433],
        EXTENSION["Scaler","0,0,0,0.02,0.02,0.001"],
        EXTENSION["Source","CARIS"]],
    PROJECTION["Transverse_Mercator"],
    PARAMETER["latitude_of_origin",0],
    PARAMETER["central_meridian",-69],
    PARAMETER["scale_factor",0.9996],
    PARAMETER["false_easting",500000],
    PARAMETER["false_northing",0],
    UNIT["metre",1]]</gco:CharacterString>
					</gmd:code>
					<gmd:codeSpace>
						<gco:CharacterString>WKT</gco:CharacterString>
					</gmd:codeSpace>
				</gmd:RS_Identifier>
			</gmd:referenceSystemIdentifier>
		</gmd:MD_ReferenceSystem>
	</gmd:referenceSystemInfo>
	<gmd:referenceSystemInfo>
		<gmd:MD_ReferenceSystem>
			<gmd:referenceSystemIdentifier>
				<gmd:RS_Identifier>
					<gmd:code>
						<gco:CharacterString>VERT_CS["Alicante height",
    VERT_DATUM["Alicante",2000]]</gco:CharacterString>
					</gmd:code>
					<gmd:codeSpace>
						<gco:CharacterString>WKT</gco:CharacterString>
					</gmd:codeSpace>
				</gmd:RS_Identifier>
			</gmd:referenceSystemIdentifier>
		</gmd:MD_ReferenceSystem>
	</gmd:referenceSystemInfo>
	<gmd:identificationInfo>
		<bag:BAG_DataIdentification>
			<gmd:citation>
				<gmd:CI_Citation>
					<gmd:title>
						<gco:CharacterString>Name of dataset input</gco:CharacterString>
					</gmd:title>
					<gmd:date>
						<gmd:CI_Date>
							<gmd:date>
								<gco:Date>2008-10-21</gco:Date>
							</gmd:date>
							<gmd:dateType>
								<gmd:CI_DateTypeCode codeList="http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#CI_DateTypeCode" codeListValue="creation">creation</gmd:CI_DateTypeCode>
							</gmd:dateType>
						</gmd:CI_Date>
					</gmd:date>
					<gmd:citedResponsibleParty>
						<gmd:CI_ResponsibleParty>
							<gmd:individualName>
								<gco:CharacterString>Person responsible for input data</gco:CharacterString>
							</gmd:individualName>
							<gmd:role>
								<gmd:CI_RoleCode codeList="http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#CI_RoleCode" codeListValue="originator">originator</gmd:CI_RoleCode>
							</gmd:role>
						</gmd:CI_ResponsibleParty>
					</gmd:citedResponsibleParty>
				</gmd:CI_Citation>
			</gmd:citation>
			<gmd:abstract>
				<gco:CharacterString>Sample Metadata</gco:CharacterString>
			</gmd:abstract>
			<gmd:status>
				<gmd:MD_ProgressCode codeList="http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#MD_ProgressCode" codeListValue="completed">completed</gmd:MD_ProgressCode>
			</gmd:status>
			<gmd:spatialRepresentationType>
				<gmd:MD_SpatialRepresentationTypeCode codeList="http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#MD_SpatialRepresentationTypeCode" codeListValue="grid">grid</gmd:MD_SpatialRepresentationTypeCode>
			</gmd:spatialRepresentationType>
			<gmd:language>
				<gmd:LanguageCode codeList="http://www.loc.gov/standards/iso639-2/" codeListValue="eng">eng</gmd:LanguageCode>
			</gmd:language>
			<gmd:characterSet>
				<gmd:MD_CharacterSetCode codeList="http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#MD_CharacterSetCode" codeListValue="utf8">utf8</gmd:MD_CharacterSetCode>
			</gmd:characterSet>
			<gmd:topicCategory>
				<gmd:MD_TopicCategoryCode>elevation</gmd:MD_TopicCategoryCode>
			</gmd:topicCategory>
			<gmd:extent>
				<gmd:EX_Extent>
					<gmd:geographicElement>
						<gmd:EX_GeographicBoundingBox>
							<gmd:westBoundLongitude>
								<gco:Decimal>-66.371629</gco:Decimal>
							</gmd:westBoundLongitude>
							<gmd:eastBoundLongitude>
								<gco:Decimal>-66.316454</gco:Decimal>
							</gmd:eastBoundLongitude>
							<gmd:southBoundLatitude>
								<gco:Decimal>50.114053</gco:Decimal>
							</gmd:southBoundLatitude>
							<gmd:northBoundLatitude>
								<gco:Decimal>50.180077</gco:Decimal>
							</gmd:northBoundLatitude>
						</gmd:EX_GeographicBoundingBox>
					</gmd:geographicElement>
				</gmd:EX_Extent>
			</gmd:extent>
			<bag:verticalUncertaintyType>
                <bag:BAG_VertUncertCode codeList="http://www.opennavsurf.org/schema/bag/bagCodelists.xml#BAG_VertUncertCode" codeListValue="rawStdDev">rawStdDev</bag:BAG_VertUncertCode>
			</bag:verticalUncertaintyType>
			<bag:depthCorrectionType>
                <bag:BAG_DepthCorrectCode codeList="http://www.opennavsurf.org/schema/bag/bagCodelists.xml#BAG_DepthCorrectCode" codeListValue="trueDepth">trueDepth</bag:BAG_DepthCorrectCode>
			</bag:depthCorrectionType>
            <bag:elevationSolutionGroupType>
                <bag:BAG_OptGroupCode codeList="http://www.opennavsurf.org/schema/bag/bagCodelists.xml#BAG_OptGroupCode" codeListValue="cube">cube</bag:BAG_OptGroupCode>
			</bag:elevationSolutionGroupType>
            <bag:nodeGroupType>
                <bag:BAG_OptGroupCode codeList="http://www.opennavsurf.org/schema/bag/bagCodelists.xml#BAG_OptGroupCode" codeListValue="product">product</bag:BAG_OptGroupCode>
			</bag:nodeGroupType>
		</bag:BAG_DataIdentification>
	</gmd:identificationInfo>
	<gmd:dataQualityInfo>
		<gmd:DQ_DataQuality>
			<gmd:scope>
				<gmd:DQ_Scope>
					<gmd:level>
						<gmd:MD_ScopeCode codeList="http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#MD_ScopeCode" codeListValue="dataset">dataset</gmd:MD_ScopeCode>
					</gmd:level>
				</gmd:DQ_Scope>
			</gmd:scope>
			<gmd:lineage>
				<gmd:LI_Lineage>
					<gmd:processStep>
						<bag:BAG_ProcessStep>
							<gmd:description>
								<gco:CharacterString>List to be determined by WG. I.e. Product Creation</gco:CharacterString>
							</gmd:description>
							<gmd:dateTime>
								<gco:DateTime>2008-10-21T12:21:53</gco:DateTime>
							</gmd:dateTime>
							<gmd:processor>
								<gmd:CI_ResponsibleParty>
									<gmd:individualName>
										<gco:CharacterString>Name of the processor</gco:CharacterString>
									</gmd:individualName>
									<gmd:role>
										<gmd:CI_RoleCode codeList="http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#CI_RoleCode" codeListValue="processor">processor</gmd:CI_RoleCode>
									</gmd:role>
								</gmd:CI_ResponsibleParty>
							</gmd:processor>
							<gmd:source>
								<gmd:LI_Source>
									<gmd:description>
										<gco:CharacterString>Source</gco:CharacterString>
									</gmd:description>
									<gmd:sourceCitation>
										<gmd:CI_Citation>
											<gmd:title>
												<gco:CharacterString>Name of dataset input</gco:CharacterString>
											</gmd:title>
											<gmd:date>
												<gmd:CI_Date>
													<gmd:date gco:nilReason="unknown"/>
													<gmd:dateType>
														<gmd:CI_DateTypeCode codeList="http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#CI_DateTypeCode" codeListValue="creation">creation</gmd:CI_DateTypeCode>
													</gmd:dateType>
												</gmd:CI_Date>
											</gmd:date>
										</gmd:CI_Citation>
									</gmd:sourceCitation>
								</gmd:LI_Source>
							</gmd:source>
							<bag:trackingId>
								<gco:CharacterString>1</gco:CharacterString>
							</bag:trackingId>
						</bag:BAG_ProcessStep>
					</gmd:processStep>
				</gmd:LI_Lineage>
			</gmd:lineage>
		</gmd:DQ_DataQuality>
	</gmd:dataQualityInfo>
	<gmd:metadataConstraints>
		<gmd:MD_LegalConstraints>
			<gmd:useConstraints>
				<gmd:MD_RestrictionCode codeList="http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#MD_RestrictionCode" codeListValue="otherRestrictions">otherRestrictions</gmd:MD_RestrictionCode>
			</gmd:useConstraints>
			<gmd:otherConstraints>
				<gco:CharacterString>some other constraints</gco:CharacterString>
			</gmd:otherConstraints>
		</gmd:MD_LegalConstraints>
	</gmd:metadataConstraints>
	<gmd:metadataConstraints>
		<gmd:MD_SecurityConstraints>
			<gmd:classification>
				<gmd:MD_ClassificationCode codeList="http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#MD_ClassificationCode" codeListValue="unclassified">unclassified</gmd:MD_ClassificationCode>
			</gmd:classification>
			<gmd:userNote>
				<gco:CharacterString>some user node</gco:CharacterString>
			</gmd:userNote>
		</gmd:MD_SecurityConstraints>
	</gmd:metadataConstraints>
</gmi:MI_Metadata>)"};

}  // namespace

//  static std::shared_ptr<Dataset> open(const std::string &fileName,
//      OpenMode openMode);
TEST_CASE("test descriptor constructors and destructor",
    "[descriptor][constructor][destructor][move assignment][setVersion][getVersion]")
{
    UNSCOPED_INFO("Verify the Descriptor default constructor and destructor do not throw.");
    REQUIRE_NOTHROW(Descriptor{});

    Descriptor descriptor;

    constexpr char kExpectedVersion[] = "1.5.0";

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
    "[descriptor][getLayerTypes]")
{
	{
		Descriptor descriptor;

		UNSCOPED_INFO("Verify getting layer types when there are none does not throw.");
		REQUIRE_NOTHROW(descriptor.getLayerTypes());
		UNSCOPED_INFO("Verify there are no layer types by default.");
		CHECK(descriptor.getLayerTypes().size() == 0);
	}

    TestUtils::RandomFileGuard tmpBagFile;

    Metadata metadata;
    metadata.loadFromBuffer(kXMLv2MetadataBuffer);

    constexpr uint64_t chunkSize = 100;
    constexpr int compressionLevel = 6;

    auto pDataset = Dataset::create(tmpBagFile, std::move(metadata), chunkSize,
        compressionLevel);
    REQUIRE(pDataset);

	auto& descriptor = pDataset->getDescriptor();

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
    "[descriptor][getLayerDescriptors]")
{
    TestUtils::RandomFileGuard tmpBagFile;

    Metadata metadata;
    metadata.loadFromBuffer(kXMLv2MetadataBuffer);

    constexpr uint64_t chunkSize = 100;
    constexpr int compressionLevel = 6;

    auto pDataset = Dataset::create(tmpBagFile, std::move(metadata), chunkSize,
        compressionLevel);
    REQUIRE(pDataset);

	auto& descriptor = pDataset->getDescriptor();

    UNSCOPED_INFO("Verify getting layer descriptors when there are some does not throw.");
    REQUIRE_NOTHROW(descriptor.getLayerDescriptors());

    const auto& layerDescriptors = descriptor.getLayerDescriptors();
    CHECK(layerDescriptors.size() == 2);
}

//  const LayerDescriptor& getLayerDescriptor(uint32_t id) const &;
TEST_CASE("test descriptor get layer descriptor",
    "[descriptor][addLayerDescriptor][getLayerDescriptor]")
{
	{
		Descriptor descriptor;

		UNSCOPED_INFO("Verify looking for a non-existing layer descriptor throws.");
		REQUIRE_THROWS(descriptor.getLayerDescriptor(42));
	}

    TestUtils::RandomFileGuard tmpBagFile;

    Metadata metadata;
    metadata.loadFromBuffer(kXMLv2MetadataBuffer);

    constexpr uint64_t chunkSize = 100;
    constexpr int compressionLevel = 6;

    auto pDataset = Dataset::create(tmpBagFile, std::move(metadata), chunkSize,
        compressionLevel);
    REQUIRE(pDataset);

	auto& descriptor = pDataset->getDescriptor();

    UNSCOPED_INFO("Verify layer descriptor Elevation exists.");
    REQUIRE_NOTHROW(descriptor.getLayerDescriptor(Elevation));
    UNSCOPED_INFO("Verify layer descriptor Uncertainty exists.");
    REQUIRE_NOTHROW(descriptor.getLayerDescriptor(Uncertainty));
}

//  const std::string& getHorizontalReferenceSystem() const & noexcept;
//  const std::string& getVerticalReferenceSystem() const & noexcept;
//  const std::tuple<uint32_t, uint32_t>& getDims() const & noexcept;
//  const std::tuple<double, double, double, double>&
//      getProjectedCover() const & noexcept;
//  const std::tuple<double, double>& getOrigin() const & noexcept;
//  const std::tuple<double, double>& getGridSpacing() const & noexcept;
TEST_CASE("test descriptor created from metadata",
    "[descriptor][constructor][getHorizontalReferenceSystem]"
    "[getVerticalReferenceSystem][getDims][getProjectedCover]"
    "[getOrigin][getGridSpacing]")
{
    Metadata metadata;
    UNSCOPED_INFO("Load buffer into Metadata.");
    REQUIRE_NOTHROW(metadata.loadFromBuffer(kXMLv2MetadataBuffer));

    Descriptor descriptor(metadata);

    UNSCOPED_INFO("Check expected horizontal reference system.");
    const std::string kExpectedHorizontalReferenceSystem{R"(PROJCS["UTM-19N-Nad83",
    GEOGCS["unnamed",
        DATUM["North_American_Datum_1983",
            SPHEROID["North_American_Datum_1983",6378137,298.2572201434276],
            TOWGS84[0,0,0,0,0,0,0]],
        PRIMEM["Greenwich",0],
        UNIT["degree",0.0174532925199433],
        EXTENSION["Scaler","0,0,0,0.02,0.02,0.001"],
        EXTENSION["Source","CARIS"]],
    PROJECTION["Transverse_Mercator"],
    PARAMETER["latitude_of_origin",0],
    PARAMETER["central_meridian",-69],
    PARAMETER["scale_factor",0.9996],
    PARAMETER["false_easting",500000],
    PARAMETER["false_northing",0],
    UNIT["metre",1]])"};
    CHECK(descriptor.getHorizontalReferenceSystem() == kExpectedHorizontalReferenceSystem);

    UNSCOPED_INFO("Check expected vertical reference system.");
    const std::string kExpectedVerticalReferenceSystem{R"(VERT_CS["Alicante height",
    VERT_DATUM["Alicante",2000]])"};
    CHECK(descriptor.getVerticalReferenceSystem() == kExpectedVerticalReferenceSystem);

    UNSCOPED_INFO("Check expected dimensions.");
    const std::tuple<uint32_t, uint32_t> kExpectedDims{100, 100};
    CHECK(descriptor.getDims() == kExpectedDims);

    UNSCOPED_INFO("Check expected projected cover.");
    const std::tuple<double, double, double, double> kExpectedProjectedCover{
        687910.0, 5554620.0, 691590.0, 5562100.0};
    CHECK(descriptor.getProjectedCover() == kExpectedProjectedCover);

    UNSCOPED_INFO("Check expected origin.");
    const std::tuple<double, double> kExpectedOrigin{687910.0, 5554620.0};
    CHECK(descriptor.getOrigin() == kExpectedOrigin);

    UNSCOPED_INFO("Check expected grid spacing.");
    const std::tuple<double, double> kExpectedGridSpacing{10., 10.};
    CHECK(descriptor.getGridSpacing() == kExpectedGridSpacing);
}

//  void setHorizontalReferenceSystem(
//      const std::string& horizontalReferenceSystem) & noexcept;
//  void setVerticalReferenceSystem(
//      const std::string& verticalReferenceSystem) & noexcept;
//  void setDims(uint32_t rows, uint32_t columns) & noexcept;
//  void setProjectedCover(double llX, double llY, double urX,
//      double urY) & noexcept;
//  void setOrigin(double llX, double llY) & noexcept;
//  void setGridSpacing(double xSpacing, double ySpacing) & noexcept;
TEST_CASE("test descriptor created from metadata with sets",
    "[descriptor][constructor][getHorizontalReferenceSystem]"
    "[getVerticalReferenceSystem][getDims][getProjectedCover]"
    "[getOrigin][getGridSpacing][setHorizontalReferenceSystem]"
    "[setVerticalReferenceSystem][setDims][getProjectedCover]"
    "[setOrigin][setGridSpacing]")
{
    Descriptor descriptor;

    {
        UNSCOPED_INFO("Check setting horizontal reference system does not throw.");
        const std::string kExpected{"abc"};
        REQUIRE_NOTHROW(descriptor.setHorizontalReferenceSystem(kExpected));

        UNSCOPED_INFO("Check expected horizontal reference system.");
        CHECK(descriptor.getHorizontalReferenceSystem() == kExpected);
    }

    {
        UNSCOPED_INFO("Check setting horizontal reference system does not throw.");
        const std::string kExpected{"def"};
        REQUIRE_NOTHROW(descriptor.setVerticalReferenceSystem(kExpected));

        UNSCOPED_INFO("Check expected horizontal reference system.");
        CHECK(descriptor.getVerticalReferenceSystem() == kExpected);
    }

    {
        UNSCOPED_INFO("Check setting dimensions does not throw.");
        const uint32_t kExpectedDim0 = 24;
        const uint32_t kExpectedDim1 = 68;
        REQUIRE_NOTHROW(descriptor.setDims(kExpectedDim0, kExpectedDim1));

        UNSCOPED_INFO("Check expected dimensions.");
        CHECK(descriptor.getDims() == std::make_tuple(kExpectedDim0,
            kExpectedDim1));
    }

    {
        UNSCOPED_INFO("Check setting projected cover does not throw.");
        const double kExpectedLLx = 0;
        const double kExpectedLLy = 1;
        const double kExpectedURx = 1001;
        const double kExpectedURy = 2020;
        REQUIRE_NOTHROW(descriptor.setProjectedCover(kExpectedLLx, kExpectedLLy,
            kExpectedURx, kExpectedURy));

        UNSCOPED_INFO("Check expected projected cover.");
        CHECK(descriptor.getProjectedCover() == std::make_tuple(kExpectedLLx,
            kExpectedLLy, kExpectedURx, kExpectedURy));
    }

    {
        UNSCOPED_INFO("Check setting origin does not throw.");
        const double kExpectedLLx = 12.3456;
        const double kExpectedLLy = 98.7654321;
        REQUIRE_NOTHROW(descriptor.setOrigin(kExpectedLLx, kExpectedLLy));

        UNSCOPED_INFO("Check expected origin.");
        CHECK(descriptor.getOrigin() == std::make_tuple(kExpectedLLx,
            kExpectedLLy));
    }

    {
        UNSCOPED_INFO("Check setting grid spacing does not throw.");
        const double kExpectedXspacing = 12.3456;
        const double kExpectedYspacing = 98.7654321;
        REQUIRE_NOTHROW(descriptor.setGridSpacing(kExpectedXspacing,
            kExpectedYspacing));

        UNSCOPED_INFO("Check expected origin.");
        CHECK(descriptor.getOrigin() == std::make_tuple(kExpectedXspacing,
            kExpectedYspacing));
    }
}

