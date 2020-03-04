
#include "test_utils.h"
#include <bag_dataset.h>
#include <bag_metadata.h>
#include <bag_simplelayer.h>
#include <bag_types.h>

#include <array>
#include <catch2/catch.hpp>
#include <cstdlib>  // std::getenv
#include <string>


using BAG::Dataset;
using BAG::Layer;

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

//  const LayerDescriptor& getDescriptor() const;
TEST_CASE("test simple layer get name", "[simplelayer][getDescriptor]")
{
    const std::string bagFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"};

    const auto pDataset = Dataset::open(bagFileName, BAG_OPEN_READONLY);

    REQUIRE(pDataset);
    REQUIRE_NOTHROW(pDataset->getLayer(Elevation));

    const auto& layer = pDataset->getLayer(Elevation);

    REQUIRE_NOTHROW(layer.getDescriptor());

    const auto& descriptor = layer.getDescriptor();

    CHECK(std::string{descriptor.getName()} == BAG::kLayerTypeMapString.at(Elevation));
}

//  virtual std::unique_ptr<uint8_t[]> read(uint32_t rowStart,
//      uint32_t columnStart, uint32_t rowEnd, uint32_t columnEnd) const;
TEST_CASE("test simple layer read", "[simplelayer][read]")
{
    const std::string bagFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/NAVO_data/JD211_public_Release_1-4_UTM.bag"};

    const auto pDataset = Dataset::open(bagFileName, BAG_OPEN_READONLY);

    REQUIRE(pDataset);

    constexpr BAG::LayerType kLayerType = Elevation;

    REQUIRE_NOTHROW(pDataset->getLayer(kLayerType));
    const auto& elevLayer = pDataset->getLayer(kLayerType);

    constexpr int32_t rowStart = 288;
    constexpr int32_t rowEnd = 289;
    constexpr int32_t columnStart = 249;
    constexpr int32_t columnEnd = 251;
    auto buffer = elevLayer.read(rowStart, columnStart, rowEnd, columnEnd); // 2x3

    constexpr size_t kExpectedNumNodes = 6;
    constexpr int32_t rows = (rowEnd - rowStart) + 1;
    constexpr int32_t columns = (columnEnd - columnStart) + 1;

    REQUIRE(buffer);
    REQUIRE((kExpectedNumNodes * sizeof(float)) ==
        (Layer::getElementSize(Layer::getDataType(kLayerType)) * rows * columns));

    std::array<float, kExpectedNumNodes> kExpectedBuffer{
        1'000'000.0f, -52.161003f, -52.172005f,
        1'000'000.0f, -52.177002f, -52.174004f};

    const float* floats = reinterpret_cast<const float*>(buffer.get());
    for (size_t i=0; i<kExpectedNumNodes; ++i)
        CHECK(kExpectedBuffer[i] == floats[i]);
}

