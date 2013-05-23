//************************************************************************
//
//      Open Navigation Surface Working Group, 2013
//
//************************************************************************
#include "bag.h"
#include "bag_metadata.h"
#include "bag_errors.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <algorithm>

//This may not work for everyone.
#ifndef __cplusplus
#define _strdup strdup
#endif


//************************************************************************
/*!
\brief Initialize a BAG_RESPONSIBLE_PARTY structure.

\param responsibleParty
    \li The structure to be initialized.
\return
    \li True if the structure is initialized, False if \e responsibleParty
        is NULL.
*/
//************************************************************************
Bool initResponsibleParty(BAG_RESPONSIBLE_PARTY * responsibleParty)
{
    if (responsibleParty == NULL)
    {
        fprintf(stderr,"ERROR: Exception when attempting to intialize data structure. Exception message is: Null pointer\n");
        return False;
    }

    (*responsibleParty).individualName = NULL;
    (*responsibleParty).organisationName = NULL;
    (*responsibleParty).positionName = NULL;
    (*responsibleParty).role = NULL;

    return True;
}

//************************************************************************
/*!
\brief Free a BAG_RESPONSIBLE_PARTY structure.

\param responsibleParty
    \li The structure to be freed.
*/
//************************************************************************
void freeResponsibleParty(BAG_RESPONSIBLE_PARTY * responsibleParty)
{
    if (responsibleParty == NULL)
        return;

    free(responsibleParty->individualName);
    free(responsibleParty->organisationName);
    free(responsibleParty->positionName);
    free(responsibleParty->role);
}

//************************************************************************
/*!
\brief Initialize a BAG_IDENTIFICATION structure.

\param dataIdentificationInfo
    \li The structure to be initialized.
\return
    \li True if the structure is initialized, False if \e dataIdentificationInfo
        is NULL.
*/
//************************************************************************
Bool initDataIdentificationInfo(BAG_IDENTIFICATION * dataIdentificationInfo)
{
    if (dataIdentificationInfo == NULL)
    {
        fprintf(stderr,"ERROR: Exception when attempting to intialize data structure. Exception message is: Null pointer\n");
        return False;
    }

    (*dataIdentificationInfo).title = NULL;
	(*dataIdentificationInfo).date = NULL;
	(*dataIdentificationInfo).dateType = NULL; 
	(*dataIdentificationInfo).abstractString = NULL;
	(*dataIdentificationInfo).status = NULL;
	(*dataIdentificationInfo).spatialRepresentationType = NULL;
	(*dataIdentificationInfo).language = NULL;
    (*dataIdentificationInfo).character_set = NULL;
	(*dataIdentificationInfo).topicCategory = NULL;

    (*dataIdentificationInfo).verticalUncertaintyType = NULL;
	(*dataIdentificationInfo).depthCorrectionType = NULL;
	(*dataIdentificationInfo).elevationSolutionGroupType = NULL;
	(*dataIdentificationInfo).nodeGroupType = NULL;

	(*dataIdentificationInfo).westBoundingLongitude = INIT_VALUE;
	(*dataIdentificationInfo).eastBoundingLongitude = INIT_VALUE;   
	(*dataIdentificationInfo).southBoundingLatitude = INIT_VALUE;       
	(*dataIdentificationInfo).northBoundingLatitude = INIT_VALUE;

    (*dataIdentificationInfo).responsibleParties = NULL;
    (*dataIdentificationInfo).numberOfResponsibleParties = 0;

    return True;
}

//************************************************************************
/*!
\brief Free a BAG_IDENTIFICATION structure.

\param dataIdentificationInfo
    \li The structure to be freed.
*/
//************************************************************************
void freeDataIdentificationInfo(BAG_IDENTIFICATION * dataIdentificationInfo)
{
    if (dataIdentificationInfo == NULL)
        return;

    free(dataIdentificationInfo->title);
	free(dataIdentificationInfo->date);
	free(dataIdentificationInfo->dateType);
	free(dataIdentificationInfo->abstractString);
	free(dataIdentificationInfo->status);
	free(dataIdentificationInfo->spatialRepresentationType);
	free(dataIdentificationInfo->language);
	free(dataIdentificationInfo->topicCategory);

    free(dataIdentificationInfo->verticalUncertaintyType);
	free(dataIdentificationInfo->depthCorrectionType);
	free(dataIdentificationInfo->elevationSolutionGroupType);
	free(dataIdentificationInfo->nodeGroupType);

    for (u32 i = 0; i < dataIdentificationInfo->numberOfResponsibleParties; i++)
        freeResponsibleParty(&dataIdentificationInfo->responsibleParties[i]);
    free(dataIdentificationInfo->responsibleParties);
}

//************************************************************************
/*!
\brief Initialize a BAG_LEGAL_CONSTRAINTS structure.

\param legalConstraints
    \li The structure to be initialized.
\return
    \li True if the structure is initialized, False if \e legalConstraints
        is NULL.
*/
//************************************************************************
Bool initLegalConstraints(BAG_LEGAL_CONSTRAINTS * legalConstraints)
{
    if (legalConstraints == NULL)
    {
        fprintf(stderr,"ERROR: Exception when attempting to intialize data structure. Exception message is: Null pointer\n");
        return False;
    }

   (*legalConstraints).useConstraints = NULL;
   (*legalConstraints).otherConstraints = NULL;

    return True;
}

