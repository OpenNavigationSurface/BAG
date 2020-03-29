//************************************************************************
//
//      Open Navigation Surface Working Group, 2013
//
//************************************************************************
#include "bag_metadatatypes.h"

#include <H5Cpp.h>
#include <string>


namespace {

char* copyString(const char* source)
{
    if (!source || source[0] == '\0')
        return nullptr;

    char* result = new char[strlen(source) + 1];
    strcpy(result, source);

    return result;
}

}  // namespace

//************************************************************************
/*!
\brief Initialize a BagResponsibleParty structure.

\param responsibleParty
    \li The structure to be initialized.
\return
    \li true if the structure is initialized, false if \e responsibleParty
        is NULL.
*/
//************************************************************************
void initResponsibleParty(BagResponsibleParty& responsibleParty) noexcept
{
    responsibleParty.individualName = nullptr;
    responsibleParty.organisationName = nullptr;
    responsibleParty.positionName = nullptr;
    responsibleParty.role = nullptr;
}

//************************************************************************
/*!
\brief Free a BagResponsibleParty structure.

\param responsibleParty
    \li The structure to be freed.
*/
//************************************************************************
void freeResponsibleParty(BagResponsibleParty& responsibleParty) noexcept
{
    delete[] responsibleParty.individualName;
    delete[] responsibleParty.organisationName;
    delete[] responsibleParty.positionName;
    delete[] responsibleParty.role;
}

//************************************************************************
/*!
\brief Initialize a BagIdentification structure.

\param dataIdentificationInfo
    \li The structure to be initialized.
\return
    \li true if the structure is initialized, false if \e dataIdentificationInfo
        is NULL.
*/
//************************************************************************
void initDataIdentificationInfo(BagIdentification& dataIdentificationInfo) noexcept
{
    dataIdentificationInfo.title = nullptr;
	dataIdentificationInfo.date = nullptr;
	dataIdentificationInfo.dateType = nullptr;
	dataIdentificationInfo.abstractString = nullptr;
	dataIdentificationInfo.status = nullptr;
	dataIdentificationInfo.spatialRepresentationType = nullptr;
	dataIdentificationInfo.language = nullptr;
    dataIdentificationInfo.characterSet = nullptr;
	dataIdentificationInfo.topicCategory = nullptr;

    dataIdentificationInfo.verticalUncertaintyType = nullptr;
    dataIdentificationInfo.depthCorrectionType = nullptr;
	dataIdentificationInfo.elevationSolutionGroupType = nullptr;
	dataIdentificationInfo.nodeGroupType = nullptr;

	dataIdentificationInfo.westBoundingLongitude = INIT_VALUE;
	dataIdentificationInfo.eastBoundingLongitude = INIT_VALUE;
	dataIdentificationInfo.southBoundingLatitude = INIT_VALUE;
	dataIdentificationInfo.northBoundingLatitude = INIT_VALUE;

    dataIdentificationInfo.responsibleParties = nullptr;
    dataIdentificationInfo.numberOfResponsibleParties = 0;
}

//************************************************************************
/*!
\brief Free a BagIdentification structure.

\param dataIdentificationInfo
    \li The structure to be freed.
*/
//************************************************************************
void freeDataIdentificationInfo(BagIdentification& dataIdentificationInfo) noexcept
{
    delete[] dataIdentificationInfo.title;
	delete[] dataIdentificationInfo.date;
	delete[] dataIdentificationInfo.dateType;
	delete[] dataIdentificationInfo.abstractString;
	delete[] dataIdentificationInfo.status;
	delete[] dataIdentificationInfo.spatialRepresentationType;
	delete[] dataIdentificationInfo.language;
	delete[] dataIdentificationInfo.characterSet;
	delete[] dataIdentificationInfo.topicCategory;

    delete[] dataIdentificationInfo.verticalUncertaintyType;
	delete[] dataIdentificationInfo.depthCorrectionType;
	delete[] dataIdentificationInfo.elevationSolutionGroupType;
	delete[] dataIdentificationInfo.nodeGroupType;

    for (uint32_t i = 0; i < dataIdentificationInfo.numberOfResponsibleParties; ++i)
        freeResponsibleParty(dataIdentificationInfo.responsibleParties[i]);

    delete[] dataIdentificationInfo.responsibleParties;
}