//  virtual void write(uint32_t rowStart, uint32_t columnStart, uint32_t rowEnd,
//      uint32_t columnEnd, const uint8_t* buffer) const;
TEST_CASE("test simple layer write", "[simplelayer][write]")
{
    const TestUtils::RandomFileGuard tmpFileName;

    constexpr BAG::LayerType kLayerType = Elevation;
    constexpr size_t kExpectedNumNodes = 12;
    constexpr float kFloatValue = 123.456f;

    // Create the dataset.
    {
        UNSCOPED_INFO("Create the dataset.");
        BAG::Metadata metadata;
        metadata.loadFromBuffer(kMetadataXML);

        constexpr uint64_t chunkSize = 100;
        constexpr unsigned int compressionLevel = 6;
        const auto pDataset = Dataset::create(tmpFileName, std::move(metadata),
            chunkSize, compressionLevel);
        REQUIRE(pDataset);

        UNSCOPED_INFO("Check the Elevation layer exists.");
        REQUIRE_NOTHROW(pDataset->getLayer(kLayerType));

        UNSCOPED_INFO("Check the dimensions of Elevation are expected.");
        const auto fileDims = pDataset->getDescriptor().getDims();
        const auto kExpectedRows =
            pDataset->getMetadata().getStruct().spatialRepresentationInfo->
                numberOfRows;
        const auto kExpectedColumns =
            pDataset->getMetadata().getStruct().spatialRepresentationInfo->
                numberOfColumns;
        CHECK(fileDims == std::make_tuple(kExpectedRows, kExpectedColumns));
    }

    // Open the dataset read/write and write to it.
    {
        UNSCOPED_INFO("Open the dataset for writing.");
        auto pDataset = Dataset::open(tmpFileName, BAG_OPEN_READ_WRITE);
        REQUIRE(pDataset);

        UNSCOPED_INFO("Write data to the Elevation layer.");
        auto& elevLayer = pDataset->getLayer(kLayerType);

        std::array<float, kExpectedNumNodes> buffer;
        buffer.fill(kFloatValue);

        REQUIRE_NOTHROW(elevLayer.write(1, 2, 3, 5,
            reinterpret_cast<uint8_t*>(buffer.data()))); // 3x4
    }

    // Open the dataset and read what was written.
    {
        UNSCOPED_INFO("Open the dataset for reading.");
        auto pDataset = Dataset::open(tmpFileName, BAG_OPEN_READONLY);
        REQUIRE(pDataset);

        const auto& elevLayer = pDataset->getLayer(kLayerType);

        UNSCOPED_INFO("Read the new data in the Elevation layer.");
        auto buffer = elevLayer.read(1, 2, 3, 5); // 3x4
        REQUIRE(buffer);

        const auto* floatBuffer = reinterpret_cast<const float*>(buffer.get());

        std::array<float, kExpectedNumNodes> kExpectedBuffer;
        kExpectedBuffer.fill(kFloatValue);

        for (size_t i=0; i<kExpectedNumNodes; ++i)
            CHECK(floatBuffer[i] == kExpectedBuffer[i]);
    }
}

//  void writeAttributesProxy() const override;
TEST_CASE("test simple layer write attributes", "[simplelayer][write]")
{
    // open a copy of sample.
    const std::string bagFileName{std::string{std::getenv("BAG_SAMPLES_PATH")} +
        "/sample.bag"};

    const TestUtils::RandomFileGuard tmpFileName;
    TestUtils::copyFile(bagFileName, tmpFileName);

    constexpr BAG::LayerType kLayerType = Elevation;

    float kExpectedMin = 0.f;
    float kExpectedMax = 0.f;

    {
        const auto pDataset = Dataset::open(tmpFileName, BAG_OPEN_READ_WRITE);
        REQUIRE(pDataset);

        REQUIRE_NOTHROW(pDataset->getLayer(kLayerType));
        auto& elevLayer = pDataset->getLayer(kLayerType);

        REQUIRE_NOTHROW(elevLayer.getDescriptor());
        auto& descriptor = elevLayer.getDescriptor();

        // change min & max values
        const auto originalMinMax = descriptor.getMinMax();
        kExpectedMin = std::get<0>(originalMinMax) - 12.34f;
        kExpectedMax = std::get<1>(originalMinMax) + 56.789f;

        REQUIRE_NOTHROW(descriptor.setMinMax(kExpectedMin, kExpectedMax));
        REQUIRE_NOTHROW(elevLayer.writeAttributes());
    }

    const auto pDataset = Dataset::open(tmpFileName, BAG_OPEN_READONLY);
    REQUIRE(pDataset);

    const auto& elevLayer = pDataset->getLayer(kLayerType);
    const auto& descriptor = elevLayer.getDescriptor();

    const auto actualMinMax = descriptor.getMinMax();
    CHECK(actualMinMax == std::make_tuple(kExpectedMin, kExpectedMax));
}

