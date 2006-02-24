/********************************************************************
 *
 * Module Name : bag_surfaces.c
 *
 * Author/Date : ONSWG, July 2005
 *
 * Description : 
 *               Surfaces are datasets.  So is the XML Metadata.
 *               There is a restriction here which requires all the
 *               proper surfaces to have the exact same extents/dimensions.
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 * who  when      what
 * ---  ----      ----
 * Webb McDonald -- Fri Jan 27 15:43:15 2006
 *   -added XY position functions to supplement reading from the surface datasets
 *   -bagFillPos function to populate arrays with corresponding position values.
 *   -allocation for reading node or rows was not correct and has been removed,
 *    the calling application should provide sufficent memory buffer to these
 *    functions.  bagReadRegion and bagReadDataset still use the internal memory
 *    buffers and pointers.
 *   -Revision to the bagUpdateMinMax function.  Makes use of the internal arrays.
 *
 * Classification : Unclassified
 *
 * References : 
 *
 ********************************************************************/


#include "bag_private.h"

/****************************************************************************************
 * 
 * Read and write indiviidual nodes of a surface
 *
 ****************************************************************************************/
bagError bagWriteNode (bagHandle bag, u32 row, u32 col, s32 type, void *data)
{
    return bagAlignNode (bag, row, col, type, data, WRITE_BAG);
}

bagError bagReadNode (bagHandle bag, u32 row, u32 col, s32 type, void *data)
{
    return bagAlignNode (bag, row, col, type, data, READ_BAG);
}

/* 
 *  Function : bagReadNodePos
 *
 *  Description :
 *    Same as bagreadNode, but also populates x and y with the position of the node,
 *    based on the coordinate system of the bag.
 */
bagError bagReadNodePos (bagHandle bag, u32 row, u32 col, s32 type, void *data, f64 **x, f64 **y)
{
    bagError status;
    
    if ((status = bagFillPos (bag, row, col, row, col, x, y)) != BAG_SUCCESS)
        return status;
    return bagAlignNode (bag, row, col, type, data, READ_BAG);
}

/****************************************************************************************
 *
 * This is only slightly different from bagAlignRegion. 
 *  -The out of range conditions are based on a point, not a region
 *  -The dataspaces are just 1dimension
 *  -Uses element select, instead of hyperslab
 *
 ****************************************************************************************/
bagError bagAlignNode (bagHandle bagHandle, u32 row, u32 col, s32 type, void *data, s32 read_or_write)
{
    herr_t      status;
    
    hsize_t     snode[1] = {1};
    hssize_t	offset[1][2];
    hid_t       memspace_id, 
                datatype_id,
                dataset_id,
                filespace_id;
    u8         *dataset_name;

    if (bagHandle == NULL)
        return BAG_INVALID_BAG_HANDLE;

    /* some error checking on surfaces extents */
    if (row < 0 || row >= bagHandle->bag.def.nrows || col >= bagHandle->bag.def.ncols || col < 0)
    {
        fprintf(stderr, "Fail to access out of bounds row/col = %d/%d out of possible size: %d/%d. Aborting\n",
                row, col, bagHandle->bag.def.nrows, bagHandle->bag.def.ncols);
        fflush(stderr);
        return BAG_HDF_ACCESS_EXTENTS_ERROR;
    }

    /*
     * Depending on the type:
     *    1) set the datatype_id to the correct unit for 
     *       this kind of surface.
     *    2) alloc data if necessary
     *    3) set the pathname within the Bag to the
     *       dataset for the desired surface
     */
    switch (type)
    {
    case Elevation:
        datatype_id = H5Tcopy(H5T_NATIVE_FLOAT);
        if (data == NULL && read_or_write == WRITE_BAG)
            return BAG_HDF_CANNOT_WRITE_NULL_DATA;
        dataset_name = ELEVATION_PATH;
        break;
    case Uncertainty:
        datatype_id = H5Tcopy(H5T_NATIVE_FLOAT);
        if (data == NULL && read_or_write == WRITE_BAG)
            return BAG_HDF_CANNOT_WRITE_NULL_DATA;
        dataset_name = UNCERTAINTY_PATH;
        break;
    default:
        return BAG_HDF_TYPE_NOT_FOUND;
        break;
    }
    
    if (data == NULL)
        return  BAG_INVALID_FUNCTION_ARGUMENT;

    if (datatype_id < 0)
        return BAG_HDF_TYPE_COPY_FAILURE;

    /* Open an existing dataset. */
    dataset_id = H5Dopen(bagHandle->file_id, dataset_name);
    if (dataset_id < 0)
        return BAG_HDF_DATASET_OPEN_FAILURE; 

    /* Obtain file space  */
    filespace_id = H5Dget_space(dataset_id);
    if (filespace_id < 0)
    {
        H5Dclose (dataset_id);
        H5Tclose (datatype_id);
        return BAG_HDF_DATASPACE_CORRUPTED;
    }

    /* Create space in memory */
    memspace_id = H5Screate_simple( 1, snode, NULL);
    if (memspace_id < 0)
    {
        H5Sclose(filespace_id);
        H5Dclose (dataset_id);
        H5Tclose (datatype_id);
        return BAG_HDF_DATASPACE_CORRUPTED;
    }
    
    /* Set coordinates to read  */
    offset[0][0] = row;  
    offset[0][1] = col;

    /* Select grid cell within dataset file space. */
    status = H5Sselect_elements (filespace_id, H5S_SELECT_SET, 1, (const hssize_t **)offset);
    check_hdf_status();

    /*  perform read_or_write on element */
    if (read_or_write == READ_BAG)
        status = H5Dread (dataset_id, datatype_id, memspace_id, filespace_id, 
                          H5P_DEFAULT, data);
    else if (read_or_write == WRITE_BAG)
        status = H5Dwrite (dataset_id, datatype_id, memspace_id, filespace_id, 
                           H5P_DEFAULT, data);
    else
        ; /*  error? */
    check_hdf_status();

    /* close these things */
    status = H5Sclose (memspace_id);
    check_hdf_status();
    status = H5Sclose (filespace_id);
    check_hdf_status();
    status = H5Tclose (datatype_id);
    check_hdf_status();
    status = H5Dclose (dataset_id);
    check_hdf_status();

    if (status < 0)
        return BAG_HDF_INTERNAL_ERROR;
    else
        return BAG_SUCCESS;
}

