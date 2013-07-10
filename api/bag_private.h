/*! \file bag_private.h
 * \brief Private declarations for Open Navigation Surface API.
 ********************************************************************
 *
 * Authors/Date : 
 *      Webb McDonald -- Mon Mar 27 15:50:39 2006
 *
 * Initial concepts developed during The Open Navigation Surface Workshop
 *   attended by above contributors meeting at CCOM/JHC.
 *
 * Description : 
 *   This is a private header file of internal constructs that are only
 *   needed within the library.  Outside applications should not need
 *   access to these definitions.
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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "hdf5.h"
#include "bag.h"

#ifndef __BAG_PRIVATE_H__
#define __BAG_PRIVATE_H__

#ifndef MAX_STR
#define MAX_STR 255
#endif

#define RANK 2
#define TRACKING_LIST_BLOCK_SIZE       10

/*! Path names for BAG entities */
#define ROOT_PATH          "/BAG_root"
#define METADATA_PATH      ROOT_PATH"/metadata"
#define ELEVATION_PATH     ROOT_PATH"/elevation"
#define UNCERTAINTY_PATH   ROOT_PATH"/uncertainty"
#define TRACKING_LIST_PATH ROOT_PATH"/tracking_list"

/*! Path names for optional BAG entities */
#define NOMINAL_ELEVATION_PATH  ROOT_PATH"/nominal_elevation"
#define VERT_DATUM_CORR_PATH    ROOT_PATH"/vertical_datum_corrections"
#define NUM_HYPOTHESES_PATH		ROOT_PATH"/num_hypotheses"
#define AVERAGE_PATH			ROOT_PATH"/average"
#define AVERAGE_PATH			ROOT_PATH"/average"
#define NODE_GROUP_PATH			ROOT_PATH"/node"
#define ELEVATION_SOLUTION_GROUP_PATH			ROOT_PATH"/elevation_solution"
#define STANDARD_DEV_PATH		ROOT_PATH"/standard_dev"

/*! Names for BAG Attributes */
#define BAG_VERSION_NAME     "Bag Version"                /*!< Name for version attribute, value set in bag.h */
#define	MIN_ELEVATION_NAME   "Minimum Elevation Value"    /*!< Name for min elevation attribute, value stored in bagData */
#define	MAX_ELEVATION_NAME   "Maximum Elevation Value"    /*!< Name for max elevation attribute, value stored in bagData */
#define	MIN_UNCERTAINTY_NAME "Minimum Uncertainty Value"  /*!< Name for min uncertainty attribute, value stored in bagData */
#define	MAX_UNCERTAINTY_NAME "Maximum Uncertainty Value"  /*!< Name for max uncertainty attribute, value stored in bagData */
#define TRACKING_LIST_LENGTH_NAME "Tracking List Length"  /*!< Name for the tracking list length attribute */
#define VERT_DATUM_CORR_NSX "Node Spacing X"              /*!<Name for the node spacing X attribute for vert datum set */
#define VERT_DATUM_CORR_NSY "Node Spacing Y"              /*!<Name for the node spacing Y attribute for vert datum set */
#define VERT_DATUM_CORR_SWX "SW Corner X"                 /*!<Name for the sw corner X attribute for vert datum set */
#define VERT_DATUM_CORR_SWY "SW Corner Y"                 /*!<Name for the sw corner Y attribute for vert datum set */

#define check_hdf_status()  if (status < 0) return BAG_HDF_INTERNAL_ERROR


/* Structs */
/*! \brief The internal BagHandle object is only accessed within the library 
 *
 * The BagHandle type is only used privately.  It contains essential
 * private file information for a BAG such as the HDF file descriptor ID,
 * the actual bag filename, a copy of the optional ONSCryptoBlock in case we're
 * in READ_BAG mode, and internal  memory structures for communication between
 * the user and the actual HDF BAG.
 */
typedef struct _t_bagHandle {

    bagData bag;

    /*! contiguous 1D arrays for HDF I/O of the dataset surfaces */
    f32    *uncertaintyArray;
    f32    *elevationArray;

    /*! contiguous 1D arrays for HDF I/O of the optional dataset surfaces */
    f32    *dataArray[BAG_OPT_SURFACE_LIMIT];

    /*! scratch buffer for the ONSCryptoBlock, see bagFileOpen() */
    u8     *cryptoBlock;
    u32     cryptoID;  /*sigID*/

    u8      filename[MAX_STR];

    /*! HDF structs identifiers */
    hid_t   file_id;
    hid_t   bagGroupID; 
    hid_t   opt_memspace_id[BAG_OPT_SURFACE_LIMIT],
            opt_dataset_id[BAG_OPT_SURFACE_LIMIT],
            opt_filespace_id[BAG_OPT_SURFACE_LIMIT],
            opt_datatype_id[BAG_OPT_SURFACE_LIMIT];
    hid_t   unc_memspace_id,
            trk_memspace_id,
            mta_memspace_id,
            elv_memspace_id,
            unc_dataset_id,
            trk_dataset_id,
            mta_dataset_id,
            elv_dataset_id,
            unc_filespace_id,
            trk_filespace_id,
            mta_filespace_id,
            elv_filespace_id,
            unc_datatype_id,
            trk_datatype_id,
            mta_datatype_id,
            elv_datatype_id,
            mta_cparms_id;
} BagHandle;