//************************************************************************
/*!
\brief Free a BAG_LEGAL_CONSTRAINTS structure.

\param legalConstraints
    \li The structure to be freed.
*/
//************************************************************************
void freeLegalConstraints(BAG_LEGAL_CONSTRAINTS * legalConstraints)
{
    if (legalConstraints == NULL)
        return;
    
   free(legalConstraints->useConstraints);
   free(legalConstraints->otherConstraints);
}

//************************************************************************
/*!
\brief Initialize a BAG_SECURITY_CONSTRAINTS structure.

\param securityConstraints
    \li The structure to be initialized.
\return
    \li True if the structure is initialized, False if \e securityConstraints
        is NULL.
*/
//************************************************************************
Bool initSecurityConstraints(BAG_SECURITY_CONSTRAINTS * securityConstraints)
{
    if (securityConstraints == NULL)
    {
        fprintf(stderr,"ERROR: Exception when attempting to intialize data structure. Exception message is: Null pointer\n");
        return False;
    }
    
    (*securityConstraints).classification = NULL;
	(*securityConstraints).userNote = NULL;

    return True;
}

//************************************************************************
/*!
\brief Free a BAG_SECURITY_CONSTRAINTS structure.

\param securityConstraints
    \li The structure to be freed.
*/
//************************************************************************
void freeSecurityConstraints(BAG_SECURITY_CONSTRAINTS * securityConstraints)
{
    if (securityConstraints == NULL)
        return;
    
    free(securityConstraints->classification);
	free(securityConstraints->userNote);
}

//************************************************************************
/*!
\brief Initialize a BAG_SOURCE structure.

\param sourceInfo
    \li The structure to be initialized.
\return
    \li True if the structure is initialized, False if \e sourceInfo
        is NULL.
*/
//************************************************************************
Bool initSourceInfo(BAG_SOURCE * sourceInfo)
{
    if (sourceInfo == NULL)
    {
        fprintf(stderr,"ERROR: Exception when attempting to intialize data structure. Exception message is: Null pointer\n");
        return False;
    }

    (*sourceInfo).description = NULL;
    (*sourceInfo).title = NULL;
    (*sourceInfo).date = NULL;
    (*sourceInfo).dateType = NULL;

    (*sourceInfo).responsibleParties = NULL;
    (*sourceInfo).numberOfResponsibleParties = NULL;

    return True;
}

//************************************************************************
/*!
\brief Free a BAG_SOURCE structure.

\param sourceInfo
    \li The structure to be freed.
*/
//************************************************************************
void freeSourceInfo(BAG_SOURCE * sourceInfo)
{
    if (sourceInfo == NULL)
        return;

    free(sourceInfo->description);
    free(sourceInfo->title);
    free(sourceInfo->date);
    free(sourceInfo->dateType);

    for (u32 i = 0; i < sourceInfo->numberOfResponsibleParties; i++)
        freeResponsibleParty(&sourceInfo->responsibleParties[i]);
}

//************************************************************************
/*!
\brief Initialize a BAG_PROCESS_STEP structure.

\param processStep
    \li The structure to be initialized.
\return
    \li True if the structure is initialized, False if \e processStep
        is NULL.
*/
//************************************************************************
Bool initProcessStep(BAG_PROCESS_STEP * processStep)
{
    if (processStep == NULL)
    {
        fprintf(stderr,"ERROR: Exception when attempting to intialize data structure. Exception message is: Null pointer\n");
        return False;
    }

    (*processStep).description = NULL;
    (*processStep).dateTime = NULL;
    (*processStep).trackingId = NULL;

    (*processStep).lineageSources = NULL;
    (*processStep).numberOfSources = 0;

    (*processStep).processors = NULL;
    (*processStep).numberOfProcessors = 0;

    return True;
}

//************************************************************************
/*!
\brief Free a BAG_PROCESS_STEP structure.

\param processStep
    \li The structure to be freed.
*/
//************************************************************************
void freeProcessStep(BAG_PROCESS_STEP * processStep)
{
    if (processStep == NULL)
        return;

    free(processStep->description);
    free(processStep->dateTime);
    free(processStep->trackingId);

    freeSourceInfo(processStep->lineageSources);
    free(processStep->lineageSources);

    for (u32 i = 0; i < processStep->numberOfProcessors; i++)
        freeResponsibleParty(&processStep->processors[i]);
}

//************************************************************************
/*!
\brief Initialize a BAG_DATA_QUALITY structure.

\param dataQualityInfo
    \li The structure to be initialized.
\return
    \li True if the structure is initialized, False if \e dataQualityInfo
        is NULL.
*/
//************************************************************************
Bool initDataQualityInfo(BAG_DATA_QUALITY * dataQualityInfo)
{
    if (dataQualityInfo == NULL)
    {
        fprintf(stderr,"ERROR: Exception when attempting to intialize data structure. Exception message is: Null pointer\n");
        return False;
    }

    (*dataQualityInfo).scope = (u8*)_strdup("dataset");
    (*dataQualityInfo).lineageProcessSteps = NULL;
    (*dataQualityInfo).numberOfProcessSteps = 0;

    return True;
}