bagError bagWriteRow (bagHandle bagHandle, u32 k, u32 start_col, u32 end_col, s32 type, void *data)
{
    return bagAlignRow (bagHandle, k, start_col, end_col, type, WRITE_BAG, data);
}

bagError bagReadRow (bagHandle bagHandle, u32 k, u32 start_col, u32 end_col, s32 type, void *data)
{
    return bagAlignRow (bagHandle, k, start_col, end_col, type, READ_BAG, data);
}


/* 
 *  Function : bagReadRowPos
 *
 *  Description :
 *    Same as bagReadRow, but also populates x and y with the positions,
 *    based on the coordinate system of the bag. Y will just have one value of course.
 */
bagError bagReadRowPos (bagHandle bag, u32 row, u32 start_col, u32 end_col, s32 type, 
                        void *data, f64 **x, f64 **y)
{
    bagError status;
    
    if ((status = bagFillPos (bag, row, start_col, row, end_col, x, y)) != BAG_SUCCESS)
        return status;
    return bagAlignRow (bag, row, start_col, end_col, type, READ_BAG, data);
}

/****************************************************************************************
 *
 * 
 *
 ****************************************************************************************/
bagError bagAlignRow (bagHandle bagHandle, u32 row, u32 start_col, 
                      u32 end_col, s32 type, s32 read_or_write, void *data)
{
    herr_t      status;

    /* hyperslab selection parameters */
    hsize_t	    count[10];
    hssize_t	offset[10];
    hid_t       memspace_id, 
                datatype_id,
                dataset_id,
                filespace_id;

    u8         *dataset_name;

    if (bagHandle == NULL)
        return BAG_INVALID_BAG_HANDLE;

    if (start_col < 0 || end_col < 0 || end_col >= bagHandle->bag.def.ncols ||
        row < 0 || row >= bagHandle->bag.def.nrows ||
        start_col > end_col)
    {
        fprintf(stderr, "Internal error, bad parameters given to access surface extents! Aborting...\n");
        fprintf(stderr, "\tCannot access region, %d-%d / %d-%d, with surface extents 0-%d / 0-%d\n",
                row, start_col, row, end_col, bagHandle->bag.def.nrows, bagHandle->bag.def.ncols);
        fflush(stderr);
        return BAG_HDF_ACCESS_EXTENTS_ERROR;
    }

    /*
     * Depending on the type:
     *    1) set the datatype_id to the correct unit for 
     *       this kind of surface.
     *    2) set the pathname within the Bag to the
     *       dataset for the desired surface
     *    3) set the buffers with bagallocarray
     *    4) point data to the correct
     *       surface within the BagDef structure
     */
    switch (type)
    {
    case Elevation:
        datatype_id = H5Tcopy(H5T_NATIVE_FLOAT);
        dataset_name = ELEVATION_PATH;
        if (data == NULL && read_or_write == WRITE_BAG)
            return BAG_HDF_CANNOT_WRITE_NULL_DATA;
        break;
    case Uncertainty:
        datatype_id = H5Tcopy(H5T_NATIVE_FLOAT);
        dataset_name = UNCERTAINTY_PATH;
        if (data == NULL && read_or_write == WRITE_BAG)
            return BAG_HDF_CANNOT_WRITE_NULL_DATA;
        break;
    default:
        return BAG_HDF_TYPE_NOT_FOUND;
        break;
    }

    if (data == NULL)
        return  BAG_INVALID_FUNCTION_ARGUMENT;


    /* Open an existing dataset. */
    dataset_id = H5Dopen(bagHandle->file_id, dataset_name);
    if (dataset_id < 0)
        return BAG_HDF_DATASET_OPEN_FAILURE; 

    /* define the hyperslab parameters */
    count[0] = 1;
    count[1] = (end_col - start_col) + 1;
    offset[0] = row;
    offset[1] = start_col;
    
    /* Prepare the dataspaces */
    memspace_id = H5Screate_simple(RANK, count, NULL);
    filespace_id = H5Dget_space(dataset_id);
    if (memspace_id < 0 || filespace_id < 0)
    {
        H5Tclose (datatype_id);
        H5Dclose (dataset_id);
        return BAG_HDF_DATASPACE_CORRUPTED;
    }

    H5Sselect_hyperslab (filespace_id, H5S_SELECT_SET, offset, NULL, count, NULL);

    
    /* perform read_or_write on hyperslab */
    if (read_or_write == READ_BAG)
        status = H5Dread (dataset_id, datatype_id, memspace_id, filespace_id, 
                          H5P_DEFAULT, data);
    else if (read_or_write == WRITE_BAG)
        status = H5Dwrite (dataset_id, datatype_id, memspace_id, filespace_id, 
                           H5P_DEFAULT, data);
    else
        ; /* error? */
    check_hdf_status();

    /* did what we came to do, now close up */
    status = H5Dclose (dataset_id);
    check_hdf_status();
    status = H5Sclose (memspace_id);
    check_hdf_status();
    status = H5Sclose (filespace_id);
    check_hdf_status();
    status = H5Tclose (datatype_id);
    check_hdf_status();

    if (status < 0)
        return BAG_HDF_INTERNAL_ERROR;
    else
        return BAG_SUCCESS;
}

