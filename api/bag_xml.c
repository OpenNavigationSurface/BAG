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
 * Mike Van Duzee -- Wed Aug 3 15:48:50 2011
 *   -The bagFreeXMLMeta() function would cause a crash if called more than once.
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

#define MAX_GRIDS 20
#define MAX_NCOORD_SYS 32
#define MAX_DATUMS 3

#define DATUM_NAME(k) DATUM_NAME_LIST[k].name
#define COORD_SYS_NAME(k) COORDINATE_SYS_LIST[k].name


bagMetaData **metadataCache = NULL;
char cacheString [MAX_GRIDS] [XML_METADATA_MAX_LENGTH];
int  cacheStringInit = 1;

struct COORDINATE_SYS_TYPE
{
	char *name;
};
struct DATUM_NAME_TYPE
{
	char *name;
};

static struct COORDINATE_SYS_TYPE COORDINATE_SYS_LIST[]=
{
  {"Geodetic"},
  {"GEOREF"},
  {"Geocentric"},
  {"Local_Cartesian"},
  {"MGRS"},
  {"UTM"},
  {"UPS"},
  {"Albers_Equal_Area_Conic"},
  {"Azimuthal_Equidistant"},
  {"BNG"},
  {"Bonne"},
  {"Cassini"},
  {"Cylindrical_Equal_Area"},
  {"Eckert4"},
  {"Eckert6"},
  {"Equidistant_Cylindrical"},
  {"Gnomonic"},
  {"Lambert_Conformal_Conic"},
  {"Mercator"},
  {"Miller_Cylindrical"},
  {"Mollweide"},
  {"Neys"},
  {"NZMG"},
  {"Oblique_Mercator"},
  {"Orthographic"},
  {"Polar_Stereo"},
  {"Polyconic"},
  {"Sinusoidal"},
  {"Stereographic"},
  {"Transverse_Cylindrical_Equal_Area"},
  {"Transverse_Mercator"},
  {"Van_der_Grinten"}

};


static struct DATUM_NAME_TYPE DATUM_NAME_LIST[]=
{
  {"WGS84"},
  {"WGS72"},
  {"NAD83"}
};


/******************************************************************************/
Coordinate_Type bagCoordsys( char *str )
{
	long i;
	for(i = 0; i < MAX_NCOORD_SYS; i++)
		if ( strncmp(str, COORD_SYS_NAME(i), strlen(COORD_SYS_NAME(i))) == 0 )
			return i;
	return -1;
} /* bagCoordsys */

/******************************************************************************/
bagDatum bagDatumID( char *str )
{
	long i;

	for(i = 0; i < MAX_DATUMS; i++)
		if ( strncmp(str, DATUM_NAME(i), strlen(DATUM_NAME(i))) == 0 )
			return i;
	return -1;
} /* bagDatumID */