//************************************************************************
/*!
\brief Free a BAG_DATA_QUALITY structure.

\param dataQualityInfo
    \li The structure to be freed.
*/
//************************************************************************
void freeDataQualityInfo(BAG_DATA_QUALITY * dataQualityInfo)
{
    if (dataQualityInfo == NULL)
        return;

    free(dataQualityInfo->scope);

    freeProcessStep(dataQualityInfo->lineageProcessSteps);
    free(dataQualityInfo->lineageProcessSteps);
}

//************************************************************************
/*!
\brief Initialize a BAG_SPATIAL_REPRESENTATION structure.

\param spatialRepresentationInfo
    \li The structure to be initialized.
\return
    \li True if the structure is initialized, False if \e spatialRepresentationInfo
        is NULL.
*/
//************************************************************************
Bool initSpatialRepresentationInfo(BAG_SPATIAL_REPRESENTATION * spatialRepresentationInfo)
{
    if (spatialRepresentationInfo == NULL)
    {
        fprintf(stderr,"ERROR: Exception when attempting to intialize data structure. Exception message is: Null pointer\n");
        return False;
    }

    (*spatialRepresentationInfo).numberOfRows = 0;
    (*spatialRepresentationInfo).rowResolution = 0.0;
    (*spatialRepresentationInfo).numberOfColumns = 0;
    (*spatialRepresentationInfo).columnResolution = 0.0;
    (*spatialRepresentationInfo).resolutionUnit = NULL;

    (*spatialRepresentationInfo).cellGeometry = (u8*)_strdup("point");
    (*spatialRepresentationInfo).transformationParameterAvailability = False;
    (*spatialRepresentationInfo).checkPointAvailability = False;              

    (*spatialRepresentationInfo).llCornerX = INIT_VALUE;                                 
    (*spatialRepresentationInfo).llCornerY = INIT_VALUE;                                  
    (*spatialRepresentationInfo).urCornerX = INIT_VALUE;                               
    (*spatialRepresentationInfo).urCornerY = INIT_VALUE;  

    return True;
}

//************************************************************************
/*!
\brief Free a BAG_SPATIAL_REPRESENTATION structure.

\param spatialRepresentationInfo
    \li The structure to be freed.
*/
//************************************************************************
void freeSpatialRepresentationInfo(BAG_SPATIAL_REPRESENTATION * spatialRepresentationInfo)
{
    if (spatialRepresentationInfo == NULL)
        return;

    free(spatialRepresentationInfo->resolutionUnit);
    free(spatialRepresentationInfo->cellGeometry);
}

//************************************************************************
/*!
\brief Initialize a BAG_REFERENCE_SYSTEM structure.

\param referenceInfo
    \li The structure to be initialized.
\return
    \li True if the structure is initialized, False if \e referenceInfo
        is NULL.
*/
//************************************************************************
Bool initReferenceSystemInfo(BAG_REFERENCE_SYSTEM *referenceInfo)
{
    if (referenceInfo == NULL)
    {
        fprintf(stderr,"ERROR: Exception when attempting to intialize data structure. Exception message is: Null pointer\n");
        return False;
    }

    referenceInfo->definition = NULL;
    referenceInfo->type = NULL;

    return True;
}

//************************************************************************
/*!
\brief Free a BAG_REFERENCE_SYSTEM structure.

\param referenceInfo
    \li The structure to be freed.
*/
//************************************************************************
void freeReferenceSystemInfo(BAG_REFERENCE_SYSTEM *referenceInfo)
{
    if (referenceInfo == NULL)
        return;

    free(referenceInfo->definition);
    free(referenceInfo->type);
}

