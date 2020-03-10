/*!
\file bag_metadatatypes.h
\brief Definition of the BAG metadata structures.
*/
//************************************************************************
//
//      Open Navigation Surface Working Group, 2013
//
//************************************************************************
#ifndef BAG_METADATATYPES_H
#define BAG_METADATATYPES_H

#include "bag_c_types.h"

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

/* Value to which floats and ints will be initialized. */
#define INIT_VALUE -999

//! This structure contains the contents of the gmd:CI_ResponsibleParty node.
struct BagResponsibleParty
{
    //! Contains the contents of the gmd:individualName node.
    //! Required if organisationName and positionName are not specified.
    char* individualName;
    //! Contains the contents of the gmd:organisationName node.
    //! Required if individualName and positionName are not specified.
    char* organisationName;
    //! Contains the contents of the gmd:positionName node.
    //! Required if organisationName and individualName are not specified.
    char* positionName;
    //! Contains the contents of the gmd:role node.
    //! Required
    char* role;
};

//! This structure contains the contents of the gmd:MD_LegalConstraints node.
struct BagLegalConstraints
{
    //! Contains the contents of the gmd:useConstraints node.
    //! See codelist for appropriate values. (http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#MD_RestrictionCode)
    //! Required
    char* useConstraints;
    //! Contains the contents of the gmd:otherConstraints node.
    char* otherConstraints;
};

//! This structure contains the contens of the gmd:MD_SecurityConstraints node.
struct BagSecurityConstraints
{
    //! Contains the contents of the gmd:classification node.
    //! See codelist for appropriate values. (http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#MD_ClassificationCode)
    //! Required
    char* classification;
    //! Contains the contents of the gmd:userNote node.
    //! May be used to specify distribution, declass authority, declass date.
    //! Required
    char* userNote;
};

//! This structure contains the contents of the gmd:LI_Source node.
struct BagSource
{
    //! Contains the contents of the gmd:description node.
    //! Required
    char* description;
    //! Contains the contents of the gmd:CI_Citation/gmd:title node.
    //! Required if a citation is desired.
    char* title;
    //! Contains the contents of the gmd:CI_Citation//gmd:date node.
    //! Required if a citation is desired.
    char* date;
    //! Contains the contents of the gmd:CI_Citation//gmd:dateType node.
    //! Required if a citation is desired.
    char* dateType;
    //! Contains the contents of the gmd:CI_Citation/gmd:citedResponsibleParty node.
    BagResponsibleParty* responsibleParties;
    //! The number of responsible parties.
    uint32_t numberOfResponsibleParties;
};

//! This structure contains the contents of the bag:BAG_ProcessStep node.
struct BagProcessStep
{
    //! Contains the contents of the gmd:description node.
    //! Required
    char* description;
    //! Contains the contents of the gmd:dateTime node.
    char* dateTime;
    //! Contains the contents of the gmd:processor node.
    BagResponsibleParty* processors;
    //! The number of processors.
    uint32_t numberOfProcessors;
    //! Contains the contents of the bag:trackingId node.
    //! Required.
    char* trackingId;
    //! Contains the contents of the gmd:source node.
    //! Required (at least one)
    BagSource* lineageSources;
    //! The number of sources.
    uint32_t numberOfSources;
};

//! This structure contains the contents of the gmd:dataQualityInfo node.
struct BagDataQuality
{
    //! Contains the contents of the gmd:scope node.
    //! Typically set to 'dataset'.
    //! Required
    char* scope;
    //! Contains the contents of the gmd:lineage node.
    //! Required (at least one)
    BagProcessStep* lineageProcessSteps;
    //! The number of process steps.
    uint32_t numberOfProcessSteps;
};

