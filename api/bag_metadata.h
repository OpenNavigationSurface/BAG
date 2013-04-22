/*!
\file bag_metadata.h
\brief Definition of the BAG metadata structures.
*/
//************************************************************************
//
//      Open Navigation Surface Working Group, 2013
//
//************************************************************************
#ifndef BAG_METADATA_H
#define BAG_METADATA_H

#include "stdtypes.h"
#include "bag_config.h"

/* Value to which floats and ints will be initialized. */
#define INIT_VALUE  -999    

//! This structure contains the contents of the gmd:CI_ResponsibleParty node.
typedef struct
{
    //! Contains the contents of the gmd:individualName node.
    //! Required if organisationName and positionName are not specified.
    u8 *individualName;
    //! Contains the contents of the gmd:organisationName node.
    //! Required if individualName and positionName are not specified.
    u8 *organisationName;
    //! Contains the contents of the gmd:positionName node.
    //! Required if organisationName and individualName are not specified.
    u8 *positionName;
    //! Contains the contents of the gmd:role node.
    //! Required
    u8 *role;
}
BAG_RESPONSIBLE_PARTY;

//! This structure contains the contents of the gmd:MD_LegalConstraints node.
typedef struct
{
    //! Contains the contents of the gmd:useConstraints node.
    //! See codelist for appropriate values. (http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#MD_RestrictionCode)
    //! Required
    u8 *useConstraints;
    //! Contains the contents of the gmd:otherConstraints node.
    u8 *otherConstraints;
}
BAG_LEGAL_CONSTRAINTS;

//! This structure contains the contens of the gmd:MD_SecurityConstraints node.
typedef struct
{
    //! Contains the contents of the gmd:classification node.
    //! See codelist for appropriate values. (http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#MD_ClassificationCode)
    //! Required
    u8 *classification;
    //! Contains the contents of the gmd:userNote node.
    //! May be used to specify distribution, declass authority, declass date.
    //! Required
    u8 *userNote;
}
BAG_SECURITY_CONSTRAINTS;

//! This structure contains the contents of the gmd:LI_Source node.
typedef struct
{
    //! Contains the contents of the gmd:description node.
    //! Required
    u8 *description;
    //! Contains the contents of the gmd:CI_Citation/gmd:title node.
    //! Required if a citation is desired.
    u8 *title;
    //! Contains the contents of the gmd:CI_Citation//gmd:date node.
    //! Required if a citation is desired.
    u8 *date;
    //! Contains the contents of the gmd:CI_Citation//gmd:dateType node.
    //! Required if a citation is desired.
    u8 *dateType;
    //! Contains the contents of the gmd:CI_Citation/gmd:citedResponsibleParty node.
    BAG_RESPONSIBLE_PARTY  *responsibleParties;
    //! The number of responsible parties.
    u32 numberOfResponsibleParties;
}
BAG_SOURCE;

//! This structure contains the contents of the bag:BAG_ProcessStep node.
typedef struct
{
    //! Contains the contents of the gmd:description node.
    //! Required
    u8 *description;
    //! Contains the contents of the gmd:dateTime node.
    u8 *dateTime;
    //! Contains the contents of the gmd:processor node.
    BAG_RESPONSIBLE_PARTY *processors;
    //! The number of processors.
    u32 numberOfProcessors;
    //! Contains the contents of the bag:trackingId node.
    //! Required.
    u8 *trackingId;
    //! Contains the contents of the gmd:source node.
    //! Required (at least one)
    BAG_SOURCE *lineageSources;
    //! The number of sources.
    u32 numberOfSources;
}
BAG_PROCESS_STEP;

//! This structure contains the contents of the gmd:dataQualityInfo node.
typedef struct
{
    //! Contains the contents of the gmd:scope node.
    //! Typically set to 'dataset'.
    //! Required
    u8 *scope;
    //! Contains the contents of the gmd:lineage node.
    //! Required (at least one)
    BAG_PROCESS_STEP *lineageProcessSteps;
    //! The number of process steps.
    u32 numberOfProcessSteps;
}
BAG_DATA_QUALITY;

