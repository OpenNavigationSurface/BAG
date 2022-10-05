
#include "test_utils.h"
#include <bag_dataset.h>
#include <bag_surfacecorrections.h>
#include <bag_surfacecorrectionsdescriptor.h>

#include <array>
#include <catch2/catch_all.hpp>
#include <cstdlib>  // std::getenv
#include <string>


using BAG::Dataset;
using BAG::SurfaceCorrectionsDescriptor;
using BAG::VerticalDatumCorrections;

namespace {

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

}  // namespace

//  More of an integration test to see if the surface corrections is read.
TEST_CASE("test surface corrections read irregular",
    "[surfacecorrections][constructor][destructor][BAG_SURFACE_IRREGULARLY_SPACED]")
{
    const std::string bagFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"};

    const auto pDataset = Dataset::open(bagFileName, BAG_OPEN_READONLY);
    UNSCOPED_INFO("Check that the dataset was loaded.");
    REQUIRE(pDataset);

    UNSCOPED_INFO("Check that the surface corrections exist.");
    const auto pCorrections = pDataset->getSurfaceCorrections();
    REQUIRE(pCorrections);

    UNSCOPED_INFO("Check that the surface corrections descriptor exists.");
    REQUIRE_NOTHROW(pCorrections->getDescriptor());

    UNSCOPED_INFO("Check that the surface corrections descriptor exists.");
    auto pDescriptor =
        std::dynamic_pointer_cast<const SurfaceCorrectionsDescriptor>(
            pCorrections->getDescriptor());
    CHECK(pDescriptor);

    UNSCOPED_INFO("Check that the surface type being corrected is gridded.");
    CHECK(pDescriptor->getSurfaceType() == BAG_SURFACE_IRREGULARLY_SPACED);

    UNSCOPED_INFO("Check that the expected number of correctors were loaded.");
    constexpr size_t kExpectedNumCorrectors= 2;
    CHECK(pDescriptor->getNumCorrectors() == kExpectedNumCorrectors);
}

TEST_CASE("test surface corrections create empty irregular",
    "[surfacecorrections][constructor][destructor][create][BAG_SURFACE_IRREGULARLY_SPACED]")
{
    const TestUtils::RandomFileGuard tmpFileName;

    UNSCOPED_INFO("Check dataset was created successfully.");
    constexpr uint64_t chunkSize = 100;
    constexpr int compressionLevel = 6;
    auto pDataset = Dataset::create(tmpFileName, BAG::Metadata{}, chunkSize,
        compressionLevel);
    REQUIRE(pDataset);

    constexpr auto kExpectedSurfaceType = BAG_SURFACE_IRREGULARLY_SPACED;
    constexpr uint8_t kExpectedNumCorrectors = 4;
    const auto& corrections = pDataset->createSurfaceCorrections(
        kExpectedSurfaceType, kExpectedNumCorrectors, chunkSize,
        compressionLevel);

    UNSCOPED_INFO("Check surface corrections descriptor is the default pDescriptor->");
    auto pDescriptor =
        std::dynamic_pointer_cast<const SurfaceCorrectionsDescriptor>(
            corrections.getDescriptor());
    CHECK(pDescriptor);

    UNSCOPED_INFO("Check surface corrections num correctors.");
    CHECK(pDescriptor->getNumCorrectors() == kExpectedNumCorrectors);
    UNSCOPED_INFO("Check surface corrections surface type.");
    CHECK(pDescriptor->getSurfaceType() == kExpectedSurfaceType);
    UNSCOPED_INFO("Check surface corrections default datums.");
    CHECK(pDescriptor->getVerticalDatums() == "");
    UNSCOPED_INFO("Check surface corrections default southwest X, Y.");
    CHECK(pDescriptor->getOrigin() == std::make_tuple(0., 0.));
    UNSCOPED_INFO("Check surface corrections default X and Y spacing.");
    CHECK(pDescriptor->getSpacing() == std::make_tuple(0., 0.));
}

