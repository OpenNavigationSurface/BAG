
#include <cstring>

#include <bag.h>
#include <bag_c_types.h>
#include <bag_errors.h>

#include <catch2/catch_all.hpp>

#include <algorithm>
#include <string>
#include <vector>

#include "test_utils.h"

namespace
{
    const std::string kMetadataXML{R"(<?xml version="1.0" encoding="UTF-8" standalone="no" ?>
<gmi:MI_Metadata xmlns:gmi="http://www.isotc211.org/2005/gmi"
    xmlns:bag="http://www.opennavsurf.org/schema/bag"
    xmlns:gco="http://www.isotc211.org/2005/gco"
    xmlns:gmd="http://www.isotc211.org/2005/gmd"
    xmlns:gml="http://www.opengis.net/gml/3.2"
    xmlns:xlink="http://www.w3.org/1999/xlink"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.opennavsurf.org/schema/bag http://www.opennavsurf.org/schema/bag/bag.xsd">
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
                    <gml:coordinates cs="," decimal="." ts=" ">687910.000000,5554620.000000 691590.000000,5562100.000000</gml:coordinates>
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
</gmi:MI_Metadata>
)"};

    const std::string sampleBagPath()
    {
        const std::string bagFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"};
        return bagFileName;
    }

    const std::string sampleMetadataPath()
    {
        const std::string metdataFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.xml"};
        return metdataFileName;
    }

    BagHandle* openSampleBag(BAG_OPEN_MODE mode = BAG_OPEN_MODE::BAG_OPEN_READONLY)
    {
        BagHandle* handle = nullptr;
        auto bagFile = sampleBagPath();
        REQUIRE(bagFileOpen(&handle, mode, bagFile.c_str()) == BAG_SUCCESS);
        return handle;
    }

    BagHandle* openBagRW(const std::string& bagFile)
    {
        BagHandle* handle = nullptr;
        REQUIRE(bagFileOpen(&handle, BAG_OPEN_READ_WRITE, bagFile.c_str()) == BAG_SUCCESS);
        return handle;
    }

    BagHandle* createEmptyBagRW(const std::string& bagFile)
    {
        constexpr uint64_t kChunkSize = 100;
        constexpr int kCompressionLevel = 6;

        // Create dataset
        {
            BAG::Metadata metadata;
            metadata.loadFromBuffer(kMetadataXML);
            auto pDataset = BAG::Dataset::create(bagFile, std::move(metadata), kChunkSize,
                kCompressionLevel);
            REQUIRE(pDataset);
        }
        BagHandle* handle = nullptr;
        REQUIRE(bagFileOpen(&handle, BAG_OPEN_READ_WRITE, bagFile.c_str()) == BAG_SUCCESS);
        return handle;
    }
}

// =============================================================================
// FILE LIFECYCLE
// =============================================================================