//************************************************************************
/*!
\brief Initialize a BagLegalConstraints structure.

\param legalConstraints
    \li The structure to be initialized.
\return
    \li true if the structure is initialized, false if \e legalConstraints
        is NULL.
*/
//************************************************************************
void initLegalConstraints(BagLegalConstraints& legalConstraints) noexcept
{
    legalConstraints.useConstraints = nullptr;
    legalConstraints.otherConstraints = nullptr;
}

//************************************************************************
/*!
\brief Free a BagLegalConstraints structure.

\param legalConstraints
    \li The structure to be freed.
*/
//************************************************************************
void freeLegalConstraints(BagLegalConstraints& legalConstraints) noexcept
{
    delete[] legalConstraints.useConstraints;
    delete[] legalConstraints.otherConstraints;
}

//************************************************************************
/*!
\brief Initialize a BagSecurityConstraints structure.

\param securityConstraints
    \li The structure to be initialized.
\return
    \li true if the structure is initialized, false if \e securityConstraints
        is NULL.
*/
//************************************************************************
void initSecurityConstraints(BagSecurityConstraints& securityConstraints) noexcept
{
    securityConstraints.classification = nullptr;
	securityConstraints.userNote = nullptr;
}

//************************************************************************
/*!
\brief Free a BagSecurityConstraints structure.

\param securityConstraints
    \li The structure to be freed.
*/
//************************************************************************
void freeSecurityConstraints(BagSecurityConstraints& securityConstraints) noexcept
{
    delete[] securityConstraints.classification;
	delete[] securityConstraints.userNote;
}

//************************************************************************
/*!
\brief Initialize a BagSource structure.

\param sourceInfo
    \li The structure to be initialized.
\return
    \li true if the structure is initialized, false if \e sourceInfo
        is NULL.
*/
//************************************************************************
void initSourceInfo(BagSource& sourceInfo) noexcept
{
    sourceInfo.description = nullptr;
    sourceInfo.title = nullptr;
    sourceInfo.date = nullptr;
    sourceInfo.dateType = nullptr;

    sourceInfo.responsibleParties = nullptr;
    sourceInfo.numberOfResponsibleParties = 0;
}

//************************************************************************
/*!
\brief Free a BagSource structure.

\param sourceInfo
    \li The structure to be freed.
*/
//************************************************************************
void freeSourceInfo(BagSource& sourceInfo) noexcept
{
    delete[] sourceInfo.description;
    delete[] sourceInfo.title;
    delete[] sourceInfo.date;
    delete[] sourceInfo.dateType;

    for (uint32_t i = 0; i < sourceInfo.numberOfResponsibleParties; i++)
        freeResponsibleParty(sourceInfo.responsibleParties[i]);

    delete[] sourceInfo.responsibleParties;
}

//************************************************************************
/*!
\brief Initialize a BagProcessStep structure.

\param processStep
    \li The structure to be initialized.
\return
    \li true if the structure is initialized, false if \e processStep
        is NULL.
*/
//************************************************************************
void initProcessStep(BagProcessStep& processStep) noexcept
{
    processStep.description = nullptr;
    processStep.dateTime = nullptr;
    processStep.trackingId = nullptr;

    processStep.lineageSources = nullptr;
    processStep.numberOfSources = 0;

    processStep.processors = nullptr;
    processStep.numberOfProcessors = 0;
}