bagError bagWriteDataset (bagHandle bagHandle, s32 type)
{
    return bagAlignRegion (bagHandle, 0, 0, bagHandle->bag.def.nrows - 1, 
                           bagHandle->bag.def.ncols - 1, type, WRITE_BAG, DISABLE_STRIP_MINING);
}

bagError bagReadDataset (bagHandle bagHandle, s32 type)
{
    return bagAlignRegion (bagHandle, 0, 0, bagHandle->bag.def.nrows - 1, 
                           bagHandle->bag.def.ncols - 1, type, READ_BAG, DISABLE_STRIP_MINING);
}

/* 
 *  Function : bagReadDatasetPos
 *
 *  Description :
 *    Same as bagReadDataset, but also populates x and y with the positions.
 */
bagError bagReadDatasetPos (bagHandle bagHandle, s32 type, f64 **x, f64 **y)
{
    bagError status;
    
    if ((status = bagFillPos (bagHandle, 0, 0, bagHandle->bag.def.nrows - 1, 
                              bagHandle->bag.def.ncols - 1, x, y)) != BAG_SUCCESS)
        return status;
    return bagAlignRegion (bagHandle, 0, 0, bagHandle->bag.def.nrows - 1, 
                           bagHandle->bag.def.ncols - 1, type, READ_BAG, DISABLE_STRIP_MINING);
}

bagError bagReadRegion (bagHandle bagHandle, u32 start_row, u32 start_col, u32 end_row, u32 end_col, s32 type)
{
    return bagAlignRegion (bagHandle, start_row, start_col,  end_row,  end_col, type, READ_BAG, H5P_DEFAULT);
}

bagError bagWriteRegion (bagHandle bagHandle, u32 start_row, u32 start_col, u32 end_row, u32 end_col, s32 type)
{
    return bagAlignRegion (bagHandle, start_row, start_col, end_row, end_col, type, WRITE_BAG, H5P_DEFAULT);
}

