/********************************************************************
 *
 * Module Name : bag_xml.c
 *
 * Authors/Date : Tue Sep  6 13:07:49 2005
 *     Brian Calder      (CCOM/JHC)
 *     Jim Case          (CCOM/JHC)
 *     Rick Brennan      (CCOM/JHC)
 *     Bill Lamey        (CARIS Ltd)
 *     Mark Paton        (IVS Ltd)
 *     Shannon Byrne     (SAIC, Newport)
 *     Webb McDonald     (SAIC, Newport)
 *     Dave Fabre        (NAVOCEANO)
 *     Jan Depner        (NAVOCEANO)
 *     Wade Ladner       (NAVOCEANO)
 *     Barry Gallagher   (NOAA HSTP)
 *     Friedhelm Moggert (7Cs GmbH)
 *     Shep Smith        (NOAA)
 *     Jack Riley        (NOAA HSTP)
 *
 * Initial concepts developed during The Open Navigation Surface Workshop
 *   attended by above contributors meeting at CCOM/JHC.
 *
 * Description : 
 *                  This is the API interface to the ons_xml library.
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 * who when      what
 * --- ----      ----
 *
 * Classification : 
 *  Unclassified
 *
 * Distribution :
 * 
 * Usage And Licensing :
 *
 * References :
 *     Bathymetric Attributed Grid Format Specification Document
 *     http://hdf.ncsa.uiuc.edu/HDF5/
 * 
 ********************************************************************/

#include "bag.h"
#include "ons_xml.h"


bagMetaData bufferMetaData = NULL;
char lastBuffer[XML_METADATA_MAX_LENGTH] = "";


bagError bagFreeXMLMeta ()
{
    if (bufferMetaData != NULL)
    {
        bagFreeMetadata(bufferMetaData);
        bufferMetaData = NULL;
    }

    /* terminate the support. */
    bagTermMetadata();
    
    return BAG_SUCCESS;
}

/* Routine:     bagInitDefinition
 * Purpose:     Populate the bag definition structure from the meta data file.
 * Inputs:      *definition     The definition structure to be populated.
 *          metaData        The hanlde to the metadata.
 * Outputs:     Returns 0 if the function succeeds, non-zerof if the function fails.
 * Comment:
 */
 

bagError bagInitDefinition(
    bagDef *definition,
    bagMetaData metaData
    )
{
    bagError error = 0;
    f64 urx, ury, longOfProjCenter;
	f64 scaleFactAtEq, scaleAtCenterLine, scaleAtProjOrigin, heightOfPersPoint; /* dhf */
    char projectionId[256];
    char ellipId[256];
    Coordinate_Type coordType;


    /* read the grid spacing */
    error = bagGetGridSpacing(metaData, &definition->nodeSpacingX, &definition->nodeSpacingY);
    if (error)
        return error;

    /* read the cell dimensions (rows and columns) */
    error = bagGetCellDims(metaData, &definition->nrows, &definition->ncols);
    if (error)
        return error;

    /* read the projection information */
    error = bagGetProjectionParams(metaData, projectionId, 256,
        &definition->geoParameters.zone, &definition->geoParameters.std_parallel_1,
        &definition->geoParameters.central_meridian, &definition->geoParameters.origin_latitude, 
        &definition->geoParameters.false_easting, &definition->geoParameters.false_northing,
        &scaleFactAtEq, /* dhf - used if Mercator coord sys */
        &heightOfPersPoint, /* dhf - for space oblique mercator (not in bag) */
        &longOfProjCenter,  /* dhf - for oblique conformal conic (not in bag) */
        &definition->geoParameters.std_parallel_2, 
        &scaleAtCenterLine, /* dhf - used if oblique mercator (not bag implemented) */
        &definition->geoParameters.longitude_down_from_pole,
        &scaleAtProjOrigin /* used for polar stereographic & transverse mercator */
	);

    if (error)
        return error;

    /* read vertical uncertainty type, if possible */
    error = bagGetUncertantyType(metaData, &definition->uncertType);
    if (error != BAG_SUCCESS)
    {
        char *errstr;
        if (bagGetErrorString (error, &errstr) == BAG_SUCCESS)
        {
            fprintf(stderr, "Error in metadata initialization: {%s}\n", errstr);
            fflush(stderr);
        }
        return error;
    }

    /* retrieve the horizontal datum */
    error = bagGetHorizDatum(metaData, ellipId, 256);
    if (error)
        return error;

    /*convert the projection id to a supported type */
    coordType = bagCoordsys(projectionId);

    definition->coordSys = coordType;
    
    /* read the cover information */
    /* if (coordType == Geodetic) */
/*         error = bagGetGeoCover (metaData, &definition->swCornerX,  */
/*                                 &definition->swCornerY, &urx, &ury); */
/*     else */
    error = bagGetProjectedCover (metaData, &definition->swCornerX, 
                                  &definition->swCornerY, &urx, &ury);
    if (error)
        return error;

    /* dhf */
    /* scaleFactAtEq - for mercator */
    /* scaleAtCenterLine - for oblique mercator (not supported) */
    /* scaleAtProjOrigin - for polar stereographic & transverse mercator */
    if ( coordType == Mercator )
    	definition->geoParameters.scale_factor = scaleFactAtEq;
    if ( coordType == Transverse_Mercator || coordType == Polar_Stereo )
    	definition->geoParameters.scale_factor = scaleAtProjOrigin;

    /* convert the ellipsoid type */
    definition->geoParameters.datum = bagDatumID(ellipId);

    return error;
}

