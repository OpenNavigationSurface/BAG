#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "hdf5.h"
#include "bag.h"

#ifndef __MYBAG_H__
#define __MYBAG_H__

#ifndef MAX_STR
#define MAX_STR 255
#endif

#define RANK 2
#define TRACKING_LIST_BLOCK_SIZE       10

/* Path names for BAG entities */
#define ROOT_PATH          "/BAG_root"
#define METADATA_PATH      ROOT_PATH"/metadata"
#define ELEVATION_PATH     ROOT_PATH"/elevation"
#define UNCERTAINTY_PATH   ROOT_PATH"/uncertainty"
#define TRACKING_LIST_PATH ROOT_PATH"/tracking_list"

/* Names for BAG Attributes */
#define BAG_VERSION_NAME     "Bag Version"                /* Name for version attribute, value set in bag.h */
#define	MIN_ELEVATION_NAME   "Minimum Elevation Value"    /* Name for min elevation attribute, value stored in bagData */
#define	MAX_ELEVATION_NAME   "Maximum Elevation Value"    /* Name for max elevation attribute, value stored in bagData */
#define	MIN_UNCERTAINTY_NAME "Minimum Uncertainty Value"  /* Name for min uncertainty attribute, value stored in bagData */
#define	MAX_UNCERTAINTY_NAME "Maximum Uncertainty Value"  /* Name for max uncertainty attribute, value stored in bagData */
#define TRACKING_LIST_LENGTH_NAME "Tracking List Length"  /* Name for the tracking list length attribute */

#define check_hdf_status()  if (status < 0) return BAG_HDF_INTERNAL_ERROR


typedef struct _t_bagHandle {

    bagData bag;

    /* contiguous 1D arrays for HDF I/O of the dataset surfaces */
    f32    *uncertaintyArray;
    f32    *elevationArray;

    /* scratch buffer for the ONSCryptoBlock, see bagFileOpen() */
    u8     *cryptoBlock;
    u32     cryptoID;  /*sigID*/

    u8      filename[MAX_STR];

    /* HDF structs identifiers */
    hid_t   file_id;
    hid_t   bagGroupID; 

} BagHandle;

enum bagAttrTypes {
    BAG_ATTR_F32         = 1,
    BAG_ATTR_F64         = 2,
    BAG_ATTR_U8          = 3,
    BAG_ATTR_S32         = 4,
    BAG_ATTR_U32         = 5,
    BAG_ATTR_CS1         = 6
};

enum READ_WRITE_BAG {
    READ_BAG             = 0,
    WRITE_BAG            = 1,
    DISABLE_STRIP_MINING = 2
};

enum READ_TRACK_MODE {
    READ_TRACK_RC     = 0,
    READ_TRACK_SERIES = 1,
    READ_TRACK_CODE   = 2
};

/* local function prototypes */
bagError bagCreateAttribute (bagHandle hnd, hid_t lid, u8 *attr_name, u32 max, u32 type);
bagError bagWriteAttribute  (bagHandle hnd, hid_t lid, u8 *attr_name, void *value);
bagError bagReadAttribute   (bagHandle hnd, hid_t lid, u8 *attr_name, void *value);
bagError bagAlignAttribute  (bagHandle hnd, hid_t lid, u8 *attr_name, void *value, u32 read_or_write);
bagError bagReadSurfaceDims (bagHandle hnd, hsize_t *max_dims);
bagError bagAlignXMLStream  (bagHandle hnd, s32 read_or_write);
bagError bagAlignRow        (bagHandle hnd, u32 row, u32 start_col,u32 end_col, s32 type, s32 read_or_write, void *data);
bagError bagAlignRegion     (bagHandle hnd, u32 start_row, u32 start_col, u32 end_row, u32 end_col, s32 type, s32 read_or_write, hid_t xfer);
bagError bagAlignNode       (bagHandle hnd, u32 row, u32 col, s32 type, void *data, s32 read_or_write);
bagError bagUpdateMinMax    (bagHandle hnd, u32 type);
bagError bagReadTrackingList(bagHandle hnd, u16 mode, u32 inp1, u32 inp2, bagTrackingItem **items, u32 *rtn_len);
bagError bagFillPos         (bagHandle hnd, u32 r1, u32 c1 , u32 r2, u32 c2, f64 **x, f64 **y);
bagError bagSortTrackingList(bagHandle hnd, u16 mode);
s32 bagCompareTrackIndices  (const void *a, const void *b);
s32 bagCompareTrackNodes    (const void *a, const void *b);
s32 bagCompareTrackCodes    (const void *a, const void *b);
#endif