/* 
 *  Function : bagReadRegionPos
 *
 *  Description :
 *    Same as bagReadRegion, but also populates x and y with the positions.
 */
bagError bagReadRegionPos (bagHandle bag, u32 start_row, u32 start_col, 
                           u32 end_row, u32 end_col, s32 type, f64 **x, f64 **y)
{
    bagError status;
    
    if ((status = bagFillPos (bag, start_row, start_col, end_row, end_col, x, y)) != BAG_SUCCESS)
        return status;
    return bagAlignRegion (bag, start_row, start_col, end_row, end_col, type, WRITE_BAG, H5P_DEFAULT);
}

/****************************************************************************************
 *
 * I made a choice so far in this function - notice that void *data is just a temporary
 * memory is either already allocated to the arrays in bagHandle bagHandle,
 * or gets allocated here for a read.  It is up to the caller to free this memory later!
 *
 ****************************************************************************************/
bagError bagAlignRegion (bagHandle bagHandle, u32 start_row, u32 start_col, 
                    u32 end_row, u32 end_col, s32 type, s32 read_or_write, hid_t xfer)
{
    herr_t      status;

    /* hyperslab selection parameters */
    hsize_t	    count[10];
    hssize_t	offset[10];
    hid_t       memspace_id, 
                datatype_id,
                dataset_id,
                filespace_id;

    /* xfer params */
    hssize_t    size;               /* Temp buffer size */
    hid_t       xfer_plist;
    size_t      type_size;

    void       *data;
    u8         *dataset_name;


    if (bagHandle == NULL)
        return BAG_INVALID_BAG_HANDLE;

    if (start_col < 0 || end_col < 0 || end_col >= bagHandle->bag.def.ncols ||
        start_row < 0 || end_row < 0 || end_row >= bagHandle->bag.def.nrows ||
        start_row > end_row || 
        start_col > end_col)
    {
        fprintf(stderr, "Internal error, bad parameters given to access surface extents! Aborting...\n");
        fprintf(stderr, "\tCannot access region, %d-%d / %d-%d, with surface extents 0-%d / 0-%d\n",
                start_row, start_col, end_row, end_col, bagHandle->bag.def.nrows, bagHandle->bag.def.ncols);
        fflush(stderr);
        return BAG_HDF_ACCESS_EXTENTS_ERROR;
    }


    /*
     * Depending on the type:
     *    1) set the datatype_id to the correct unit for 
     *       this kind of surface.
     *    2) set the pathname within the Bag to the
     *       dataset for the desired surface
     *    3) set the buffers with bagallocarray
     *    4) point data to the correct
     *       surface within the BagDef structure
     */
    switch (type)
    {
    case Elevation:
        datatype_id = H5Tcopy(H5T_NATIVE_FLOAT);
        dataset_name = ELEVATION_PATH;
        if (read_or_write == READ_BAG)
        {
            if (bagAllocArray ( bagHandle,  start_row,  start_col, 
                                end_row,  end_col,  type) != BAG_SUCCESS)
                data = NULL;
            else
                data = bagHandle->elevationArray;
        }
        else
            data = bagHandle->elevationArray;
        break;
    case Uncertainty:
        datatype_id = H5Tcopy(H5T_NATIVE_FLOAT);
        dataset_name = UNCERTAINTY_PATH;
        if (read_or_write == READ_BAG)
        {
            if (bagAllocArray ( bagHandle,  start_row,  start_col, 
                                end_row,  end_col,  type) != BAG_SUCCESS)
                data = NULL;
            else
                data = bagHandle->uncertaintyArray;
        }
        else
            data = bagHandle->uncertaintyArray;

        break;
    default:
        return BAG_HDF_TYPE_NOT_FOUND;
        break;
    }

    if (data == NULL)
        return BAG_MEMORY_ALLOCATION_FAILED;

    /* Open an existing dataset. */
    dataset_id = H5Dopen(bagHandle->file_id, dataset_name);
    if (dataset_id < 0)
        return BAG_HDF_DATASET_OPEN_FAILURE; 

    /* define the hyperslab parameters */
    count[0] = (end_row - start_row) + 1;
    count[1] = (end_col - start_col) + 1;
    offset[0] = start_row;
    offset[1] = start_col;
    
    /* Prepare the dataspaces */
    memspace_id = H5Screate_simple(RANK, count, NULL);
    filespace_id = H5Dget_space(dataset_id);
    if (memspace_id < 0 || filespace_id < 0)
    {
        H5Sclose (memspace_id);
        H5Tclose (datatype_id);
        H5Dclose (dataset_id);
        return BAG_HDF_DATASPACE_CORRUPTED;
    }

    H5Sselect_hyperslab (filespace_id, H5S_SELECT_SET, offset, NULL, count, NULL);

    /* xfer params */
    if (xfer == DISABLE_STRIP_MINING)
    {
        type_size = H5Tget_size(datatype_id);
        size = H5Sget_simple_extent_npoints(filespace_id) * type_size;
        xfer_plist = H5Pcreate (H5P_DATASET_XFER);
        H5Pset_buffer(xfer_plist, size, NULL, NULL);
        xfer = xfer_plist;
    }

    /* perform read_or_write on hyperslab */
    if (read_or_write == READ_BAG)
        status = H5Dread (dataset_id, datatype_id, memspace_id, filespace_id, 
                          xfer, data);
    else if (read_or_write == WRITE_BAG)
        status = H5Dwrite (dataset_id, datatype_id, memspace_id, filespace_id, 
                           xfer, data);
    else
        ; /*  error? */
    check_hdf_status();

    /* did what we came to do, now close up */
    status = H5Dclose (dataset_id);
    check_hdf_status();
    status = H5Sclose (memspace_id);
    check_hdf_status();
    status = H5Sclose (filespace_id);
    check_hdf_status();
    status = H5Tclose (datatype_id);
    check_hdf_status();

    if (status < 0)
        return BAG_HDF_INTERNAL_ERROR;
    else
        return BAG_SUCCESS;
}