//! This structure contains the contents of the gmd:spatialRepresentationInfo node.
typedef struct
{
    //! Contains the contents of the axisDimensionProperties//dimensionSize node.
    //! Required
    u32 numberOfRows;
    //! Contains the contents of the axisDimensionProperties//resolution node.
    //! Required
    f64 rowResolution;
    //! Contains the contents of the axisDimensionProperties//dimensionSize node.
    //! Required
    u32 numberOfColumns;
    //! Contains the contents of the axisDimensionProperties//resolution node.
    //! Required
    f64 columnResolution;
    //! Contains the row and column resolution units. Typically metres.
    //! Required
    u8 *resolutionUnit;
    //! Contains the contents of the gmd:cellGeometry node.
    //! Required
    u8 *cellGeometry;
    //! Contains the contents of the gmd:transformationParameterAvailability node.
    //! Required
    Bool transformationParameterAvailability;
    //! Contains the contents of the gmd:checkPointAvailability node.
    //! Required
    Bool checkPointAvailability;
    //! Lower left x value of the gmd:cornerPoints node.
    //! Required
    f64 llCornerX;                                
    //! Lower left y value of the gmd:cornerPoints node.
    //! Required
    f64 llCornerY;                                 
    //! Upper right x value of the gmd:cornerPoints node.
    //! Required
    f64 urCornerX;                              
    //! Upper right y value of the gmd:cornerPoints node.
    //! Required
    f64 urCornerY;                            
}
BAG_SPATIAL_REPRESENTATION;

//! This structure contains the contents of the gmd:identificationInfo node.
typedef struct
{
    //! Contains the contents of the gmd:CI_Citation/gmd:title node.
    //! Required if a citation is desired.
    u8 *title;
    //! Contains the contents of the gmd:CI_Citation//gmd:date node.
    //! Required if a citation is desired.
    u8 *date;
    //! Contains the contents of the gmd:CI_Citation//gmd:dateType node.
    //! Required if a citation is desired.
    u8 *dateType;
    //! Contains the contents of the gmd:identificationInfo node.
    BAG_RESPONSIBLE_PARTY *responsibleParties;
    //! The number of responsible parties.
    u32 numberOfResponsibleParties;
    //! Contains the contents of the gmd:abstract node.
    //! Required
    u8 *abstractString;        
    //! Contains the contents of the gmd:status node.
    u8 *status;                   
    //! Contains the contents of the gmd:spatialRepresentationType node.
    //! See codelist for appropriate values. (http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#MD_SpatialRepresentationTypeCode)
    //! Typically set to 'grid'.
    u8 *spatialRepresentationType;
    //! Contains the contents of the gmd:language node.
    //! See codelist for appropriate values. (http://www.loc.gov/standards/iso639-2/)
    //! Typically set to 'en'.
    //! Required
    u8*language;
    //! Contains the contents of the gmd:characterSet node.
    //! See codelist for appropriate values. (http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#MD_CharacterSetCode)
    //! Typically set to 'utf8'.
    //! Required
    u8*character_set;
    //! Contains the contents of the gmd:topicCategory node.
    //! See codelist for appropriate values. (http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#MD_TopicCategoryCode)
    //! Typically set to 'grid'.
    //! Required
    u8 *topicCategory;
    //! Contains the contents of the gmd:extent/gmd:westBoundLongitude node.
    //! Required if an extent is desired.
    f64 westBoundingLongitude;
    //! Contains the contents of the gmd:extent/gmd:eastBoundLongitude node.
    //! Required if an extent is desired.
    f64 eastBoundingLongitude;
    //! Contains the contents of the gmd:extent/gmd:southBoundLatitude node.
    //! Required if an extent is desired.
    f64 southBoundingLatitude;
    //! Contains the contents of the gmd:extent/gmd:northBoundLatitude node.
    //! Required if an extent is desired.
    f64 northBoundingLatitude;
    //! Contains the contents of the bag:verticalUncertaintyType node.
    //! See codelist for appropriate values. (http://www.opennavsurf.org/schema/bag/bagCodelists.xml#BAG_VertUncertCode)
    //! Required
    u8 *verticalUncertaintyType;
    //! Contains the contents of the bag:depthCorrectionType node.
    //! See codelist for appropriate values. (http://www.opennavsurf.org/schema/bag/bagCodelists.xml#BAG_DepthCorrectCode)
    u8 *depthCorrectionType;
    //! Contains the contents of the bag:nodeGroupType node.
    //! See codelist for appropriate values. (http://www.opennavsurf.org/schema/bag/bagCodelists.xml#BAG_OptGroupCode)
    u8 *nodeGroupType;
    //! Contains the contents of the bag:elevationSolutionGroupType node.
    //! See codelist for appropriate values. (http://www.opennavsurf.org/schema/bag/bagCodelists.xml#BAG_OptGroupCode)
    u8 *elevationSolutionGroupType;
}
BAG_IDENTIFICATION;

//! This structure contains the contents of the gmd:MD_ReferenceSystem node.
typedef struct
{
    //! Contains the contents of the referenceSystemIdentifier/RS_Identifier/code node.
    //! Would typically contain the WKT (Well Known Text) definition.
    //! Required
    u8 *definition;
    //! Contains the contents of the referenceSystemIdentifier/RS_Identifier/codeSpace node.
    //! If the definition is in WKT, this value should be WKT.
    //! Required
    u8 *type;
}
BAG_REFERENCE_SYSTEM;

