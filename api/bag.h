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

/*typedef s32 bagError;*/

#include "bag_config.h"
#include "bag_metadata.h"
#include "bag_errors.h"

/* This typedef must match the hsize_t type defined in HDF5 */
typedef unsigned long long HDF_size_t;

/* This typedef must match the hid_t type defined in HDF5 */
typedef int HDF_hid_t;

#define BAG_VERSION         "1.6.2"
#define BAG_VERSION_LENGTH  32              /* 32 bytes of space reserved in BAG attribute for VERSION string */
#define BAG_VER_MAJOR       1
#define BAG_VER_MINOR       6
#define BAG_VER_REVISION    2

#define XML_METADATA_MIN_LENGTH     1024    /* Encoded XML string expected to be at least this long to be valid */
#define XML_METADATA_MAX_LENGTH     1000000
#define DEFAULT_KEY_LEN             1024    /* taken from onscrypto.c */
#define BAG_DEFAULT_COMPRESSION     1
#define BAG_OPT_SURFACE_LIMIT       14      /* The maximum number of optional surfaces in a single BAG */
#define BAG_SURFACE_CORRECTOR_LIMIT 10      /* The maximum number of datum correctors per bagVerticalCorrector */
#define REF_SYS_MAX_LENGTH          2048    /* The maximum length of the reference system definition string */


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

/* Definitions for NULL values */
#define BAG_NULL_ELEVATION      1000000  // 1e6
#define BAG_NULL_UNCERTAINTY    1000000
#define BAG_NULL_STD_DEV        1000000
#define BAG_NULL_GENERIC	    1000000
#define BAG_NULL_VARRES_INDEX   0xFFFFFFFF

/* Define convenience data structure for BAG geographic definitions */
enum BAG_COORDINATES {

        BAG_COORDINATES_GEOGRAPHIC = 1,               /* values in XY array will be degrees on the earth */
        BAG_COORDINATES_PROJECTED  = 2                /* values in XY array will distance on projection  */
};
    
enum BAG_COORD_UNITS {
                                       
        BAG_COORD_UNITS_DEGREES =  1,                 /* values in XY array are geographic in degrees    */
        BAG_COORD_UNITS_METERS  =  2                  /* values in XY array are projected in meters      */
};

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


/* Reference system definition */
typedef struct t_bagReferenceSystem
{
    u8  horizontalReference[REF_SYS_MAX_LENGTH];    /* horizontal reference system definition */
    u8  verticalReference[REF_SYS_MAX_LENGTH];      /* vertical reference system definition */
} bagReferenceSystem;

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

typedef struct _t_bag_definition
{
    u32    nrows;                                     /* number of rows of data contained in the arrays               */
    u32    ncols;                                     /* number of columns of data contained in the arrays            */
    f64    nodeSpacingX;                              /* node spacing in x dimension in units defined by coord system */
    f64    nodeSpacingY;                              /* node spacing in y dimension in units defined by coord system */
    f64    swCornerX;                                 /* X coordinate of SW corner of BAG in BAG_COORDINATES          */
    f64    swCornerY;                                 /* Y coordinate of SW corner of BAG in BAG_COORDINATES          */
    u16    trackingID;                                /* index of the current metadata lineage of tracking list edits */
    u32    uncertType;                                /* The type of Uncertainty encoded in this BAG.                 */
    u32	   depthCorrectionType;	                      /* The type of depth correction */
    u8	   nodeGroupType;    	                      /* The type of optional node group */
    u8	   elevationSolutionGroupType;	              /* The type of optional elevation solution group */
    u8     surfaceCorrectionTopography;               /* The type of topography of the surface correction opt dataset  */
    bagReferenceSystem referenceSystem;               /* The spatial reference system information.                     */
} bagDef;