bagError bagFreeXMLMeta ()
{
    if (metadataCache != NULL)
    {
        u32 i;
        for (i=0; i < MAX_GRIDS; i++)
        {
            if (metadataCache[i] != NULL)
              bagFreeMetadata((bagMetaData) metadataCache[i]);
        }

        free (metadataCache);
        metadataCache = NULL;
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
    char projectionId[XML_ATTR_MAXSTR];
    char datumId[XML_ATTR_MAXSTR];
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

    error = bagGetProjectionParams(metaData, projectionId, XML_ATTR_MAXSTR,
        &definition->geoParameters.zone, &definition->geoParameters.std_parallel_1,
        &definition->geoParameters.central_meridian, &definition->geoParameters.origin_latitude, 
        &definition->geoParameters.false_easting, &definition->geoParameters.false_northing,
        &scaleFactAtEq,                   /* dhf - used if Mercator coord sys */
        &heightOfPersPoint,               /* dhf - for space oblique mercator (not in bag) */
        &longOfProjCenter,                /* dhf - for oblique conformal conic (not in bag) */
        &definition->geoParameters.std_parallel_2, 
        &scaleAtCenterLine,               /* dhf - used if oblique mercator (not bag implemented) */
        &definition->geoParameters.longitude_down_from_pole,
        &scaleAtProjOrigin                /* used for polar stereographic & transverse mercator */
	);

    if (error)
        return error;

    /* read vertical uncertainty type, if possible */
    error = bagGetUncertantyType(metaData, &definition->uncertType);
    if (error != BAG_SUCCESS)
    {
        char *errstr;
        if (bagGetErrorString (error, (u8 **)&errstr) == BAG_SUCCESS)
        {
            fprintf(stderr, "Error in metadata initialization: {%s}\n", errstr);
            fflush(stderr);
        }
        return error;
    }

	/* retrieve the depth correction type */
	error = bagGetDepthCorrectionType(metaData, &definition->depthCorrectionType);
    if (error == BAG_METADTA_DPTHCORR_MISSING)
	{
		/* bag made pre-addition of the depthCorrectionType */
		definition->depthCorrectionType = Unknown_Correction;
	}
	else if (error != BAG_SUCCESS)
    {
        char *errstr;
        if (bagGetErrorString (error, (u8 **)&errstr) == BAG_SUCCESS)
        {
            fprintf(stderr, "Error in metadata initialization: {%s}\n", errstr);
            fflush(stderr);
        }
        return error;
    }

    /* retrieve the ellipsoid */
    error = bagGetEllipsoid(metaData, (char *)definition->geoParameters.ellipsoid, XML_ATTR_MAXSTR);
    if (error)
        return error;

    /* retrieve the horizontal datum */
    error = bagGetHorizDatum(metaData, datumId, XML_ATTR_MAXSTR);
    if (error)
        return error;

    /* retrieve the vertical datum */
    error = bagGetVertDatum(metaData, (char *)definition->geoParameters.vertical_datum, XML_ATTR_MAXSTR);
    if (error)
        return error;

    /*convert the projection id to a supported type */
    coordType = bagCoordsys(projectionId);

    definition->coordSys = coordType;
    
    /* read the cover information */
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

    /* convert the datum type */
    definition->geoParameters.datum = bagDatumID(datumId);  

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
    error = bagGetXMLBuffer(metaData, (char *)data->metadata, &bufferLen);
	data->metadata = realloc(data->metadata, sizeof(char) * (bufferLen +1));
	if(data->metadata == NULL)
		return (BAG_MEMORY_ALLOCATION_FAILED);
	else
		data->metadata[bufferLen] = 0;


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
    u32 i=0;
    bagError error = BAG_SUCCESS;
    u32 bufferLen = 0;
    u32 chng      = 0;

    if (data == NULL || buffer == NULL)
        return BAG_METADTA_INSUFFICIENT_BUFFER;

    /* check the size of the input buffer */
    if (bufferSize >= XML_METADATA_MAX_LENGTH)
        return BAG_METADTA_BUFFER_EXCEEDED;

    /* either grab a previously loaded metadata buffer, or create a new one */
    if (cacheStringInit || metadataCache == NULL)
    {
        if (metadataCache == NULL)
            metadataCache = (bagMetaData **) calloc (MAX_GRIDS, sizeof(bagMetaData *));
        for (i=0; i < MAX_GRIDS; i++)
        {
            metadataCache[i] = NULL;
            strcpy (cacheString[i], "");
        }
        chng = 1;
        cacheStringInit = 0;
        i=0;
    } else {
        for (i=0; i < MAX_GRIDS; i++)
        {
            chng = (metadataCache[i] == NULL || strlen((char *)cacheString[i]) == 0);
            /* use empty slot, or we've matched */
            /* terminate loop with i set to proper buffer */
            if (chng || strncmp ((char *)cacheString[i], (char *)buffer, XML_METADATA_MAX_LENGTH) == 0)
                break;
        }
        /* if too many grids are loaded, then bump off the 0th */
        if (i >= MAX_GRIDS)
        {
            chng = 1;
            i=0;
        }
    }

    /* we will recycle cacheString and metadataCache if buffer has not changed */
    if (chng)
    {
        if (metadataCache[i] != NULL)
        {
          bagFreeMetadata ((bagMetaData) metadataCache[i]);
            metadataCache[i] = NULL;
        }

        strncpy (cacheString[i], (char *)buffer, bufferSize);

        /* need to make sure that the buffer is NULL terminated. */
        cacheString[i][bufferSize] = '\0';
        
    }

    /* initialize the metadata module */
    bagInitMetadata();

    if (chng)
    {
        /* open and validate the XML file. */
      metadataCache[i] = (bagMetaData *) bagValidateMetadataBuffer(cacheString[i], bufferSize, &error);
    }


    if (error)
        return error;

    /* retrieve the necessary parameters */
    error = bagInitDefinition(&data->def, (bagMetaData) metadataCache[i]);
    if (error)
    {
        /* free the meta data */
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
        strncpy((char *)data->metadata, (char *)buffer, bufferLen);
        /*error = bagGetXMLBuffer(metadataCache[i], data->metadata, &bufferLen);*/
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
    {
        ;
    }
    else
    {
        pData = bagGetDataPointer(hnd);
        stat = bagInitDefinitionFromBuffer(pData, pData->metadata, strlen((char *)pData->metadata));
    }
    return stat;
}
