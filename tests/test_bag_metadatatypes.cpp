#include <bag_metadatatypes.h>

#include <catch2/catch_all.hpp>



TEST_CASE("bagInitMetadata/bagFreeMetadata", "[metadataTypes][init][free]")
{
    BagMetadata m;
    bagInitMetadata(m);
    CHECK(m.fileIdentifier == nullptr);
    CHECK(m.dateStamp == nullptr);
    CHECK(std::string{"en"} == m.language);
    CHECK(std::string{"utf8"} == m.characterSet);
    CHECK(std::string{"dataset"} == m.hierarchyLevel);
    CHECK(std::string{"ISO 19115"} == m.metadataStandardName);
    CHECK(std::string{"2003/Cor.1:2006"} == m.metadataStandardVersion);
    CHECK(m.contact != nullptr);
    CHECK(m.spatialRepresentationInfo != nullptr);
    CHECK(m.horizontalReferenceSystem != nullptr);
    CHECK(m.verticalReferenceSystem != nullptr);
    CHECK(m.identificationInfo != nullptr);
    CHECK(m.dataQualityInfo != nullptr);
    CHECK(m.legalConstraints != nullptr);
    CHECK(m.securityConstraints != nullptr);
    m.contact->individualName = new char[8];
    strncpy(m.contact->individualName, "testing", 8);

    bagFreeMetadata(m);
    CHECK(m.fileIdentifier == nullptr);
    CHECK(m.dateStamp == nullptr);
    CHECK(m.language == nullptr);
    CHECK(m.characterSet == nullptr);
    CHECK(m.hierarchyLevel == nullptr);
    CHECK(m.metadataStandardName == nullptr);
    CHECK(m.metadataStandardVersion == nullptr);
    CHECK(m.contact == nullptr);
    CHECK(m.spatialRepresentationInfo == nullptr);
    CHECK(m.horizontalReferenceSystem == nullptr);
    CHECK(m.verticalReferenceSystem == nullptr);
    CHECK(m.identificationInfo == nullptr);
    CHECK(m.dataQualityInfo == nullptr);
    CHECK(m.legalConstraints == nullptr);
    CHECK(m.securityConstraints == nullptr);
}