/* Structure to hold an optional dataset being loaded into the bag */
typedef struct _t_bag_data_opt
{
    u32    nrows;                                 /* number of rows of data contained in the arrays               */
    u32    ncols;                                 /* number of columns of data contained in the arrays            */
    f32    **data;                                /* 2D array of values for each node						      */
    f32      min;								  /* Minimum value in the opt dataset						      */
    f32      max;								  /* Maximum value in the opt dataset    					      */
	HDF_hid_t    datatype;						  /* HDF5 datatype identifier									  */
} bagDataOpt;

typedef struct _t_bag_data
{
    bagDef   def;                                     /* Geospatial definitions                                       */
    u8       version[BAG_VERSION_LENGTH];             /* Mapped from HDF file, defines BAG version of current file    */
    u8      *metadata;                                /* Mapped from XML metadata                                     */
    BAG_METADATA *metadataDef;                        /* The BAG metadata definition.                                 */
    f32    **elevation;                               /* 2D array of Mandatory elevation values for each node         */
    f32      min_elevation;                           /* Minimum elevation value in the elevation dataset             */
    f32      max_elevation;                           /* Maximum elevation value in the elevation dataset             */
    f32    **uncertainty;                             /* 2D array of Mandatory uncertainty values for each node       */
    f32      min_uncertainty;                         /* Minimum elevation value in the elevation dataset             */
    f32      max_uncertainty;                         /* Maximum elevation value in the elevation dataset             */
    bagDataOpt opt[BAG_OPT_SURFACE_LIMIT];            /* Optional Surface Dataset info                                */
    bagTrackingItem *tracking_list;                   /* Tracking list array                                          */
    u8       compressionLevel;                        /* The requested compression level for surface datasets         */
    u32      chunkSize;                               /* The chunk size for disk I/O access of surface datasets       */
} bagData;

typedef struct _t_bag_vorigin
{
    f64    nodeSpacingX; /* node spacing in x dimension in units defined by coord system */ 
    f64    nodeSpacingY; /* node spacing in y dimension in units defined by coord system */
    f64    swCornerX;    /* X coordinate of SW corner of vertical corrector surface in BAG_COORDINATES   */
    f64    swCornerY;    /* Y coordinate of SW corner of vertical corrector surface in BAG_COORDINATES */
}  bagVerticalCorrectorDef;

typedef struct _t_bag_vdatum
{
    f64 x;
    f64 y;
    f32 z[BAG_SURFACE_CORRECTOR_LIMIT];

} bagVerticalCorrector;

typedef struct _t_bag_vnode
{
    f32 z[BAG_SURFACE_CORRECTOR_LIMIT];

} bagVerticalCorrectorNode;


typedef struct _t_bag_optNodeGroup
{
    f32 hyp_strength;
    u32 num_hypotheses;

} bagOptNodeGroup;


typedef struct _t_bag_optElevationSolutionGroup
{
    f32 shoal_elevation;
    f32 stddev;
    u32 num_soundings;

} bagOptElevationSolutionGroup;

typedef struct _t_bag_varResMetadataGroup
{
	u32 index;
	u32 dimensions_x;   /*!< Number of nodes in easting */
    u32 dimensions_y;   /*!< Number of nodes in northing */
	f32 resolution_x;   /*!< Node spacing in easting */
    f32 resolution_y;   /*!< Node spacing in northing */
    f32 sw_corner_x;    /*!< Offset east from SW corner of surrounding low-res cell to SW-most node */
    f32 sw_corner_y;    /*!< Offset north from SW corner of surrounding low-res cell to SW-most node */
} bagVarResMetadataGroup;

typedef struct _t_bag_varResRefinementGroup
{
	f32 depth;
	f32 depth_uncrt;
} bagVarResRefinementGroup;

typedef struct _t_bag_varResNodeGroup
{
	f32 hyp_strength;
	u32 num_hypotheses;
	u32 n_samples;
} bagVarResNodeGroup;

