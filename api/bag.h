/*! \file bag.h
 * \brief Declarations of Open Navigation Surface API.
 ********************************************************************
 *
 * Authors/Date : Fri Jan 27 14:41:13 2006
 *     Brian Calder      (CCOM/JHC)
 *     Rick Brennan      (CCOM/JHC)
 *     Bill Lamey        (CARIS Ltd)
 *     Mark Paton        (IVS Ltd)
 *     Shannon Byrne     (SAIC, Newport)
 *     Jim Case          (SAIC, Newport)
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
 *   This is the header file for the Bathymetric Attributed Grid (BAG) 
 *   access library.  This file defines the data structures and function 
 *   prototypes for accessing BAG files.
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

#ifndef __BAG_H__
#define __BAG_H__

/* Get the required standard C include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include "stdtypes.h"

/* Get the required HDF5 include files */
#include <hdf5.h>

#ifdef __cplusplus
extern          "C"
{
#endif


#define BAG_VERSION         "1.3.0"
#define BAG_VERSION_LENGTH  32           /* 32 bytes of space reserved in BAG attribute for VERSION string */
#define XML_METADATA_MIN_LENGTH 1024   /* Encoded XML string expected to be at least this long to be valid */
#define XML_METADATA_MAX_LENGTH 1000000
#define DEFAULT_KEY_LEN		1024  /* taken from onscrypto.c */


/* General conventions:
 * --------------------
 *    1) Elevation values are expressed in meters, with positive values being 
 *       above the specified vertical datum.  Depths below the vertical datum 
 *       are negative.
 *    2) Uncertainty values are expressed as a positive value in meters, the 
 *       sign has no meaning.
 *    3) The BAG access functions return an integer value.  A value of zero 
 *       indicates successful completion.  A value of -1 indicates that some 
 *       failure occurred within the library.  The functions BagGetErrorValue 
 *       and BagGetErrorString may be used to obtain information about the 
 *       last failure that occurred for a given BAG.
 *    4) The BAG APIs supported multi-threaded access.  However, multi-threaded 
 *       applications must have linked with the appropriate multi-threaded 
 *       versions of HDF5 and all other library dependencies.
 */

/*! Definitions for error conditions */
#define BAG_GENERAL_ERROR_BASE                    0
#define BAG_CRYPTO_ERROR_BASE                   200
#define BAG_METADATA_ERROR_BASE                 400
#define BAG_HDFV_ERROR_BASE                     600



/*! General error conditions, including success */
enum BAG_ERRORS {
    BAG_SUCCESS                                =   0, /*!< Normal, successful completion */
    BAG_BAD_FILE_IO_OPERATION                  =   1, /*!< A basic file IO operation failed */
    BAG_NO_FILE_FOUND                          =   2, /*!< Specified file name could not be found */
    BAG_NO_ACCESS_PERMISSION                   =   3, /* Used ? */
    BAG_MEMORY_ALLOCATION_FAILED               =   4, /*!< Memory allocation failed */
    BAG_INVALID_BAG_HANDLE                     =   5, /*!< bagHandle cannot be NULL */
    BAG_INVALID_FUNCTION_ARGUMENT              =   6, /*!< Inconsistency or illegal value contained in function arguments */
    BAG_INVALID_ERROR_CODE                     =   7, /*!< An undefined bagError code was encountered */
	
	BAG_CRYPTO_SIGNATURE_OK                    = 200, /*!< Signature found, and valid */
	BAG_CRYPTO_NO_SIGNATURE_FOUND              = 201, /*!< No signature found in file */
	BAG_CRYPTO_BAD_SIGNATURE_BLOCK             = 202, /*!< Signature found, but invalid */
	BAG_CRYPTO_BAD_KEY                         = 203, /*!< Internal key format is invalid */
	BAG_CRYPTO_WRONG_KEY                       = 204, /*!< Wrong key type passed */
	BAG_CRYPTO_GENERAL_ERROR                   = 205, /*!< Something else went wrong */
	BAG_CRYPTO_INTERNAL_ERROR                  = 206, /*!< Something went wrong that the library didn't expect */
  
    BAG_METADTA_NO_HOME                        = 400, /*!< BAG_HOME directory not set. */
    BAG_METADTA_SCHEMA_FILE_MISSING            = 401, /*!< Unable to locate schema file. */
    BAG_METADTA_PARSE_MEM_EXCEPTION            = 402, /*!< Unhandled exception while parsing.  Out of memory. */
    BAG_METADTA_PARSE_EXCEPTION                = 403, /*!< Unhandled exception while parsing.  Parser error. */
    BAG_METADTA_PARSE_DOM_EXCEPTION            = 404, /*!< Unhandled exception while parsing.  DOM error. */
    BAG_METADTA_PARSE_UNK_EXCEPTION            = 405, /*!< Unhandled exception while parsing.  Unknown error. */
    BAG_METADTA_PARSE_FAILED                   = 406, /*!< Unable to parse input file. */
    BAG_METADTA_PARSE_FAILED_MEM               = 407, /*!< Unable to parse specified input buffer. */
    BAG_METADTA_VALIDATE_FAILED                = 408, /*!< XML validation failed. */
    BAG_METADTA_INVALID_HANDLE                 = 409, /*!< Invalid (NULL) handle supplied to an accessor method. */
    BAG_METADTA_INIT_FAILED                    = 410, /*!< Initialization of the low level XML support system failed. */
    BAG_METADTA_NO_PROJECTION_INFO             = 411, /*!< No projection information was found in the XML supplied. */
    BAG_METADTA_INSUFFICIENT_BUFFER            = 412, /*!< The supplied buffer is not large enough to hold the extracted contents. */
    BAG_METADTA_INCOMPLETE_COVER               = 413, /*!< One or more elements of the requested cover are missing from the XML file. */
    BAG_METADTA_INVLID_DIMENSIONS              = 414, /*!< The number of dimensions is incorrect. (not equal to 2). */
    BAG_METADTA_UNCRT_MISSING                  = 415, /*!< The 'uncertaintyType' information is missing from the XML structure. */
    BAG_METADTA_BUFFER_EXCEEDED                = 416, /*!< The supplied buffer is to large to be stored in the internal array. */
	BAG_METADTA_DPTHCORR_MISSING               = 417, /*!< The 'depthCorrectionType' information is missing from the XML structure. */

    BAG_NOT_HDF5_FILE                          = 602, /*!< HDF Bag is not an HDF5 File */
    BAG_HDF_RANK_INCOMPATIBLE                  = 605, /*!< HDF Bag's rank is incompatible with expected Rank of the Datasets */
    BAG_HDF_TYPE_NOT_FOUND                     = 606, /*!< HDF Bag surface Datatype parameter not available */
    BAG_HDF_DATASPACE_CORRUPTED                = 607, /*!< HDF Dataspace for a bag surface is corrupted or could not be read */
    BAG_HDF_ACCESS_EXTENTS_ERROR               = 608, /*!< HDF Failure in request for access outside the extents of a bag surface's Dataset */
    BAG_HDF_CANNOT_WRITE_NULL_DATA             = 609, /*!< HDF Cannot write NULL or uninitialized data to Dataset */
    BAG_HDF_INTERNAL_ERROR                     = 610, /*!< HDF There was an internal HDF error detected */
    BAG_HDF_CREATE_FILE_FAILURE                = 611, /*!< HDF Unable to create new HDF Bag File */
    BAG_HDF_CREATE_DATASPACE_FAILURE           = 612, /*!< HDF Unable to create the Dataspace */
    BAG_HDF_CREATE_PROPERTY_CLASS_FAILURE      = 613, /*!< HDF Unable to create the Property class */
    BAG_HDF_SET_PROPERTY_FAILURE               = 614, /*!< HDF Unable to set value of Property class */
    BAG_HDF_TYPE_COPY_FAILURE                  = 615, /*!< HDF Failed to copy Datatype parameter for Dataset access */
    BAG_HDF_CREATE_DATASET_FAILURE             = 616, /*!< HDF Unable to create the Dataset */
    BAG_HDF_DATASET_EXTEND_FAILURE             = 617, /*!< HDF Cannot extend Dataset extents */
    BAG_HDF_CREATE_ATTRIBUTE_FAILURE           = 618, /*!< HDF Unable to create Attribute */
    BAG_HDF_CREATE_GROUP_FAILURE               = 619, /*!< HDF Unable to create Group */
    BAG_HDF_WRITE_FAILURE                      = 620, /*!< HDF Failure writing to Dataset */
    BAG_HDF_READ_FAILURE                       = 621, /*!< HDF Failure reading from Dataset */
    BAG_HDF_GROUP_CLOSE_FAILURE                = 622, /*!< HDF Failure closing Group */
    BAG_HDF_FILE_CLOSE_FAILURE                 = 623, /*!< HDF Failure closing File */
    BAG_HDF_FILE_OPEN_FAILURE                  = 624, /*!< HDF Unable to open File */
    BAG_HDF_GROUP_OPEN_FAILURE                 = 625, /*!< HDF Unable to open Group */
    BAG_HDF_ATTRIBUTE_OPEN_FAILURE		       = 626, /*!< HDF Unable to open Attribute */
    BAG_HDF_ATTRIBUTE_CLOSE_FAILURE		       = 627, /*!< HDF Failure closing Attribute */
    BAG_HDF_DATASET_CLOSE_FAILURE              = 628, /*!< HDF Failure closing Dataset */
    BAG_HDF_DATASET_OPEN_FAILURE               = 629, /*!< HDF Unable to open Dataset */
    BAG_HDF_TYPE_CREATE_FAILURE                = 630, /*!< HDF Unable to create Datatype */

};


/* Error conditions from the cryptographic library */

/* Definitions for NULL values */
#define NULL_ELEVATION      1e6
#define NULL_UNCERTAINTY    1e6
#define NULL_STD_DEV        1e6
#define NULL_GENERIC	    1e6


/* 
 * Unknown Uncertainty :
 * If there is not enough information present to perform a 
 * computation of a node's uncertainty, then this value will
 * be used to distinguish it from NULL.
 */
#define UNK_UNCERTAINTY 0


/* Define convenience data structure for BAG geographic definitions */
enum BAG_COORDINATES {

        BAG_COORDINATES_GEOGRAPHIC = 1,               /* values in XY array will be degrees on the earth */
        BAG_COORDINATES_PROJECTED  = 2                /* values in XY array will distance on projection  */
};
    
enum BAG_COORD_UNITS {
                                       
        BAG_COORD_UNITS_DEGREES =  1,                 /* values in XY array are geographic in degrees    */
        BAG_COORD_UNITS_METERS  =  2                  /* values in XY array are projected in meters      */
};
    
typedef s32 bagError;

typedef enum 
{
    bagConvertASCIIToBin = 1,
    bagConvertBinToASCII = 2
} bagConvDir;

typedef enum
{
    bagCryptoKey = 1,
    bagCryptoSignature = 2
} bagCryptoObject;

typedef enum bagDatums
{
        wgs84,
        wgs72,
        nad83
} bagDatum;

/* Coordinate Type Enumeration */
typedef enum Coordinate_Types
{
  Geodetic,
  GEOREF,
  Geocentric,
  Local_Cartesian,    
  MGRS,
  UTM,
  UPS,
  Albers_Equal_Area_Conic,
  Azimuthal_Equidistant,
  BNG,
  Bonne,
  Cassini,
  Cylindrical_Equal_Area,
  Eckert4,
  Eckert6,
  Equidistant_Cylindrical,
  Gnomonic,
  Lambert_Conformal_Conic,
  Mercator,
  Miller_Cylindrical,
  Mollweide,
  Neys,
  NZMG,
  Oblique_Mercator,
  Orthographic,
  Polar_Stereo,
  Polyconic,
  Sinusoidal,
  Stereographic,
  Transverse_Cylindrical_Equal_Area,
  Transverse_Mercator,
  Van_der_Grinten
} Coordinate_Type;


/* structure for parameters of all bag supported horizontal coord. sys.
        mercator, tm, utm, ps, ups, lambert, & geodetic. */
typedef struct t_bagProjectionParameters
{
        bagDatum datum;                               /* wgs84, wgs72, nad83, ...   */
        u8  ellipsoid[256];                           /* ellipsoid                  */
        u8  vertical_datum[256];                      /* vertical datum             */
        f64 origin_latitude;                          /* degrees                    */
        f64 central_meridian;                         /* degrees                    */
        f64 std_parallel_1;                           /* degrees                    */
        f64 std_parallel_2;                           /* degrees                    */
        f64 false_easting;                            /* meters                     */
        f64 false_northing;                           /* meters                     */
        f64 scale_factor;                             /* unitless                   */
        f64 latitude_of_true_scale;                   /* degrees                    */
        f64 longitude_down_from_pole;                 /* degrees                    */
        s32 zone;                                     /* utm zone 1-60              */
        s32 override;                                 /* utm: 0=autozone,1=use zone */
} bagProjectionParameters;

/* some basic tracking list codes */
enum bagTrackCode
{
    bagManualEdit,
    bagDesignatedSndg,
	bagRecubedSurfaces,
	bagDeleteNode
} ;

/* tracking list structure */
typedef struct t_bagTrackingItem
{
    u32 row;         /* location of the node of the BAG that was modified      */
    u32 col;
    f32 depth;       /* original depth before this change                      */
    f32 uncertainty; /* original uncertainty before this change                */
    u8  track_code;  /* reason code indicating why the modification was made   */
    u16 list_series; /* index number indicating the item in the metadata that */
                     /* describes the modifications                            */
} bagTrackingItem;

typedef struct _t_bagHandle *bagHandle;
typedef struct _t_bagHandle_opt *bagHandle_opt; /* bag handle to optional dataset */

typedef struct _t_bag_definition
{
    u32    nrows;                                     /* number of rows of data contained in the arrays               */
    u32    ncols;                                     /* number of columns of data contained in the arrays            */
    f64    nodeSpacingX;                              /* node spacing in x dimension in units defined by coord system */
    f64    nodeSpacingY;                              /* node spacing in y dimension in units defined by coord system */
    f64    swCornerX;                                 /* X coordinate of SW corner of BAG in BAG_COORDINATES          */
    f64    swCornerY;                                 /* Y coordinate of SW corner of BAG in BAG_COORDINATES          */
    Coordinate_Type coordSys;                         /* either Geodetic or Mercator Transvers_Mercator,etc */
    bagProjectionParameters geoParameters;            /* Parameters for projection information                        */
    u16    trackingID;                                /* index of the current metadata lineage of tracking list edits */
    u32    uncertType;                                /* The type of Uncertainty encoded in this BAG.                 */
    u32	   depthCorrectionType;	                      /* The type of depth correction */
    u8     surfaceCorrectionTopography;                 /* The type of topography of the surface correction opt dataset   */
} bagDef;

#define	BAG_NAME_MAX_LENGTH 256  

typedef struct _t_bag_data
{
    bagDef   def;                                     /* Geospatial definitions                                       */
    u8       version[BAG_VERSION_LENGTH];             /* Mapped from HDF file, defines BAG version of current file    */
    u8       bagName[BAG_NAME_MAX_LENGTH];            /* Mapped from XML metadata for convenient access (To Be Done)  */
    u8      *metadata;                                /* Mapped from XML metadata                                     */
    f32    **elevation;                               /* 2D array of Mandatory elevation values for each node         */
    f32      min_elevation;                           /* Minimum elevation value in the elevation dataset             */
    f32      max_elevation;                           /* Maximum elevation value in the elevation dataset             */
    f32    **uncertainty;                             /* 2D array of Mandatory uncertainty values for each node       */
    f32      min_uncertainty;                         /* Minimum elevation value in the elevation dataset             */
    f32      max_uncertainty;                         /* Maximum elevation value in the elevation dataset             */
    bagTrackingItem *tracking_list;                   /* Tracking list array                                          */
} bagData;

/* Structure to hold an optional dataset being loaded into the bag */
typedef struct _t_bag_data_opt
{
    bagDef   def;                                     /* Geospatial definitions                                       */
    u8       version[BAG_VERSION_LENGTH];             /* Mapped from HDF file, defines BAG version of current file    */
    s32		 type;									  /* BAG_SURFACE_PARAMS data type								  */
	u8       bagName[BAG_NAME_MAX_LENGTH];            /* Mapped from XML metadata for convenient access (To Be Done)  */
    u8      *metadata;                                /* Mapped from XML metadata                                     */
    f32    **opt_data;                                /* 2D array of values for each node						      */
    f32      min;									  /* Minimum value in the dataset							      */
    f32      max;									  /* Maximum value in the dataset							      */
	hid_t    datatype;								  /* HDF5 datatype identifier									  */
    f32		 datanull;								  /* value for null data										  */
	bagTrackingItem *tracking_list;                   /* Tracking list array									      */
} bagDataOpt;

/* The maximum number of datum correctors per bagVerticalCorrector */
#define BAG_SURFACE_CORRECTOR_LIMIT 10


typedef struct _t_bag_vdatum
{
    f64 x;
    f64 y;
    f32 z[BAG_SURFACE_CORRECTOR_LIMIT];

} bagVerticalCorrector;


/* The type of Uncertainty encoded in this BAG. */
enum BAG_UNCERT_TYPES
{
    Unknown_Uncert      = 0, /* "Unknown" - The uncertainty layer is an unknown type. */
    Raw_Std_Dev         = 1, /* "Raw Std Dev" - Raw standard deviation of soundings the contributed to the node */
    CUBE_Std_Dev        = 2, /* "CUBE Std Dev" - Standard deviation of soundings captured by a CUBE hypothesis (i.e., CUBE’s standard output of uncertainty) */
    Product_Uncert      = 3, /* "Product Uncert" - NOAA standard product uncertainty V1.0 (a blend of CUBE uncertainty and other measures). */
    Historical_Std_Dev  = 4  /* "Historical Std Dev" - Estimated standard deviation based on historical/archive data. */
};

enum BAG_DEPTH_CORRECTION_TYPES
{
	True_Depth				= 0, /* "True Depth" - Depth corrected for sound velocity */
	Nominal_Depth_Meters	= 1, /* "Nominal at 1500 m/s " - Depth at assumed sound velocity of 1500m/s*/
	Nominal_Depth_Feet		= 2, /* "Nominal at 4800 ft/s" - Depth at assumed sound velocity of 4800ft/s*/	
	Corrected_Carters		= 3, /* "Corrected via Carter's Tables" - Corrected depth using Carter's tables */
	Corrected_Matthews		= 4, /* "Corrected via Matthew's Tables" - Corrected depth using Matthew's tables*/
	Unknown_Correction		= 5  /* "Unknown" - Unknown depth correction type or mixture of above types */
};

/* Surface Correction dataset coordinate topography types */
enum BAG_SURFACE_CORRECTION_TOPOGRAPHY { 
    BAG_SURFACE_UNKNOWN = 0,        /* Unknown */
    BAG_SURFACE_GRID_EXTENTS,       /* Optional corrector dataset grid coordinates, spanning the required BAG surface extents */
    BAG_SURFACE_IRREGULARLY_SPACED, /* Irregularly spaced corrector values in optional corrector dataset */
};

/* ELEVATION, UNCERTAINTY are mandatory BAG datasets, the rest are optional. */
enum BAG_SURFACE_PARAMS {
    Metadata       = 0,
    Elevation      = 1,
    Uncertainty    = 2, 
    Num_Hypotheses = 3,
    Average        = 4,
    Standard_Dev   = 5,
	Nominal_Elevation = 6,
	Surface_Correction = 7
};

/* Definitions for file open access modes */
enum BAG_OPEN_MODE { 
    BAG_OPEN_READONLY   = 1,
    BAG_OPEN_READ_WRITE = 2
};

/* Bit flag definitions for file open access ID */
#define BAG_ACCESS_DEFAULT           0


/* Function prototypes */

/* bag_hdf.c */  
extern bagError bagFileOpen(bagHandle *bagHandle, s32 accessMode, const u8 *fileName); 
/* Description:
 *     This function opens a BAG file stored in HDF5.  The library supports 
 *     access for up to 32 separate BAG files at a time.
 *
 * Arguments:
 * 
 * Return value:
 *     
 */

/* bag_hdf.c */                             
extern bagError bagFileCreate(const u8 *file_name, bagData *data, bagHandle *bag_handle);
/* Description:
 *     This function opens a BAG file stored in HDF5.  The library supports 
 *     access for up to 32 separate BAG files at a time.
 *
 * Arguments:  wxm - additional argument to specify whether to create the optional
 *                   BAG datasets (optional extensions in later version).
 * 
 * Return value:
 *     On success, \a bagError is set to \a BAG_SUCCESS.                  
 *     On failure, \a bagError is set to a proper code from \a BAG_ERRORS.
 */                          

/* bag_hdf.c */  
extern bagError bagFileClose(bagHandle bagHandle);
/* Description
 *     This function closes a BAG file previously opened via bagFileOpen.
 *
 * Arguments:
 *
 * Return value:
 *    On success, \a bagError is set to \a BAG_SUCCESS.                  
 *    On failure, \a bagError is set to a proper code from \a BAG_ERRORS.
 */       


/*  bag_surfaces.c */
/* TBD */
extern bagError bagReadNodeLL(bagHandle bagHandle, f64 x, f64 y, s32 type, void *data);
extern bagError bagReadNode(bagHandle bagHandle, u32 row, u32 col, s32 type, void *data);
/* Description:
 *     This function reads one node value from the BAG specified by bagHandle.  
 *     The "type" argument specifies the surface parameter of interest.
 *            
 * Arguments:
 *
 * Return value:
 *     On success, the data value for the specified type from the specified row 
 *     and column is returned.  On failure a value of FLOAT_MAX is returned.
 */

extern bagError bagReadNodePos (bagHandle bag, u32 row, u32 col, s32 type, void *data, f64 **x, f64 **y);
/* 
 *  Function : bagReadNodePos
 *
 *  Description :
 *    Same as bagreadNode, but also populates x and y with the position of the node,
 *    based on the coordinate system of the bag.
 */


 
/* TBD */
extern bagError bagWriteNodeLL(bagHandle bagHandle, f64 x, f64 y, s32 type, void *data);
extern bagError bagWriteOptNode (bagHandle bag, bagHandle_opt bagHandle_opt, u32 row, u32 col, s32 type, void *data);
/* Description:
 *     This function writes a value to the specified node in the specified BAG for the optional dataset.
 *     The "type" argument defines which surface parameter is updated.
 *
 * Arguments:
 *
 * Return value:
 *     On success, a value of zero is returned.  On failure a value of -1 is returned.
 */

extern bagError bagWriteNode(bagHandle bagHandle, u32 row, u32 col, s32 type, void *data);
/* Description:
 *     This function writes a value to the specified node in the specified BAG.  
 *     The "type" argument defines which surface parameter is updated.  
 *
 * Arguments:
 * 
 * Return value:
 *     On success, a value of zero is returned.  On failure a value of -1 is returned.  
 */

extern bagError bagGetOptDatasetInfo(bagHandle_opt *bag_handle_opt, s32 type);

extern bagError bagReadOptRow (bagHandle bagHandle, bagHandle_opt bagHandle_opt, u32 k, u32 start_col, u32 end_col, s32 type, void *data);
extern bagError bagReadOptNode (bagHandle bagHandle, bagHandle_opt bagHandle_opt, u32 k, u32 start_col, s32 type, void *data);

extern bagError bagReadCorrectorVerticalDatum  (bagHandle hnd, bagHandle_opt hnd_opt, u32, u8 * datum);
extern bagError bagWriteCorrectorVerticalDatum (bagHandle hnd, bagHandle_opt hnd_opt, u32, u8 * datum);
extern bagError bagReadCorrectedDataset(bagHandle bagHandle, bagHandle_opt bagOptHandle, u32 corrIndex, u32 surfIndex, f32 *data);
extern bagError bagReadCorrectedRegion (bagHandle bagHandle, bagHandle_opt bagOptHandle, u32 startrow, u32 endrow, u32 startcol, u32 endcol, u32 corrIndex, u32 surfIndex, f32 *data);
extern bagError bagReadCorrectedRow    (bagHandle bagHandle, bagHandle_opt bagOptHandle, u32 row, u32 corrIndex, u32 surfIndex, f32 *data);
extern bagError bagReadCorrectedNode   (bagHandle bagHandle, bagHandle_opt bagOptHandle, u32 row, u32 col, u32 corrIndex, u32 surfIndex, f32 *data);

extern bagError bagGetNumSurfaceCorrectors (bagHandle_opt hnd_opt, u32 *num);
extern bagError bagGetSurfaceCorrectionTopography(bagHandle hnd, bagHandle_opt hnd_opt, u8 *type);



extern bagError bagReadRow(bagHandle bagHandle, u32 row, u32 start_col, u32 end_col, s32 type, void *data);
/* Description:
 *     This function reads one row of values from the BAG specified by bagHandle.  
 *     The "type" argument specifies the surface parameter of interest. The calling 
 *     application is required to manage the memory for the data array.  
 *     *data must have sufficient space allocated externally to fulfill the request.
 *
 * Arguments:
 * 
 * Return value:
 */


extern bagError bagReadRowPos (bagHandle bag, u32 row, u32 start_col, u32 end_col, s32 type, void *data, f64 **x, f64 **y);
/* 
 *  Function : bagReadRowPos
 *
 *  Description :
 *    Same as bagReadRow, but also populates x and y with the positions,
 *    based on the coordinate system of the bag. Y will just have one value of course.
 */

 
extern bagError bagWriteOptRow(bagHandle bagHandle, bagHandle_opt bagHandle_opt, u32 row, u32 start_col, u32 end_col, s32 type, void *data);
/* Description:
 *     This function writes the row of data values for the surface parameter
 *     specified by type to the specified optional dataset row for the BAG specified by bagHandle_opt.
 *     The intended usage of this function is for initial load of data into a BAG
 *     The tracking list is not updated for this operation.
 *
 * Arguments:
 *
 * Return value:
 *     On success, a value of zero is returned.  On failure a value of -1 is returned. 
 */

extern bagError bagWriteRow(bagHandle bagHandle, u32 row, u32 start_col, u32 end_col, s32 type, void *data);
/* Description:
 *     This function writes the row of data values for the surface parameter 
 *     specified by type to the specified row for the BAG specified by bagHandle.
 *     The intended usage of this function is for initial load of data into a BAG
 *     The tracking list is not updated for this operation.
 * 
 * Arguments:
 *
 * Return value:
 *     On success, a value of zero is returned.  On failure a value of -1 is returned.  
 */

extern bagError bagWriteDataset (bagHandle bagHandle, s32 type);
/* Description:
 *     This function writes an entire buffer of data to a bag surface.
 * 
 * Arguments:
 *           bagHandle - pointer to the structure which ultimately contains the bag
 *           type      - the type is an element of BAG_SURFACE_PARAMS
 *
 * Return value:
 *     On success, a value of zero is returned.  On failure a value of -1 is returned.  
 */

extern bagError bagReadDataset  (bagHandle bag, s32 type);
/* Description:
 *     This function reads an entire buffer of data from a bag surface.
 * 
 * Arguments:
 *           bagHandle - pointer to the structure which ultimately contains the bag
 *           type      - the type is an element of BAG_SURFACE_PARAMS
 *
 * Return value:
 *     On success, a value of zero is returned.  On failure a value of -1 is returned.  
 */

extern bagError bagReadDatasetPos (bagHandle bag, s32 type, f64 **x, f64 **y);
/* 
 *  Function : bagReadDatasetPos
 *
 *  Description :
 *    Same as bagReadDataset, but also populates x and y with the positions.
 */

extern bagError bagReadRegion (bagHandle bagHandle, u32 start_row, u32 start_col, 
                               u32 end_row, u32 end_col, s32 type);
extern bagError bagWriteRegion (bagHandle bagHandle, u32 start_row, u32 start_col, 
                                u32 end_row, u32 end_col, s32 type);
extern bagError bagReadOptRegion (bagHandle bagHandle, bagHandle_opt bagHandle_opt, u32 start_row, u32 start_col, 
                               u32 end_row, u32 end_col, s32 type);
extern bagError bagWriteOptRegion (bagHandle bagHandle, bagHandle_opt bagHandle_opt, u32 start_row, u32 start_col, 
                                u32 end_row, u32 end_col, s32 type);
extern bagError bagReadRegionPos (bagHandle bag, u32 start_row, u32 start_col, 
                                  u32 end_row, u32 end_col, s32 type, f64 **x, f64 **y);
/* 
 *  Function : bagReadRegionPos
 *
 *  Description :
 *    Same as bagReadRegion, but also populates x and y with the positions.
 */

/****************************************************************************************/
extern bagError bagWriteXMLStream (bagHandle bagHandle);
/*! \brief bagWriteXMLStream stores the string at \a bagDef's metadata field into the Metadata dataset
 *
 * \param bagHandle  External reference to the private \a bagHandle object
 * \return \li On success, \a bagError is set to \a BAG_SUCCESS
 *         \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS
 */
extern bagError bagReadXMLStream  (bagHandle bagHandle);

/*! \brief bagReadXMLStream populates the \a bagDef metadata field with a string derived from the Metadata dataset
 *
 * \param bagHandle  External reference to the private \a bagHandle object
 * \return \li On success, \a bagError is set to \a BAG_SUCCESS
 *         \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS
 */

extern bagError bagGetGridDimensions(bagHandle hnd, u32 *rows, u32 *cols);
/* Description:
 *     This function simply stores grid dims into slot at *rows and *cols.
 * 
 * Arguments:
 *           hnd - pointer to the structure which ultimately contains the bag
 *           *rows   - pointer where number of rows of the surfaces will be assigned
 *           *cols   - pointer where number of cols of the surfaces will be assigned
 *
 * Return value:
 *     On success, a value of zero is returned.  On failure a value of -1 is returned.  
 */


/* Routine:     bagInitDefinitionFromFile
 * Purpose:     Populate the bag definition structure from the XML file.
 * Inputs:      *data     The bag data structure to be populated.
 *          *fileName The name of the XML file to be read.
 * Outputs:     Returns 0 if the function succeeds, non-zero if the function fails.
 * Comment: This function opens and validates the XML file specified by fileName
 *          against the ISO19139 schema.
 */
extern bagError bagInitDefinitionFromFile(bagData *data, char *fileName);

/* Routine:     bagInitDefinitionFromBuffer
 * Purpose:     Populate the bag definition structure from the XML memory buffer.
 * Inputs:  *data     The bag data structure to be populated.
 *          *buffer   The memory buffer containing the XML data.
 *          bufferSize  The size of buffer in bytes.
 * Outputs:     Returns 0 if the function succeeds, non-zero if the function fails.
 * Comment: This function validates the XML data in buffer against the 
 *          ISO19139 schema.
 */
extern bagError bagInitDefinitionFromBuffer(bagData *data, u8 *buffer, u32 bufferSize);

extern bagError bagInitDefinitionFromBag(bagHandle hnd);


/* Routine:	bagComputeMessageDigest
 * Purpose:	Compute, from a BAG file, the Message Digest needed for the signature construction
 * Inputs:	*file		Name of the BAG file to digest (see comment)
 *			signatureID	Sequential signature ID --- the reference number from the corresponding metadata element
 * Outputs:	*nBytes		Set to the number of bytes generated in the Message Digest
 *			Returns pointer to the message digest byte sequence
 * Comment:	This calls through to ons_gen_digest(), and supplies the appropriate user data to make the MD
 *			compatible with the ONS Digital Security Scheme.
 */

extern u8 *bagComputeMessageDigest(char *file, u32 signatureID, u32 *nBytes);

/* Routine:     bagSignMessageDigest
 * Purpose:     Compute, from a Message Digest and a Secret Key, the Signature sequence
 * Inputs:      *md                     Message Digest to be signed
 *                      md_len          Message Digest length in bytes
 *                      *secKey         Secret key byte sequence to use in signing
 * Outputs:     *errcode        Error code (or BAG_SUCCESS if all's OK)
 *                      Returns pointer to the signature stream on success, otherwise NULL on failure
 * Comment:     This calls through to ons_sign_digest() and does appropriate error code translation
 */

extern u8 *bagSignMessageDigest(u8 *md, u32 mdLen, u8 *secKey, bagError *errcode);

/* Routine:     bagReadCertification
 * Purpose:     Read signature stream from file, if it exists
 * Inputs:      *file   BAG file to read signature stream from
 *                      *sig    Buffer space to read signature into
 *                      nBuffer Number of spaces in the *sig buffer
 * Outputs:     *sigID  The signature's Sequential ID (cross-ref to meta-data)
 *                      Returns appropriate error code, BAG_SUCCESS on successful completion.
 * Comment:     This reads the BAG file for an ONSCrypto block, if it exists.  If the
 *                      block exists, the signature is read, validated, and then returned.  If
 *                      the block doesn't exist, or doesn't validate, error codes are returned.
 */

extern bagError bagReadCertification(char *file, u8 *sig, u32 nBuffer, u32 *sigID);

/* Routine:     bagWriteCertification
 * Purpose:     Write signature stream into file, appending if an ONSCrypto block doesn't exist
 * Inputs:      *file   BAG file to write signature stream into
 *                      *sig    Signature stream to write to the file
 *                      sigID   Signature Sequential ID number to write into ONSCrypto block
 * Outputs:     Returns appropriate error code, BAG_SUCCESS on successful completion.
 * Comment:     This verifies the signature is valid and intact, and then writes into the output
 *                      file indicated.  If the output file doesn't have the ONSCrypto block, one is appended.
 */

extern bagError bagWriteCertification(char *file, u8 *sig, u32 sigID);

/* Routine:     bagVerifyCertification
 * Purpose:     Verify that a signature, held internally, is valid
 * Inputs:      *sig    The Signature to verify
 *                      *pubKey The putative SA's public key sequence
 *                      *md             The message digest of the BAG file that was signed
 *                      *mdLen  Length of the message digest in bytes
 * Outputs:     Returns True if the signature, public key and digest match; otherwise False
 * Comment:     -
 */

extern Bool bagVerifyCertification(u8 *sig, u8 *pubKey, u8 *md, u32 mdLen);

/* Routine:     bagComputeFileSignature
 * Purpose:     Convenience function to compute a signature given a file and the SA secret key
 * Inputs:      *name   BAG file name to compute signature over
 *                      sigID   Signature Sequential ID number
 *                      *secKey SA's secret key sequence
 * Outputs:     Returns pointer to signature byte sequence, or NULL on failure
 * Comment:     This is primarily for convenience, since it just sequences other calls in the
 *                      library.  This computes the MD, then the signature and the associated CRC32, and
 *                      returns it for the user.
 */
 
extern u8 *bagComputeFileSignature(char *name, u32 sigID, u8 *secKey);

/* Routine:     bagSignFile
 * Purpose:     Convenience function to sequence all of the functions to sign a file ab initio
 * Inputs:      *name   BAG file name to sign
 *                      *secKey SA's secret key
 *                      sigID   Signature Sequential ID number (cross-ref to the metadata)
 * Outputs:     Returns True on success, otherwise False
 * Comment:     This is primarily for convenience --- one call interface to do everything required
 *                      to sign a file from scratch.
 */

extern Bool bagSignFile(char *name, u8 *secKey, u32 sigID);

/* Routine:     bagVerifyFile
 * Purpose:     Convenience function to sequence all of the functions required to verify a file ab initio
 * Inputs:      *name   BAG file to check and verify signature
 *                      *pubKey SA's public key byte sequence
 *                      sigID   Signature Sequential ID number (cross-ref to metadata)
 * Outputs:     True if the signature is valid, otherwise False
 * Comment:     This is primarily for convenience --- one call interface to do everything required
 *                      to read and check the signature, compute the MD for the file, and then verify that
 *                      the MD, the signature and the Public Key all agree.
 */

extern Bool bagVerifyFile(char *name, u8 *pubKey, u32 sigID);

/* Routine:     bagGenerateKeyPair
 * Purpose:     Generate an ONS asymmetric cryptography key pair
 * Inputs:      -
 * Outputs:     **pubKey        *pubKey points to the Public Key byte sequence
 *                      **secKey        *secKey points to the Secret Key byte sequence
 *                      Returns BAG_SUCCESS on success, otherwise an informative error code
 * Comment:     This generates a DSA key-pair for asymmetric cryptography (e.g., for signature schemes).
 *                      The secret key should, of course, be kept secret and not divulged.
 */

extern bagError bagGenerateKeyPair(u8 **pubKey, u8 **secKey);

/* Routine:     bagConvertCryptoFormat
 * Purpose:     Convert representation format of cryptographic objects (bin <-> ASCII)
 * Inputs:      *object Cryptographic object to convert
 *                      objType Input object type (bagCryptoObject enum)
 *                      convDir Direction of the conversion to be attempted
 * Outputs:     **converted             *converted points to converted object
 *                      Returns BAG_SUCCESS on success, or appropriate error message on failure
 * Comment:     This acts as a switch to convert keys or signatures to and from ASCII and internal
 *                      representations.
 */

extern bagError bagConvertCryptoFormat(u8 *object, bagCryptoObject objType, bagConvDir convDir, u8 **converted);


extern Coordinate_Type bagCoordsys(char *str);
extern bagDatum        bagDatumID(char *str);

extern bagData *bagGetDataPointer(bagHandle bag_handle);
extern bagDataOpt *bagGetOptDataPointer(bagHandle_opt bag_handle_opt);

/****************************************************************************************
 * The array functions manage private memory within the bagHandle
 * which is used to buffer data from the surface datasets.
 * The user is able to access this data from the bagData's
 * 2D **elevation and **uncertainty pointers.
 ****************************************************************************************/
extern bagError bagAllocArray (bagHandle hnd, u32 start_row, u32 start_col, u32 end_row, u32 end_col, s32 type);
extern bagError bagFreeArray (bagHandle hnd, s32 type);

/****************************************************************************************
 * The array functions manage private memory within the bagHandle
 * which is used to buffer data from the surface datasets.
 * The user is able to access this data from the bagData's
 * 2D **elevation and **uncertainty pointers.
 ****************************************************************************************/
extern bagError bagAllocOptArray (bagHandle_opt hnd, u32 start_row, u32 start_col, u32 end_row, u32 end_col);
extern bagError bagFreeOptArray (bagHandle_opt hnd);

/*
 *  bagFreeXMLMeta ():
 *  
 * Ideally this would be called at the termination handler of whoever has
 * initialized the Bag library.
 */
extern bagError bagFreeXMLMeta ();

extern bagError bagUpdateSurface (bagHandle hnd, u32 type);
extern bagError bagUpdateOptSurface (bagHandle hnd, bagHandle_opt hnd_opt, u32 type);

/* 
 * Routine:     bagTrackingListLength
 * Purpose:     Read the tracking list length attribute. This is the total 
 *              length for the entire list, comprising all series of edits
 *              for this bag's surfaces.  
 * Inputs:      bagHandle    Handle for the Bag file
 *              *len         Will be assigned the length of the list.
 * Outputs:     bagError     Will be set if there is an error accessing the 
 *                           bagHandle
 * Comment:     BAG_SUCCESS 
 */

extern bagError bagTrackingListLength (bagHandle bagHandle, u32 *len);

/* 
 * Routine:     bagReadTrackingListNode
 * Purpose:     Read all tracking list items from a particular node.
 * Inputs:      bagHandle    Handle for the Bag file
 *              row          Coordinate  
 *              col          Coordinate  
 *              *items       pointer will be set to an allocated array of 
 *                           bagTrackingItems, or will be left NULL if there
 *                           are none at the node give by row/col. Pointer 
 *                           MUST be set to NULL before calling this function!
 *              *rtn_len     the length of the list will be set to the number
 *                           of elements allocated in *items
 * Outputs:     bagError     Will be set if there is an error accessing the 
 *                           bagHandle or its tracking_list dataset
 * Comment:     Caller must free the memory at items if length is greater than 0.
 *              Caller must assign items a NULL value before using this function!
 */
extern bagError bagReadTrackingListNode(bagHandle bagHandle, u32 row, u32 col, bagTrackingItem **items, u32 *length);

/****************************************************************************************
 * Routine:     bagReadTrackingListCode
 * Purpose:     Read all tracking list items from a particular node.
 * Inputs:      bagHandle    Handle for the Bag file
 *              code         bagTrackCode for accessing all instances of a
 *                           reason code of bagTrackingItem  
 *              *items       Pointer will be set to an allocated array of 
 *                           bagTrackingItems, or will be left NULL if there
 *                           are none at the node give by row/col. Pointer 
 *                           MUST be set to NULL before calling this function!
 *              *length      The length of the list will be set to the number
 *                           of elements allocated in *items
 * Outputs:     bagError     Will be set if there is an error accessing the 
 *                           bagHandle or its tracking_list dataset
 * Comment:     Caller must free the memory at items if length is greater than 0.
 *              Caller must assign items a NULL value before using this function!
 ****************************************************************************************/
extern bagError bagReadTrackingListCode(bagHandle bagHandle, u8 code, bagTrackingItem **items, u32 *length);

/* Routine:     bagReadTrackingListSeries
 * Purpose:     Read all tracking list items from a specific list series index.
 *              In case the index is not ever changing, then this function can
 *              actually be used to read all tracking list items.
 * Inputs:      bagHandle    Handle for the Bag file
 *              index        tracking list series index
 *              *items       pointer will be set to an allocated array of 
 *                           bagTrackingItems, or will left NULL if there
 *                           are none at the node give by row/col. Pointer 
 *                           MUST be set to NULL before calling this function!
 *              *length      the length of the list will be set to the number
 *                           of elements allocated in *items
 * Outputs:     bagError     Will be set if there is an error accessing the 
 *                           bagHandle or its tracking_list dataset
 * Comment:     Caller must free the memory at items if length is greater than 0.
 *              The index corresponds to a batch of tracking list updates whose
 *                   lineage will be maintained in the Bag's metadata.
 *              Caller must assign items a NULL value before using this function!
 */
extern bagError bagReadTrackingListSeries(bagHandle bagHandle, u16 index, bagTrackingItem **items, u32 *length);

/* Routine:     bagReadTrackingListIndex
 * Purpose:     Read the one list item at the index provided. 
 *              This is mostly to allow application to read out of the entire list
 *              one element at a time.
 * Inputs:      bagHandle    Handle for the Bag file
 *              index        tracking list index
 *              *items       This pointer must point to memory already allocated
 *                           by the caller for a bagTrackingItem.
 * Outputs:     bagError     Will be set if there is an error accessing the 
 *                           bagHandle or its tracking_list dataset
 * Comment:    
 */
extern bagError bagReadTrackingListIndex (bagHandle bagHandle, u16 index, bagTrackingItem *item);

/* Routine:     bagWriteTrackingListItem
 * Purpose:     Write a single bagTrackingItem into the tracking_list dataset.
 * Inputs:      bagHandle    Handle for the Bag file
 *              *item        pointer to tracking list item
 * Outputs:     bagError     Will be set if there is an error accessing the 
 *                           bagHandle, or if item is NULL
 * Comment:     BAG_SUCCESS 
 */
extern bagError bagWriteTrackingListItem(bagHandle bagHandle, bagTrackingItem *item);


/****************************************************************************************
 * Routine:     bagSortTrackingList
 * Purpose:     Read the entire tracking list into memory. This is the total 
 *              length for the entire list, comprising all series of edits
 *              for this bag's surfaces. Sort  according to row/col 
 *              combination (for spatial locality).
 * Inputs:      bagHandle    Handle for the Bag file
 * Outputs:     bagError     Will be set if there is an error accessing the 
 *                           bagHandle
 * Comment:     BAG_SUCCESS  Edits to BAG, resulting in tracking list items,
 *              could be appended somewhat in a chaotic fashion.  
 *              The user will later want them accessed usually in
 *              a logical ordering though, so these sorting routines are 
 *              offered for assistance and speed of future access.
 *
 ****************************************************************************************/
extern bagError bagSortTrackingListByNode (bagHandle bagHandle);

/****************************************************************************************
 * Routine:     bagSortTrackingList
 * Purpose:     Read the entire tracking list into memory. This is the total 
 *              length for the entire list, comprising all series of edits
 *              for this bag's surfaces. Sort  according to list_series index.
 * Inputs:      bagHandle    Handle for the Bag file
 * Outputs:     bagError     Will be set if there is an error accessing the 
 *                           bagHandle
 * Comment:     BAG_SUCCESS .    Edits to BAG, resulting in tracking list items,
 *              could be appended somewhat in a chaotic fashion.  
 *              The user will laterwant them accessed usually in
 *              a logical ordering though, so these sorting routines are 
 *              offered for assistance and speed of future access.
 *
 ****************************************************************************************/
extern bagError bagSortTrackingListBySeries (bagHandle bagHandle);
extern bagError bagSortTrackingListByCode (bagHandle bagHandle);

/* Description:
 *     This function provides a short text description for the last error that 
 *     occurred on the BAG specified by bagHandle. Memory for the text string 
 *     is maintained within the library and calling applications do not free this memory.
 *
 * Arguments:
 *     code      - error string will be assigned based on the bagError argument.
 *     error     - The address of a pointer to a char which will be updated 
 *                 to point to short text string describing error condition.
 *
 * Return value:
 *     On success the function returns BAG_SUCCESS. You can always call this
 *     function again to see what the error was from the results of the
 *     original call to bagGetErrorString.
 */

extern bagError bagGetErrorString(bagError code, u8 **error);

/*! \brief  bagReadSurfaceDims
 * Description:
 *     This function retrieves the surface dimensions from the dataspace HDF object,
 *     which is defined when the surface dataset is created. \a bagDef should have the
 *     same dimensions in \a nrows and \a ncols if the bagInitDefinition function 
 *     successfully parses the XML Metadata.  The \a *max_dims should equal the values
 *     passed from a call to \a bagGetGridDimensions.
 * 
 *  \param    hnd      pointer to the structure which ultimately contains the bag
 *  \param   *max_dims pointer to an array of HDF structures that should have the same rank as the datasets.
 *
 * \return On success, a value of zero is returned.  On failure a value of -1 is returned.  
 */
extern bagError bagReadSurfaceDims (bagHandle hnd, hsize_t *max_dims);


/*! \brief  bagCreateOptionalDataset
 * Description:
 *     This function creates optional datasets as defined in BAG_SURFACE_PARAMS
 * 
 *  \param    *file_name		pointer to the Bag file name to be written to
 *  \param	  *data				pointer to the structure containing information to create the dataset
 *	\param	  type				BAG_SURFACE_PARAMS for the dataset
 *
 * \return On success, a value of zero is returned.  On failure a value of -1 is returned.  
 */
extern bagError bagCreateOptionalDataset  (bagHandle bagHandle, bagHandle_opt *bagHandle_opt, bagDataOpt *data,  s32 type);
extern bagError bagCreateCorrectorDataset (bagHandle hnd, bagHandle_opt *hnd_opt, bagDataOpt *opt_def,
                                           u32 numCorrectors, u8 type);
extern bagError bagGetOptDatasets(bagHandle_opt *bag_handle_opt,const u8 *file_name, s32 *num_opt_datasets, 
                                  int opt_dataset_names[10]);




#ifdef __cplusplus

}
#endif

#endif