//! This structure contains the contents of the gmi:MI_Metadata node.
typedef struct
{
    //! Contents of the gmd:fileIdentifier node.
    //! Must be a unique identifier for the metadata.
    //! Required
    u8 *fileIdentifier;
    //! Contents of the gmd:language node.
    //! See codelist for appropriate values. (http://www.loc.gov/standards/iso639-2/)
    //! Required
    u8 *language;
    //! Contents of the gmd:characterSet node.
    //! See codelist for appropriate values. (http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#MD_CharacterSetCode)
    //! Required
    u8 *characterSet;
    //! Contents of the gmd:hierarchyLevel node.
    //! See codelist for appropriate values. (http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml#MD_ScopeCode)
    //! Required
    u8 *hierarchyLevel;
    //! Contents of the gmd:contact node.
    //! Required
    BAG_RESPONSIBLE_PARTY *contact;
    //! Contents of the gmd:dateStamp node.
    //! Required
    u8 *dateStamp;
    //! Contents of the gmd:metadataStandardName node.
    //! Typically initialized to 'ISO 19115'.
    //! Required
    u8 *metadataStandardName;
    //! Contents of the gmd:metadataStandardVersion node.
    //! Typically initialized to '2003/Cor.1:2006'.
    //! Required
    u8 *metadataStandardVersion;
    //! Contents of the gmd:spatialRepresentationInfo node.
    //! Required
    BAG_SPATIAL_REPRESENTATION *spatialRepresentationInfo;
    //! Contents of the gmd:referenceSystemInfo node (horizontal).
    //! Required
    BAG_REFERENCE_SYSTEM *horizontalReferenceSystem;
    //! Contents of the gmd:referenceSystemInfo node (vertical).
    //! Required
    BAG_REFERENCE_SYSTEM *verticalReferenceSystem;
    //! Contents of the gmd:identificationInfo node.
    //! Required
    BAG_IDENTIFICATION *identificationInfo;
    //! Contents of the gmd:dataQualityInfo node.
    //! Required
    BAG_DATA_QUALITY *dataQualityInfo;
    //! Contents of the gmd:metadataConstraints node (legal).
    //! Required
    BAG_LEGAL_CONSTRAINTS *legalConstraints;
    //! Contents of the gmd:metadataConstraints node (security).
    //! Required
    BAG_SECURITY_CONSTRAINTS *securityConstraints;
}
BAG_METADATA;

BAG_EXTERNAL bagError bagInitMetadata(BAG_METADATA * metadata);
BAG_EXTERNAL void bagFreeMetadata(BAG_METADATA * metadata);

BAG_EXTERNAL bagError bagImportMetadataFromXmlBuffer(const u8 *xmlBuffer, u32 bufferSize, BAG_METADATA * metadata, Bool doValidation);
BAG_EXTERNAL bagError bagImportMetadataFromXmlFile(const u8 *fileName, BAG_METADATA * metadata, Bool doValidation);
BAG_EXTERNAL u32 bagExportMetadataToXmlBuffer(BAG_METADATA *metadata, u8** xmlString);

BAG_EXTERNAL void bagSetHomeFolder(const u8 *metadataFolder);

BAG_EXTERNAL bagError bagGetCellDims(BAG_METADATA *metaData, u32 *nRows, u32 *nCols);
BAG_EXTERNAL bagError bagGetGeoCover(BAG_METADATA *metaData, f64 *llLat, f64 *llLong, f64 *urLat, f64 *urLong);
BAG_EXTERNAL bagError bagGetProjectedCover(BAG_METADATA *metaData, f64 *llx, f64 *lly, f64 *urx, f64 *ury);
BAG_EXTERNAL bagError bagGetGridSpacing(BAG_METADATA *metaData, f64 *dx, f64 *dy);

BAG_EXTERNAL bagError bagGetHReferenceSystem(BAG_METADATA *metaData, char *buffer, u32 bufferSize);
BAG_EXTERNAL bagError bagGetVReferenceSystem(BAG_METADATA *metaData, char *buffer, u32 bufferSize);

BAG_EXTERNAL bagError bagGetUncertaintyType(BAG_METADATA *metaData, u32 *uncrtType);
BAG_EXTERNAL bagError bagGetDepthCorrectionType(BAG_METADATA *metaData, u32 *depthCorrectionType);
BAG_EXTERNAL bagError bagGetNodeGroupType(BAG_METADATA *metaData, u8 *);
BAG_EXTERNAL bagError bagGetElevationSolutionType(BAG_METADATA *metaData, u8 *);

#endif