//! This structure contains the contents of the gmd:spatialRepresentationInfo node.
struct BagSpatialRepresentation
{
    //! Contains the contents of the axisDimensionProperties//dimensionSize node.
    //! Required
    uint32_t numberOfRows;
    //! Contains the contents of the axisDimensionProperties//resolution node.
    //! Required
    double rowResolution;
    //! Contains the contents of the axisDimensionProperties//dimensionSize node.
    //! Required
    uint32_t numberOfColumns;
    //! Contains the contents of the axisDimensionProperties//resolution node.
    //! Required
    double columnResolution;
    //! Contains the row and column resolution units. Typically metres.
    //! Required
    char* resolutionUnit;
    //! Contains the contents of the gmd:cellGeometry node.
    //! Required
    char* cellGeometry;
    //! Contains the contents of the gmd:transformationParameterAvailability node.
    //! Required
    bool transformationParameterAvailability;
    //! Contains the contents of the gmd:checkPointAvailability node.
    //! Required
    bool checkPointAvailability;
    //! Lower left x value of the gmd:cornerPoints node.
    //! Required
    double llCornerX;
    //! Lower left y value of the gmd:cornerPoints node.
    //! Required
    double llCornerY;
    //! Upper right x value of the gmd:cornerPoints node.
    //! Required
    double urCornerX;
    //! Upper right y value of the gmd:cornerPoints node.
    //! Required
    double urCornerY;
    //! Transformation Dimension Description.
    //! Optional
    char* transformationDimensionDescription;
    //! Transformation Dimension Mapping.
    //! Optional
    char* transformationDimensionMapping;
};

//! This structure contains the contents of the gmd:identificationInfo node.
struct BagIdentification
{
    //! Contains the contents of the gmd:CI_Citation/gmd:title node.
    //! Required if a citation is desired.
    char* title;
    //! Contains the contents of the gmd:CI_Citation//gmd:date node.
    //! Required if a citation is desired.
    char* date;
    //! Contains the contents of the gmd:CI_Citation//gmd:dateType node.
    //! Required if a citation is desired.
    char* dateType;
    //! Contains the contents of the gmd:identificationInfo node.
    BagResponsibleParty* responsibleParties;
    //! The number of responsible parties.
    uint32_t numberOfResponsibleParties;
    //! Contains the contents of the gmd:abstract node.
    //! Required
    char* abstractString;
    //! Contains the contents of the gmd:status node.
    char* status;
    //! Contains the contents of the gmd:spatialRepresentationType node.
    //! See codelist for appropriate values. (http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#MD_SpatialRepresentationTypeCode)
    //! Typically set to 'grid'.
    char* spatialRepresentationType;
    //! Contains the contents of the gmd:language node.
    //! See codelist for appropriate values. (http://www.loc.gov/standards/iso639-2/)
    //! Typically set to 'en'.
    //! Required
    char* language;
    //! Contains the contents of the gmd:characterSet node.
    //! See codelist for appropriate values. (http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#MD_CharacterSetCode)
    //! Typically set to 'utf8'.
    //! Required
    char* characterSet;
    //! Contains the contents of the gmd:topicCategory node.
    //! See codelist for appropriate values. (http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#MD_TopicCategoryCode)
    //! Typically set to 'grid'.
    //! Required
    char* topicCategory;
    //! Contains the contents of the gmd:extent/gmd:westBoundLongitude node.
    //! Required if an extent is desired.
    double westBoundingLongitude;
    //! Contains the contents of the gmd:extent/gmd:eastBoundLongitude node.
    //! Required if an extent is desired.
    double eastBoundingLongitude;
    //! Contains the contents of the gmd:extent/gmd:southBoundLatitude node.
    //! Required if an extent is desired.
    double southBoundingLatitude;
    //! Contains the contents of the gmd:extent/gmd:northBoundLatitude node.
    //! Required if an extent is desired.
    double northBoundingLatitude;
    //! Contains the contents of the bag:verticalUncertaintyType node.
    //! See codelist for appropriate values. (http://www.opennavsurf.org/schema/bag/bagCodelists.xml#BAG_VertUncertCode)
    //! Required
    char* verticalUncertaintyType;
    //! Contains the contents of the bag:depthCorrectionType node.
    //! See codelist for appropriate values. (http://www.opennavsurf.org/schema/bag/bagCodelists.xml#BAG_DepthCorrectCode)
    char* depthCorrectionType;
    //! Contains the contents of the bag:nodeGroupType node.
    //! See codelist for appropriate values. (http://www.opennavsurf.org/schema/bag/bagCodelists.xml#BAG_OptGroupCode)
    char* nodeGroupType;
    //! Contains the contents of the bag:elevationSolutionGroupType node.
    //! See codelist for appropriate values. (http://www.opennavsurf.org/schema/bag/bagCodelists.xml#BAG_OptGroupCode)
    char* elevationSolutionGroupType;
};