//************************************************************************
/*!
\brief Initialize the BAG_METADATA structure.

    The caller must call bagFreeMetadata() to ensure no memory leaks
    during cleanup.

\param metadata
    \li The structure to be initialized.
\return
    \li 0 on success, a bagError if an error occurs.
*/
//************************************************************************
bagError bagInitMetadata(BAG_METADATA * metadata)
{
    if (metadata == NULL)
    {
        fprintf(stderr,"ERROR: Exception when attempting to intialize data structure. Exception message is: Null pointer\n");
        return BAG_METADTA_INVALID_HANDLE;
    }

    metadata->fileIdentifier = NULL;
    metadata->dateStamp = NULL;
    metadata->language = (u8 *)_strdup("en");
    metadata->characterSet = (u8 *)_strdup("utf8");
    metadata->hierarchyLevel = (u8 *)_strdup("dataset");
    metadata->metadataStandardName = (u8 *)_strdup("ISO 19115");
    metadata->metadataStandardVersion = (u8 *)_strdup("2003/Cor.1:2006");

    metadata->contact = (BAG_RESPONSIBLE_PARTY *)malloc(sizeof(BAG_RESPONSIBLE_PARTY));
    if (metadata->contact == NULL)
        return BAG_MEMORY_ALLOCATION_FAILED;
    if (!initResponsibleParty(metadata->contact))
        return BAG_MEMORY_ALLOCATION_FAILED;

    metadata->spatialRepresentationInfo = (BAG_SPATIAL_REPRESENTATION *)malloc(sizeof(BAG_SPATIAL_REPRESENTATION));
    if (metadata->spatialRepresentationInfo == NULL)
        return BAG_MEMORY_ALLOCATION_FAILED;
    if (!initSpatialRepresentationInfo(metadata->spatialRepresentationInfo))
        return BAG_MEMORY_ALLOCATION_FAILED;

    metadata->horizontalReferenceSystem = (BAG_REFERENCE_SYSTEM *)malloc(sizeof(BAG_REFERENCE_SYSTEM));
    if (metadata->horizontalReferenceSystem == NULL)
        return BAG_MEMORY_ALLOCATION_FAILED;
    if (!initReferenceSystemInfo(metadata->horizontalReferenceSystem))
        return BAG_MEMORY_ALLOCATION_FAILED;

    metadata->verticalReferenceSystem = (BAG_REFERENCE_SYSTEM *)malloc(sizeof(BAG_REFERENCE_SYSTEM));
    if (metadata->verticalReferenceSystem == NULL)
        return BAG_MEMORY_ALLOCATION_FAILED;
    if (!initReferenceSystemInfo(metadata->verticalReferenceSystem))
        return BAG_MEMORY_ALLOCATION_FAILED;

    metadata->identificationInfo = (BAG_IDENTIFICATION *)malloc(sizeof(BAG_IDENTIFICATION));
    if (metadata->identificationInfo == NULL)
        return BAG_MEMORY_ALLOCATION_FAILED;
    if (!initDataIdentificationInfo(metadata->identificationInfo))
        return BAG_MEMORY_ALLOCATION_FAILED;

    metadata->dataQualityInfo = (BAG_DATA_QUALITY *)malloc(sizeof(BAG_DATA_QUALITY));
    if (metadata->dataQualityInfo == NULL)
        return BAG_MEMORY_ALLOCATION_FAILED;
    if (!initDataQualityInfo(metadata->dataQualityInfo))
        return BAG_MEMORY_ALLOCATION_FAILED;

    metadata->legalConstraints = (BAG_LEGAL_CONSTRAINTS *)malloc(sizeof(BAG_LEGAL_CONSTRAINTS));
    if (metadata->legalConstraints == NULL)
        return BAG_MEMORY_ALLOCATION_FAILED;
    if (!initLegalConstraints(metadata->legalConstraints))
        return BAG_MEMORY_ALLOCATION_FAILED;

    metadata->securityConstraints = (BAG_SECURITY_CONSTRAINTS *)malloc(sizeof(BAG_SECURITY_CONSTRAINTS));
    if (metadata->securityConstraints == NULL)
        return BAG_MEMORY_ALLOCATION_FAILED;
    if (!initSecurityConstraints(metadata->securityConstraints))
        return BAG_MEMORY_ALLOCATION_FAILED;

    return BAG_SUCCESS;
}

//************************************************************************
/*!
\brief Free a BAG_METADATA structure.

\param metadata
    \li The structure to be freed.
*/
//************************************************************************
void bagFreeMetadata(BAG_METADATA * metadata)
{
    if (metadata == NULL)
        return;

    free(metadata->fileIdentifier);
    free(metadata->dateStamp);
    free(metadata->language);
    free(metadata->characterSet);
    free(metadata->hierarchyLevel);
    free(metadata->metadataStandardName);
    free(metadata->metadataStandardVersion);

    freeResponsibleParty(metadata->contact);
    free(metadata->contact);

    freeSpatialRepresentationInfo(metadata->spatialRepresentationInfo);
    free(metadata->spatialRepresentationInfo);

    freeReferenceSystemInfo(metadata->horizontalReferenceSystem);
    free(metadata->horizontalReferenceSystem);

    freeReferenceSystemInfo(metadata->verticalReferenceSystem);
    free(metadata->verticalReferenceSystem);

    freeDataIdentificationInfo(metadata->identificationInfo);
    free(metadata->identificationInfo);

    freeDataQualityInfo(metadata->dataQualityInfo);
    free(metadata->dataQualityInfo);

    freeLegalConstraints(metadata->legalConstraints);
    free(metadata->legalConstraints);

    freeSecurityConstraints(metadata->securityConstraints);
    free(metadata->securityConstraints);
}

//************************************************************************
//! Get the cell dimensions from the metadata.
/*!
\param metaData
    \li The input meta data handle.
\param nRows
    \li Modified to contain the number of rows in the metadata.
\param nCols
    \li Modified to contain the number of columns in the metadata.
\return
    \li 0 if the function is successful, non-zero if the function fails.
*/
//************************************************************************
bagError bagGetCellDims(
    BAG_METADATA *metaData,
    u32 *nRows,
    u32 *nCols
    )
{
    if (metaData == NULL)
        return BAG_METADTA_INVALID_HANDLE;

    if (metaData->spatialRepresentationInfo == NULL)
        return BAG_METADTA_NOT_INITIALIZED;

    *nRows = metaData->spatialRepresentationInfo->numberOfRows;
    *nCols = metaData->spatialRepresentationInfo->numberOfColumns;

    return 0;
}