/*
 *       set up the pointers for the 2d array in bagData
 *       to point to the correct offset within the actual
 *       1d array being used in bagHandle for contiguous HDF I/O
 */
bagError bagAllocArray (bagHandle hnd, u32 start_row, u32 start_col, 
                         u32 end_row, u32 end_col, s32 type)
{
    s32 i;
    bagError status;

    if (hnd == NULL)
        return BAG_INVALID_BAG_HANDLE;

    /* free previously allocated buffer if present */
    status = bagFreeArray (hnd, type);
    if (status != BAG_SUCCESS)
        return status;

    switch (type) 
    {
    case Uncertainty:
        
        /* alloc the contiguous 1d array */
        hnd->uncertaintyArray = (f32 *)calloc ((end_col-start_col+1)*(end_row-start_row+1), sizeof(f32));
        if (hnd->uncertaintyArray == NULL)
            return BAG_MEMORY_ALLOCATION_FAILED;

        /* alloc the array of pointers to floats */
        hnd->bag.uncertainty = (f32 **)calloc ((end_row-start_row+1), sizeof (f32 *));
        if (hnd->bag.uncertainty == NULL)
            return BAG_MEMORY_ALLOCATION_FAILED;

        /* now the 2d is tied to the contiguous 1d */
        hnd->bag.uncertainty[0] = hnd->uncertaintyArray;

        /* set the rest of the pointers */
        for (i=1; i < (end_row-start_row+1); i++)
        {
            hnd->bag.uncertainty[i] = hnd->bag.uncertainty[i-1] + (end_col-start_col+1);
        }

        /* init data to NULL values */
        for (i=0; i < ((end_col-start_col+1)*(end_row-start_row+1)); i++)
            hnd->uncertaintyArray [i] = NULL_UNCERTAINTY;
        break;

    case Elevation:
        
        /* alloc the contiguous 1d array */
        hnd->elevationArray = (f32 *)calloc ((end_col-start_col+1)*(end_row-start_row+1), sizeof(f32));
        if (hnd->elevationArray == NULL)
            return BAG_MEMORY_ALLOCATION_FAILED;

        /* alloc the array of pointers to floats */
        hnd->bag.elevation = (f32 **)calloc ((end_row-start_row+1), sizeof (f32 *));
        if (hnd->bag.elevation == NULL)
            return BAG_MEMORY_ALLOCATION_FAILED;

        /* now the 2d is tied to the contiguous 1d */
        hnd->bag.elevation[0] = hnd->elevationArray;

        /* set the rest of the pointers */
        for (i=1; i < (end_row-start_row+1); i++)
        {
            hnd->bag.elevation[i] = hnd->bag.elevation[i-1] + (end_col-start_col+1);
        }

        /* init data to NULL values */
        for (i=0; i < ((end_col-start_col+1)*(end_row-start_row+1)); i++)
            hnd->elevationArray [i] = NULL_ELEVATION;
        break;

    default:
        return BAG_HDF_TYPE_NOT_FOUND;
        break;
    }
    
    return BAG_SUCCESS;
}