/* Routine:     bagInitDefinitionFromFile
 * Purpose:     Populate the bag definition structure from the XML file.
 * Inputs:      *data     The bag data structure to be populated.
 *          *fileName The name of the XML file to be read.
 * Outputs:     Returns 0 if the function succeeds, non-zero if the function fails.
 * Comment: This function opens and validates the XML file specified by fileName
 *          against the ISO19139 schema.
 */

bagError bagInitDefinitionFromFile(bagData *data, char *fileName)
{
    bagMetaData metaData = NULL;
    bagError error = 0;
    u32 bufferLen;
    if (data == NULL || fileName == NULL)
    {
        return error;
    }

    /* initialize the metadata module */
    error = bagInitMetadata();
    if (error)
        return error;

    /* open and validate the XML file. */
    metaData = bagValidateMetadataFile(fileName, &error);

    if (error)
        return error;

    /* retrieve the necessary parameters */
    error = bagInitDefinition(&data->def, metaData);
    if (error)
    {
        /* free the meta data */
        bagFreeMetadata(metaData);

        /* terminate the support. */
        bagTermMetadata();

        return error;
    }

    /* attach the XML stream to the structure */
    bufferLen = XML_METADATA_MAX_LENGTH;
    data->metadata = malloc(sizeof(char) * bufferLen);
    error = bagGetXMLBuffer(metaData, data->metadata, &bufferLen);

    /* free the meta data */
    bagFreeMetadata(metaData);

    /* terminate the support. */
    bagTermMetadata();

    return error;
}

/* Routine:     bagInitDefinitionFromBuffer
 * Purpose:     Populate the bag definition structure from the XML memory buffer.
 * Inputs:      *data     The bag data structure to be populated.
 *          *buffer   The memory buffer containing the XML data.
 *          bufferSize  The size of buffer in bytes.
 * Outputs:     Returns 0 if the function succeeds, non-zero if the function fails.
 * Comment: This function validates the XML data in buffer against the 
 *          ISO19139 schema.
 */
bagError bagInitDefinitionFromBuffer(bagData *data, u8 *buffer, u32 bufferSize)
{
    bagError error = BAG_SUCCESS;
    u32 bufferLen = 0;
    u32 chng      = 0;

    if (data == NULL || buffer == NULL)
        return error;

    /* check the size of the input buffer */
    if (bufferSize >= XML_METADATA_MAX_LENGTH)
        return BAG_METADTA_BUFFER_EXCEEDED;

    chng = (bufferMetaData == NULL || strlen(lastBuffer) == 0 ||
            strncmp (lastBuffer, buffer, XML_METADATA_MAX_LENGTH) != 0);

    /* we will recycle lastBuffer and bufferMetaData if buffer has not changed */
    if (chng)
    {
        if (bufferMetaData != NULL)
        {
            bagFreeMetadata(bufferMetaData); 
            bufferMetaData = NULL;
        }

        strncpy (lastBuffer, buffer, bufferSize);

        /* need to make sure that the buffer is NULL terminated. */
        lastBuffer[bufferSize] = '\0';
        
    }

    /* initialize the metadata module */
    bagInitMetadata();

    if (chng)
        /* open and validate the XML file. */
        bufferMetaData = bagValidateMetadataBuffer(lastBuffer, bufferSize, &error);

    if (error)
        return error;

    /* retrieve the necessary parameters */
    error = bagInitDefinition(&data->def, bufferMetaData);
    if (error)
    {
        /* free the meta data */
        bagFreeMetadata(bufferMetaData);
        bagFreeXMLMeta();
        return error;
    }

    if (chng)
    {
        void *tmp;
        /* attach the XML stream to the structure */
        bufferLen = XML_METADATA_MAX_LENGTH;
        tmp = realloc(data->metadata, sizeof(char) * bufferLen);
        if (tmp != NULL)
            data->metadata = tmp;
        else
            return BAG_MEMORY_ALLOCATION_FAILED;
        strncpy(data->metadata, buffer, bufferLen);
        //error = bagGetXMLBuffer(bufferMetaData, data->metadata, &bufferLen);
    }

    return error;
}
/* Routine:     bagInitDefinitionFromBag
 * Purpose:     Populate the bag definition structure from yer own metadata.
 * Inputs:      bagHandle pointer to a BagHandle
 * Outputs:     Returns 0 if the function succeeds, non-zero if the function fails.
 * Comment:   Just a shortcut to bagInitDefinitionFromBuffer()
 */
bagError bagInitDefinitionFromBag(bagHandle hnd)
{
    bagError stat;
    bagData *pData = NULL;

    if ((stat = bagReadXMLStream(hnd)) != BAG_SUCCESS)
        return stat;
    else
    {
        pData = bagGetDataPointer(hnd);
        return bagInitDefinitionFromBuffer(pData, pData->metadata, strlen(pData->metadata));
    }
}