typedef struct _t_bag_varResTrackingList
{
    u32 row;            /* location of the low-resolution node of the BAG that was modified      */
    u32 col;
    u32 sub_row;        /*!< Row within the refined grid that was modified       */
    u32 sub_col;        /*!< Column within the refined grid that was modified    */
    f32 depth;          /* original depth before this change                     */
    f32 uncertainty;    /* original uncertainty before this change               */
    u8  track_code;     /* reason code indicating why the modification was made  */
    u16 list_series;    /* index number indicating the item in the metadata that */
                        /* describes the modifications                           */
} bagVarResTrackingItem;

/* The type of Uncertainty encoded in this BAG. */
enum BAG_UNCERT_TYPES
{
    Unknown_Uncert      = 0, /* "Unknown" - The uncertainty layer is an unknown type. */
    Raw_Std_Dev         = 1, /* "Raw Std Dev" - Raw standard deviation of soundings the contributed to the node */
    CUBE_Std_Dev        = 2, /* "CUBE Std Dev" - Standard deviation of soundings captured by a CUBE hypothesis (i.e., CUBE's standard output of uncertainty) */
    Product_Uncert      = 3, /* "Product Uncert" - NOAA standard product uncertainty V1.0 (a blend of CUBE uncertainty and other measures). */
    Historical_Std_Dev  = 4, /* "Historical Std Dev" - Estimated standard deviation based on historical/archive data. */
    Average_TPE         = 5  /* "Average Total Propagated Error" -  Average of all of the contributing sounding TPE's within the node */
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

enum BAG_OPT_GROUP_TYPES
{
	Unknown_Solution = 0, /* None, Node and Elevation Solution Groups unused */
	CUBE_Solution,	      /* CUBE contributed the elevation solution */
    Product_Solution,     /* "Product Uncert" - NOAA standard product uncertainty V1.0 (a blend of CUBE uncertainty and other measures). */
    Average_TPE_Solution, /* "Average Total Propagated Error" -  Average of all of the contributing sounding TPE's within the node */
};

/* Surface Correction dataset coordinate topography types */
enum BAG_SURFACE_CORRECTION_TOPOGRAPHY { 
    BAG_SURFACE_UNKNOWN = 0,        /* Unknown */
    BAG_SURFACE_GRID_EXTENTS,       /* Optional corrector dataset grid coordinates, spanning the required BAG surface extents */
    BAG_SURFACE_IRREGULARLY_SPACED, /* Irregularly spaced corrector values in optional corrector dataset */
};

/* ELEVATION, UNCERTAINTY are mandatory BAG datasets, the rest are optional. */
enum BAG_SURFACE_PARAMS {
    Metadata                    = 0,
    Elevation                   = 1,
    Uncertainty                 = 2,
    Num_Hypotheses              = 3,
    Average                     = 4,
    Standard_Dev                = 5,
	Nominal_Elevation           = 6,
	Surface_Correction          = 7,  /* 7 and above are assumed to be composite datasets */
	Node_Group                  = 8,
	Elevation_Solution_Group    = 9,
	VarRes_Metadata_Group       = 10,
	VarRes_Refinement_Group     = 11,
	VarRes_Node_Group           = 12,
    VarRes_Tracking_List        = 13
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
BAG_EXTERNAL bagError bagFileOpen(bagHandle *bagHandle, s32 accessMode, const u8 *fileName); 
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
BAG_EXTERNAL bagError bagFileCreate(const u8 *file_name, bagData *data, bagHandle *bag_handle);

/*! \brief bagCreateVariableResolutionLayers
 * Construct all of the optional layers required for variable resolution BAG files.
 * This automates the construction based on the size of the grids already configured
 * for the fixed resolution layer (which becomes the meta-layer for a variable
 * resolution BAG, i.e., the depth/uncertainty that non-aware readers get, and the
 * "synopsis" description of the bathymetry in the area), and therefore this should
 * only be called after the basic size of the grids have been established by a call
 * to bagFileCreate() or bagFileOpen().  The variable resolution grid can be configured
 * to contain layers to maintain auxiliary tracking information for all of the refinements
 * as well as the low-resolution synopsis grid --- this turns on the Node_Group
 * optional layer, as well as the VarRes_Node_Group optional layer.  If the file
 * is just holding the results of a grid (i.e., the final output, not an intermediate
 * product to be used for data inspection) then you probably don't need these.
 *
 * \param handle        bagHandle for the file to enhance
 * \param nRefinements  Total umber of refinement nodes expected in the grid
 * \param aux_layers    Flag: true => set up for auxiliary layers
 * \return bagError with any error code appropriate, or BAG_SUCCESS.
 */

BAG_EXTERNAL bagError bagCreateVariableResolutionLayers(bagHandle handle, u32 const nRefinements, Bool aux_layers);

/*! \brief Check whether the BAG file open contains variable resolution extensions
 * This checks the data layers present in the BAG file to see whether it has all
 * of the requirements for a variable-resolution implementation.  In addition, if
 * the data has the extended attributes (hypothesis strength, number of hypotheses,
 * sample counts, etc.) associated with an intermediate processing product, then
 * \a has_extended_data is set True.
 *
 * \param handle            BAG handle to assess
 * \param is_var_res        Flag: True if BAG has variable resolution extensions
 * \param has_extended_data Flag: True if auxiliary data is also available
 * \return BAG_SUCCESS, or an appropriate error condition
 */

BAG_EXTERNAL bagError bagCheckVariableResolution(bagHandle handle, Bool * const is_var_res, Bool * const has_extended_data);

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
BAG_EXTERNAL bagError bagFileClose(bagHandle bagHandle);
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
BAG_EXTERNAL bagError bagReadNodeLL(bagHandle bagHandle, f64 x, f64 y, s32 type, void *data);
BAG_EXTERNAL bagError bagReadNode(bagHandle bagHandle, u32 row, u32 col, s32 type, void *data);
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

BAG_EXTERNAL bagError bagReadNodePos (bagHandle bag, u32 row, u32 col, s32 type, void *data, f64 **x, f64 **y);
/* 
 *  Function : bagReadNodePos
 *
 *  Description :
 *    Same as bagreadNode, but also populates x and y with the position of the node,
 *    based on the coordinate system of the bag.
 */


 
/* TBD */
BAG_EXTERNAL bagError bagWriteNodeLL(bagHandle bagHandle, f64 x, f64 y, s32 type, void *data);

/* Description:
 *     This function writes a value to the specified node in the specified BAG for the optional dataset.
 *     The "type" argument defines which surface parameter is updated.
 *
 * Arguments:
 *
 * Return value:
 *     On success, a value of zero is returned.  On failure a value of -1 is returned.
 */

BAG_EXTERNAL bagError bagWriteNode(bagHandle bagHandle, u32 row, u32 col, s32 type, void *data);
/* Description:
 *     This function writes a value to the specified node in the specified BAG.  
 *     The "type" argument defines which surface parameter is updated.  
 *
 * Arguments:
 * 
 * Return value:
 *     On success, a value of zero is returned.  On failure a value of -1 is returned.  
 */

BAG_EXTERNAL bagError bagGetOptDatasetInfo(bagHandle *bag_handle_opt, s32 type);
BAG_EXTERNAL bagError bagFreeInfoOpt  (bagHandle);


BAG_EXTERNAL bagError bagReadCorrectorVerticalDatum  (bagHandle hnd, u32, u8 * datum);
BAG_EXTERNAL bagError bagWriteCorrectorVerticalDatum (bagHandle hnd, u32, u8 * datum);
BAG_EXTERNAL bagError bagReadCorrectedDataset(bagHandle bagHandle, u32 corrIndex, u32 surfIndex, f32 *data);
BAG_EXTERNAL bagError bagReadCorrectedRegion (bagHandle bagHandle, u32 startrow, u32 endrow, u32 startcol, u32 endcol, u32 corrIndex, u32 surfIndex, f32 *data);
BAG_EXTERNAL bagError bagReadCorrectedRow    (bagHandle bagHandle, u32 row, u32 corrIndex, u32 surfIndex, f32 *data);
BAG_EXTERNAL bagError bagReadCorrectedNode   (bagHandle bagHandle, u32 row, u32 col, u32 corrIndex, u32 surfIndex, f32 *data);

BAG_EXTERNAL bagError bagGetNumSurfaceCorrectors (bagHandle hnd_opt, u32 *num);
BAG_EXTERNAL bagError bagGetSurfaceCorrectionTopography(bagHandle hnd, u8 *type);



BAG_EXTERNAL bagError bagReadRow(bagHandle bagHandle, u32 row, u32 start_col, u32 end_col, s32 type, void *data);
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


BAG_EXTERNAL bagError bagReadRowPos (bagHandle bag, u32 row, u32 start_col, u32 end_col, s32 type, void *data, f64 **x, f64 **y);
/* 
 *  Function : bagReadRowPos
 *
 *  Description :
 *    Same as bagReadRow, but also populates x and y with the positions,
 *    based on the coordinate system of the bag. Y will just have one value of course.
 */

 


BAG_EXTERNAL bagError bagWriteRow(bagHandle bagHandle, u32 row, u32 start_col, u32 end_col, s32 type, void *data);
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

BAG_EXTERNAL bagError bagWriteDataset (bagHandle bagHandle, s32 type);
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

BAG_EXTERNAL bagError bagReadDataset  (bagHandle bag, s32 type);
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

BAG_EXTERNAL bagError bagReadDatasetPos (bagHandle bag, s32 type, f64 **x, f64 **y);
/* 
 *  Function : bagReadDatasetPos
 *
 *  Description :
 *    Same as bagReadDataset, but also populates x and y with the positions.
 */

BAG_EXTERNAL bagError bagReadRegion (bagHandle bagHandle, u32 start_row, u32 start_col, 
                               u32 end_row, u32 end_col, s32 type);
BAG_EXTERNAL bagError bagWriteRegion (bagHandle bagHandle, u32 start_row, u32 start_col, 
                                u32 end_row, u32 end_col, s32 type);
BAG_EXTERNAL bagError bagReadRegionPos (bagHandle bag, u32 start_row, u32 start_col, 
                                  u32 end_row, u32 end_col, s32 type, f64 **x, f64 **y);
/* 
 *  Function : bagReadRegionPos
 *
 *  Description :
 *    Same as bagReadRegion, but also populates x and y with the positions.
 */

/****************************************************************************************/
BAG_EXTERNAL bagError bagWriteXMLStream (bagHandle bagHandle);
/*! \brief bagWriteXMLStream stores the string at \a bagDef's metadata field into the Metadata dataset
 *
 * \param bagHandle  External reference to the private \a bagHandle object
 * \return \li On success, \a bagError is set to \a BAG_SUCCESS
 *         \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS
 */
BAG_EXTERNAL bagError bagReadXMLStream  (bagHandle bagHandle);

/*! \brief bagReadXMLStream populates the \a bagDef metadata field with a string derived from the Metadata dataset
 *
 * \param bagHandle  External reference to the private \a bagHandle object
 * \return \li On success, \a bagError is set to \a BAG_SUCCESS
 *         \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS
 */

BAG_EXTERNAL bagError bagGetGridDimensions(bagHandle hnd, u32 *rows, u32 *cols);
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
BAG_EXTERNAL bagError bagInitDefinitionFromFile(bagData *data, char *fileName);

/* Routine:     bagInitDefinitionFromBuffer
 * Purpose:     Populate the bag definition structure from the XML memory buffer.
 * Inputs:  *data     The bag data structure to be populated.
 *          *buffer   The memory buffer containing the XML data.
 *          bufferSize  The size of buffer in bytes.
 * Outputs:     Returns 0 if the function succeeds, non-zero if the function fails.
 * Comment: This function validates the XML data in buffer against the 
 *          ISO19139 schema.
 */
BAG_EXTERNAL bagError bagInitDefinitionFromBuffer(bagData *data, u8 *buffer, u32 bufferSize);

BAG_EXTERNAL bagError bagInitDefinitionFromBag(bagHandle hnd);


/* Routine:	bagComputeMessageDigest
 * Purpose:	Compute, from a BAG file, the Message Digest needed for the signature construction
 * Inputs:	*file		Name of the BAG file to digest (see comment)
 *			signatureID	Sequential signature ID --- the reference number from the corresponding metadata element
 * Outputs:	*nBytes		Set to the number of bytes generated in the Message Digest
 *			Returns pointer to the message digest byte sequence
 * Comment:	This calls through to ons_gen_digest(), and supplies the appropriate user data to make the MD
 *			compatible with the ONS Digital Security Scheme.
 */

BAG_EXTERNAL u8 *bagComputeMessageDigest(char *file, u32 signatureID, u32 *nBytes);

/* Routine:     bagSignMessageDigest
 * Purpose:     Compute, from a Message Digest and a Secret Key, the Signature sequence
 * Inputs:      *md                     Message Digest to be signed
 *                      md_len          Message Digest length in bytes
 *                      *secKey         Secret key byte sequence to use in signing
 * Outputs:     *errcode        Error code (or BAG_SUCCESS if all's OK)
 *                      Returns pointer to the signature stream on success, otherwise NULL on failure
 * Comment:     This calls through to ons_sign_digest() and does appropriate error code translation
 */

BAG_EXTERNAL u8 *bagSignMessageDigest(u8 *md, u32 mdLen, u8 *secKey, bagError *errcode);

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

BAG_EXTERNAL bagError bagReadCertification(char *file, u8 *sig, u32 nBuffer, u32 *sigID);

/* Routine:     bagWriteCertification
 * Purpose:     Write signature stream into file, appending if an ONSCrypto block doesn't exist
 * Inputs:      *file   BAG file to write signature stream into
 *                      *sig    Signature stream to write to the file
 *                      sigID   Signature Sequential ID number to write into ONSCrypto block
 * Outputs:     Returns appropriate error code, BAG_SUCCESS on successful completion.
 * Comment:     This verifies the signature is valid and intact, and then writes into the output
 *                      file indicated.  If the output file doesn't have the ONSCrypto block, one is appended.
 */

BAG_EXTERNAL bagError bagWriteCertification(char *file, u8 *sig, u32 sigID);

/* Routine:     bagVerifyCertification
 * Purpose:     Verify that a signature, held internally, is valid
 * Inputs:      *sig    The Signature to verify
 *                      *pubKey The putative SA's public key sequence
 *                      *md             The message digest of the BAG file that was signed
 *                      *mdLen  Length of the message digest in bytes
 * Outputs:     Returns True if the signature, public key and digest match; otherwise False
 * Comment:     -
 */

BAG_EXTERNAL Bool bagVerifyCertification(u8 *sig, u8 *pubKey, u8 *md, u32 mdLen);

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
 
BAG_EXTERNAL u8 *bagComputeFileSignature(char *name, u32 sigID, u8 *secKey);

/* Routine:     bagSignFile
 * Purpose:     Convenience function to sequence all of the functions to sign a file ab initio
 * Inputs:      *name   BAG file name to sign
 *                      *secKey SA's secret key
 *                      sigID   Signature Sequential ID number (cross-ref to the metadata)
 * Outputs:     Returns True on success, otherwise False
 * Comment:     This is primarily for convenience --- one call interface to do everything required
 *                      to sign a file from scratch.
 */

BAG_EXTERNAL Bool bagSignFile(char *name, u8 *secKey, u32 sigID);

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

BAG_EXTERNAL Bool bagVerifyFile(char *name, u8 *pubKey, u32 sigID);

/* Routine:     bagGenerateKeyPair
 * Purpose:     Generate an ONS asymmetric cryptography key pair
 * Inputs:      -
 * Outputs:     **pubKey        *pubKey points to the Public Key byte sequence
 *                      **secKey        *secKey points to the Secret Key byte sequence
 *                      Returns BAG_SUCCESS on success, otherwise an informative error code
 * Comment:     This generates a DSA key-pair for asymmetric cryptography (e.g., for signature schemes).
 *                      The secret key should, of course, be kept secret and not divulged.
 */

BAG_EXTERNAL bagError bagGenerateKeyPair(u8 **pubKey, u8 **secKey);

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

BAG_EXTERNAL bagError bagConvertCryptoFormat(u8 *object, bagCryptoObject objType, bagConvDir convDir, u8 **converted);


BAG_EXTERNAL bagData *bagGetDataPointer(bagHandle bag_handle);

/****************************************************************************************
 * The array functions manage private memory within the bagHandle
 * which is used to buffer data from the surface datasets.
 * The user is able to access this data from the bagData's
 * 2D **elevation and **uncertainty pointers.
 ****************************************************************************************/
BAG_EXTERNAL bagError bagAllocArray (bagHandle hnd, u32 start_row, u32 start_col, u32 end_row, u32 end_col, s32 type);
BAG_EXTERNAL bagError bagFreeArray (bagHandle hnd, s32 type);

BAG_EXTERNAL bagError bagUpdateSurface (bagHandle hnd, u32 type);
BAG_EXTERNAL bagError bagUpdateOptSurface (bagHandle hnd, u32 type);
BAG_EXTERNAL bagError bagReadMinMaxNodeGroup (bagHandle hnd,
                                        bagOptNodeGroup *minGroup, bagOptNodeGroup *maxGroup);
BAG_EXTERNAL bagError bagReadMinMaxElevationSolutionGroup (bagHandle hnd,
                                                     bagOptElevationSolutionGroup *minGroup, bagOptElevationSolutionGroup *maxGroup);
BAG_EXTERNAL bagError bagReadMinMaxVarResMetadataGroup(bagHandle hnd, bagVarResMetadataGroup *minGroup, bagVarResMetadataGroup *maxGroup);
BAG_EXTERNAL bagError bagReadMinMaxVarResRefinementGroup(bagHandle hnd, bagVarResRefinementGroup *minGroup, bagVarResRefinementGroup *maxGroup);
BAG_EXTERNAL bagError bagReadMinMaxVarResNodeGroup(bagHandle hnd, bagVarResNodeGroup *minGroup, bagVarResNodeGroup *maxGroup);

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

BAG_EXTERNAL bagError bagTrackingListLength (bagHandle bagHandle, u32 *len);
BAG_EXTERNAL bagError bagVarResTrackingListLength(bagHandle bagHandle, u32 *len);

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
BAG_EXTERNAL bagError bagReadTrackingListNode(bagHandle bagHandle, u32 row, u32 col, bagTrackingItem **items, u32 *length);
BAG_EXTERNAL bagError bagReadVarResTrackingListNode(bagHandle bagHandle, u32 row, u32 col, bagVarResTrackingItem **items, u32 *length);
BAG_EXTERNAL bagError bagReadVarResTrackingListSubnode(bagHandle bagHandle, u32 row, u32 col, u32 sub_row, u32 sub_col, bagVarResTrackingItem **items, u32 *length);

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
BAG_EXTERNAL bagError bagReadTrackingListCode(bagHandle bagHandle, u8 code, bagTrackingItem **items, u32 *length);
BAG_EXTERNAL bagError bagReadVarResTrackingListCode(bagHandle bagHandle, u8 code, bagVarResTrackingItem **items, u32 *length);

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
BAG_EXTERNAL bagError bagReadTrackingListSeries(bagHandle bagHandle, u16 index, bagTrackingItem **items, u32 *length);
BAG_EXTERNAL bagError bagReadVarResTracklingListSeries(bagHandle, u16 index, bagVarResTrackingItem **items, u32 *length);

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
BAG_EXTERNAL bagError bagReadTrackingListIndex (bagHandle bagHandle, u16 index, bagTrackingItem *item);
BAG_EXTERNAL bagError bagReadVarResTrackingListIndex(bagHandle bagHandle, u16 index, bagVarResTrackingItem *item);

/* Routine:     bagWriteTrackingListItem
 * Purpose:     Write a single bagTrackingItem into the tracking_list dataset.
 * Inputs:      bagHandle    Handle for the Bag file
 *              *item        pointer to tracking list item
 * Outputs:     bagError     Will be set if there is an error accessing the 
 *                           bagHandle, or if item is NULL
 * Comment:     BAG_SUCCESS 
 */
BAG_EXTERNAL bagError bagWriteTrackingListItem(bagHandle bagHandle, bagTrackingItem *item);
BAG_EXTERNAL bagError bagWriteVarResTrackingListItem(bagHandle bagHandle, bagVarResTrackingItem *item);


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
BAG_EXTERNAL bagError bagSortTrackingListByNode (bagHandle bagHandle);
BAG_EXTERNAL bagError bagSortVarResTrackingListByNode(bagHandle bagHandle);
BAG_EXTERNAL bagError bagSortVarResTrackingListBySubNode(bagHandle bagHandle);

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
BAG_EXTERNAL bagError bagSortTrackingListBySeries (bagHandle bagHandle);
BAG_EXTERNAL bagError bagSortVarResTrackingListBySeries(bagHandle bagHandle);
BAG_EXTERNAL bagError bagSortTrackingListByCode (bagHandle bagHandle);
BAG_EXTERNAL bagError bagSortVarResTrackingListByCode(bagHandle bagHandle);

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

BAG_EXTERNAL bagError bagGetErrorString(bagError code, u8 **error);

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
BAG_EXTERNAL bagError bagReadSurfaceDims (bagHandle hnd, HDF_size_t *max_dims);


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
BAG_EXTERNAL bagError bagCreateOptionalDataset  (bagHandle bagHandle, bagData *data,  s32 type);
BAG_EXTERNAL bagError bagCreateNodeGroup  (bagHandle hnd, bagData *opt_data);
BAG_EXTERNAL bagError bagCreateElevationSolutionGroup (bagHandle hnd, bagData *opt_data);
BAG_EXTERNAL bagError bagCreateCorrectorDataset   (bagHandle hnd, bagData *opt_data, u32 numCorrectors, u8 type);
BAG_EXTERNAL bagError bagWriteCorrectorDefinition (bagHandle hnd, bagVerticalCorrectorDef *def);
BAG_EXTERNAL bagError bagReadCorrectorDefinition  (bagHandle hnd, bagVerticalCorrectorDef *def);
BAG_EXTERNAL bagError bagGetOptDatasets(bagHandle *bag_handle, s32 *num_opt_datasets, int opt_dataset_names[BAG_OPT_SURFACE_LIMIT]);

BAG_EXTERNAL bagError bagCreateVarResMetadataGroup(bagHandle hnd, bagData *data);
BAG_EXTERNAL bagError bagCreateVarResRefinementGroup(bagHandle hnd, bagData *data, u32 const n_cells);
BAG_EXTERNAL bagError bagCreateVarResNodeGroup(bagHandle hnd, bagData *data, u32 const n_cells);
BAG_EXTERNAL bagError bagCreateVarResTrackingList(bagHandle hnd, bagData *data);

#endif