//************************************************************************
/*!
\brief Free a BagProcessStep structure.

\param processStep
    \li The structure to be freed.
*/
//************************************************************************
void freeProcessStep(BagProcessStep& processStep) noexcept
{
    delete[] processStep.description;
    delete[] processStep.dateTime;
    delete[] processStep.trackingId;

    for (uint32_t i = 0; i < processStep.numberOfSources; i++)
        freeSourceInfo(processStep.lineageSources[i]);

    delete[] processStep.lineageSources;

    for (uint32_t i = 0; i < processStep.numberOfProcessors; i++)
        freeResponsibleParty(processStep.processors[i]);

    delete[] processStep.processors;
}

//************************************************************************
/*!
\brief Initialize a BagDataQuality structure.

\param dataQualityInfo
    \li The structure to be initialized.
\return
    \li true if the structure is initialized, false if \e dataQualityInfo
        is NULL.
*/
//************************************************************************
void initDataQualityInfo(BagDataQuality& dataQualityInfo) noexcept
{
    dataQualityInfo.scope = copyString("dataset");
    dataQualityInfo.lineageProcessSteps = nullptr;
    dataQualityInfo.numberOfProcessSteps = 0;
}

//************************************************************************
/*!
\brief Free a BagDataQuality structure.

\param dataQualityInfo
    \li The structure to be freed.
*/
//************************************************************************
void freeDataQualityInfo(BagDataQuality& dataQualityInfo) noexcept
{
    delete[] dataQualityInfo.scope;

    for (uint32_t i = 0; i < dataQualityInfo.numberOfProcessSteps; ++i)
        freeProcessStep(dataQualityInfo.lineageProcessSteps[i]);

    delete[] dataQualityInfo.lineageProcessSteps;
}

//************************************************************************
/*!
\brief Initialize a BagSpatialRepresentation structure.

\param spatialRepresentationInfo
    \li The structure to be initialized.
\return
    \li true if the structure is initialized, false if \e spatialRepresentationInfo
        is NULL.
*/
//************************************************************************
void initSpatialRepresentationInfo(BagSpatialRepresentation& spatialRepresentationInfo)
{
    spatialRepresentationInfo.numberOfRows = 0;
    spatialRepresentationInfo.rowResolution = 0.0;
    spatialRepresentationInfo.numberOfColumns = 0;
    spatialRepresentationInfo.columnResolution = 0.0;
    spatialRepresentationInfo.resolutionUnit = nullptr;

    spatialRepresentationInfo.cellGeometry = copyString("point");
    spatialRepresentationInfo.transformationParameterAvailability = false;
    spatialRepresentationInfo.checkPointAvailability = false;

    spatialRepresentationInfo.llCornerX = INIT_VALUE;
    spatialRepresentationInfo.llCornerY = INIT_VALUE;
    spatialRepresentationInfo.urCornerX = INIT_VALUE;
    spatialRepresentationInfo.urCornerY = INIT_VALUE;

    spatialRepresentationInfo.transformationDimensionDescription = nullptr;
    spatialRepresentationInfo.transformationDimensionMapping = nullptr;
}

//************************************************************************
/*!
\brief Free a BagSpatialRepresentation structure.

\param spatialRepresentationInfo
    \li The structure to be freed.
*/
//************************************************************************
void freeSpatialRepresentationInfo(BagSpatialRepresentation& spatialRepresentationInfo) noexcept
{
    delete[] spatialRepresentationInfo.resolutionUnit;
    delete[] spatialRepresentationInfo.cellGeometry;
    delete[] spatialRepresentationInfo.transformationDimensionDescription;
    delete[] spatialRepresentationInfo.transformationDimensionMapping;
}

//************************************************************************
/*!
\brief Initialize a BagReferenceSystem structure.

\param referenceInfo
    \li The structure to be initialized.
\return
    \li true if the structure is initialized, false if \e referenceInfo
        is NULL.
*/
//************************************************************************
void initReferenceSystemInfo(BagReferenceSystem& referenceInfo) noexcept
{
    referenceInfo.definition = nullptr;
    referenceInfo.type = nullptr;
}