TEST_CASE("FileOpen - Happy path", "[bag_c_api][lifecycle][file_open][happy]") {
    BagHandle* handle = nullptr;
    auto bfp = sampleBagPath();
    CHECK(bagFileOpen(&handle, BAG_OPEN_MODE::BAG_OPEN_READONLY, bfp.c_str()) == BAG_SUCCESS);
    REQUIRE(handle != nullptr);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("FileOpen - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][lifecycle][file_open][errors][nullHandle]") {
    REQUIRE(bagFileOpen(nullptr, BAG_OPEN_MODE::BAG_OPEN_READONLY, "test.bag") == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("FileOpen - NULL filename returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][lifecycle][file_open][errors][nullFilename]") {
    BagHandle* handle = nullptr;
    REQUIRE(bagFileOpen(&handle, BAG_OPEN_MODE::BAG_OPEN_READONLY, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    CHECK(handle == nullptr);
}

TEST_CASE("FileOpen - Non-existent file returns BAG_NO_FILE_FOUND", "[bag_c_api][lifecycle][file_open][errors][noent]") {
    BagHandle* handle = nullptr;
    REQUIRE(bagFileOpen(&handle, BAG_OPEN_MODE::BAG_OPEN_READONLY, "non_existent_bag_file_12345.bag") == BAG_NO_FILE_FOUND);
    CHECK(handle == nullptr);
}

TEST_CASE("FileClose - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][lifecycle][file_close][errors][nullHandle]") {
    REQUIRE(bagFileClose(nullptr) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("FileClose - Happy path returns BAG_SUCCESS", "[bag_c_api][lifecycle][file_close][happy]") {
    BagHandle* handle = openSampleBag();
    CHECK(handle != nullptr);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

// =============================================================================
// CREATION
// =============================================================================

TEST_CASE("CreateFromFile - Happy Path", "[bag_c_api][creation][file][happy]")
{
    const TestUtils::RandomFileGuard tmpFileName;
    BagHandle* handle = nullptr;
    BagError err = bagCreateFromFile(&handle, tmpFileName.m_fileName.c_str(),
        sampleMetadataPath().c_str());
    REQUIRE(err == BAG_SUCCESS);
    // Cover bagCreateLayer
    err = bagCreateLayer(handle, Nominal_Elevation);
    CHECK(err == BAG_SUCCESS);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("CreateFromFile - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][creation][file][errors][nullHandle]") {
    REQUIRE(bagCreateFromFile(nullptr, "test.bag", "metadata.xml") == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("CreateFromFile - NULL fileName returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][creation][file][errors][nullFilename]") {
    BagHandle* handle = nullptr;
    REQUIRE(bagCreateFromFile(&handle, nullptr, "metadata.xml") == BAG_INVALID_FUNCTION_ARGUMENT);
    CHECK(handle == nullptr);
}

TEST_CASE("CreateFromFile - NULL metadataFile returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][creation][file][errors][nullMetadata]") {
    BagHandle* handle = nullptr;
    REQUIRE(bagCreateFromFile(&handle, "test.bag", nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    CHECK(handle == nullptr);
}

TEST_CASE("CreateFromBuffer - NULL filename returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][creation][buffer][errors][nullFilename]") {
    BagHandle* handle = nullptr;
    uint8_t dummy[] = {0xDE, 0xAD, 0xBE, 0xEF};
    REQUIRE(bagCreateFromBuffer(&handle, nullptr, dummy, sizeof(dummy)) == BAG_INVALID_FUNCTION_ARGUMENT);
    CHECK(handle == nullptr);
}

TEST_CASE("CreateFromBuffer - NULL buffer returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][creation][buffer][errors][nullMetadata]") {
    BagHandle* handle = nullptr;
    REQUIRE(bagCreateFromBuffer(&handle, "test.bag", nullptr, 10) == BAG_INVALID_FUNCTION_ARGUMENT);
    CHECK(handle == nullptr);
}

// =============================================================================
// CREATE LAYER
// =============================================================================

TEST_CASE("CreateLayer - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][creation][layer][errors][nullHandle]") {
    REQUIRE(bagCreateLayer(nullptr, Elevation) == BAG_INVALID_BAG_HANDLE);
}

// =============================================================================
// GRID & SPATIAL INFO
// =============================================================================

TEST_CASE("GetGridDimensions - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][grid][errors][nullHandle]") {
    uint32_t rows = 0, cols = 0;
    REQUIRE(bagGetGridDimensions(nullptr, &rows, &cols) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("GetGridDimensions - NULL rows pointer returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][grid][errors][nullRows]") {
    BagHandle* handle = openSampleBag();
    uint32_t cols = 0;
    CHECK(bagGetGridDimensions(handle, nullptr, &cols) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetGridDimensions - NULL cols pointer returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][grid][errors][nullCols]") {
    BagHandle* handle = openSampleBag();
    uint32_t rows = 0;
    CHECK(bagGetGridDimensions(handle, &rows, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetGridDimensions - Happy path", "[bag_c_api][grid][happy]") {
    BagHandle* handle = openSampleBag();
    uint32_t rows = 0, cols = 0;
    CHECK(bagGetGridDimensions(handle, &rows, &cols) == BAG_SUCCESS);
    CHECK(rows > 0);
    CHECK(cols > 0);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetSpacing - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][grid][errors][nullHandle]") {
    double rs = 0, cs = 0;
    REQUIRE(bagGetSpacing(nullptr, &rs, &cs) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("GetSpacing - NULL rowSpacing returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][grid][errors][nullRowSpacing]") {
    BagHandle* handle = openSampleBag();
    double cs = 0;
    CHECK(bagGetSpacing(handle, nullptr, &cs) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetSpacing - NULL columnSpacing returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][grid][errors][nullColumnSpacing]") {
    BagHandle* handle = openSampleBag();
    double rs = 0;
    CHECK(bagGetSpacing(handle, &rs, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetSpacing - Happy path", "[bag_c_api][grid][happy]") {
    BagHandle* handle = openSampleBag();
    double rs = 0.0, cs = 0.0;
    CHECK(bagGetSpacing(handle, &rs, &cs) == BAG_SUCCESS);
    CHECK(rs > 0.0);
    CHECK(cs > 0.0);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetGeoCover - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][grid][errors][nullHandle]") {
    double llx, lly, urx, ury;
    REQUIRE(bagGetGeoCover(nullptr, &llx, &lly, &urx, &ury) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("GetGeoCover - NULL llx returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][grid][errors][nullLlx]") {
    BagHandle* handle = openSampleBag();
    double lly, urx, ury;
    CHECK(bagGetGeoCover(handle, nullptr, &lly, &urx, &ury) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetGeoCover - NULL lly returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][grid][errors][nullLly]") {
    BagHandle* handle = openSampleBag();
    double llx, urx, ury;
    CHECK(bagGetGeoCover(handle, &llx, nullptr, &urx, &ury) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetGeoCover - NULL urx returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][grid][errors][nullUrx]") {
    BagHandle* handle = openSampleBag();
    double llx, lly, ury;
    CHECK(bagGetGeoCover(handle, &llx, &lly, nullptr, &ury) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetGeoCover - NULL ury returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][grid][errors][nullUry]") {
    BagHandle* handle = openSampleBag();
    double llx, lly, urx;
    CHECK(bagGetGeoCover(handle, &llx, &lly, &urx, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetGeoCover - Happy path", "[bag_c_api][grid][happy]") {
    BagHandle* handle = openSampleBag();
    double llx, lly, urx, ury;
    CHECK(bagGetGeoCover(handle, &llx, &lly, &urx, &ury) == BAG_SUCCESS);
    CHECK(urx > llx);
    CHECK(ury > lly);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

// =============================================================================
// SIMPLE LAYER MIN/MAX
// =============================================================================

TEST_CASE("GetMinMaxSimple - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][minmax][read][errors][nullHandle]") {
    float mn, mx;
    REQUIRE(bagGetMinMaxSimple(nullptr, Elevation, &mn, &mx) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("GetMinMaxSimple - NULL minValue returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][minmax][read][errors][nullMin]") {
    BagHandle* handle = openSampleBag();
    float mx = 0.0f;
    CHECK(bagGetMinMaxSimple(handle, Elevation, nullptr, &mx) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetMinMaxSimple - NULL maxValue returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][minmax][read][errors][nullMax]") {
    BagHandle* handle = openSampleBag();
    float mn = 0.0f;
    CHECK(bagGetMinMaxSimple(handle, Elevation, &mn, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetMinMaxSimple - Layer not found returns BAG_LAYER_MISSING", "[bag_c_api][minmax][read][errors][layerNotFound]") {
    BagHandle* handle = openSampleBag();
    float mn, mx;
    CHECK(bagGetMinMaxSimple(handle, static_cast<BAG_LAYER_TYPE>(999), &mn, &mx) == BAG_LAYER_MISSING);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetMinMaxSimple - Happy path", "[bag_c_api][minmax][read][happy]") {
    BagHandle* handle = openSampleBag();
    float mn = 0.0f, mx = 0.0f;
    CHECK(bagGetMinMaxSimple(handle, Elevation, &mn, &mx) == BAG_SUCCESS);
    CHECK(mx >= mn);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

// =============================================================================
// SET MIN/MAX SIMPLE
// =============================================================================

TEST_CASE("SetMinMaxSimple - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][minmax][write][errors][nullHandle]") {
    REQUIRE(bagSetMinMaxSimple(nullptr, Elevation, 0.0f, 1.0f) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("SetMinMaxSimple - NULL minValue (0.0) returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][minmax][write][errors][nullMin]") {
    BagHandle* handle = openSampleBag();
    CHECK(bagSetMinMaxSimple(handle, Elevation, 0.0f, 1.0f) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("SetMinMaxSimple - NULL maxValue (0.0) returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][minmax][write][errors][nullMax]") {
    BagHandle* handle = openSampleBag();
    CHECK(bagSetMinMaxSimple(handle, Elevation, 1.0f, 0.0f) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("SetMinMaxSimple - Layer missing returns BAG_SIMPLE_LAYER_MISSING", "[bag_c_api][minmax][write][errors][layerNotFound]") {
    BagHandle* handle = openSampleBag();
    CHECK(bagSetMinMaxSimple(handle, static_cast<BAG_LAYER_TYPE>(999), 1.0f, 2.0f) == BAG_SIMPLE_LAYER_MISSING);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("SetMinMaxSimple - Happy path (read-write)", "[bag_c_api][minmax][write][happy]") {
    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(std::string{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"}, tmpFileName);
    BagHandle* handle = openBagRW(tmpFileName);
    CHECK(bagSetMinMaxSimple(handle, Elevation, 1.0f, 100.0f) == BAG_SUCCESS);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

// =============================================================================
// NUM LAYERS
// =============================================================================

TEST_CASE("GetNumLayers - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][layers][errors]") {
    uint32_t num = 0;
    REQUIRE(bagGetNumLayers(nullptr, &num) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("GetNumLayers - NULL numLayers returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][layers][errors]") {
    BagHandle* handle = openSampleBag();
    CHECK(bagGetNumLayers(handle, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetNumLayers - Happy path", "[bag_c_api][layers][happy]") {
    BagHandle* handle = openSampleBag();
    uint32_t num = 0;
    CHECK(bagGetNumLayers(handle, &num) == BAG_SUCCESS);
    CHECK(num > 0);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

// =============================================================================
// CONTAINS LAYER
// =============================================================================

TEST_CASE("ContainsLayer - NULL handle returns false + BAG_INVALID_BAG_HANDLE", "[bag_c_api][layers][errors]") {
    BagError err;
    bool result = bagContainsLayer(nullptr, Elevation, "elevation", &err);
    CHECK_FALSE(result);
    REQUIRE(err == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("ContainsLayer - Georef_Metadata with NULL name returns false + error", "[bag_c_api][layers][errors]") {
    BagHandle* handle = openSampleBag();
    BagError err;
    bool result = bagContainsLayer(handle, Georef_Metadata, nullptr, &err);
    CHECK_FALSE(result);
    CHECK(err == BAG_GEOREF_METADATA_LAYER_NAME_MISSING);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("ContainsLayer - Georef_Metadata with empty name returns false + error", "[bag_c_api][layers][errors]") {
    BagHandle* handle = openSampleBag();
    BagError err;
    bool result = bagContainsLayer(handle, Georef_Metadata, "", &err);
    CHECK_FALSE(result);
    CHECK(err == BAG_GEOREF_METADATA_LAYER_NAME_MISSING);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("ContainsLayer - Happy path (existing layer)", "[bag_c_api][layers][happy]") {
    BagHandle* handle = openSampleBag();
    BagError err;
    bool result = bagContainsLayer(handle, Elevation, "elevation", &err);
    CHECK(err == BAG_SUCCESS);
    CHECK(result == true);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("ContainsLayer - Non-existent layer returns false", "[bag_c_api][layers][happy]") {
    BagHandle* handle = openSampleBag();
    BagError err;
    bool result = bagContainsLayer(handle, Elevation, "nonexistent_layer_xyz", &err);
    CHECK(err == BAG_SUCCESS);
    CHECK_FALSE(result);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

// =============================================================================
// READ / WRITE
// =============================================================================

TEST_CASE("Read - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][read_write][errors]") {
    uint8_t* data = nullptr;
    double x, y;
    REQUIRE(bagRead(nullptr, 0, 0, 1, 1, Elevation, "elevation", &data, &x, &y) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("Read - NULL data returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][read_write][errors]") {
    BagHandle* handle = openSampleBag();
    double x, y;
    CHECK(bagRead(handle, 0, 0, 1, 1, Elevation, "elevation", nullptr, &x, &y) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("Read - NULL x returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][read_write][errors]") {
    BagHandle* handle = openSampleBag();
    uint8_t* data = nullptr;
    double y;
    CHECK(bagRead(handle, 0, 0, 1, 1, Elevation, "elevation", &data, nullptr, &y) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("Read - NULL y returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][read_write][errors]") {
    BagHandle* handle = openSampleBag();
    uint8_t* data = nullptr;
    double x;
    CHECK(bagRead(handle, 0, 0, 1, 1, Elevation, "elevation", &data, &x, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("Read - Georef_Metadata without name returns BAG_GEOREF_METADATA_LAYER_NAME_MISSING", "[bag_c_api][read_write][errors]") {
    BagHandle* handle = openSampleBag();
    uint8_t* data = nullptr;
    double x, y;
    CHECK(bagRead(handle, 0, 0, 1, 1, Georef_Metadata, nullptr, &data, &x, &y) == BAG_GEOREF_METADATA_LAYER_NAME_MISSING);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("Read - Non-existent layer returns BAG_HDF_DATASET_OPEN_FAILURE", "[bag_c_api][read_write][errors]") {
    BagHandle* handle = openSampleBag();
    uint8_t* data = nullptr;
    double x, y;
    CHECK(bagRead(handle, 0, 0, 1, 1, Elevation, "nonexistent", &data, &x, &y) == BAG_HDF_DATASET_OPEN_FAILURE);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("Read - Happy path", "[bag_c_api][read_write][happy]") {
    BagHandle* handle = openSampleBag();
    uint32_t rows, cols;
    CHECK(bagGetGridDimensions(handle, &rows, &cols) == BAG_SUCCESS);

    uint8_t* data = nullptr;
    double x, y;
    BagError err = bagRead(handle, 0, 0, 1, 1, Elevation, "elevation", &data, &x, &y);
    if (err == BAG_SUCCESS)
    {
        CHECK(data != nullptr);
        CHECK(x != 0.0);
        CHECK(y != 0.0);
        bagFree(data);
    }
    else
    {
        // sample.bag may not have elevation layer with that name
        CHECK(err == BAG_HDF_DATASET_OPEN_FAILURE);
    }
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("Write - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][read_write][errors]") {
    BagHandle* h = nullptr;
    uint8_t *d = nullptr;
    const auto l = "elevation";
    auto result = bagWrite(h, 0u, 0u, 1u, 1u, Elevation, l, d);
    REQUIRE(result == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("Write - NULL data returns BAG_SUCCESS (nothing to write)", "[bag_c_api][read_write][errors]") {
    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(std::string{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"}, tmpFileName);
    BagHandle* handle = openBagRW(tmpFileName);
    CHECK(bagWrite(handle, 0, 0, 1, 1, Elevation, "elevation", nullptr) == BAG_SUCCESS);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("Write - Georef_Metadata without name returns BAG_GEOREF_METADATA_LAYER_NAME_MISSING", "[bag_c_api][read_write][errors]") {
    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(std::string{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"}, tmpFileName);
    BagHandle* handle = openBagRW(tmpFileName);
    uint8_t dummy = 0;
    CHECK(bagWrite(handle, 0, 0, 1, 1, Georef_Metadata, nullptr, &dummy) == BAG_GEOREF_METADATA_LAYER_NAME_MISSING);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("Write - Non-existent layer returns BAG_HDF_DATASET_OPEN_FAILURE", "[bag_c_api][read_write][errors]") {
    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(std::string{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"}, tmpFileName);
    BagHandle* handle = openBagRW(tmpFileName);
    uint8_t dummy = 0;
    CHECK(bagWrite(handle, 0, 0, 1, 1, Elevation, "nonexistent", &dummy) == BAG_HDF_DATASET_OPEN_FAILURE);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

// =============================================================================
// GET ERROR STRING - base errors
// =============================================================================

TEST_CASE("GetErrorString - NULL error pointer returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][errors][string]") {
    CHECK(bagGetErrorString(BAG_SUCCESS, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
}

TEST_CASE("GetErrorString - BAG_SUCCESS", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_SUCCESS, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Bag returned a successful completion");
}

TEST_CASE("GetErrorString - BAG_BAD_FILE_IO_OPERATION", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_BAD_FILE_IO_OPERATION, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Bad status with file IO operation");
}

TEST_CASE("GetErrorString - BAG_NO_FILE_FOUND", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_NO_FILE_FOUND, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "File not found");
}

TEST_CASE("GetErrorString - BAG_NO_ACCESS_PERMISSION", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_NO_ACCESS_PERMISSION, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Failed to access Bag because of permissions");
}

TEST_CASE("GetErrorString - BAG_MEMORY_ALLOCATION_FAILED", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_MEMORY_ALLOCATION_FAILED, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Memory allocation failed");
}

TEST_CASE("GetErrorString - BAG_INVALID_BAG_HANDLE", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_INVALID_BAG_HANDLE, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Invalid or NULL bagHandle");
}

TEST_CASE("GetErrorString - BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_INVALID_FUNCTION_ARGUMENT, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Invalid function argument or illegal value passed to Bag");
}

TEST_CASE("GetErrorString - BAG_LAYER_MISSING", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_LAYER_MISSING, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "The specified layer is missing");
}

TEST_CASE("GetErrorString - BAG_SIMPLE_LAYER_MISSING", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_SIMPLE_LAYER_MISSING, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "The specified simple layer is missing");
}

TEST_CASE("GetErrorString - BAG_WRONG_DESCRIPTOR_FOUND", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_WRONG_DESCRIPTOR_FOUND, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "The wrong type of descriptor found for this layer");
}

TEST_CASE("GetErrorString - BAG_INVALID_LAYER_TYPE", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_INVALID_LAYER_TYPE, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Layer type larger than UNKNOWN_LAYER_TYPE encountered");
}

TEST_CASE("GetErrorString - BAG_UNSPECIFIED_ERROR", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_UNSPECIFIED_ERROR, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "An unspecified error has been encountered");
}

TEST_CASE("GetErrorString - BAG_SURFACE_CORRECTIONS_MISSING", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_SURFACE_CORRECTIONS_MISSING, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "The surface corrections layer is missing");
}

TEST_CASE("GetErrorString - BAG_MORE_BAG_INSTANCES_PRESENT", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_MORE_BAG_INSTANCES_PRESENT, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "There are still instances of the BAG in memory");
}

// =============================================================================
// GET ERROR STRING - Metadata errors
// =============================================================================

TEST_CASE("GetErrorString - BAG_METADTA_NO_HOME", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_METADTA_NO_HOME, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "The BAG_HOME environment variable must be set to the configdata directory of the openns distribution");
}

TEST_CASE("GetErrorString - BAG_METADTA_SCHEMA_FILE_MISSING", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_METADTA_SCHEMA_FILE_MISSING, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Metadata schema file(s) (*.xsd) was missing, they should be within configdata under smXML or ISO19139/bag. Check BAG_HOME path");
}

TEST_CASE("GetErrorString - BAG_METADTA_PARSE_MEM_EXCEPTION", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_METADTA_PARSE_MEM_EXCEPTION, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Metadata exceeded memory limit during parsing");
}

TEST_CASE("GetErrorString - BAG_METADTA_PARSE_EXCEPTION", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_METADTA_PARSE_EXCEPTION, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Metadata parsing encountered an fatal exception");
}

TEST_CASE("GetErrorString - BAG_METADTA_PARSE_DOM_EXCEPTION", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_METADTA_PARSE_DOM_EXCEPTION, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Metadata parsing encountered a DOM (document object model) exception, impossible to complete operation");
}

TEST_CASE("GetErrorString - BAG_METADTA_PARSE_UNK_EXCEPTION", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_METADTA_PARSE_UNK_EXCEPTION, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Metadata parsing encountered an unknown error");
}

TEST_CASE("GetErrorString - BAG_METADTA_PARSE_FAILED", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_METADTA_PARSE_FAILED, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Metadata parsing failed, unable to parse input file");
}

TEST_CASE("GetErrorString - BAG_METADTA_PARSE_FAILED_MEM", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_METADTA_PARSE_FAILED_MEM, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Metadata parsing failed, unable to parse specified input buffer memory overflow");
}

TEST_CASE("GetErrorString - BAG_METADTA_VALIDATE_FAILED", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_METADTA_VALIDATE_FAILED, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Metadata XML validation failed");
}

TEST_CASE("GetErrorString - BAG_METADTA_INVALID_HANDLE", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_METADTA_INVALID_HANDLE, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Metadata Invalid (NULL) bagHandle supplied to an accessor method");
}

TEST_CASE("GetErrorString - BAG_METADTA_INIT_FAILED", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_METADTA_INIT_FAILED, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Metadata Initialization of the low level XML support system failed");
}

TEST_CASE("GetErrorString - BAG_METADTA_NO_PROJECTION_INFO", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_METADTA_NO_PROJECTION_INFO, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Metadata No projection information was found in the XML supplied");
}

TEST_CASE("GetErrorString - BAG_METADTA_INSUFFICIENT_BUFFER", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_METADTA_INSUFFICIENT_BUFFER, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Metadata supplied buffer is not large enough to hold the extracted contents from XML");
}

TEST_CASE("GetErrorString - BAG_METADTA_INCOMPLETE_COVER", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_METADTA_INCOMPLETE_COVER, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Metadata One or more elements of the requested coverage are missing from the XML file");
}

TEST_CASE("GetErrorString - BAG_METADTA_INVLID_DIMENSIONS", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_METADTA_INVLID_DIMENSIONS, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Metadata The number of dimensions is incorrect (not equal to 2)");
}

TEST_CASE("GetErrorString - BAG_METADTA_UNCRT_MISSING", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_METADTA_UNCRT_MISSING, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Metadata 'uncertaintyType' information is missing from the XML structure");
}

TEST_CASE("GetErrorString - BAG_METADTA_BUFFER_EXCEEDED", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_METADTA_BUFFER_EXCEEDED, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Metadata supplied buffer is too large to be stored in the internal array");
}

TEST_CASE("GetErrorString - BAG_METADTA_DPTHCORR_MISSING", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_METADTA_DPTHCORR_MISSING, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "The 'depthCorrectionType' information is missing from the XML structure");
}

TEST_CASE("GetErrorString - BAG_METADTA_RESOLUTION_MISSING", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_METADTA_RESOLUTION_MISSING, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Metadata resolution information is missing from the XML structure");
}

TEST_CASE("GetErrorString - BAG_METADTA_INVALID_PROJECTION", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_METADTA_INVALID_PROJECTION, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Metadata unsupported projection being used");
}

TEST_CASE("GetErrorString - BAG_METADTA_INVALID_DATUM", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_METADTA_INVALID_DATUM, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Metadata unsupported datum being used");
}

TEST_CASE("GetErrorString - BAG_METADTA_INVALID_HREF", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_METADTA_INVALID_HREF, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Metadata horizontal reference system is invalid");
}

TEST_CASE("GetErrorString - BAG_METADTA_INVALID_VREF", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_METADTA_INVALID_VREF, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Metadata vertical reference system is invalid");
}

TEST_CASE("GetErrorString - BAG_METADTA_SCHEMA_SETUP_FAILED", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_METADTA_SCHEMA_SETUP_FAILED, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Failed to setup the xml schema");
}

TEST_CASE("GetErrorString - BAG_METADTA_SCHEMA_VALIDATION_SETUP_FAILED", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_METADTA_SCHEMA_VALIDATION_SETUP_FAILED, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Failed to setup the xml schema validation");
}

TEST_CASE("GetErrorString - BAG_METADTA_EMPTY_DOCUMENT", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_METADTA_EMPTY_DOCUMENT, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "The metadata document is emtpy");
}

TEST_CASE("GetErrorString - BAG_METADTA_MISSING_MANDATORY_ITEM", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_METADTA_MISSING_MANDATORY_ITEM, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "The metadata is missing a mandatory item");
}

TEST_CASE("GetErrorString - BAG_METADTA_NOT_INITIALIZED", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_METADTA_NOT_INITIALIZED, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "The metadata has not been initialized correctly");
}

// =============================================================================
// GET ERROR STRING - HDF errors
// =============================================================================

TEST_CASE("GetErrorString - BAG_NOT_HDF5_FILE", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_NOT_HDF5_FILE, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "HDF Bag is not an HDF5 File");
}

TEST_CASE("GetErrorString - BAG_HDF_RANK_INCOMPATIBLE", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_HDF_RANK_INCOMPATIBLE, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "HDF Bag's rank is incompatible with expected Rank of the Datasets: 2");
}

TEST_CASE("GetErrorString - BAG_HDF_TYPE_NOT_FOUND", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_HDF_TYPE_NOT_FOUND, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "HDF Bag surface Datatype parameter not available");
}

TEST_CASE("GetErrorString - BAG_HDF_DATASPACE_CORRUPTED", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_HDF_DATASPACE_CORRUPTED, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "HDF Dataspace for a bag surface is corrupted or could not be read");
}

TEST_CASE("GetErrorString - BAG_HDF_ACCESS_EXTENTS_ERROR", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_HDF_ACCESS_EXTENTS_ERROR, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "HDF Failure in request for access outside the extents of a bag surface's Dataset");
}

TEST_CASE("GetErrorString - BAG_HDF_CANNOT_WRITE_NULL_DATA", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_HDF_CANNOT_WRITE_NULL_DATA, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "HDF Cannot write NULL or uninitialized data to Dataset");
}

TEST_CASE("GetErrorString - BAG_HDF_INTERNAL_ERROR", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_HDF_INTERNAL_ERROR, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "HDF There was an internal HDF error detected");
}

TEST_CASE("GetErrorString - BAG_HDF_CREATE_FILE_FAILURE", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_HDF_CREATE_FILE_FAILURE, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "HDF Unable to create new HDF Bag File");
}

TEST_CASE("GetErrorString - BAG_HDF_CREATE_DATASPACE_FAILURE", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_HDF_CREATE_DATASPACE_FAILURE, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "HDF Unable to create the Dataspace");
}

TEST_CASE("GetErrorString - BAG_HDF_CREATE_PROPERTY_CLASS_FAILURE", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_HDF_CREATE_PROPERTY_CLASS_FAILURE, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "HDF Unable to create the Property class");
}

TEST_CASE("GetErrorString - BAG_HDF_SET_PROPERTY_FAILURE", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_HDF_SET_PROPERTY_FAILURE, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "HDF Unable to set value of Property class");
}

TEST_CASE("GetErrorString - BAG_HDF_TYPE_COPY_FAILURE", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_HDF_TYPE_COPY_FAILURE, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "HDF Failed to copy Datatype parameter for Dataset access");
}

TEST_CASE("GetErrorString - BAG_HDF_CREATE_DATASET_FAILURE", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_HDF_CREATE_DATASET_FAILURE, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "HDF Unable to create the Dataset");
}

TEST_CASE("GetErrorString - BAG_HDF_DATASET_EXTEND_FAILURE", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_HDF_DATASET_EXTEND_FAILURE, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "HDF Cannot extend Dataset extents");
}

TEST_CASE("GetErrorString - BAG_HDF_CREATE_ATTRIBUTE_FAILURE", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_HDF_CREATE_ATTRIBUTE_FAILURE, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "HDF Unable to create Attribute");
}

TEST_CASE("GetErrorString - BAG_HDF_CREATE_GROUP_FAILURE", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_HDF_CREATE_GROUP_FAILURE, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "HDF Unable to create Group");
}

TEST_CASE("GetErrorString - BAG_HDF_WRITE_FAILURE", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_HDF_WRITE_FAILURE, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "HDF Failure writing to Dataset");
}

TEST_CASE("GetErrorString - BAG_HDF_READ_FAILURE", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_HDF_READ_FAILURE, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "HDF Failure reading from Dataset");
}

TEST_CASE("GetErrorString - BAG_HDF_GROUP_CLOSE_FAILURE", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_HDF_GROUP_CLOSE_FAILURE, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "HDF Failure closing Group");
}

TEST_CASE("GetErrorString - BAG_HDF_FILE_CLOSE_FAILURE", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_HDF_FILE_CLOSE_FAILURE, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "HDF Failure closing File");
}

TEST_CASE("GetErrorString - BAG_HDF_FILE_OPEN_FAILURE", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_HDF_FILE_OPEN_FAILURE, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "HDF Unable to open File");
}

TEST_CASE("GetErrorString - BAG_HDF_GROUP_OPEN_FAILURE", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_HDF_GROUP_OPEN_FAILURE, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "HDF Unable to open Group");
}

TEST_CASE("GetErrorString - BAG_HDF_ATTRIBUTE_OPEN_FAILURE", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_HDF_ATTRIBUTE_OPEN_FAILURE, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "HDF Unable to open Attribute");
}

TEST_CASE("GetErrorString - BAG_HDF_ATTRIBUTE_CLOSE_FAILURE", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_HDF_ATTRIBUTE_CLOSE_FAILURE, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "HDF Failure closing Attribute");
}

TEST_CASE("GetErrorString - BAG_HDF_DATASET_CLOSE_FAILURE", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_HDF_DATASET_CLOSE_FAILURE, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "HDF Failure closing Dataset");
}

TEST_CASE("GetErrorString - BAG_HDF_DATASET_OPEN_FAILURE", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_HDF_DATASET_OPEN_FAILURE, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "HDF Unable to open Dataset");
}

TEST_CASE("GetErrorString - BAG_HDF_TYPE_CREATE_FAILURE", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_HDF_TYPE_CREATE_FAILURE, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "HDF Unable to create Datatype");
}

TEST_CASE("GetErrorString - BAG_HDF_INVALID_COMPRESSION_LEVEL", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_HDF_INVALID_COMPRESSION_LEVEL, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "HDF compression level not in acceptable range of 0 to 9");
}

TEST_CASE("GetErrorString - BAG_HDF_WRITE_ATTRIBUTE_FAILURE", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_HDF_WRITE_ATTRIBUTE_FAILURE, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "HDF Unable to write to Attribute");
}

// =============================================================================
// GET ERROR STRING - Georef metadata errors
// =============================================================================

TEST_CASE("GetErrorString - BAG_GEOREF_METADATA_LAYER_MISSING", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_GEOREF_METADATA_LAYER_MISSING, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Georef: The specified layer does not exist");
}

TEST_CASE("GetErrorString - BAG_GEOREF_METADATA_LAYER_RECORD_NOT_FOUND", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_GEOREF_METADATA_LAYER_RECORD_NOT_FOUND, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Georef: Unable to find record");
}

TEST_CASE("GetErrorString - BAG_GEOREF_METADATA_LAYER_FIELD_NOT_FOUND", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_GEOREF_METADATA_LAYER_FIELD_NOT_FOUND, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Georef: Unable to find field");
}

TEST_CASE("GetErrorString - BAG_GEOREF_METADATA_LAYER_NO_VALUE_FOUND", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_GEOREF_METADATA_LAYER_NO_VALUE_FOUND, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Georef: Unable to find the value");
}

TEST_CASE("GetErrorString - BAG_GEOREF_METADATA_LAYER_INVALID_RECORD_DEFINITION", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_GEOREF_METADATA_LAYER_INVALID_RECORD_DEFINITION, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Georef: The provided record does not match the definition in the georeferenced metadata layer");
}

TEST_CASE("GetErrorString - BAG_GEOREF_METADATA_LAYER_NAME_MISSING", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_GEOREF_METADATA_LAYER_NAME_MISSING, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Georef: A georeferenced metadata layer name is required");
}

TEST_CASE("GetErrorString - BAG_GEOREF_METADATA_LAYER_EXISTS", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_GEOREF_METADATA_LAYER_EXISTS, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Georef: The specified layer already exists");
}

TEST_CASE("GetErrorString - BAG_GEOREF_METADATA_LAYER_PROFILE_UNKNOWN", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_GEOREF_METADATA_LAYER_PROFILE_UNKNOWN, &err) == BAG_SUCCESS);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "Georef: The specified metadata profile is unknown");
}

TEST_CASE("GetErrorString - BAG_INVALID_ERROR_CODE", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(BAG_INVALID_ERROR_CODE, &err) == BAG_INVALID_ERROR_CODE);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "An undefined bagError code was encountered");
}

// =============================================================================
// GET ERROR STRING - Invalid error code
// =============================================================================

TEST_CASE("GetErrorString - Invalid code (default) returns BAG_INVALID_ERROR_CODE", "[bag_c_api][errors][string]") {
    uint8_t* err = nullptr;
    CHECK(bagGetErrorString(static_cast<BagError>(99999), &err) == BAG_INVALID_ERROR_CODE);
    CHECK(std::string(reinterpret_cast<char*>(err)) == "An undefined bagError code was encountered");
}

// =============================================================================
// COORDINATES
// =============================================================================

TEST_CASE("ComputePosition - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][coordinates][errors]") {
    double x, y;
    REQUIRE(bagComputePostion(nullptr, 0, 0, &x, &y) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("ComputePosition - NULL x returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][coordinates][errors]") {
    BagHandle* handle = openSampleBag();
    double y;
    CHECK(bagComputePostion(handle, 0, 0, nullptr, &y) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("ComputePosition - NULL y returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][coordinates][errors]") {
    BagHandle* handle = openSampleBag();
    double x;
    CHECK(bagComputePostion(handle, 0, 0, &x, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("ComputePosition - Happy path", "[bag_c_api][coordinates][happy]") {
    BagHandle* handle = openSampleBag();
    double x, y;
    CHECK(bagComputePostion(handle, 0, 0, &x, &y) == BAG_SUCCESS);
    CHECK(x != 0.0);
    CHECK(y != 0.0);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("ComputeIndex - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][coordinates][errors]") {
    uint32_t row, col;
    REQUIRE(bagComputeIndex(nullptr, 1.0, 1.0, &row, &col) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("ComputeIndex - NULL row returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][coordinates][errors]") {
    BagHandle* handle = openSampleBag();
    uint32_t col;
    CHECK(bagComputeIndex(handle, 1.0, 1.0, nullptr, &col) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("ComputeIndex - NULL col returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][coordinates][errors]") {
    BagHandle* handle = openSampleBag();
    uint32_t row;
    CHECK(bagComputeIndex(handle, 1.0, 1.0, &row, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("ComputeIndex - Happy path", "[bag_c_api][coordinates][happy]") {
    BagHandle* handle = openSampleBag();
    uint32_t row = 999, col = 999;
    CHECK(bagComputeIndex(handle, 0.0, 0.0, &row, &col) == BAG_SUCCESS);
    CHECK(row == 0);
    CHECK(col == 0);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

// =============================================================================
// ALLOCATE BUFFER
// =============================================================================

TEST_CASE("AllocateBuffer - NULL handle returns nullptr", "[bag_c_api][memory][errors]") {
    BagError err;
    uint8_t* buf = bagAllocateBuffer(nullptr, 10, 10, Elevation, "elevation", &err);
    CHECK(buf == nullptr);
}

TEST_CASE("AllocateBuffer - NULL bagError returns nullptr", "[bag_c_api][memory][errors]") {
    BagHandle* handle = openSampleBag();
    uint8_t* buf = bagAllocateBuffer(handle, 10, 10, Elevation, "elevation", nullptr);
    CHECK(buf == nullptr);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("AllocateBuffer - Georef_Metadata with NULL name returns nullptr + error", "[bag_c_api][memory][errors]") {
    BagHandle* handle = openSampleBag();
    BagError err;
    uint8_t* buf = bagAllocateBuffer(handle, 10, 10, Georef_Metadata, nullptr, &err);
    CHECK(buf == nullptr);
    CHECK(err == BAG_GEOREF_METADATA_LAYER_NAME_MISSING);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("AllocateBuffer - Georef_Metadata with empty name returns nullptr + error", "[bag_c_api][memory][errors]") {
    BagHandle* handle = openSampleBag();
    BagError err;
    uint8_t* buf = bagAllocateBuffer(handle, 10, 10, Georef_Metadata, "", &err);
    CHECK(buf == nullptr);
    CHECK(err == BAG_GEOREF_METADATA_LAYER_NAME_MISSING);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("AllocateBuffer - Non-existent layer returns nullptr + BAG_LAYER_MISSING", "[bag_c_api][memory][errors]") {
    BagHandle* handle = openSampleBag();
    BagError err;
    uint8_t* buf = bagAllocateBuffer(handle, 10, 10, Elevation, "nonexistent_layer", &err);
    CHECK(buf == nullptr);
    CHECK(err == BAG_LAYER_MISSING);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("AllocateBuffer - Happy path (existing layer)", "[bag_c_api][memory][happy]") {
    BagHandle* handle = openSampleBag();
    BagError err;
    uint8_t* buf = bagAllocateBuffer(handle, 5, 5, Elevation, "elevation", &err);
    if (err == BAG_SUCCESS)
    {
        CHECK(buf != nullptr);
        bagFree(buf);
    }
    else
    {
        // Layer may not exist in sample
        CHECK(err == BAG_LAYER_MISSING);
    }
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

// =============================================================================
// ALLOCATE / FREE
// =============================================================================

TEST_CASE("Allocate - returns valid buffer of correct size", "[bag_c_api][memory][happy]") {
    const uint32_t size = 256;
    uint8_t* buf = bagAllocate(size);
    REQUIRE(buf != nullptr);
    std::memset(buf, 0x42, size);
    for (uint32_t i = 0; i < size; ++i)
        CHECK(buf[i] == 0x42);
    bagFree(buf);
}

TEST_CASE("Free - NULL pointer is safe (no-op / no crash)", "[bag_c_api][memory][happy]") {
    bagFree(nullptr);  // Should not crash
}

// =============================================================================
// SURFACE CORRECTIONS - READ/WRITE VERTICAL DATUM
// =============================================================================

TEST_CASE("ReadCorrectorVerticalDatum - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][surface_corrections][errors]") {
    uint8_t datum[256] = {0};
    REQUIRE(bagReadCorrectorVerticalDatum(nullptr, 1, datum) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("ReadCorrectorVerticalDatum - corrector=0 returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][surface_corrections][errors]") {
    BagHandle* handle = openSampleBag();
    uint8_t datum[256] = {0};
    CHECK(bagReadCorrectorVerticalDatum(handle, 0, datum) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("ReadCorrectorVerticalDatum - corrector=11 returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][surface_corrections][errors]") {
    BagHandle* handle = openSampleBag();
    uint8_t datum[256] = {0};
    CHECK(bagReadCorrectorVerticalDatum(handle, 11, datum) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("ReadCorrectorVerticalDatum - NULL datum returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][surface_corrections][errors]") {
    BagHandle* handle = openSampleBag();
    CHECK(bagReadCorrectorVerticalDatum(handle, 1, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("WriteCorrectorVerticalDatum - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][surface_corrections][vdatum][errors][nullHandle]") {
    uint8_t datum[] = "MLLW";
    REQUIRE(bagWriteCorrectorVerticalDatum(nullptr, 1, datum) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("WriteCorrectorVerticalDatum - corrector=0 returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][surface_corrections][vdatum][errors][correctorZero]") {
    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(std::string{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"}, tmpFileName);
    BagHandle* handle = openBagRW(tmpFileName);
    uint8_t datum[] = "MLLW";
    CHECK(bagWriteCorrectorVerticalDatum(handle, 0, datum) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("WriteCorrectorVerticalDatum - corrector=11 returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][surface_corrections][errors]") {
    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(std::string{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"}, tmpFileName);
    BagHandle* handle = openBagRW(tmpFileName);
    uint8_t datum[] = "MLLW";
    CHECK(bagWriteCorrectorVerticalDatum(handle, 11, datum) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("WriteCorrectorVerticalDatum - NULL inDatum returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][surface_corrections][errors]") {
    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(std::string{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"}, tmpFileName);
    BagHandle* handle = openBagRW(tmpFileName);
    CHECK(bagWriteCorrectorVerticalDatum(handle, 1, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("WriteCorrectorVerticalDatum - No surface corrections returns BAG_SURFACE_CORRECTIONS_MISSING", "[bag_c_api][surface_corrections][errors][noCorrections]") {
    const TestUtils::RandomFileGuard tmpFileName;
    BagHandle* handle = createEmptyBagRW(tmpFileName);
    uint8_t datum[] = "MLLW";
    BagError err = bagWriteCorrectorVerticalDatum(handle, 1, datum);
    CHECK(err == BAG_SURFACE_CORRECTIONS_MISSING);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

// =============================================================================
// SURFACE CORRECTIONS - CREATE/READ CORRECTED LAYER/REGION/ROW/NODE
// =============================================================================

TEST_CASE("CreateReadCorrectedLayer - Happy Path", "[bag_c_api][surface_corrections][readWrite][happy]")
{
    // Forward declare stack variables for later use (we have to do this above calls to goto cleanup).
    uint8_t *surfRead = nullptr;
    double *xRead = nullptr;
    double *yRead = nullptr;
    const BAG_SURFACE_CORRECTION_TOPOGRAPHY kExpectedSurfaceType = BAG_SURFACE_GRID_EXTENTS;
    const uint8_t kExpectedNumCorrectors = 3;
    BAG_SURFACE_CORRECTION_TOPOGRAPHY topo{};
    uint8_t numCorr = 0;
    const float kExpectedItem0[] = {9.87f, 6.543f, 2.109876f};
    float *corrected = nullptr;
    std::string datum1 = "MLLW";
    std::string datum2 = "Ellipsoid";
    std::string datum3 = "NAVD88";
    uint8_t datumRead[256];

    bool success = true;
    const TestUtils::RandomFileGuard tmpFile;
    auto tmpFileName = tmpFile.m_fileName;
    BagHandle* handle = nullptr;
    auto mdBuff = const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(kMetadataXML.data()));
    // Create new BAG
    BagError err = bagCreateFromBuffer(&handle, tmpFileName.c_str(), mdBuff, kMetadataXML.size());
    REQUIRE(err == BAG_SUCCESS);
    // Write some elevation data so that we can read corrected data below
    const uint32_t kGridSize = 100;
    uint8_t *surf = (uint8_t *)malloc(sizeof(float) * kGridSize * kGridSize);
    // Write the data.
    constexpr uint32_t columnStart = 0;
    constexpr uint32_t columnEnd = kGridSize - 1;
    for(uint32_t row=0; row<kGridSize; ++row) {
        for (uint32_t column=0; column<kGridSize; ++column) {
            size_t idx = column + kGridSize * row;
            auto val_base = column + 1;
            surf[idx] = ((val_base * row) % kGridSize) +
                (val_base / static_cast<float>(kGridSize));
        }
    }
    err = bagWrite(handle, 0, columnStart, 0, columnEnd,
            Elevation, "elevation", surf);
    if (err != BAG_SUCCESS) {
        success = false;
        goto cleanup;
    }

    // Read back the data
    surfRead = (uint8_t *)malloc(sizeof(float) * kGridSize * kGridSize);
    xRead = (double *)malloc(sizeof(double) * kGridSize);
    yRead = (double *)malloc(sizeof(double) * kGridSize);
    err = bagRead(handle, 0, 0, columnEnd, columnEnd,
        Elevation, "elevation", &surfRead, xRead, yRead);
    if (err != BAG_SUCCESS) {
        success = false;
        goto cleanup;
    }
    for(uint32_t row=0; row<kGridSize; ++row) {
        for (uint32_t column=0; column<kGridSize; ++column) {
            CHECK(surfRead[column] == surf[column]);
        }
    }

    // Create surface corrections
    err = bagCreateCorrectorLayer(handle, kExpectedNumCorrectors, kExpectedSurfaceType);
    if (err != BAG_SUCCESS) {
        success = false;
        goto cleanup;
    }
    err = bagGetSurfaceCorrectionTopography(handle, &topo);
    CHECK(err == BAG_SUCCESS);
    CHECK(topo == kExpectedSurfaceType);
    err = bagGetNumSurfaceCorrectors(handle, &numCorr);
    CHECK(err == BAG_SUCCESS);
    CHECK(numCorr == kExpectedNumCorrectors);

    err = bagWrite(handle, 0, 0, 0, 0,
    Surface_Correction, "Surface_Correction", (uint8_t *)(&kExpectedItem0));
    CHECK(err == BAG_SUCCESS);

    // Write corrector vertical datums (maybe we have to write one datum for each corrector?)
    err = bagWriteCorrectorVerticalDatum(handle, 1, reinterpret_cast<const uint8_t *>(datum1.data()));
    CHECK(err == BAG_SUCCESS);
    err = bagWriteCorrectorVerticalDatum(handle, 2, reinterpret_cast<const uint8_t *>(datum2.data()));
    CHECK(err == BAG_SUCCESS);
    err = bagWriteCorrectorVerticalDatum(handle, 3, reinterpret_cast<const uint8_t *>(datum3.data()));
    CHECK(err == BAG_SUCCESS);
    // Read vertical datums back
    err = bagReadCorrectorVerticalDatum(handle, 1, datumRead);
    CHECK(err == BAG_SUCCESS);
    CHECK(datum1 == std::string{reinterpret_cast<char *>(datumRead)});
    memset(datumRead, '\0', 256);
    err = bagReadCorrectorVerticalDatum(handle, 2, datumRead);
    CHECK(err == BAG_SUCCESS);
    CHECK(datum2 == std::string{reinterpret_cast<char *>(datumRead)});
    memset(datumRead, '\0', 256);
    err = bagReadCorrectorVerticalDatum(handle, 2, datumRead);
    CHECK(err == BAG_SUCCESS);
    CHECK(datum2 == std::string{reinterpret_cast<char *>(datumRead)});
    memset(datumRead, '\0', 256);

    // Read correctors back
    // Cover bagReadCorrectedRow()
    corrected = (float *)malloc(sizeof(float) * kGridSize);
    err = bagReadCorrectedRow(handle, 0, 1, Elevation, &corrected);
    CHECK(err == BAG_SUCCESS);
    CHECK_THAT(corrected[0], Catch::Matchers::WithinAbs(9.87f, 0.000001));
    free(corrected);
    corrected = nullptr;

    // Cover bagReadCorrectedNode
    corrected = (float *)malloc(sizeof(float));
    err = bagReadCorrectedNode(handle, 0, 0, 1, Elevation, &corrected);
    CHECK(err == BAG_SUCCESS);
    CHECK_THAT(*corrected, Catch::Matchers::WithinAbs(9.87f, 0.000001));
    free(corrected);
    corrected = nullptr;

    // Read corrected surface back
    corrected = (float *)malloc(sizeof(float));
    err = bagReadCorrectedRegion(handle, 0, 0, 0, 0, 1, Elevation, &corrected);
    CHECK(err == BAG_SUCCESS);
    // TODO: Make sure this value makes sense
    CHECK_THAT(*corrected, Catch::Matchers::WithinAbs(1.86372696e-43, 0.000001));
    free(corrected);
    corrected = nullptr;
    // Read corrected surface back, this time using bagReadCorrectedLayer
    corrected = (float *)malloc(sizeof(float));
    err = bagReadCorrectedLayer(handle, 1, Elevation, &corrected);
    CHECK(err == BAG_SUCCESS);
    // TODO: Make sure this value makes sense
    CHECK_THAT(*corrected, Catch::Matchers::WithinAbs(1.86372696e-43, 0.000001));
    free(corrected);
    corrected = nullptr;

    // Close bag, re-open read-only, and re-read corrector vertical datum
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
    handle = nullptr;
    err = bagFileOpen(&handle, BAG_OPEN_READONLY, tmpFileName.c_str());
    REQUIRE(err == BAG_SUCCESS);
    err = bagReadCorrectorVerticalDatum(handle, 1, datumRead);
    CHECK(err == BAG_SUCCESS);
    CHECK(datum1 == std::string{reinterpret_cast<char *>(datumRead)});
    memset(datumRead, '\0', 256);
    err = bagReadCorrectorVerticalDatum(handle, 2, datumRead);
    CHECK(err == BAG_SUCCESS);
    CHECK(datum2 == std::string{reinterpret_cast<char *>(datumRead)});
    memset(datumRead, '\0', 256);
    err = bagReadCorrectorVerticalDatum(handle, 2, datumRead);
    CHECK(err == BAG_SUCCESS);
    CHECK(datum2 == std::string{reinterpret_cast<char *>(datumRead)});
    memset(datumRead, '\0', 256);

cleanup:
    free(surf);
    if (surfRead) free(surfRead);
    if (xRead) free(xRead);
    if (yRead) free(yRead);
    if (corrected) free(corrected);
    if (handle) REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
    REQUIRE(success);
}

TEST_CASE("ReadCorrectedLayer - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][surface_corrections][errors]") {
    float* data = nullptr;
    REQUIRE(bagReadCorrectedLayer(nullptr, 1, Elevation, &data) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("ReadCorrectedLayer - NULL data returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][surface_corrections][errors]") {
    BagHandle* handle = openSampleBag();
    CHECK(bagReadCorrectedLayer(handle, 1, Elevation, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("ReadCorrectedLayer - sample.bag has irregular corrections, but read corrected node only works on regular grid corrections", "[bag_c_api][surface_corrections][errors][noRegGridCorr]") {
    BagHandle* handle = openSampleBag();
    float* data = nullptr;
    BagError err = bagReadCorrectedLayer(handle, 1, Elevation, &data);
    CHECK(err == BAG_SURFACE_CORRECTIONS_MISSING);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("ReadCorrectedLayer - No simple layer returns BAG_SIMPLE_LAYER_MISSING", "[bag_c_api][surface_corrections][errors][invalidLayerType]") {
    BagHandle* handle = openSampleBag();
    float* data = nullptr;
    BagError err = bagReadCorrectedLayer(handle, 1, static_cast<BAG_LAYER_TYPE>(999), &data);
    CHECK(err == BAG_SIMPLE_LAYER_MISSING);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("ReadCorrectedRegion - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][surface_corrections][errors]") {
    float* data = nullptr;
    REQUIRE(bagReadCorrectedRegion(nullptr, 0, 0, 1, 1, 1, Elevation, &data) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("ReadCorrectedRegion - NULL data returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][surface_corrections][region][errors][nullData]") {
    BagHandle* handle = openSampleBag();
    CHECK(bagReadCorrectedRegion(handle, 0, 0, 1, 1, 1, Elevation, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("ReadCorrectedRegion - No surface corrections returns BAG_SURFACE_CORRECTIONS_MISSING", "[bag_c_api][surface_corrections][region][errors]") {
    BagHandle* handle = openSampleBag();
    float* data = nullptr;
    BagError err = bagReadCorrectedRegion(handle, 0, 0, 1, 1, 1, Elevation, &data);
    CHECK(err == BAG_SURFACE_CORRECTIONS_MISSING);
    if (err == BAG_SUCCESS && data) bagFree(reinterpret_cast<uint8_t*>(data));
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("ReadCorrectedRegion - Invalid layer type", "[bag_c_api][surface_corrections][region][errors][invalidLayerType]") {
    BagHandle* handle = openSampleBag();
    float* data = nullptr;
    BagError err = bagReadCorrectedRegion(handle, 0, 0, 1, 1, 1, static_cast<BAG_LAYER_TYPE>(999), &data);
    CHECK(err == BAG_LAYER_MISSING);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("ReadCorrectedRow - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][surface_corrections][errors]") {
    float* data = nullptr;
    REQUIRE(bagReadCorrectedRow(nullptr, 0, 1, Elevation, &data) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("ReadCorrectedRow - NULL data returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][surface_corrections][errors]") {
    BagHandle* handle = openSampleBag();
    CHECK(bagReadCorrectedRow(handle, 0, 1, Elevation, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("ReadCorrectedRow - sample.bag has irregular corrections, but read corrected row only works on regular grid corrections", "[bag_c_api][surface_corrections][row][errors][noRegGridCorr]") {
    BagHandle* handle = openSampleBag();
    float* data = nullptr;
    BagError err = bagReadCorrectedRow(handle, 0, 1, Elevation, &data);
    CHECK(err == BAG_SURFACE_CORRECTIONS_MISSING);
    if (err == BAG_SUCCESS && data) bagFree(reinterpret_cast<uint8_t*>(data));
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("ReadCorrectedRow - Invalid layer type", "[bag_c_api][surface_corrections][row][errors][invalidLayerType]") {
    BagHandle* handle = openSampleBag();
    float* data = nullptr;
    BagError err = bagReadCorrectedRow(handle, 0, 1, static_cast<BAG_LAYER_TYPE>(999), &data);
    CHECK(err == BAG_INVALID_LAYER_TYPE);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("ReadCorrectedNode - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][surface_corrections][errors]") {
    float* data = nullptr;
    REQUIRE(bagReadCorrectedNode(nullptr, 0, 0, 1, Elevation, &data) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("ReadCorrectedNode - NULL data returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][surface_corrections][errors][nullData]") {
    BagHandle* handle = openSampleBag();
    CHECK(bagReadCorrectedNode(handle, 0, 0, 1, Elevation, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("ReadCorrectedNode - sample.bag has irregular corrections, but read corrected node only works on regular grid corrections", "[bag_c_api][surface_corrections][node][errors][noRegGridCorr]") {
    BagHandle* handle = openSampleBag();
    float* data = nullptr;
    BagError err = bagReadCorrectedNode(handle, 0, 0, 1, Elevation, &data);
    CHECK(err == BAG_SURFACE_CORRECTIONS_MISSING);
    if (err == BAG_SUCCESS && data) bagFree(reinterpret_cast<uint8_t*>(data));
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("ReadCorrectedNode - Invalid layer type", "[bag_c_api][surface_corrections][node][errors][invalidLayerType]") {
    BagHandle* handle = openSampleBag();
    float* data = nullptr;
    BagError err = bagReadCorrectedNode(handle, 0, 0, 1, static_cast<BAG_LAYER_TYPE>(999), &data);
    CHECK(err == BAG_INVALID_LAYER_TYPE);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

// =============================================================================
// SURFACE CORRECTIONS - NUM CORRECTORS / TOPOGRAPHY / CREATE / DEFINITION
// =============================================================================

TEST_CASE("GetNumSurfaceCorrectors - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][surface_corrections][errors]") {
    uint8_t num = 0;
    REQUIRE(bagGetNumSurfaceCorrectors(nullptr, &num) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("GetNumSurfaceCorrectors - NULL numCorrectors returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][surface_corrections][errors]") {
    BagHandle* handle = openSampleBag();
    CHECK(bagGetNumSurfaceCorrectors(handle, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetNumSurfaceCorrectors - Happy path", "[bag_c_api][surface_corrections][happy]") {
    BagHandle* handle = openSampleBag();
    uint8_t num = 0;
    BagError err = bagGetNumSurfaceCorrectors(handle, &num);
    // sample BAG should have 2
    CHECK(num == 2);
    CHECK(err == BAG_SUCCESS);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetSurfaceCorrectionTopography - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][surface_corrections][topo][errors][nullHandle]") {
    BAG_SURFACE_CORRECTION_TOPOGRAPHY topo = BAG_SURFACE_GRID_EXTENTS;
    REQUIRE(bagGetSurfaceCorrectionTopography(nullptr, &topo) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("GetSurfaceCorrectionTopography - NULL type returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][surface_corrections][errors]") {
    BagHandle* handle = openSampleBag();
    CHECK(bagGetSurfaceCorrectionTopography(handle, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetSurfaceCorrectionTopography - No surface corrections returns BAG_SURFACE_CORRECTIONS_MISSING", "[bag_c_api][surface_corrections][topo][errors][dbg]") {
    BagHandle* handle = openSampleBag();
    BAG_SURFACE_CORRECTION_TOPOGRAPHY topo;
    BagError err = bagGetSurfaceCorrectionTopography(handle, &topo);
    CHECK(err == BAG_SUCCESS);
    CHECK(topo == BAG_SURFACE_IRREGULARLY_SPACED);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("CreateCorrectorLayer - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][surface_corrections][errors][nullHandle]") {
    REQUIRE(bagCreateCorrectorLayer(nullptr, 1, BAG_SURFACE_GRID_EXTENTS) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("WriteCorrectorDefinition - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][surface_corrections][errors]") {
    REQUIRE(bagWriteCorrectorDefinition(nullptr, nullptr) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("WriteCorrectorDefinition - NULL def returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][surface_corrections][errors]") {
    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(std::string{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"}, tmpFileName);
    BagHandle* handle = openBagRW(tmpFileName);
    CHECK(bagWriteCorrectorDefinition(handle, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("WriteCorrectorDefinition - Happy Path", "[bag_c_api][surface_corrections][write][happy][emptyCorrections]") {
    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(std::string{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"}, tmpFileName);
    BagHandle* handle = openBagRW(tmpFileName);
    BagVerticalCorrectorDef def{};
    BagError err = bagWriteCorrectorDefinition(handle, &def);
    CHECK(err == BAG_SUCCESS);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("ReadCorrectorDefinition - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][surface_corrections][errors]") {
    REQUIRE(bagReadCorrectorDefinition(nullptr, nullptr) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("ReadCorrectorDefinition - NULL def returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][surface_corrections][errors]") {
    BagHandle* handle = openSampleBag();
    CHECK(bagReadCorrectorDefinition(handle, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("ReadCorrectorDefinition - Happy", "[bag_c_api][surface_corrections][defn][happy]") {
    BagHandle* handle = openSampleBag();
    BagVerticalCorrectorDef def{};
    BagError err = bagReadCorrectorDefinition(handle, &def);
    CHECK(err == BAG_SUCCESS);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

// =============================================================================
// TRACKING LIST
// =============================================================================

TEST_CASE("TrackingListLength - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][tracking][errors]") {
    uint32_t len = 0;
    REQUIRE(bagTrackingListLength(nullptr, &len) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("TrackingListLength - NULL length returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][tracking][errors]") {
    BagHandle* handle = openSampleBag();
    CHECK(bagTrackingListLength(handle, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("TrackingListLength - Happy path", "[bag_c_api][tracking][happy]") {
    BagHandle* handle = openSampleBag();
    uint32_t len = 0;
    CHECK(bagTrackingListLength(handle, &len) == BAG_SUCCESS);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("ReadTrackingListNode - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][tracking][errors]") {
    BagTrackingItem* items = nullptr;
    uint32_t num = 0;
    REQUIRE(bagReadTrackingListNode(nullptr, 0, 0, &items, &num) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("ReadTrackingListNode - NULL items returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][tracking][errors]") {
    BagHandle* handle = openSampleBag();
    uint32_t num = 0;
    CHECK(bagReadTrackingListNode(handle, 0, 0, nullptr, &num) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("ReadTrackingListNode - NULL numItems returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][tracking][errors]") {
    BagHandle* handle = openSampleBag();
    BagTrackingItem* items = nullptr;
    CHECK(bagReadTrackingListNode(handle, 0, 0, &items, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("WriteReadTrackingListNode - Happy path", "[bag_c_api][tracking][readWrite][happy]") {
    const TestUtils::RandomFileGuard tmpFileName;
    BagHandle* handle = createEmptyBagRW(tmpFileName);
    // Write some items
    BagTrackingItem item1{1, 2, 3.4f, 5.6f, 7, 8};
    BagError err = bagWriteTrackingListItem(handle, &item1);
    CHECK(err == BAG_SUCCESS);
    BagTrackingItem item2{1, 2, 33.44f, 55.66f, 77, 88};
    err = bagWriteTrackingListItem(handle, &item2);
    CHECK(err == BAG_SUCCESS);
    // Read them back
    BagTrackingItem* items = nullptr;
    uint32_t num = 0;
    CHECK(bagReadTrackingListNode(handle, 1, 2, &items, &num) == BAG_SUCCESS);
    CHECK(items[0].depth == item1.depth);
    CHECK(items[1].depth == item2.depth);
    if (num > 0 && items) delete[] items;
    // Cover bagReadTrackingListCode
    items = nullptr;
    num = 0;
    CHECK(bagReadTrackingListCode(handle, 7, &items, &num) == BAG_SUCCESS);
    CHECK(items[0].depth == item1.depth);
    if (num > 0 && items) delete[] items;
    // Cover bagReadTrackingListSeries
    items = nullptr;
    num = 0;
    CHECK(bagReadTrackingListSeries(handle, 88, &items, &num) == BAG_SUCCESS);
    CHECK(items[0].depth == item2.depth);
    if (num > 0 && items) delete[] items;

    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("ReadTrackingListCode - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][tracking][errors]") {
    BagTrackingItem* items = nullptr;
    uint32_t num = 0;
    REQUIRE(bagReadTrackingListCode(nullptr, 1, &items, &num) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("ReadTrackingListCode - NULL items returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][tracking][errors]") {
    BagHandle* handle = openSampleBag();
    uint32_t num = 0;
    CHECK(bagReadTrackingListCode(handle, 1, nullptr, &num) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("ReadTrackingListCode - NULL numItems returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][tracking][errors]") {
    BagHandle* handle = openSampleBag();
    BagTrackingItem* items = nullptr;
    CHECK(bagReadTrackingListCode(handle, 1, &items, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("ReadTrackingListSeries - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][tracking][errors]") {
    BagTrackingItem* items = nullptr;
    uint32_t num = 0;
    REQUIRE(bagReadTrackingListSeries(nullptr, 1, &items, &num) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("ReadTrackingListSeries - NULL items returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][tracking][errors]") {
    BagHandle* handle = openSampleBag();
    uint32_t num = 0;
    CHECK(bagReadTrackingListSeries(handle, 1, nullptr, &num) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("ReadTrackingListSeries - NULL numItems returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][tracking][errors]") {
    BagHandle* handle = openSampleBag();
    BagTrackingItem* items = nullptr;
    CHECK(bagReadTrackingListSeries(handle, 1, &items, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("ReadTrackingListSeries - Happy path", "[bag_c_api][tracking][happy]") {
    BagHandle* handle = openSampleBag();
    BagTrackingItem* items = nullptr;
    uint32_t num = 999;
    CHECK(bagReadTrackingListSeries(handle, 1, &items, &num) == BAG_SUCCESS);
    if (num > 0 && items) delete[] items;
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("WriteTrackingListItem - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][tracking][errors]") {
    REQUIRE(bagWriteTrackingListItem(nullptr, nullptr) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("WriteTrackingListItem - NULL item returns BAG_SUCCESS", "[bag_c_api][tracking][errors]") {
    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(std::string{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"}, tmpFileName);
    BagHandle* handle = openBagRW(tmpFileName);
    CHECK(bagWriteTrackingListItem(handle, nullptr) == BAG_SUCCESS);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("WriteTrackingListItem - Happy path", "[bag_c_api][tracking][happy]") {
    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(std::string{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"}, tmpFileName);
    BagHandle* handle = openBagRW(tmpFileName);
    BagTrackingItem item{};
    item.row = 0;
    item.col = 0;
    item.track_code = 1;
    item.list_series = 1;
    BagError err = bagWriteTrackingListItem(handle, &item);
    CHECK(err == BAG_SUCCESS);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("SortTrackingListByNode - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][tracking][errors]") {
    REQUIRE(bagSortTrackingListByNode(nullptr) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("SortTrackingListByNode - Happy path", "[bag_c_api][tracking][sort][node][happy]") {
    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(std::string{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"}, tmpFileName);
    BagHandle* handle = openBagRW(tmpFileName);
    BagError err = bagSortTrackingListByNode(handle);
    CHECK(err == BAG_SUCCESS);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("SortTrackingListBySeries - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][tracking][errors]") {
    REQUIRE(bagSortTrackingListBySeries(nullptr) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("SortTrackingListBySeries - Happy path", "[bag_c_api][tracking][sort][series][happy]") {
    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(std::string{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"}, tmpFileName);
    BagHandle* handle = openBagRW(tmpFileName);
    BagError err = bagSortTrackingListBySeries(handle);
    CHECK(err == BAG_SUCCESS);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("SortTrackingListByCode - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][tracking][errors]") {
    REQUIRE(bagSortTrackingListByCode(nullptr) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("SortTrackingListByCode - Happy path", "[bag_c_api][tracking][sort][code][happy]") {
    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(std::string{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"}, tmpFileName);
    BagHandle* handle = openBagRW(tmpFileName);
    BagError err = bagSortTrackingListByCode(handle);
    CHECK(err == BAG_SUCCESS);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

// =============================================================================
// METADATA
// =============================================================================

TEST_CASE("GetMetaData - NULL handle returns nullptr", "[bag_c_api][metadata][errors]") {
    CHECK(bagGetMetaData(nullptr) == nullptr);
}

TEST_CASE("GetMetaData - Happy path", "[bag_c_api][metadata][happy]") {
    BagHandle* handle = openSampleBag();
    const BagMetadata* meta = bagGetMetaData(handle);
    CHECK(meta != nullptr);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("SetHomeFolder - NULL folder returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][metadata][errors]") {
    REQUIRE(bagSetHomeFolder(nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
}

TEST_CASE("SetHomeFolder - Valid folder returns BAG_SUCCESS", "[bag_c_api][metadata][happy]") {
    REQUIRE(bagSetHomeFolder("/tmp/test_metadata_folder") == BAG_SUCCESS);
}

// =============================================================================
// GEOREF METADATA LAYER
// =============================================================================

TEST_CASE("CreateGeorefMetadataLayer - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][georef_meta][errors]") {
    FieldDefinition def{};
    REQUIRE(bagCreateGeorefMetadataLayer(nullptr, DT_UINT32, GEOREF_METADATA_PROFILE::UNKNOWN_METADATA_PROFILE,
        "elevation", &def, 1) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("CreateGeorefMetadataLayer - NULL layerName returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][georef_meta][errors]") {
    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(std::string{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"}, tmpFileName);
    BagHandle* handle = openBagRW(tmpFileName);
    FieldDefinition def{};
    CHECK(bagCreateGeorefMetadataLayer(handle, DT_UINT32, GEOREF_METADATA_PROFILE::UNKNOWN_METADATA_PROFILE,
        nullptr, &def, 1) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("CreateGeorefMetadataLayer - NULL definition returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][georef_meta][errors]") {
    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(std::string{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"}, tmpFileName);
    BagHandle* handle = openBagRW(tmpFileName);
    CHECK(bagCreateGeorefMetadataLayer(handle, DT_UINT32, GEOREF_METADATA_PROFILE::UNKNOWN_METADATA_PROFILE,
        "elevation", nullptr, 1) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("CreateGeorefMetadataLayer - numFields=0 returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][georef_meta][errors]") {
    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(std::string{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"}, tmpFileName);
    BagHandle* handle = openBagRW(tmpFileName);
    FieldDefinition def{};
    CHECK(bagCreateGeorefMetadataLayer(handle, DT_UINT32, GEOREF_METADATA_PROFILE::UNKNOWN_METADATA_PROFILE,
        "elevation", &def, 0) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("CreateGeorefMetadataLayer - create for nonexistant layer type returns BAG_SIMPLE_LAYER_MISSING", "[bag_c_api][georef_meta][errors][nonExistantLayerType]") {
    const TestUtils::RandomFileGuard tmpFileName;
    const std::string bagFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"};
    TestUtils::copyFile(bagFileName, tmpFileName);
    BagHandle* handle = openBagRW(tmpFileName);
    FieldDefinition def{};
    BagError err = bagCreateGeorefMetadataLayer(handle, DT_UINT32, GEOREF_METADATA_PROFILE::UNKNOWN_METADATA_PROFILE,
        "nonexistent_layer", &def, 1);
    CHECK(err == BAG_HDF_CREATE_DATASET_FAILURE);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("CreateMetadataProfileGeorefMetadataLayer - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][georef_meta][errors]") {
    REQUIRE(bagCreateMetadataProfileGeorefMetadataLayer(nullptr, DT_UINT32,
        GEOREF_METADATA_PROFILE::UNKNOWN_METADATA_PROFILE, "elevation") == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("CreateMetadataProfileGeorefMetadataLayer - Unknown georef metadata profile", "[bag_c_api][georef_meta][errors][unknownGeoRefMDProfile]") {
    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(std::string{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"}, tmpFileName);
    BagHandle* handle = openBagRW(tmpFileName);
    BagError err = bagCreateMetadataProfileGeorefMetadataLayer(handle, DT_UINT32,
        UNKNOWN_METADATA_PROFILE, "nonexistent_layer");
    CHECK(err == BAG_GEOREF_METADATA_LAYER_PROFILE_UNKNOWN);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("CreateMetadataProfileGeorefMetadataLayer - Preposterous layer", "[bag_c_api][georef_meta][layer][errors][preposterousLayer]") {
    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(std::string{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"}, tmpFileName);
    BagHandle* handle = openBagRW(tmpFileName);
    BagError err = bagCreateMetadataProfileGeorefMetadataLayer(handle, DT_UINT32,
        NOAA_OCS_2022_10_METADATA_PROFILE, "nonexistent_layer");
    CHECK(err == BAG_LAYER_MISSING);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetGeorefMetadataLayerDefinition - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][georef_meta][errors]") {
    FieldDefinition* def = nullptr;
    uint32_t num = 0;
    REQUIRE(bagGetGeorefMetadataLayerDefinition(nullptr, "elevation", &def, &num) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("GetGeorefMetadataLayerDefinition - NULL definition returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][georef_meta][errors]") {
    BagHandle* handle = openSampleBag();
    uint32_t num = 0;
    CHECK(bagGetGeorefMetadataLayerDefinition(handle, "elevation", nullptr, &num) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetGeorefMetadataLayerDefinition - NULL numFields returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][georef_meta][errors]") {
    BagHandle* handle = openSampleBag();
    FieldDefinition* def = nullptr;
    CHECK(bagGetGeorefMetadataLayerDefinition(handle, "elevation", &def, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetGeorefMetadataLayerDefinition - Layer missing returns BAG_GEOREF_METADATA_LAYER_MISSING", "[bag_c_api][georef_meta][errors]") {
    BagHandle* handle = openSampleBag();
    FieldDefinition* def = nullptr;
    uint32_t num = 0;
    CHECK(bagGetGeorefMetadataLayerDefinition(handle, "nonexistent_georef_layer", &def, &num) == BAG_GEOREF_METADATA_LAYER_MISSING);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetGeorefMetadataLayerRecords - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][georef_meta][errors]") {
    BagCompoundDataType** records = nullptr;
    uint32_t numRec = 0, numFields = 0;
    REQUIRE(bagGetGeorefMetadataLayerRecords(nullptr, "elevation", &records, &numRec, &numFields) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("GetGeorefMetadataLayerRecords - NULL layerName returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][georef_meta][errors]") {
    BagHandle* handle = openSampleBag();
    BagCompoundDataType** records = nullptr;
    uint32_t numRec = 0, numFields = 0;
    CHECK(bagGetGeorefMetadataLayerRecords(handle, nullptr, &records, &numRec, &numFields) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetGeorefMetadataLayerRecords - NULL records returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][georef_meta][errors]") {
    BagHandle* handle = openSampleBag();
    uint32_t numRec = 0, numFields = 0;
    CHECK(bagGetGeorefMetadataLayerRecords(handle, "elevation", nullptr, &numRec, &numFields) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetGeorefMetadataLayerRecords - NULL numRecords returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][georef_meta][errors]") {
    BagHandle* handle = openSampleBag();
    BagCompoundDataType** records = nullptr;
    uint32_t numFields = 0;
    CHECK(bagGetGeorefMetadataLayerRecords(handle, "elevation", &records, nullptr, &numFields) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetGeorefMetadataLayerRecords - NULL numFields returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][georef_meta][errors]") {
    BagHandle* handle = openSampleBag();
    BagCompoundDataType** records = nullptr;
    uint32_t numRec = 0;
    CHECK(bagGetGeorefMetadataLayerRecords(handle, "elevation", &records, &numRec, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetGeorefMetadataLayerRecords - Layer missing returns BAG_GEOREF_METADATA_LAYER_MISSING", "[bag_c_api][georef_meta][errors]") {
    BagHandle* handle = openSampleBag();
    BagCompoundDataType** records = nullptr;
    uint32_t numRec = 0, numFields = 0;
    CHECK(bagGetGeorefMetadataLayerRecords(handle, "nonexistent_georef_layer", &records, &numRec, &numFields) == BAG_GEOREF_METADATA_LAYER_MISSING);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetGeorefMetadataLayerValueByName - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][georef_meta][errors]") {
    BagCompoundDataType val{};
    REQUIRE(bagGetGeorefMetadataLayerValueByName(nullptr, "elevation", 1, "field", &val) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("GetGeorefMetadataLayerValueByName - NULL value returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][georef_meta][errors]") {
    BagHandle* handle = openSampleBag();
    CHECK(bagGetGeorefMetadataLayerValueByName(handle, "elevation", 1, "field", nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetGeorefMetadataLayerValueByName - Layer missing returns BAG_GEOREF_METADATA_LAYER_MISSING", "[bag_c_api][georef_meta][errors]") {
    BagHandle* handle = openSampleBag();
    BagCompoundDataType val{};
    CHECK(bagGetGeorefMetadataLayerValueByName(handle, "nonexistent", 1, "field", &val) == BAG_GEOREF_METADATA_LAYER_MISSING);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetGeorefMetadataLayerValueByIndex - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][georef_meta][errors]") {
    BagCompoundDataType val{};
    REQUIRE(bagGetGeorefMetadataLayerValueByIndex(nullptr, "elevation", 1, 0, &val) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("GetGeorefMetadataLayerValueByIndex - NULL value returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][georef_meta][errors]") {
    BagHandle* handle = openSampleBag();
    CHECK(bagGetGeorefMetadataLayerValueByIndex(handle, "elevation", 1, 0, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetGeorefMetadataLayerValueByIndex - Layer missing returns BAG_GEOREF_METADATA_LAYER_MISSING", "[bag_c_api][georef_meta][errors]") {
    BagHandle* handle = openSampleBag();
    BagCompoundDataType val{};
    CHECK(bagGetGeorefMetadataLayerValueByIndex(handle, "nonexistent", 1, 0, &val) == BAG_GEOREF_METADATA_LAYER_MISSING);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetGeorefMetadataLayerFieldIndex - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][georef_meta][errors]") {
    uint32_t idx = 0;
    REQUIRE(bagGetGeorefMetadataLayerFieldIndex(nullptr, "elevation", "field", &idx) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("GetGeorefMetadataLayerFieldIndex - NULL layerName returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][georef_meta][errors]") {
    BagHandle* handle = openSampleBag();
    uint32_t idx = 0;
    CHECK(bagGetGeorefMetadataLayerFieldIndex(handle, nullptr, "field", &idx) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetGeorefMetadataLayerFieldIndex - NULL fieldName returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][georef_meta][errors]") {
    BagHandle* handle = openSampleBag();
    uint32_t idx = 0;
    CHECK(bagGetGeorefMetadataLayerFieldIndex(handle, "elevation", nullptr, &idx) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetGeorefMetadataLayerFieldIndex - NULL fieldIndex returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][georef_meta][errors]") {
    BagHandle* handle = openSampleBag();
    CHECK(bagGetGeorefMetadataLayerFieldIndex(handle, "elevation", "field", nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetGeorefMetadataLayerFieldIndex - Layer missing returns BAG_GEOREF_METADATA_LAYER_MISSING", "[bag_c_api][georef_meta][errors]") {
    BagHandle* handle = openSampleBag();
    uint32_t idx = 0;
    CHECK(bagGetGeorefMetadataLayerFieldIndex(handle, "nonexistent", "field", &idx) == BAG_GEOREF_METADATA_LAYER_MISSING);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetGeorefMetadataLayerFieldName - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][georef_meta][errors]") {
    const char* name = nullptr;
    REQUIRE(bagGetGeorefMetadataLayerFieldName(nullptr, "elevation", 0, &name) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("GetGeorefMetadataLayerFieldName - NULL layerName returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][georef_meta][errors]") {
    BagHandle* handle = openSampleBag();
    const char* name = nullptr;
    CHECK(bagGetGeorefMetadataLayerFieldName(handle, nullptr, 0, &name) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetGeorefMetadataLayerFieldName - NULL fieldName returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][georef_meta][errors]") {
    BagHandle* handle = openSampleBag();
    CHECK(bagGetGeorefMetadataLayerFieldName(handle, "elevation", 0, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("GetGeorefMetadataLayerFieldName - Layer missing returns BAG_GEOREF_METADATA_LAYER_MISSING", "[bag_c_api][georef_meta][errors]") {
    BagHandle* handle = openSampleBag();
    const char* name = nullptr;
    CHECK(bagGetGeorefMetadataLayerFieldName(handle, "nonexistent", 0, &name) == BAG_GEOREF_METADATA_LAYER_MISSING);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("AddGeorefMetadataLayerRecord - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][georef_meta][errors]") {
    BagCompoundDataType rec{};
    uint32_t idx = 0;
    REQUIRE(bagAddGeorefMetadataLayerRecord(nullptr, "elevation", &rec, 1, &idx) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("AddGeorefMetadataLayerRecord - NULL layerName returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][georef_meta][errors]") {
    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(std::string{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"}, tmpFileName);
    BagHandle* handle = openBagRW(tmpFileName);
    BagCompoundDataType rec{};
    uint32_t idx = 0;
    CHECK(bagAddGeorefMetadataLayerRecord(handle, nullptr, &rec, 1, &idx) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("AddGeorefMetadataLayerRecord - NULL record returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][georef_meta][errors]") {
    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(std::string{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"}, tmpFileName);
    BagHandle* handle = openBagRW(tmpFileName);
    uint32_t idx = 0;
    CHECK(bagAddGeorefMetadataLayerRecord(handle, "elevation", nullptr, 1, &idx) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("AddGeorefMetadataLayerRecord - NULL recordIndex returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][georef_meta][errors]") {
    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(std::string{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"}, tmpFileName);
    BagHandle* handle = openBagRW(tmpFileName);
    BagCompoundDataType rec{};
    CHECK(bagAddGeorefMetadataLayerRecord(handle, "elevation", &rec, 1, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("AddGeorefMetadataLayerRecord - Layer missing returns BAG_GEOREF_METADATA_LAYER_MISSING", "[bag_c_api][georef_meta][errors]") {
    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(std::string{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"}, tmpFileName);
    BagHandle* handle = openBagRW(tmpFileName);
    BagCompoundDataType rec{};
    uint32_t idx = 0;
    CHECK(bagAddGeorefMetadataLayerRecord(handle, "nonexistent", &rec, 1, &idx) == BAG_GEOREF_METADATA_LAYER_MISSING);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("AddGeorefMetadataLayerRecords - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][georef_meta][errors]") {
    REQUIRE(bagAddGeorefMetadataLayerRecords(nullptr, "elevation", nullptr, 1, 1) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("AddGeorefMetadataLayerRecords - NULL layerName returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][georef_meta][errors]") {
    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(std::string{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"}, tmpFileName);
    BagHandle* handle = openBagRW(tmpFileName);
    CHECK(bagAddGeorefMetadataLayerRecords(handle, nullptr, nullptr, 1, 1) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("AddGeorefMetadataLayerRecords - NULL records returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][georef_meta][errors]") {
    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(std::string{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"}, tmpFileName);
    BagHandle* handle = openBagRW(tmpFileName);
    CHECK(bagAddGeorefMetadataLayerRecords(handle, "elevation", nullptr, 1, 1) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("AddGeorefMetadataLayerRecords - null records", "[bag_c_api][georef_meta][rec][errors][nullRecords]") {
    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(std::string{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"}, tmpFileName);
    BagHandle* handle = openBagRW(tmpFileName);
    CHECK(bagAddGeorefMetadataLayerRecords(handle, "nonexistent", nullptr, 1, 1) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("SetValueByName - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][georef_meta][errors]") {
    BagCompoundDataType val{};
    REQUIRE(bagGeorefMetadataLayerSetValueByName(nullptr, "elevation", 1, "field", &val) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("SetValueByName - NULL layerName returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][georef_meta][errors]") {
    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(std::string{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"}, tmpFileName);
    BagHandle* handle = openBagRW(tmpFileName);
    BagCompoundDataType val{};
    CHECK(bagGeorefMetadataLayerSetValueByName(handle, nullptr, 1, "field", &val) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("SetValueByName - NULL fieldName returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][georef_meta][errors]") {
    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(std::string{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"}, tmpFileName);
    BagHandle* handle = openBagRW(tmpFileName);
    BagCompoundDataType val{};
    CHECK(bagGeorefMetadataLayerSetValueByName(handle, "elevation", 1, nullptr, &val) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("SetValueByName - NULL value returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][georef_meta][errors]") {
    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(std::string{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"}, tmpFileName);
    BagHandle* handle = openBagRW(tmpFileName);
    CHECK(bagGeorefMetadataLayerSetValueByName(handle, "elevation", 1, "field", nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("SetValueByName - Layer missing returns BAG_GEOREF_METADATA_LAYER_MISSING", "[bag_c_api][georef_meta][errors]") {
    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(std::string{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"}, tmpFileName);
    BagHandle* handle = openBagRW(tmpFileName);
    BagCompoundDataType val{};
    CHECK(bagGeorefMetadataLayerSetValueByName(handle, "nonexistent", 1, "field", &val) == BAG_GEOREF_METADATA_LAYER_MISSING);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("SetValueByIndex - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][georef_meta][errors]") {
    BagCompoundDataType val{};
    REQUIRE(bagGeorefMetadataLayerSetValueByIndex(nullptr, "elevation", 1, 0, &val) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("SetValueByIndex - NULL layerName returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][georef_meta][errors]") {
    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(std::string{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"}, tmpFileName);
    BagHandle* handle = openBagRW(tmpFileName);
    BagCompoundDataType val{};
    CHECK(bagGeorefMetadataLayerSetValueByIndex(handle, nullptr, 1, 0, &val) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("SetValueByIndex - NULL value returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][georef_meta][errors]") {
    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(std::string{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"}, tmpFileName);
    BagHandle* handle = openBagRW(tmpFileName);
    CHECK(bagGeorefMetadataLayerSetValueByIndex(handle, "elevation", 1, 0, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("SetValueByIndex - Layer missing returns BAG_GEOREF_METADATA_LAYER_MISSING", "[bag_c_api][georef_meta][errors]") {
    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(std::string{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"}, tmpFileName);
    BagHandle* handle = openBagRW(tmpFileName);
    BagCompoundDataType val{};
    CHECK(bagGeorefMetadataLayerSetValueByIndex(handle, "nonexistent", 1, 0, &val) == BAG_GEOREF_METADATA_LAYER_MISSING);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("bagGetGeorefMetadata - Happy Path", "[bag_c_api][georef_meta][happy]")
{
    // Forward declare stack variables for later use (we have to do this above calls to goto cleanup).
    uint8_t *surfRead = nullptr;
    double *xRead = nullptr;
    double *yRead = nullptr;
    const BAG_SURFACE_CORRECTION_TOPOGRAPHY kExpectedSurfaceType = BAG_SURFACE_GRID_EXTENTS;
    const uint8_t kExpectedNumCorrectors = 3;
    BAG_SURFACE_CORRECTION_TOPOGRAPHY topo{};
    uint8_t numCorr = 0;
    const float kExpectedItem0[] = {9.87f, 6.543f, 2.109876f};
    float *corrected = nullptr;
    std::string datum1 = "MLLW";
    std::string datum2 = "Ellipsoid";
    std::string datum3 = "NAVD88";
    uint8_t datumRead[256];
    FieldDefinition defn;
    FieldDefinition *defnRead = nullptr;
    uint32_t numFields = 0;

    bool success = true;
    const TestUtils::RandomFileGuard tmpFile;
    auto tmpFileName = tmpFile.m_fileName;
    BagHandle* handle = nullptr;
    auto mdBuff = const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(kMetadataXML.data()));
    // Create new BAG
    BagError err = bagCreateFromBuffer(&handle, tmpFileName.c_str(), mdBuff, kMetadataXML.size());
    REQUIRE(err == BAG_SUCCESS);
    // Write some elevation data so that we can read corrected data below
    const uint32_t kGridSize = 100;
    uint8_t *surf = (uint8_t *)malloc(sizeof(float) * kGridSize * kGridSize);
    // Write the data.
    constexpr uint32_t columnStart = 0;
    constexpr uint32_t columnEnd = kGridSize - 1;
    for(uint32_t row=0; row<kGridSize; ++row) {
        for (uint32_t column=0; column<kGridSize; ++column) {
            size_t idx = column + kGridSize * row;
            auto val_base = column + 1;
            surf[idx] = ((val_base * row) % kGridSize) +
                (val_base / static_cast<float>(kGridSize));
        }
    }
    err = bagWrite(handle, 0, columnStart, 0, columnEnd,
            Elevation, "elevation", surf);
    if (err != BAG_SUCCESS) {
        success = false;
        goto cleanup;
    }

    defn = {"dummy_int", DT_UINT32};
    err = bagCreateGeorefMetadataLayer(handle, DT_UINT16, UNKNOWN_METADATA_PROFILE, "elevation", &defn, 1);
    if (err != BAG_SUCCESS)
    {
        success = false;
        goto cleanup;
    }

    err = bagGetGeorefMetadataLayerDefinition(handle, "elevation", &defnRead, &numFields);
    if (err != BAG_SUCCESS)
    {
        success = false;
        goto cleanup;
    }
    CHECK(numFields == 1);
    CHECK(defnRead->name == defn.name);
    CHECK(defnRead->type == defn.type);

 cleanup:
    free(surf);
    if (surfRead) free(surfRead);
    if (xRead) free(xRead);
    if (yRead) free(yRead);
    if (corrected) free(corrected);
    if (handle) REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
    REQUIRE(success);
}

// =============================================================================
// VARIABLE RESOLUTION LAYERS
// =============================================================================

TEST_CASE("CreateVRLayers - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][vr][errors]") {
    REQUIRE(bagCreateVRLayers(nullptr, true) == BAG_INVALID_BAG_HANDLE);
    REQUIRE(bagCreateVRLayers(nullptr, false) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("CreateVRLayers - Happy Path", "[bag_c_api][vr][create][happy]") {
    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(std::string{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"}, tmpFileName);
    BagHandle* handle = openBagRW(tmpFileName);
    BagError err = bagCreateVRLayers(handle, true);
    CHECK(err == BAG_SUCCESS);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

// =============================================================================
// VR METADATA
// =============================================================================

TEST_CASE("VRMetadataGetMinDimensions - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][vr][metadata][errors]") {
    uint32_t minX, minY;
    REQUIRE(bagVRMetadataGetMinDimensions(nullptr, &minX, &minY) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("VRMetadataGetMinDimensions - NULL minX returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][vr][metadata][errors]") {
    BagHandle* handle = openSampleBag();
    uint32_t minY;
    CHECK(bagVRMetadataGetMinDimensions(handle, nullptr, &minY) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("VRMetadataGetMinDimensions - NULL minY returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][vr][metadata][errors]") {
    BagHandle* handle = openSampleBag();
    uint32_t minX;
    CHECK(bagVRMetadataGetMinDimensions(handle, &minX, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("VRMetadataGetMinDimensions - No VR metadata returns BAG_HDF_DATASET_OPEN_FAILURE", "[bag_c_api][vr][metadata][errors][noVRMeta]") {
    BagHandle* handle = openSampleBag();
    uint32_t minX, minY;
    BagError err = bagVRMetadataGetMinDimensions(handle, &minX, &minY);
    CHECK(err == BAG_HDF_DATASET_OPEN_FAILURE );
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("VRMetadataGetMaxDimensions - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][vr][metadata][errors]") {
    uint32_t maxX, maxY;
    REQUIRE(bagVRMetadataGetMaxDimensions(nullptr, &maxX, &maxY) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("VRMetadataGetMaxDimensions - NULL maxX returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][vr][metadata][errors]") {
    BagHandle* handle = openSampleBag();
    uint32_t maxY;
    CHECK(bagVRMetadataGetMaxDimensions(handle, nullptr, &maxY) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("VRMetadataGetMaxDimensions - NULL maxY returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][vr][metadata][errors]") {
    BagHandle* handle = openSampleBag();
    uint32_t maxX;
    CHECK(bagVRMetadataGetMaxDimensions(handle, &maxX, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("VRMetadataGetMaxDimensions - No VR metadata returns BAG_HDF_DATASET_OPEN_FAILURE", "[bag_c_api][vr][metadata][maxDim][errors][noVRMeta]") {
    BagHandle* handle = openSampleBag();
    uint32_t maxX, maxY;
    BagError err = bagVRMetadataGetMaxDimensions(handle, &maxX, &maxY);
    CHECK(err == BAG_HDF_DATASET_OPEN_FAILURE);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("VRMetadataGetMinResolution - NULL handle returns BAG_INVALID_BAG_HANDLE", "[bag_c_api][vr][metadata][errors]") {
    float minX, minY;
    REQUIRE(bagVRMetadataGetMinResolution(nullptr, &minX, &minY) == BAG_INVALID_BAG_HANDLE);
}

TEST_CASE("VRMetadataGetMinResolution - NULL minX returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][vr][metadata][errors]") {
    BagHandle* handle = openSampleBag();
    float minY;
    CHECK(bagVRMetadataGetMinResolution(handle, nullptr, &minY) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}

TEST_CASE("VRMetadataGetMinResolution - NULL minY returns BAG_INVALID_FUNCTION_ARGUMENT", "[bag_c_api][vr][metadata][errors]") {
    BagHandle* handle = openSampleBag();
    float minX;
    CHECK(bagVRMetadataGetMinResolution(handle, &minX, nullptr) == BAG_INVALID_FUNCTION_ARGUMENT);
    REQUIRE(bagFileClose(handle) == BAG_SUCCESS);
}