TEST_CASE("test surface corrections create empty gridded",
    "[surfacecorrections][constructor][destructor][create][BAG_SURFACE_GRID_EXTENTS]")
{
    const TestUtils::RandomFileGuard tmpFileName;

    UNSCOPED_INFO("Check dataset was created successfully.");
    constexpr uint64_t chunkSize = 100;
    constexpr int compressionLevel = 6;
    auto pDataset = Dataset::create(tmpFileName, BAG::Metadata{}, chunkSize,
        compressionLevel);
    REQUIRE(pDataset);

    constexpr auto kExpectedSurfaceType = BAG_SURFACE_GRID_EXTENTS;
    constexpr uint8_t kExpectedNumCorrectors = 2;
    const auto& corrections = pDataset->createSurfaceCorrections(
        kExpectedSurfaceType, kExpectedNumCorrectors, chunkSize,
        compressionLevel);

    UNSCOPED_INFO("Check surface corrections descriptor is the default pDescriptor->");
    auto pDescriptor =
        std::dynamic_pointer_cast<const SurfaceCorrectionsDescriptor>(
            corrections.getDescriptor());
    CHECK(pDescriptor);

    UNSCOPED_INFO("Check surface corrections num correctors.");
    CHECK(pDescriptor->getNumCorrectors() == kExpectedNumCorrectors);
    UNSCOPED_INFO("Check surface corrections surface type.");
    CHECK(pDescriptor->getSurfaceType() == kExpectedSurfaceType);
    UNSCOPED_INFO("Check surface corrections default datums.");
    CHECK(pDescriptor->getVerticalDatums() == "");
    UNSCOPED_INFO("Check surface corrections default southwest X, Y.");
    CHECK(pDescriptor->getOrigin() == std::make_tuple(0., 0.));
    UNSCOPED_INFO("Check surface corrections default X and Y spacing.");
    CHECK(pDescriptor->getSpacing() == std::make_tuple(0., 0.));
}

TEST_CASE("test surface corrections create and write irregular",
    "[surfacecorrections][constructor][destructor][create][read][write][BAG_SURFACE_IRREGULARLY_SPACED]")
{
    const TestUtils::RandomFileGuard tmpFileName;

    UNSCOPED_INFO("Check dataset was created successfully.");
    constexpr uint64_t chunkSize = 100;
    constexpr int compressionLevel = 6;
    auto pDataset = Dataset::create(tmpFileName, BAG::Metadata{}, chunkSize,
        compressionLevel);
    REQUIRE(pDataset);

    constexpr auto kExpectedSurfaceType = BAG_SURFACE_IRREGULARLY_SPACED;
    constexpr uint8_t kExpectedNumCorrectors = 2;
    auto& corrections = pDataset->createSurfaceCorrections(kExpectedSurfaceType,
        kExpectedNumCorrectors, chunkSize, compressionLevel);

    UNSCOPED_INFO("Check surface corrections descriptor is the default pDescriptor->");
    auto pDescriptor =
        std::dynamic_pointer_cast<const SurfaceCorrectionsDescriptor>(
            corrections.getDescriptor());
    CHECK(pDescriptor);

    constexpr BAG::VerticalDatumCorrections kExpectedItem0{1.2, 2.1, {3.4f, 4.5f}};

    UNSCOPED_INFO("Write one record.");
    const uint8_t* buffer = reinterpret_cast<const uint8_t*>(&kExpectedItem0);
    constexpr uint32_t kRowStart = 0;
    constexpr uint32_t kColumnStart = 0;
    constexpr uint32_t kRowEnd = 0;
    constexpr uint32_t kColumnEnd = 0;

    REQUIRE_NOTHROW(corrections.write(kRowStart, kColumnStart, kRowEnd,
        kColumnEnd, buffer));

    UNSCOPED_INFO("Read the record back.");
    auto result = corrections.read(kRowStart, kColumnStart, kRowEnd, kColumnEnd);
    CHECK(result);

    const auto* res = reinterpret_cast<const BAG::VerticalDatumCorrections*>(result.data());
    CHECK(res->x == kExpectedItem0.x);
    CHECK(res->y == kExpectedItem0.y);
    CHECK(res->z[0] == kExpectedItem0.z[0]);
    CHECK(res->z[1] == kExpectedItem0.z[1]);
}