//************************************************************************
/*!
\brief Free a BagReferenceSystem structure.

\param referenceInfo
    \li The structure to be freed.
*/
//************************************************************************
void freeReferenceSystemInfo(BagReferenceSystem& referenceInfo) noexcept
{
    delete[] referenceInfo.definition;
    delete[] referenceInfo.type;
}

//************************************************************************
/*!
\brief Initialize the BagMetadata structure.

    The caller must call bagFreeMetadata() to ensure no memory leaks
    during cleanup.

\param metadata
    \li The structure to be initialized.
\return
    \li 0 on success, a BagError if an error occurs.
*/
//************************************************************************
void bagInitMetadata(BagMetadata& metadata)
{
    metadata.fileIdentifier = nullptr;
    metadata.dateStamp = nullptr;
    metadata.language = copyString("en");
    metadata.characterSet = copyString("utf8");
    metadata.hierarchyLevel = copyString("dataset");
    metadata.metadataStandardName = copyString("ISO 19115");
    metadata.metadataStandardVersion = copyString("2003/Cor.1:2006");

    metadata.contact = new BagResponsibleParty;
    initResponsibleParty(*metadata.contact);

    metadata.spatialRepresentationInfo = new BagSpatialRepresentation;
    initSpatialRepresentationInfo(*metadata.spatialRepresentationInfo);

    metadata.horizontalReferenceSystem = new BagReferenceSystem;
    initReferenceSystemInfo(*metadata.horizontalReferenceSystem);

    metadata.verticalReferenceSystem = new BagReferenceSystem;
    initReferenceSystemInfo(*metadata.verticalReferenceSystem);

    metadata.identificationInfo = new BagIdentification;
    initDataIdentificationInfo(*metadata.identificationInfo);

    metadata.dataQualityInfo = new BagDataQuality;
    initDataQualityInfo(*metadata.dataQualityInfo);

    metadata.legalConstraints = new BagLegalConstraints;
    initLegalConstraints(*metadata.legalConstraints);

    metadata.securityConstraints = new BagSecurityConstraints;
    initSecurityConstraints(*metadata.securityConstraints);
}

//************************************************************************
/*!
\brief Free a BagMetadata structure.

\param metadata
    \li The structure to be freed.
*/
//************************************************************************
void bagFreeMetadata(BagMetadata& metadata) noexcept
{
    delete[] metadata.fileIdentifier;
    delete[] metadata.dateStamp;
    delete[] metadata.language;
    delete[] metadata.hierarchyLevel;
    delete[] metadata.metadataStandardName;
    delete[] metadata.metadataStandardVersion;

    if (metadata.contact)
        freeResponsibleParty(*metadata.contact);

    delete metadata.contact;

    if (metadata.spatialRepresentationInfo)
        freeSpatialRepresentationInfo(*metadata.spatialRepresentationInfo);

    delete metadata.spatialRepresentationInfo;

    if (metadata.horizontalReferenceSystem)
        freeReferenceSystemInfo(*metadata.horizontalReferenceSystem);

    delete metadata.horizontalReferenceSystem;

    if (metadata.verticalReferenceSystem)
        freeReferenceSystemInfo(*metadata.verticalReferenceSystem);

    delete metadata.verticalReferenceSystem;


    if (metadata.identificationInfo)
        freeDataIdentificationInfo(*metadata.identificationInfo);

    delete metadata.identificationInfo;

    if (metadata.dataQualityInfo)
        freeDataQualityInfo(*metadata.dataQualityInfo);

    delete metadata.dataQualityInfo;


    if (metadata.legalConstraints)
        freeLegalConstraints(*metadata.legalConstraints);

    delete metadata.legalConstraints;

    if (metadata.securityConstraints)
        freeSecurityConstraints(*metadata.securityConstraints);

    delete metadata.securityConstraints;
}

