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
 * Webb McDonald -- Thu Feb 16 16:15:04 2012
 *   -Removed the 20 grid caching since metadata is no longer validated on open.
 *
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

#define MAX_NCOORD_SYS 32
#define MAX_DATUMS 3

#define DATUM_NAME(k) DATUM_NAME_LIST[k].name
#define COORD_SYS_NAME(k) COORDINATE_SYS_LIST[k].name



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
   /* terminate the support. */
    bagTermMetadata();
    
    return BAG_SUCCESS;
}

/* Routine:     bagInitDefinition
 * Purpose:     Populate the bag definition structure from the meta data file.
 * Inputs:      *definition     The definition structure to be populated.
 *              metaData        The hanlde to the metadata.
 *              version         The version of the BAG being initialized.
 * Outputs:     Returns 0 if the function succeeds, non-zerof if the function fails.
 * Comment:
 */
bagError bagInitDefinition(
    bagDef *definition,
    bagMetaData metaData,
    const u8 *version
    )
{
    bagError error = 0;
    f64 urx, ury;

    /* read the grid spacing */
    error = bagGetGridSpacing(metaData, (char *) version, &definition->nodeSpacingX, &definition->nodeSpacingY);
    if (error)
        return error;

    /* read the cell dimensions (rows and columns) */
    error = bagGetCellDims(metaData, (char *) version, &definition->nrows, &definition->ncols);
    if (error)
        return error;

    /* read vertical uncertainty type, if possible */
    error = bagGetUncertaintyType(metaData, (char *) version, &definition->uncertType);
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
    error = bagGetDepthCorrectionType(metaData, (char *) version, &definition->depthCorrectionType);
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
    error = bagGetHReferenceSystem(metaData, (char *) version, (char *) definition->referenceSystem.horizontalReference, REF_SYS_MAX_LENGTH);
    if (error)
        return error;

    /* retrieve the vertical reference system */
    error = bagGetVReferenceSystem(metaData, (char *) version, (char *) definition->referenceSystem.verticalReference, REF_SYS_MAX_LENGTH);
    if (error)
        return error;
    
    /* read the cover information */
    error = bagGetProjectedCover (metaData, (char *) version, &definition->swCornerX, &definition->swCornerY, &urx, &ury);

    if (error)
        return error;

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

    /*We need to assume that a new BAG file is being created, so set the
      correct version on the bagData so we can correctly decode the
      metadata.  */
    strcpy((char *) data->version, BAG_VERSION);

    /* initialize the metadata module */
    error = bagInitMetadata();
    if (error)
        return error;

    /* open and validate the XML file. */
	metaData = bagGetMetadataFile(fileName, True, &error);
    if (error)
        return error;

    /* retrieve the necessary parameters */
    error = bagInitDefinition(&data->def, metaData, data->version);
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

/* Routine:     bagInitAndValidateDefinition
 * Purpose:     Populate the bag definition structure from the XML memory buffer.
 * Inputs:      *data     The bag data structure to be populated.
 *              *buffer   The memory buffer containing the XML data.
 *              bufferSize  The size of buffer in bytes.
 *              validateXML True if the xml should be validated, else false.
 * Outputs:     Returns 0 if the function succeeds, non-zero if the function fails.
 * Comment: This function validates the XML data in buffer against the 
 *          ISO19139 schema.
 */
bagError bagInitAndValidateDefinition(bagData *data, u8 *buffer, u32 bufferSize, Bool validateXML)
{
    char cacheString[XML_METADATA_MAX_LENGTH];
    bagError error = BAG_SUCCESS;
    u32 bufferLen = XML_METADATA_MAX_LENGTH-1;
    bagMetaData  locmeta;
    void *tmp;

    if (data == NULL || buffer == NULL)
        return BAG_METADTA_INSUFFICIENT_BUFFER;

    /* check the size of the input buffer */
    if (bufferSize >= XML_METADATA_MAX_LENGTH)
        return BAG_METADTA_BUFFER_EXCEEDED;
   
    /* initialize the metadata module */
    bagInitMetadata();

    strncpy (cacheString, (char *)buffer, bufferLen-1);

    /* need to make sure that the buffer is NULL terminated. */
    cacheString[bufferLen] = '\0';

    /* open and validate the XML file. */
    locmeta = (bagMetaData) bagGetMetadataBuffer(cacheString, bufferSize, validateXML, &error);
    
    if (error)
    {
        /* free the meta data */
        bagFreeXMLMeta();
        return error;
    }

    /* retrieve the necessary parameters */
    error = bagInitDefinition(&data->def, locmeta, data->version);

    /* free the meta data */
    bagFreeMetadata(locmeta);

    if (error)
    {
        /* free the meta data */
        bagFreeXMLMeta();
        return error;
    }

    
    /* attach the XML stream to the structure */
    tmp = realloc(data->metadata, sizeof(u8) * bufferLen);
    if (tmp != NULL)
        data->metadata = tmp;
    else
        return BAG_MEMORY_ALLOCATION_FAILED;
    strncpy((char *)data->metadata, (char *)buffer, bufferLen-1);
    
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
    /*We need to assume that a new BAG file is being created, so set the
      correct version on the bagData so we can correctly decode the
      metadata.  */
  strcpy ((char *) data->version, BAG_VERSION);

    return bagInitAndValidateDefinition(data, buffer, bufferSize, True);
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

    if ((stat = bagReadXMLStream(hnd)) == BAG_SUCCESS)
    {
        pData = bagGetDataPointer(hnd);
        stat = bagInitAndValidateDefinition(pData, pData->metadata, strlen((char *)pData->metadata), False);
    }
    return stat;
}