bagError bagFreeArray (bagHandle hnd, s32 type)
{
    if (hnd == NULL)
        return BAG_INVALID_BAG_HANDLE;

    switch (type) 
    {
    case Uncertainty:
        if (hnd->bag.uncertainty != NULL)
        {
            free (hnd->bag.uncertainty);
            hnd->bag.uncertainty = NULL;

            free (hnd->uncertaintyArray);
            hnd->uncertaintyArray = NULL;
        }
        break;

    case Elevation:
        if (hnd->bag.elevation != NULL)
        {
            free (hnd->bag.elevation);
            hnd->bag.elevation = NULL;

            free (hnd->elevationArray);
            hnd->elevationArray = NULL;
        }
        break;

    default:
        return BAG_HDF_TYPE_NOT_FOUND;
    }

    return BAG_SUCCESS;
}

/****************************************************************************************
 *
 * Function : bagFillPos
 * 
 * Description :      This function allocates memory for two arrays of doubles.
 *                  Depending on the extents of two provided RowCol coordinates
 *                  and the SWcorner origin of the Bag, the cell-center node positions 
 *                  for each row/col will be calculated and returned in the XY arrays.
 *
 *                  The user must simply call the system free() call on the arrays
 *                  once they're done with them.
 *
 * Errors :   if Memory allocation fails or the bagHandle is insufficient
 *
 ****************************************************************************************/
bagError bagFillPos (bagHandle bagHandle, u32 r1, u32 c1 , u32 r2, u32 c2, f64 **x, f64 **y)
{
    bagError stat;
    u32  xlen, ylen, i;
    f64  xorig, yorig, xoff, yoff;

    if (bagHandle == NULL)
        return BAG_INVALID_BAG_HANDLE;

    if (c1 < 0 || c2 < 0 || c2 >= bagHandle->bag.def.ncols ||
        r1 < 0 || r2 < 0 || r2 >= bagHandle->bag.def.nrows ||
        r1 > r2 || 
        c1 > c2)
    {
        fprintf(stderr, "Internal error, bad parameters given to access surface extents! Aborting...\n");
        fprintf(stderr, "\tCannot access region, %d-%d / %d-%d, with surface extents 0-%d / 0-%d\n",
                r1, c1, r2, c2, bagHandle->bag.def.nrows, bagHandle->bag.def.ncols);
        fflush(stderr);
        return BAG_HDF_ACCESS_EXTENTS_ERROR;
    }

    /* ensures corner coord and nodespacing are available */
    if ((stat = bagInitDefinitionFromBag (bagHandle)) != BAG_SUCCESS)
    {
        return stat;
    }

    /* first step is to figure out how much memory to alloc for the arrays */
    ylen  = (r2 -r1) + 1;
    xlen  = (c2 -c1) + 1;
    xorig = bagHandle->bag.def.swCornerX;
    yorig = bagHandle->bag.def.swCornerY;
    xoff  = bagHandle->bag.def.nodeSpacingX;
    yoff  = bagHandle->bag.def.nodeSpacingY;

    /* alloc */
    (*x) = calloc (xlen, sizeof(f64));
    (*y) = calloc (ylen, sizeof(f64));
    if ((*x) == NULL || (*y) == NULL)
        return BAG_MEMORY_ALLOCATION_FAILED;

    for (i=0; i < xlen; i++)
        (*x)[i] = xorig + ((c1 + i) * xoff);
    for (i=0; i < ylen; i++)
        (*y)[i] = yorig + ((r1 + i) * yoff);

    return BAG_SUCCESS;
}


/****************************************************************************************
 *
 * 
 *
 ****************************************************************************************/

bagError bagReadXMLStream (bagHandle bagHandle)
{
    return bagAlignXMLStream (bagHandle, READ_BAG);
}

bagError bagWriteXMLStream (bagHandle bagHandle)
{
    return bagAlignXMLStream (bagHandle, WRITE_BAG);
}

/****************************************************************************************
 *
 *  Special case had to be made so that metadata is a single dimension - 
 *  and also it has no max size and can be extended as much as we'd like.
 *
 ****************************************************************************************/