/* enums */

/*! \brief bagAttrTypes define the available attribute datatypes
 *
 *  The attributes are created along with the datasets in bagFileCreate().
 *  Only the BAG_ATTR_CS1, string datatype can support a variable size dataspace.
 */
enum bagAttrTypes {
    BAG_ATTR_F32         = 1, /*!< 32bit Floating point */
    BAG_ATTR_F64         = 2, /*!< 64bit Floating point */
    BAG_ATTR_U8          = 3, /*!< 8bit Unsigned char */
    BAG_ATTR_S32         = 4, /*!< 32bit Signed integer */
    BAG_ATTR_U32         = 5, /*!< 32bit Unsigned integer */
    BAG_ATTR_CS1         = 6  /*!< Character string */
};


/*! \brief READ_WRITE_BAG define modes for accessing the dataset Bag surfaces */
enum READ_WRITE_BAG {
    READ_BAG             = 0, /*!< Read from the bag dataset */
    WRITE_BAG            = 1, /*!< Write to the bag dataset */
    DISABLE_STRIP_MINING = 2  /*!< Used to efficiently access the entire dataset by setting a transfer buffer */
};

/*! \brief READ_TRACK_MODE define modes for ordering tracking list items */
enum READ_TRACK_MODE {
    READ_TRACK_RC     = 0, /*!< Row-Column mode */
    READ_TRACK_SERIES = 1, /*!< List-Series mode */
    READ_TRACK_CODE   = 2  /*!< Track-Code mode */
};



/*! private function prototypes */
bagError bagCreateAttribute (bagHandle hnd, hid_t lid, u8 *attr_name, u32 max, u32 type);
bagError bagWriteAttribute  (bagHandle hnd, hid_t lid, u8 *attr_name, void *value);
bagError bagReadAttribute   (bagHandle hnd, hid_t lid, u8 *attr_name, void *value);
bagError bagAlignAttribute  (bagHandle hnd, hid_t lid, u8 *attr_name, void *value, u32 read_or_write);
bagError bagReadSurfaceDims (bagHandle hnd, hsize_t *max_dims);
bagError bagReadOptSurfaceDims (bagHandle hnd, s32 type);
bagError bagAlignXMLStream  (bagHandle hnd, s32 read_or_write);
bagError bagAlignRow        (bagHandle hnd, u32 row, u32 start_col,u32 end_col, s32 type, s32 read_or_write, void *data);
bagError bagAlignRegion     (bagHandle hnd, u32 start_row, u32 start_col, u32 end_row, u32 end_col, s32 type, s32 read_or_write, hid_t xfer);
bagError bagAlignNode       (bagHandle hnd, u32 row, u32 col, s32 type, void *data, s32 read_or_write);
bagError bagAlignOptRow     (bagHandle hnd, u32 row, u32 start_col,u32 end_col, s32 type, s32 read_or_write, void *data);
bagError bagAlignOptRegion  (bagHandle hnd, u32 start_row, u32 start_col, u32 end_row, u32 end_col, s32 type, s32 read_or_write, hid_t xfer);
bagError bagAlignOptNode    (bagHandle hnd, u32 row, u32 col, s32 type, void *data, s32 read_or_write);
bagError bagUpdateMinMax    (bagHandle hnd, u32 type);
bagError bagReadTrackingList(bagHandle hnd, u16 mode, u32 inp1, u32 inp2, bagTrackingItem **items, u32 *rtn_len);
bagError bagFillPos         (bagHandle hnd, u32 r1, u32 c1 , u32 r2, u32 c2, f64 **x, f64 **y);
bagError bagSortTrackingList(bagHandle hnd, u16 mode);
s32 bagCompareTrackIndices  (const void *a, const void *b);
s32 bagCompareTrackNodes    (const void *a, const void *b);
s32 bagCompareTrackCodes    (const void *a, const void *b);

#endif