//************************************************************************
//! Get the geographic cover of the BAG stored in the metadata.
/*!
\param metaData
    \li The handle to the metadata.
\param llLat
    \li Modified to contain southern most latitiude on success, if the function
    fails the contents are unmodified.
\param llLong
    \li Modified to contain western most longitude on success, if the function
    fails the contents are unmodified.
\param urLat
    \li Modified to contain northern most latitude on success, if the function
    fails the contents are unmodified.
\param urLong
    \li Modified to contain eastern most longitude on success, if the function
    fails the contents are unmodified.
\return
    \li 0 if the function is successfull, non-zero if the function fails.
*/
//************************************************************************
bagError bagGetGeoCover(
    BAG_METADATA *metaData,
    f64 *llLat,
    f64 *llLong,
    f64 *urLat,
    f64 *urLong)
{
    if (metaData == NULL)
        return BAG_METADTA_INVALID_HANDLE;

    if (metaData->identificationInfo == NULL)
        return BAG_METADTA_NOT_INITIALIZED;

    *llLong = metaData->identificationInfo->westBoundingLongitude;
    *urLong = metaData->identificationInfo->eastBoundingLongitude;
    *llLat = metaData->identificationInfo->southBoundingLatitude;
    *urLat = metaData->identificationInfo->northBoundingLatitude;

    return 0;
}

//************************************************************************
//! Get the projected (ground) extents of the BAG.
/*!
\param metaData
    \li The input meta data handle.
\param llx
    \li Modified to contain the lower left X coordinate in projected units.
\param lly
    \li Modified to contain the lower left Y coordinate in projected units.
\param urx
    \li Modified to contain the upper right X coordinate in projected units.
\param ury
    \li Modified to contain the upper right Y coordinate in projected units.
\return
    \li 0 if the function is successful, non-zero if the function fails.
*/
//************************************************************************
bagError bagGetProjectedCover(
    BAG_METADATA *metaData,
    f64 *llx,
    f64 *lly,
    f64 *urx,
    f64 *ury
    )
{
    if (metaData == NULL)
        return BAG_METADTA_INVALID_HANDLE;

    if (metaData->spatialRepresentationInfo == NULL)
        return BAG_METADTA_NOT_INITIALIZED;

    *llx = metaData->spatialRepresentationInfo->llCornerX;                                
    *lly = metaData->spatialRepresentationInfo->llCornerY;                                 
    *urx = metaData->spatialRepresentationInfo->urCornerX;                              
    *ury = metaData->spatialRepresentationInfo->urCornerY;      

    return 0;
}

//************************************************************************
//! Get the grid spacings from the metadata.
/*!
\param metaData
    \li The handle to the metadata.
\param dx
    \li Modified to contain the node spacing along the X axis.
\param dy
    \li Modified to contain the node spacing along the Y axis.
\return
    \li 0 if the function is successfull, non-zero if the function fails.
*/
//************************************************************************
bagError bagGetGridSpacing(
    BAG_METADATA *metaData,
    f64 *dx,
    f64 *dy
    )
{
    if (metaData == NULL)
        return BAG_METADTA_INVALID_HANDLE;

    if (metaData->spatialRepresentationInfo == NULL)
        return BAG_METADTA_NOT_INITIALIZED;

    *dy = metaData->spatialRepresentationInfo->rowResolution;
    *dx = metaData->spatialRepresentationInfo->columnResolution;

    return 0;
}

//************************************************************************
//! Get the type of uncertainty represented by the Uncertainty layer in the BAG file.
/*!
\param metaData
    \li The handle to the meta data structure.
\param uncrtType
    \li Modified to contain the type of uncertainty represented in this BAG.
    See BAG_UNCERT_TYPE in bag.h for a complete listing.
\return
    \li Error code.
*/
//************************************************************************
bagError bagGetUncertaintyType(
    BAG_METADATA *metaData,
    u32 *uncrtType
    )
{
    if (metaData == NULL)
        return BAG_METADTA_INVALID_HANDLE;

    if (metaData->identificationInfo == NULL || metaData->identificationInfo->verticalUncertaintyType == NULL)
        return BAG_METADTA_NOT_INITIALIZED;

    *uncrtType = Unknown_Uncert;

    std::string value((const char*)metaData->identificationInfo->verticalUncertaintyType);
    std::transform(value.begin(), value.end(), value.begin(), ::tolower);

    if (!strcmp(value.c_str(), "raw std dev") || !strcmp(value.c_str(), "rawstddev"))
        *uncrtType = Raw_Std_Dev;
    else if (!strcmp(value.c_str(), "cube std dev") || !strcmp(value.c_str(), "cubestddev"))
        *uncrtType = CUBE_Std_Dev;
    else if (!strcmp(value.c_str(), "product uncert") || !strcmp(value.c_str(), "productuncert"))
        *uncrtType = Product_Uncert;
    else if (!strcmp(value.c_str(), "average tpe") || !strcmp(value.c_str(), "averagetpe"))
        *uncrtType = Average_TPE;
    else if (!strcmp(value.c_str(), "historical std dev") || !strcmp(value.c_str(), "historicalstddev"))
        *uncrtType = Historical_Std_Dev;

    return 0;
}