bagError bagAlignXMLStream (bagHandle hnd, s32 read_or_write)
{
    herr_t      status;
    s32         rank;

    /* hyperslab selection parameters */
    hsize_t	    count[1];
    hssize_t	offset[1];
    hid_t       memspace_id, 
                datatype_id,
                dataset_id,
                filespace_id;
    
    void       *data;
    u8         *dataset_name;

    /* chunking data block */
    hsize_t     chunk_dimsr[1];
    s32         rank_chunk;
    hid_t       cparms;
    hsize_t	    extend[1];

    if (hnd == NULL)
        return BAG_INVALID_BAG_HANDLE;

    dataset_name = METADATA_PATH;

    /* Open an existing dataset. */
    dataset_id = H5Dopen(hnd->file_id, dataset_name);
    if (dataset_id < 0)
        return BAG_HDF_DATASET_OPEN_FAILURE; 

    if ((datatype_id = H5Dget_type(dataset_id)) < 0)
        return BAG_HDF_TYPE_NOT_FOUND;

    /* Open the filespace for the case where we're reading into a NULL metadata */
    filespace_id = H5Dget_space(dataset_id);
    if (filespace_id < 0)
    {
        H5Tclose (datatype_id);
        H5Dclose (dataset_id);
        return BAG_HDF_DATASPACE_CORRUPTED;
    }

    rank = H5Sget_simple_extent_dims (filespace_id, count, NULL);
    
    if (hnd->bag.metadata == NULL)
    {
        if (read_or_write == WRITE_BAG)
            return BAG_HDF_CANNOT_WRITE_NULL_DATA;
        else
            hnd->bag.metadata = calloc (count[0], sizeof (u8));

        if (hnd->bag.metadata == NULL)
        {
            return BAG_MEMORY_ALLOCATION_FAILED;
        }
        extend[0] = count[0];
    }
    else
    {
        extend[0] = strlen(hnd->bag.metadata);
    }

    data = hnd->bag.metadata;

    /* let the metadata grow if it needs to */
    status = H5Dextend (dataset_id, extend);
    check_hdf_status();

    /* Close and reopen below in case we extend the metadata XML string */
    status = H5Sclose (filespace_id);
    check_hdf_status();

    /* Prepare the dataspaces */
    if ((filespace_id = H5Dget_space(dataset_id)) < 0)
    {
        H5Tclose (datatype_id);
        H5Dclose (dataset_id);
        return BAG_HDF_DATASPACE_CORRUPTED;
    }

    rank = H5Sget_simple_extent_dims (filespace_id, count, NULL);

    /* define the hyperslab parameters */
    if ((memspace_id = H5Screate_simple (1, count, NULL)) < 0)
        return BAG_HDF_CREATE_DATASPACE_FAILURE;

    offset[0] = 0;

    if ((cparms = H5Dget_create_plist (dataset_id)) < 0)
        return BAG_HDF_CREATE_PROPERTY_CLASS_FAILURE;
    
    if (H5D_CHUNKED == H5Pget_layout (cparms))
    {
        rank_chunk = H5Pget_chunk (cparms, 1, chunk_dimsr);
    }

    status = H5Sselect_hyperslab (filespace_id, H5S_SELECT_SET, offset, NULL, count, NULL);
    check_hdf_status();

    /* perform read_or_write on hyperslab */
    if (read_or_write == READ_BAG)
        status = H5Dread (dataset_id, datatype_id, memspace_id, filespace_id, 
                          H5P_DEFAULT, data);
    else if (read_or_write == WRITE_BAG)
        status = H5Dwrite (dataset_id, datatype_id, memspace_id, filespace_id, 
                           H5P_DEFAULT, data);
    else
        ; /* error? */
    check_hdf_status();


    
    /* did what we came to do, now close up */
    status = H5Dclose (dataset_id);
    check_hdf_status();
    status = H5Pclose (cparms);
    check_hdf_status();
    status = H5Sclose (memspace_id);
    check_hdf_status();
    status = H5Sclose (filespace_id);
    check_hdf_status();
    status = H5Tclose (datatype_id);
    check_hdf_status();

    return BAG_SUCCESS;
}


/****************************************************************************************
 * 
 *  Actually seems to work...
 *
 ****************************************************************************************/