TEST_CASE("test surface corrections create, write, read gridded",
    "[surfacecorrections][constructor][destructor][create][read][write][BAG_SURFACE_GRID_EXTENTS]")
{
    const TestUtils::RandomFileGuard tmpFileName;

    UNSCOPED_INFO("Check dataset was created successfully.");
    constexpr uint64_t chunkSize = 100;
    constexpr int compressionLevel = 6;
    auto pDataset = Dataset::create(tmpFileName, BAG::Metadata{}, chunkSize,
        compressionLevel);
    REQUIRE(pDataset);

    constexpr auto kExpectedSurfaceType = BAG_SURFACE_GRID_EXTENTS;
    constexpr uint8_t kExpectedNumCorrectors = 3;
    auto& corrections = pDataset->createSurfaceCorrections(kExpectedSurfaceType,
        kExpectedNumCorrectors, chunkSize, compressionLevel);

    UNSCOPED_INFO("Check surface corrections descriptor is the default pDescriptor->");
    auto pDescriptor =
        std::dynamic_pointer_cast<const SurfaceCorrectionsDescriptor>(
            corrections.getDescriptor());
    CHECK(pDescriptor);

    constexpr BagVerticalDatumCorrectionsGridded kExpectedItem0{9.87f, 6.543f,
        2.109876f};

    UNSCOPED_INFO("Write one record.");
    const auto* buffer = reinterpret_cast<const uint8_t*>(&kExpectedItem0);
    constexpr uint32_t kRowStart = 0;
    constexpr uint32_t kColumnStart = 0;
    constexpr uint32_t kRowEnd = 0;
    constexpr uint32_t kColumnEnd = 0;

    REQUIRE_NOTHROW(corrections.write(kRowStart, kColumnStart, kRowEnd,
        kColumnEnd, buffer));

    UNSCOPED_INFO("Read the record back.");
    auto result = corrections.read(kRowStart, kColumnStart, kRowEnd, kColumnEnd);
    CHECK(result);

    const auto* res = reinterpret_cast<const BagVerticalDatumCorrectionsGridded*>(result.data());
    CHECK(res->z[0] == kExpectedItem0.z[0]);
    CHECK(res->z[1] == kExpectedItem0.z[1]);
    CHECK(res->z[2] == kExpectedItem0.z[2]);
}

TEST_CASE("test surface corrections create, write, read two gridded records",
    "[surfacecorrections][constructor][destructor][create][read][write][BAG_SURFACE_GRID_EXTENTS]")
{
    const TestUtils::RandomFileGuard tmpFileName;

    constexpr uint32_t kRowStart = 0;
    constexpr uint32_t kColumnStart = 0;
    constexpr uint32_t kRowEnd = 0;
    constexpr uint32_t kColumnEnd = 1;

    constexpr uint8_t kExpectedNumCorrectors = 3;

    const std::array<BagVerticalDatumCorrectionsGridded, 2> kExpectedItems{
        BagVerticalDatumCorrectionsGridded{1.23f, 4.56f, 7.89f},
        BagVerticalDatumCorrectionsGridded{9.87f, 6.54f, 3.21f},
    };

    // Write the surface corrections.
    {
        UNSCOPED_INFO("Check dataset was created successfully.");

        BAG::Metadata metadata;
        metadata.loadFromBuffer(kMetadataXML);

        constexpr uint64_t chunkSize = 100;
        constexpr int compressionLevel = 6;

        auto pDataset = Dataset::create(tmpFileName, std::move(metadata),
            chunkSize, compressionLevel);
        REQUIRE(pDataset);

        constexpr auto kExpectedSurfaceType = BAG_SURFACE_GRID_EXTENTS;

        auto& corrections = pDataset->createSurfaceCorrections(
            kExpectedSurfaceType, kExpectedNumCorrectors, chunkSize,
            compressionLevel);

        UNSCOPED_INFO("Check the descriptor is the expected type.");
        auto pDescriptor =
            std::dynamic_pointer_cast<const SurfaceCorrectionsDescriptor>(
                corrections.getDescriptor());
        REQUIRE(pDescriptor);

        UNSCOPED_INFO("Write two records.");
        const auto* buffer =
            reinterpret_cast<const uint8_t*>(kExpectedItems.data());

        REQUIRE_NOTHROW(corrections.write(kRowStart, kColumnStart, kRowEnd,
            kColumnEnd, buffer));
    }

    // Read the surface corrections back.
    auto pDataset = Dataset::open(tmpFileName, BAG_OPEN_READONLY);
    REQUIRE(pDataset);

    auto pCorrections = pDataset->getSurfaceCorrections();
    REQUIRE(pCorrections);

    UNSCOPED_INFO("Read the records back.");
    auto result = pCorrections->read(kRowStart, kColumnStart, kRowEnd, kColumnEnd);
    REQUIRE(result);

    const auto* res =
        reinterpret_cast<BagVerticalDatumCorrectionsGridded*>(result.data());

    // Check the results.
    CHECK(res[0].z[0] == kExpectedItems[0].z[0]);
    CHECK(res[0].z[1] == kExpectedItems[0].z[1]);
    CHECK(res[0].z[2] == kExpectedItems[0].z[2]);

    CHECK(res[1].z[0] == kExpectedItems[1].z[0]);
    CHECK(res[1].z[1] == kExpectedItems[1].z[1]);
    CHECK(res[1].z[2] == kExpectedItems[1].z[2]);
}