//************************************************************************
//! Get the type of depth correction type represented by the depth layer in the BAG file.
/*!
\param metaData
    \li The handle to the meta data structure.
\param depthCorrectionType
    \li Modified to contain the type of depth correction represented in this BAG.
    See BAG_DEPTH_CORRECTION_TYPES in bag.h for a complete listing.
\return
    \li Error code.
*/
//************************************************************************
bagError bagGetDepthCorrectionType(
    BAG_METADATA *metaData,
    u32 *depthCorrectionType
    )
{
    if (metaData == NULL)
        return BAG_METADTA_INVALID_HANDLE;

    if (metaData->identificationInfo == NULL)
        return BAG_METADTA_NOT_INITIALIZED;

    *depthCorrectionType = (u32)NULL_GENERIC;

    if (metaData->identificationInfo->depthCorrectionType != NULL)
    {
        std::string value((const char*)metaData->identificationInfo->depthCorrectionType);
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);

        if (!strcmp(value.c_str(), "true depth") || !strcmp(value.c_str(), "truedepth"))
            *depthCorrectionType = True_Depth;
        else if (!strcmp(value.c_str(), "nominal at 1500 m/s") || !strcmp(value.c_str(), "nominaldepthmetre"))
            *depthCorrectionType = Nominal_Depth_Meters;
        else if (!strcmp(value.c_str(), "nominal at 4800 ft/s") || !strcmp(value.c_str(), "nominaldepthfeet"))
            *depthCorrectionType = Nominal_Depth_Feet;
        else if (!strcmp(value.c_str(), "corrected via carter's tables") || !strcmp(value.c_str(), "correctedcarters"))
            *depthCorrectionType = Corrected_Carters;
        else if (!strcmp(value.c_str(), "corrected via matthew's tables") || !strcmp(value.c_str(), "correctedmatthews"))
            *depthCorrectionType = Corrected_Matthews;
        else if (!strcmp(value.c_str(), "unknown"))
            *depthCorrectionType = Unknown_Correction;
    }
            
    return 0;
}

//************************************************************************
//! Get the type of node group type in the BAG file.
/*!
\param metaData
    \li The handle to the meta data structure.
\param nodeGroupType
    \li Modified to contain the type of node group in this BAG.
    See BAG_OPT_GROUP_TYPES in bag.h for a complete listing.
\return
    \li Error code.
*/
//************************************************************************
bagError bagGetNodeGroupType(
    BAG_METADATA *metaData,
    u8 *nodeGroupType
    )
{
    if (metaData == NULL)
        return BAG_METADTA_INVALID_HANDLE;

    if (metaData->identificationInfo == NULL)
        return BAG_METADTA_NOT_INITIALIZED;

    *nodeGroupType = Unknown_Solution;

    if (metaData->identificationInfo->nodeGroupType != NULL)
    {
        std::string value((const char*)metaData->identificationInfo->nodeGroupType);
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);

        if (!strcmp(value.c_str(), "cube"))
            *nodeGroupType = CUBE_Solution;
        else if (!strcmp(value.c_str(), "product"))
            *nodeGroupType = Product_Solution;
        else if (!strcmp(value.c_str(), "average"))
            *nodeGroupType = Average_TPE_Solution;
        else
            *nodeGroupType = Unknown_Solution;
    }
    
    return BAG_SUCCESS;
}

//************************************************************************
//! Get the type of elevation solution group type in the BAG file.
/*!
\param metaData
    \li The handle to the meta data structure.
\param nodeGroupType
    \li Modified to contain the type of elevation solution group in this BAG.
    See BAG_OPT_GROUP_TYPES in bag.h for a complete listing.
\return
    \li Error code.
*/
//************************************************************************
bagError bagGetElevationSolutionType(
    BAG_METADATA *metaData,
    u8 *nodeGroupType
    )
{
    if (metaData == NULL)
        return BAG_METADTA_INVALID_HANDLE;

    if (metaData->identificationInfo == NULL)
        return BAG_METADTA_NOT_INITIALIZED;

    *nodeGroupType = Unknown_Solution;

    if (metaData->identificationInfo->elevationSolutionGroupType != NULL)
    {
        std::string value((const char*)metaData->identificationInfo->elevationSolutionGroupType);
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);

        if (!strcmp(value.c_str(), "cube"))
            *nodeGroupType = CUBE_Solution;
        else if (!strcmp(value.c_str(), "product"))
            *nodeGroupType = Product_Solution;
        else if (!strcmp(value.c_str(), "average"))
            *nodeGroupType = Average_TPE_Solution;
        else
            *nodeGroupType = Unknown_Solution;
    }

    return BAG_SUCCESS;
}