bagError bagReadSurfaceDims (bagHandle hnd, hsize_t *max_dims)
{
    herr_t   status;
    s32      rank;
    hid_t    dataset_id,
             dataspace_id;
    
    if (hnd == NULL)
        return BAG_INVALID_BAG_HANDLE;

    dataset_id = H5Dopen(hnd->file_id, ELEVATION_PATH);
    if (dataset_id < 0)
        return BAG_HDF_DATASET_OPEN_FAILURE; 

    dataspace_id = H5Dget_space(dataset_id);
    if (H5Sis_simple(dataspace_id))
    {
        rank = H5Sget_simple_extent_ndims(dataspace_id);
        rank = H5Sget_simple_extent_dims(dataspace_id, max_dims, NULL);

        /* seems like a reasonable requirement for BAG compatibility now? */
        if (rank != RANK)
        {
            fprintf(stderr, "Error - The BAG is corrupted.  The rank of this dataset is said to be = %d, when it should be = %d. \n",
                    rank, RANK);
            fflush(stderr);
            return BAG_HDF_RANK_INCOMPATIBLE;
        }
    }
    else
    {
        H5Dclose (dataset_id);
        return BAG_HDF_DATASPACE_CORRUPTED;
    }
    
    status = H5Sclose(dataspace_id);
    check_hdf_status();
    status = H5Dclose(dataset_id);
    check_hdf_status();
    
    return BAG_SUCCESS;
}

bagError bagGetGridDimensions(bagHandle hnd, u32 *rows, u32 *cols) 
{
    if (hnd == NULL)
        return BAG_INVALID_BAG_HANDLE;

    *rows = hnd->bag.def.nrows;
    *cols = hnd->bag.def.ncols;

    return BAG_SUCCESS;
}


bagError bagUpdateSurface (bagHandle hnd, u32 type)
{
    herr_t   status;

    if (hnd == NULL)
        return BAG_INVALID_BAG_HANDLE;

    status = bagUpdateMinMax (hnd, type);
    check_hdf_status();

    return BAG_SUCCESS;
}

/****************************************************************************************
 *
 *
 ****************************************************************************************/
bagError bagUpdateMinMax (bagHandle hnd, u32 type)
{
    herr_t status;
    u32    i, j;
    u8    *dataset_name, *max_name, *min_name;
    hid_t  dataset_id;
    f32   *min_tmp, *max_tmp, **surface_array, *omax, *omin, null_val;


    if (hnd == NULL)
        return BAG_INVALID_BAG_HANDLE;

    min_tmp = calloc (1, sizeof (f32));
    max_tmp = calloc (1, sizeof (f32));
    if (min_tmp == NULL || max_tmp == NULL)
        return BAG_MEMORY_ALLOCATION_FAILED;

    switch (type)
    {
    case Elevation:
        omin          = &hnd->bag.min_elevation;
        omax          = &hnd->bag.max_elevation;
        dataset_name  = ELEVATION_PATH;
        max_name      = MAX_ELEVATION_NAME;
        min_name      = MIN_ELEVATION_NAME;
        null_val      = NULL_ELEVATION;
        surface_array = &hnd->elevationArray;
        break;

    case Uncertainty:
        omin          = &hnd->bag.min_uncertainty;
        omax          = &hnd->bag.max_uncertainty;
        dataset_name  = UNCERTAINTY_PATH;
        max_name      = MAX_UNCERTAINTY_NAME;
        min_name      = MIN_UNCERTAINTY_NAME;
        null_val      = NULL_UNCERTAINTY;
        surface_array = &hnd->uncertaintyArray;
        break;

    default:
        return BAG_HDF_TYPE_NOT_FOUND;
        break;
    }
    

    /* Open an existing dataset. */
    dataset_id = H5Dopen(hnd->file_id, dataset_name);
    if (dataset_id < 0)
        return BAG_HDF_DATASET_OPEN_FAILURE; 

    *max_tmp = null_val;
    *min_tmp = null_val;

    for (i=0; i < hnd->bag.def.nrows; i++)
    {
        bagReadRegion (hnd, i, 0, i, hnd->bag.def.ncols-1, type);
        for (j=0; j < hnd->bag.def.ncols-1; j++)
        {
            if ((* surface_array)[j] != null_val)
            {
                if (*max_tmp == null_val)
                    *max_tmp = (* surface_array)[j];
                if (*min_tmp == null_val)
                    *min_tmp = (* surface_array)[j];
                if ((* surface_array)[j] > *max_tmp)
                    *max_tmp = (* surface_array)[j];
                if ((* surface_array)[j] < *min_tmp)
                    *min_tmp = (* surface_array)[j];
            }
        }
    }

    /* update the original bagData values */
    *omin = *min_tmp;
    *omax = *max_tmp;

    if (*max_tmp != null_val)
    {
        status = bagWriteAttribute (hnd, dataset_id, max_name, max_tmp);
        check_hdf_status();
    }
    if (*min_tmp != null_val)
    {
        status = bagWriteAttribute (hnd, dataset_id, min_name, min_tmp);
        check_hdf_status();
    }

    free (min_tmp);
    free (max_tmp);

    return BAG_SUCCESS;
}