//! This structure contains the contents of the gmd:MD_ReferenceSystem node.
struct BagReferenceSystem
{
    //! Contains the contents of the referenceSystemIdentifier/RS_Identifier/code node.
    //! Would typically contain the WKT (Well Known Text) definition.
    //! Required
    char* definition;
    //! Contains the contents of the referenceSystemIdentifier/RS_Identifier/codeSpace node.
    //! If the definition is in WKT, this value should be WKT.
    //! Required
    char* type;
};

//! This structure contains the contents of the gmi:MI_Metadata node.
struct BagMetadata
{
    //! Contents of the gmd:fileIdentifier node.
    //! Must be a unique identifier for the metadata.
    //! Required
    char* fileIdentifier;
    //! Contents of the gmd:language node.
    //! See codelist for appropriate values. (http://www.loc.gov/standards/iso639-2/)
    //! Required
    char* language;
    //! Contents of the gmd:characterSet node.
    //! See codelist for appropriate values. (http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#MD_CharacterSetCode)
    //! Required
    char* characterSet;
    //! Contents of the gmd:hierarchyLevel node.
    //! See codelist for appropriate values. (http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#MD_ScopeCode)
    //! Required
    char* hierarchyLevel;
    //! Contents of the gmd:contact node.
    //! Required
    BagResponsibleParty* contact;
    //! Contents of the gmd:dateStamp node.
    //! Required
    char* dateStamp;
    //! Contents of the gmd:metadataStandardName node.
    //! Typically initialized to 'ISO 19115'.
    //! Required
    char* metadataStandardName;
    //! Contents of the gmd:metadataStandardVersion node.
    //! Typically initialized to '2003/Cor.1:2006'.
    //! Required
    char* metadataStandardVersion;
    //! Contents of the gmd:spatialRepresentationInfo node.
    //! Required
    BagSpatialRepresentation* spatialRepresentationInfo;
    //! Contents of the gmd:referenceSystemInfo node (horizontal).
    //! Required
    BagReferenceSystem* horizontalReferenceSystem;
    //! Contents of the gmd:referenceSystemInfo node (vertical).
    //! Required
    BagReferenceSystem* verticalReferenceSystem;
    //! Contents of the gmd:identificationInfo node.
    //! Required
    BagIdentification* identificationInfo;
    //! Contents of the gmd:dataQualityInfo node.
    //! Required
    BagDataQuality* dataQualityInfo;
    //! Contents of the gmd:metadataConstraints node (legal).
    //! Required
    BagLegalConstraints* legalConstraints;
    //! Contents of the gmd:metadataConstraints node (security).
    //! Required
    BagSecurityConstraints* securityConstraints;
};

#ifdef __cplusplus
}  // extern "C"
#endif

void bagInitMetadata(BagMetadata& metadata);
void bagFreeMetadata(BagMetadata& metadata) noexcept;

void initResponsibleParty(BagResponsibleParty& responsibleParty) noexcept;
void initSourceInfo(BagSource& sourceInfo) noexcept;
void initProcessStep(BagProcessStep& processStep) noexcept;

#endif  //BAG_METADATATYPES_H