//******************************************************************************
//! Retrieve the BAG's horizontal reference system. 
/*!

    The output buffer will contain either a WKT definition, or an EPSG number. 
    If the output is EPSG the buffer will be in the following format:
    "EPSG:<number>"

\param metaData
    \li The handle to the meta data structure.
\param buffer
    \li Modified to contain the reference's system definition.
\param bufferSize
    \li The size of the	definition buffer passed in.
\return
    \li On success, a value of zero is returned. On failure an error code is returned.
*/
//******************************************************************************
bagError bagGetHReferenceSystem(
    BAG_METADATA *metaData,
    char *buffer,
    u32 bufferSize
    )
{
    if (metaData == NULL)
        return BAG_METADTA_INVALID_HANDLE;

    if (metaData->horizontalReferenceSystem == NULL || metaData->horizontalReferenceSystem->type == NULL
        || metaData->horizontalReferenceSystem->definition == NULL)
        return BAG_METADTA_NOT_INITIALIZED;

    std::string defString;

    //If the code page is not WKT then...
    if (strcmp((const char*)metaData->horizontalReferenceSystem->type, "WKT") != 0)
    {
        defString = (const char*)metaData->horizontalReferenceSystem->type;
        defString += ":";
    }

    defString += (const char*)metaData->horizontalReferenceSystem->definition;

    //Make sure our string is not too large.
    if (defString.size() > bufferSize)
        defString.resize(bufferSize);

    strcpy(buffer, defString.c_str());

    return 0;
}

//******************************************************************************
//! Retrieve the BAG's vertical reference system. 
/*!

    The output buffer will contain either a WKT definition, or an EPSG number. 
    If the output is EPSG the buffer will be in the following format:
    "EPSG:<number>"

\param metaData
    \li The handle to the meta data structure.
\param buffer
    \li Modified to contain the reference's system definition.
\param bufferSize
    \li The size of the	definition buffer passed in.
\return
    \li On success, a value of zero is returned. On failure an error code is returned.
*/
//******************************************************************************
bagError bagGetVReferenceSystem(
    BAG_METADATA *metaData,
    char *buffer,
    u32 bufferSize
    )
{
    if (metaData == NULL)
        return BAG_METADTA_INVALID_HANDLE;

    if (metaData->verticalReferenceSystem == NULL || metaData->verticalReferenceSystem->type == NULL
        || metaData->verticalReferenceSystem->definition == NULL)
        return BAG_METADTA_NOT_INITIALIZED;

    std::string defString;

    //If the code page is not WKT then...
    if (strcmp((const char*)metaData->verticalReferenceSystem->type, "WKT") != 0)
    {
        defString = (const char*)metaData->verticalReferenceSystem->type;
        defString += ":";
    }

    defString += (const char*)metaData->verticalReferenceSystem->definition;

    //Make sure our string is not too large.
    if (defString.size() > bufferSize)
        defString.resize(bufferSize);

    strcpy(buffer, defString.c_str());

    return 0;
}

//************************************************************************
/*!
\brief Populate the bag definition structure from the meta data file.

\param definition
    \li The definition structure to be populated.
\param metaData
    \li The hanlde to the metadata.
\return
    \li Returns 0 if the function succeeds, non-zerof if the function fails.
*/
//************************************************************************
bagError bagInitDefinition(
    bagDef *definition,
    BAG_METADATA *metaData
    )
{
    bagError error = 0;
    f64 urx, ury;

    /* read the grid spacing */
    error = bagGetGridSpacing(metaData, &definition->nodeSpacingX, &definition->nodeSpacingY);
    if (error)
        return error;

    /* read the cell dimensions (rows and columns) */
    error = bagGetCellDims(metaData, &definition->nrows, &definition->ncols);
    if (error)
        return error;

    /* read vertical uncertainty type, if possible */
    error = bagGetUncertaintyType(metaData, &definition->uncertType);
    if (error != BAG_SUCCESS)
    {
        u8 *errstr;
        if (bagGetErrorString (error, &(errstr)) == BAG_SUCCESS)
        {
            fprintf(stderr, "Error in metadata initialization: {%s}\n", (char*)errstr);
            fflush(stderr);
        }
        return error;
    }

    /*! retrieve the optional node, elevation solution group types */
	error = bagGetNodeGroupType(metaData, &definition->nodeGroupType);
    error = bagGetElevationSolutionType(metaData, &definition->elevationSolutionGroupType);

    /* retrieve the depth correction type */
    error = bagGetDepthCorrectionType(metaData, &definition->depthCorrectionType);
    if (error == BAG_METADTA_DPTHCORR_MISSING)
	{
		/* bag made pre-addition of the depthCorrectionType */
		definition->depthCorrectionType = Unknown_Correction;
	}
	else if (error != BAG_SUCCESS)
    {
        u8 *errstr;
        if (bagGetErrorString (error, &errstr) == BAG_SUCCESS)
        {
            fprintf(stderr, "Error in metadata initialization: {%s}\n", (char*)errstr);
            fflush(stderr);
        }
        return error;
    }

    /* retrieve the horizontal reference system */
    error = bagGetHReferenceSystem(metaData, (char *) definition->referenceSystem.horizontalReference, REF_SYS_MAX_LENGTH);
    if (error)
        return error;

    /* retrieve the vertical reference system */
    error = bagGetVReferenceSystem(metaData, (char *) definition->referenceSystem.verticalReference, REF_SYS_MAX_LENGTH);
    if (error)
        return error;
    
    /* read the cover information */
    error = bagGetProjectedCover (metaData, &definition->swCornerX, &definition->swCornerY, &urx, &ury);
    if (error)
        return error;

    return 0;
}

//************************************************************************
/*!
\brief Populate the bag definition structure from the XML file.

    This function opens and validates the XML file specified by fileName
    against the ISO19139 schema.

\param data
    \li The bag data structure to be populated.
\param fileName
    \li The name of the XML file to be read.
\return
    \li Returns 0 if the function succeeds, non-zero if the function fails.
*/
//************************************************************************
bagError bagInitDefinitionFromFile(bagData *data, char *fileName)
{
    bagError error = 0;

    if (data == NULL || fileName == NULL)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    /*We need to assume that a new BAG file is being created, so set the
      correct version on the bagData so we can correctly decode the
      metadata.  */
    strcpy((char *) data->version, BAG_VERSION);

    /* Initialize the metadata from the specified xml file. */
    /* Validate the xml file to ensure it is correct. */
    data->metadataDef = (BAG_METADATA*)malloc(sizeof(BAG_METADATA));
    if (data->metadataDef == NULL)
        return BAG_MEMORY_ALLOCATION_FAILED;

    error = bagInitMetadata(data->metadataDef);
    if (error)
    {
        /* cleanup the metadata. */
        bagFreeMetadata(data->metadataDef);
        free(data->metadataDef);
        data->metadataDef = NULL;
        return error;
    }

    error = bagImportMetadataFromXmlFile((const u8*)fileName, data->metadataDef, True);
    if (error)
    {
        /* cleanup the metadata. */
        bagFreeMetadata(data->metadataDef);
        free(data->metadataDef);
        data->metadataDef = NULL;
        return error;
    }

    /* retrieve the necessary parameters */
    error = bagInitDefinition(&data->def, data->metadataDef);
    if (error)
    {
        /* cleanup the metadata. */
        bagFreeMetadata(data->metadataDef);
        free(data->metadataDef);
        data->metadataDef = NULL;
        return error;
    }

    //Populate the metadata buffer too.
    bagExportMetadataToXmlBuffer(data->metadataDef, &data->metadata);

    return 0;
}

//************************************************************************
/*!
\brief Populate the bag definition structure from the XML memory buffer.

    This function validates the XML data in buffer against the 
    ISO19139 schema.

\param data
    \li The bag data structure to be populated.
\param buffer
    \li The memory buffer containing the XML data.
\param bufferSize
    \li The size of buffer in bytes.
\return
    \li Returns 0 if the function succeeds, non-zero if the function fails.
*/
//************************************************************************
bagError bagInitDefinitionFromBuffer(bagData *data, u8 *buffer, u32 bufferSize)
{
    bagError error = 0;

    if (data == NULL || buffer == NULL)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    /*We need to assume that a new BAG file is being created, so set the
      correct version on the bagData so we can correctly decode the
      metadata.  */
    strcpy((char *) data->version, BAG_VERSION);

    /* Initialize the metadata from the specified xml file. */
    /* Validate the xml file to ensure it is correct. */
    data->metadataDef = (BAG_METADATA*)malloc(sizeof(BAG_METADATA));
    if (data->metadataDef == NULL)
        return BAG_MEMORY_ALLOCATION_FAILED;

    error = bagInitMetadata(data->metadataDef);
    if (error)
    {
        /* cleanup the metadata. */
        bagFreeMetadata(data->metadataDef);
        free(data->metadataDef);
        data->metadataDef = NULL;
        return error;
    }

    error = bagImportMetadataFromXmlBuffer(buffer, bufferSize, data->metadataDef, True);
    if (error)
    {
        /* cleanup the metadata. */
        bagFreeMetadata(data->metadataDef);
        free(data->metadataDef);
        data->metadataDef = NULL;
        return error;
    }

    /* retrieve the necessary parameters */
    error = bagInitDefinition(&data->def, data->metadataDef);
    if (error)
    {
        /* cleanup the metadata. */
        bagFreeMetadata(data->metadataDef);
        free(data->metadataDef);
        data->metadataDef = NULL;
        return error;
    }

    //Populate the metadata buffer too.
    bagExportMetadataToXmlBuffer(data->metadataDef, &data->metadata);

    return 0;
}

//************************************************************************
/*!
\brief Populate the bag definition structure from yer own metadata.

    Just a shortcut to bagInitDefinitionFromBuffer()

\param hnd
    \li bagHandle pointer to a BagHandle
\return
    \li Returns 0 if the function succeeds, non-zero if the function fails.
*/
//************************************************************************
bagError bagInitDefinitionFromBag(bagHandle hnd)
{
    bagError error = 0;
    bagData *data = NULL;

    error = bagReadXMLStream(hnd);
    if (error)
        return error;

    data = bagGetDataPointer(hnd);

    //Now that the metadata buffer has been read, lets populate our metadata.
    //No need to validate, we assume that all BAGs have valid metadata.
    data->metadataDef = (BAG_METADATA*)malloc(sizeof(BAG_METADATA));
    if (data->metadataDef == NULL)
        return BAG_MEMORY_ALLOCATION_FAILED;

    error = bagInitMetadata(data->metadataDef);
    if (error)
        return error;

    error = bagImportMetadataFromXmlBuffer(data->metadata, (u32)strlen((const char *)data->metadata), data->metadataDef, False);
    if (error)
        return error;

    /* retrieve the necessary parameters */
    return bagInitDefinition(&data->def, data->metadataDef);
}