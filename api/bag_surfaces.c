/*! \file bag_surfaces.c
 * \brief This module contains functions for accessing the surface datasets and the XML metadata.
 ********************************************************************
 *
 * Module Name : bag_surfaces.c
 *
 * Author/Date : ONSWG, July 2005
 *
 * Description : 
 *               Surfaces are datasets.
 *               There is a restriction here which requires all the
 *               proper surfaces to have the exact same extents/dimensions.
 *               The XML Metdata and the tracking_list are not 
 *               surfaces, but they are stored in a similar HDF dataset structure.
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 * who  when      what
 * ---  ----      ----
 * Mike Van Duzee -- Wed Aug 3 15:48:50 2011
 *   -The bagUpdateMinMax() function was missing the last column.
 *
 * Webb McDonald -- Wed Jun 29 15:32:44 2011
 *   -fixes for compile warnings
 *
 * Webb McDonald -- Fri Mar  2 14:13:35 2007
 *   -each "surface" now has a bunch of HDF structs initialized at the
 *    bag_hdf.c level now instead of temporarily being opened and 
 *    closed within every I/O call here.
 *   -dataset, datatype, filespace \a hid_t will already be opened up there,
 *    but memspace varies and might be reopened within each call.
 *
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
/*! \brief : bagWriteNode
 *
 * Description : 
 *        Write indiviidual nodes of a surface
 * 
 * \param  bag   External reference to the private \a bagHandle object
 * \param  row   row offset within \a bag to access
 * \param  col   col offset within \a bag to access
 * \param  type  Indicates which data surface type to access, defined by enum \a BAG_SURFACE_PARAMS
 * \param *data  Pointer to memory for reading from or writing to the \a bag.
 *               Cannot be NULL!
 *
 * \return : \li On success, \a bagError is set to \a BAG_SUCCESS.
 *           \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS.
 * 
 ********************************************************************/

bagError bagWriteNode (bagHandle bag, u32 row, u32 col, s32 type, void *data)
{
    return bagAlignNode (bag, row, col, type, data, WRITE_BAG);
}

/****************************************************************************************/
/*! \brief : bagReadNode
 *
 * Description : 
 *        Read indiviidual nodes of a surface
 * 
 * \param  bag   External reference to the private \a bagHandle object
 * \param  row   row offset within \a bag to access
 * \param  col   col offset within \a bag to access
 * \param  type  Indicates which data surface type to access, defined by enum \a BAG_SURFACE_PARAMS
 * \param *data  Pointer to memory for reading from or writing to the \a bag.
 *               Cannot be NULL!
 *
 * \return : \li On success, \a bagError is set to \a BAG_SUCCESS.
 *           \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS.
 * 
 ********************************************************************/
bagError bagReadNode (bagHandle bag, u32 row, u32 col, s32 type, void *data)
{
    return bagAlignNode (bag, row, col, type, data, READ_BAG);
}

/****************************************************************************************/
/*! \brief : bagReadNodePos
 *
 *  Description :
 *    Same as bagreadNode, but also populates x and y with the geodetic or projected 
 *    position of the node, based on the coordinate system of the bag.
 *
 * \param  bag   External reference to the private \a bagHandle object
 * \param  row   row offset within \a bag to access
 * \param  col   col offset within \a bag to access
 * \param  type  Indicates which data surface type to access, defined by enum \a BAG_SURFACE_PARAMS
 * \param *data  Pointer to memory for reading from or writing to the \a bag.
 *               Cannot be NULL!
 * \param **x    Address of a pointer to an f64. \a *x must be freed when no longer needed.
 * \param **y    Address of a pointer to an f64. \a *y must be freed when no longer needed.
 *
 * \return : \li On success, \a bagError is set to \a BAG_SUCCESS.
 *           \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS.
 * 
 ********************************************************************/
bagError bagReadNodePos (bagHandle bag, u32 row, u32 col, s32 type, void *data, f64 **x, f64 **y)
{
    bagError status;
    
    if ((status = bagFillPos (bag, row, col, row, col, x, y)) != BAG_SUCCESS)
        return status;
    return bagAlignNode (bag, row, col, type, data, READ_BAG);
}

/****************************************************************************************
 *
 * \brief This is only slightly different from bagAlignRegion. 
 *  -The out of range conditions are based on a point, not a region
 *  -The dataspaces are just 1dimension
 *  -Uses element select(prone to memory leaks though), instead of hyperslab 
 *
 ****************************************************************************************/
bagError bagAlignNode (bagHandle bagHandle, u32 row, u32 col, s32 type, void *data, s32 read_or_write)
{
    u32            srow, scol;
    herr_t         status;
    hsize_t        snode[2] = {1,1};
    hssize_t	     offset[1][2];
    hid_t          dataset_id,
                   memspace_id,
                   datatype_id,
                   filespace_id;


    if (bagHandle == NULL)
        return BAG_INVALID_BAG_HANDLE;

    if (type >= BAG_OPT_SURFACE_LIMIT)
        return  BAG_INVALID_FUNCTION_ARGUMENT;

    if (type > Uncertainty)
    {
        srow=bagHandle->bag.opt[type].nrows;
        scol=bagHandle->bag.opt[type].ncols;
    }
    else
    {
        srow=bagHandle->bag.def.nrows;
        scol=bagHandle->bag.def.ncols;
    }

    /*! some error checking on surfaces extents */
    if (row >= srow || col >= scol)
    {
        fprintf(stderr, "Fail to access out of bounds row/col = %d/%d out of possible size: %d/%d. Aborting\n",
                row, col, srow, scol);
        fflush(stderr);
        return BAG_HDF_ACCESS_EXTENTS_ERROR;
    }

    if (data == NULL && read_or_write == WRITE_BAG)
        return BAG_HDF_CANNOT_WRITE_NULL_DATA;
    if (data == NULL)
        return  BAG_INVALID_FUNCTION_ARGUMENT;

    /*! Set coordinates for access  */
    offset[0][0] = row;
    offset[0][1] = col;
    
    /*!
     * Depending on the type: 
     *       set the memspace_id to the correct unit for 
     *       this kind of surface and snode
     */
    switch (type)
    {
        u32 nct=0;
    case Elevation:
        
        if (bagHandle->elv_memspace_id >= 0)
        {
            nct = (u32)H5Sget_select_npoints (bagHandle->elv_memspace_id);
        }

        if (bagHandle->elv_memspace_id < 0 || nct != snode[1])
        {
            if (bagHandle->elv_memspace_id >= 0)
            {
                 status = H5Sclose (bagHandle->elv_memspace_id);
                 check_hdf_status();
            }

            /*! Create space in memory */
            bagHandle->elv_memspace_id = H5Screate_simple(RANK, snode, NULL);
            if (bagHandle->elv_memspace_id < 0)
            {
                return BAG_HDF_DATASPACE_CORRUPTED;
            }
        }

        datatype_id  = bagHandle->elv_datatype_id;
        memspace_id  = bagHandle->elv_memspace_id;
        filespace_id = bagHandle->elv_filespace_id;
        dataset_id   = bagHandle->elv_dataset_id;
        break;
    case Uncertainty:

        if (bagHandle->unc_memspace_id >= 0)
            nct = (u32)H5Sget_select_npoints (bagHandle->unc_memspace_id);

        if (bagHandle->unc_memspace_id < 0 || nct != snode[1])
        {
            if (bagHandle->unc_memspace_id >= 0)
            {
                 status = H5Sclose (bagHandle->unc_memspace_id);
                 check_hdf_status();
            }

            /*! Create space in memory */
            bagHandle->unc_memspace_id = H5Screate_simple(RANK, snode, NULL);
            if (bagHandle->unc_memspace_id < 0)
            {
                return BAG_HDF_DATASPACE_CORRUPTED;
            }
        }
            
        datatype_id  = bagHandle->unc_datatype_id;
        memspace_id  = bagHandle->unc_memspace_id;
        filespace_id = bagHandle->unc_filespace_id;
        dataset_id   = bagHandle->unc_dataset_id;
        break;
	case Nominal_Elevation:
	case Surface_Correction:
	case Node_Group:
	case Elevation_Solution_Group:
        if (bagHandle->opt_memspace_id[type] >= 0)
        {
            nct = (u32)H5Sget_select_npoints (bagHandle->opt_memspace_id[type]);
        }

        if (bagHandle->opt_memspace_id[type] < 0 || nct != snode[1])
        {
            if (bagHandle->opt_memspace_id[type] >= 0)
            {
                 status = H5Sclose (bagHandle->opt_memspace_id[type]);
                 check_hdf_status();
            }

            /*! Create space in memory */
            bagHandle->opt_memspace_id[type] = H5Screate_simple(RANK, snode, NULL);
            if (bagHandle->opt_memspace_id < 0)
            {
                return BAG_HDF_DATASPACE_CORRUPTED;
            }
        }

        datatype_id  = bagHandle->opt_datatype_id[type];
        memspace_id  = bagHandle->opt_memspace_id[type];
        filespace_id = bagHandle->opt_filespace_id[type];
        dataset_id   = bagHandle->opt_dataset_id[type];
        break;
    default:
        return BAG_HDF_TYPE_NOT_FOUND;
        break;
    }

    /*! Select grid cell within dataset file space. */
    status = H5Sselect_elements (filespace_id, H5S_SELECT_SET, 1, (const hsize_t *)offset);
    check_hdf_status();
    
    /*!  perform read_or_write on element */
    if (read_or_write == READ_BAG)
        status = H5Dread (dataset_id, datatype_id, memspace_id, filespace_id, 
                          H5P_DEFAULT, data);
    else if (read_or_write == WRITE_BAG)
        status = H5Dwrite (dataset_id, datatype_id, memspace_id, filespace_id, 
                           H5P_DEFAULT, data);
    check_hdf_status();

    if (status < 0)
        return BAG_HDF_INTERNAL_ERROR;
    else
        return BAG_SUCCESS;
}

/****************************************************************************************/
/*! \brief : bagWriteRow
 *
 * Description : 
 *     This function writes the \a row of data values for the surface parameter 
 *     specified by \a type to the specified \a row for the BAG specified by \a bagHandle.
 *     The intended usage of this function is for initial load of data into a BAG.
 *     \a *data must have sufficient space allocated externally to fulfill the request.
 * 
 * \param  bagHandle   External reference to the private \a bagHandle object
 * \param  k           Row offset within \a bag to access
 * \param  start_col   Starting col offset within \a bag row, 
 *                     cannot be less than zero or greater than \a end_col
 * \param  end_col     Ending col offset within \a bag row,
 *                     cannot be less than \a start_col or greater than the extent of the row.
 * \param  type        Indicates which data surface type to access, defined by enum \a BAG_SURFACE_PARAMS
 * \param *data        Pointer to memory for reading from or writing to the \a bag. Cannot be NULL!
 *
 * \return : \li On success, \a bagError is set to \a BAG_SUCCESS.
 *           \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS.
 * 
 ********************************************************************/
bagError bagWriteRow (bagHandle bagHandle, u32 k, u32 start_col, u32 end_col, s32 type, void *data)
{
    return bagAlignRow (bagHandle, k, start_col, end_col, type, WRITE_BAG, data);
}

/****************************************************************************************/
/*! \brief : bagReadRow
 *
 * Description : 
 *     This function reads the \a row of data values for the surface parameter 
 *     specified by \a type to the specified \a row for the BAG specified by \a bagHandle.
 *     The calling application is required to manage the memory for the data array.
 *     \a *data must have sufficient space allocated externally to fulfill the request.
 * 
 * \param  bagHandle   External reference to the private \a bagHandle object
 * \param  k           Row offset within \a bag to access
 * \param  start_col   Starting col offset within \a bag row, 
 *                     cannot be less than zero or greater than \a end_col
 * \param  end_col     Ending col offset within \a bag row,
 *                     cannot be less than \a start_col 
 *                     or greater than the \a ncols extent of the \a bagDef
 * \param  type        Indicates which data surface type to access, defined by enum \a BAG_SURFACE_PARAMS
 * \param *data        Pointer to memory for reading from or writing to the \a bag. Cannot be NULL!
 *
 * \return : \li On success, \a bagError is set to \a BAG_SUCCESS.
 *           \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS.
 * 
 ********************************************************************/
bagError bagReadRow (bagHandle bagHandle, u32 k, u32 start_col, u32 end_col, s32 type, void *data)
{
    return bagAlignRow (bagHandle, k, start_col, end_col, type, READ_BAG, data);
}

/****************************************************************************************/
/*! \brief : bagReadRowPos
 *
 *  Description :
 *    Same as bagReadRow, but also populates \a **x and \a **y with the positions,
 *    based on the coordinate system of the \a bag. Y will just have one trivial value, of course.
 *
 * \param  bag         External reference to the private \a bagHandle object
 * \param  row         Row offset within \a bag to access
 * \param  start_col   Starting col offset within \a bag row, 
 *                     cannot be less than zero or greater than \a end_col
 * \param  end_col     Ending col offset within \a bag row,
 *                     cannot be less than \a start_col or greater than the extent of the row.
 * \param  type        Indicates which data surface type to access, defined by enum \a BAG_SURFACE_PARAMS
 * \param *data        Pointer to memory for reading from or writing to the \a bag. Cannot be NULL!
 * \param **x          Address of a pointer to a space of f64s. \a *x must be freed when no longer needed.
 * \param **y          Address of a pointer to an f64. \a *y must be freed when no longer needed.
 *
 * \return : \li On success, \a bagError is set to \a BAG_SUCCESS.
 *           \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS.
 * 
 ********************************************************************/
bagError bagReadRowPos (bagHandle bag, u32 row, u32 start_col, u32 end_col, s32 type, 
                        void *data, f64 **x, f64 **y)
{
    bagError status;
    
    if ((status = bagFillPos (bag, row, start_col, row, end_col, x, y)) != BAG_SUCCESS)
        return status;
    return bagAlignRow (bag, row, start_col, end_col, type, READ_BAG, data);
}

/****************************************************************************************/
bagError bagAlignRow (bagHandle bagHandle, u32 row, u32 start_col, 
                      u32 end_col, s32 type, s32 read_or_write, void *data)
{
    u32         srow, scol;
    herr_t      status = 0;

    /* hyperslab selection parameters */
    hsize_t	  count[RANK];
    hssize_t	  offset[RANK];
    hid_t       memspace_id, 
                datatype_id,
                dataset_id,
                filespace_id;


    if (bagHandle == NULL)
        return BAG_INVALID_BAG_HANDLE;

    if (type >= BAG_OPT_SURFACE_LIMIT)
        return  BAG_INVALID_FUNCTION_ARGUMENT;

    if (type > Uncertainty)
    {
        srow=bagHandle->bag.opt[type].nrows;
        scol=bagHandle->bag.opt[type].ncols;
    }
    else
    {
        srow=bagHandle->bag.def.nrows;
        scol=bagHandle->bag.def.ncols;
    }

    if (end_col >= scol ||
        row >= srow ||
        start_col > end_col)
    {
        fprintf(stderr, "Internal error, bad parameters given to access surface extents! Aborting...\n");
        fprintf(stderr, "\tCannot access region, %d-%d / %d-%d, with surface extents 0-%d / 0-%d\n",
                row, start_col, row, end_col, srow, scol);
        fflush(stderr);
        return BAG_HDF_ACCESS_EXTENTS_ERROR;
    }

    /*! define the hyperslab parameters */
    count[0] = 1;
    count[1] = (end_col - start_col) + 1;
    offset[0] = row;
    offset[1] = start_col;
    

    /*!
     * Depending on the type: 
     *       set the memspace_id to the correct unit for 
     *       this kind of surface and snode
     */
    switch (type)
    {
        u32 nct=0;
    case Elevation:
        
        if (bagHandle->elv_memspace_id >= 0)
        {
            nct = (u32) H5Sget_select_npoints (bagHandle->elv_memspace_id);
        }
        
        if (bagHandle->elv_memspace_id < 0 || nct != count[1])
        {
            if (bagHandle->elv_memspace_id >= 0)
            {
                status = H5Sclose (bagHandle->elv_memspace_id);
                check_hdf_status();
            }
            
            /*! Create space in memory */
            bagHandle->elv_memspace_id = H5Screate_simple(RANK, count, NULL);
            if (bagHandle->elv_memspace_id < 0)
                return BAG_HDF_DATASPACE_CORRUPTED;
        }
        
        datatype_id  = bagHandle->elv_datatype_id;
        memspace_id  = bagHandle->elv_memspace_id;
        filespace_id = bagHandle->elv_filespace_id;
        dataset_id   = bagHandle->elv_dataset_id;
        break;
    case Uncertainty:

        if (bagHandle->unc_memspace_id >= 0)
            nct = (u32) H5Sget_select_npoints (bagHandle->unc_memspace_id);
        
        if (bagHandle->unc_memspace_id < 0 || nct != count[1])
        {
            if (bagHandle->unc_memspace_id >= 0)
            {
                status = H5Sclose (bagHandle->unc_memspace_id);
                check_hdf_status();
            }
            
            /*! Create space in memory */
            bagHandle->unc_memspace_id = H5Screate_simple(RANK, count, NULL);
            if (bagHandle->unc_memspace_id < 0)
                return BAG_HDF_DATASPACE_CORRUPTED;
        }
        
        datatype_id  = bagHandle->unc_datatype_id;
        memspace_id  = bagHandle->unc_memspace_id;
        filespace_id = bagHandle->unc_filespace_id;
        dataset_id   = bagHandle->unc_dataset_id;
        break;
	
	case Nominal_Elevation: 
	case Surface_Correction: 
	case Node_Group:
	case Elevation_Solution_Group:
        if (bagHandle->opt_memspace_id[type] >= 0)
        {
            nct = (u32) H5Sget_select_npoints (bagHandle->opt_memspace_id[type]);
        }
        
        if (bagHandle->opt_memspace_id[type] < 0 || nct != count[1])
        {
            if (bagHandle->opt_memspace_id[type] >= 0)
            {
                status = H5Sclose (bagHandle->opt_memspace_id[type]);
                check_hdf_status();
            }
            
            /*! Create space in memory */
            bagHandle->opt_memspace_id[type] = H5Screate_simple(RANK, count, NULL);
            if (bagHandle->opt_memspace_id[type] < 0)
                return BAG_HDF_DATASPACE_CORRUPTED;
        }
        
        datatype_id  = bagHandle->opt_datatype_id[type];
        memspace_id  = bagHandle->opt_memspace_id[type];
        filespace_id = bagHandle->opt_filespace_id[type];
        dataset_id   = bagHandle->opt_dataset_id[type];
        break;
       
    default:
        return BAG_HDF_TYPE_NOT_FOUND;
        break;
    }

    if (data == NULL)
        return  BAG_INVALID_FUNCTION_ARGUMENT;

    H5Sselect_hyperslab (filespace_id, H5S_SELECT_SET, (hsize_t *) offset, NULL, count, NULL);

    /*! perform read_or_write on hyperslab */
    if (read_or_write == READ_BAG)
        status = H5Dread (dataset_id, datatype_id, memspace_id, filespace_id, 
                          H5P_DEFAULT, data);
    else if (read_or_write == WRITE_BAG)
        status = H5Dwrite (dataset_id, datatype_id, memspace_id, filespace_id, 
                           H5P_DEFAULT, data);
    check_hdf_status();

    if (status < 0)
        return BAG_HDF_INTERNAL_ERROR;
    else
        return BAG_SUCCESS;
}

/****************************************************************************************/
/*! \brief bagWriteDataset writes an entire buffer of data to a bag surface
 *
 *  \param bagHandle  External reference to the private \a bagHandle object
 *  \param type       Indicates which data surface type to access, element of \a BAG_SURFACE_PARAMS
 *
 *  \return : \li On success, \a bagError is set to \a BAG_SUCCESS.
 *            \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS.
 * 
 ********************************************************************/
bagError bagWriteDataset (bagHandle bagHandle, s32 type)
{
    return bagAlignRegion (bagHandle, 0, 0, bagHandle->bag.def.nrows - 1, 
                           bagHandle->bag.def.ncols - 1, type, WRITE_BAG, DISABLE_STRIP_MINING);
}

/****************************************************************************************/
/*! \brief bagReadDataset reads an entire buffer of data from a bag surface
 *
 *  \param bagHandle  External reference to the private \a bagHandle object
 *  \param type       Indicates which data surface type to access, element of \a BAG_SURFACE_PARAMS
 *
 *  \return : \li On success, \a bagError is set to \a BAG_SUCCESS.
 *            \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS.
 * 
 ********************************************************************/
bagError bagReadDataset (bagHandle bagHandle, s32 type)
{
    return bagAlignRegion (bagHandle, 0, 0, bagHandle->bag.def.nrows - 1, 
                           bagHandle->bag.def.ncols - 1, type, READ_BAG, DISABLE_STRIP_MINING);
}

/****************************************************************************************/
/*! \brief : bagReadDatasetPos
 *
 *  Description :
 *    Same as \a bagReadDataset, but also populates \a **x and \a **y with the positions.
 *
 *  \param bagHandle  External reference to the private \a bagHandle object
 *  \param type       Indicates which data surface type to access, element of \a BAG_SURFACE_PARAMS
 *  \param **x          Address of a pointer to a space of f64s. \a *x must be freed when no longer needed.
 *  \param **y          Address of a pointer to a space of f64s. \a *y must be freed when no longer needed.
 *
 *  \return : \li On success, \a bagError is set to \a BAG_SUCCESS.
 *            \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS.
 * 
 ********************************************************************/
bagError bagReadDatasetPos (bagHandle bagHandle, s32 type, f64 **x, f64 **y)
{
    bagError status;
    
    if ((status = bagFillPos (bagHandle, 0, 0, bagHandle->bag.def.nrows - 1, 
                              bagHandle->bag.def.ncols - 1, x, y)) != BAG_SUCCESS)
        return status;
    return bagAlignRegion (bagHandle, 0, 0, bagHandle->bag.def.nrows - 1, 
                           bagHandle->bag.def.ncols - 1, type, READ_BAG, DISABLE_STRIP_MINING);
}

/****************************************************************************************/
/*! \brief bagReadRegion reads an entire buffer of data, 
 *                       defined by starting and ending coordinates, from a bag surface
 *
 *  \param bagHandle   External reference to the private \a bagHandle object
 *  \param  start_row  Starting Row offset within \a bag to access
 *                     start_row must be less than end_row and greater than or equal to zero.
 *  \param  start_col  Starting Col offset within \a bag to access
                       start_col must be less than end_col and greater than or equal to zero.
 *  \param  end_row    Ending row offset within \a bag to access
 *                     end_row must be greater than start_row and 
 *                     within the \a nrows extent of the \a bagDef
 *  \param  end_col    Ending col offset within \a bag to access
 *                     end_col must be greater than start_col and
 *                     within the \a ncols extent of the \a bagDef
 *  \param type        Indicates which data surface type to access, element of \a BAG_SURFACE_PARAMS
 *
 *  \return : \li On success, \a bagError is set to \a BAG_SUCCESS.
 *            \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS.
 * 
 ********************************************************************/
bagError bagReadRegion (bagHandle bagHandle, u32 start_row, u32 start_col, u32 end_row, u32 end_col, s32 type)
{
    return bagAlignRegion (bagHandle, start_row, start_col,  end_row,  end_col, type, READ_BAG, H5P_DEFAULT);
}

/****************************************************************************************/
/*! \brief bagWriteRegion writes an entire buffer of data, 
 *                       defined by starting and ending coordinates, from a bag surface
 *
 *  \param  bagHandle  External reference to the private \a bagHandle object
 *  \param  start_row  Starting Row offset within \a bag to access
 *                     start_row must be less than end_row and greater than or equal to zero.
 *  \param  start_col  Starting Col offset within \a bag to access
                       start_col must be less than end_col and greater than or equal to zero.
 *  \param  end_row    Ending row offset within \a bag to access
 *                     end_row must be greater than start_row and 
 *                     within the \a nrows extent of the \a bagDef
 *  \param  end_col    Ending col offset within \a bag to access
 *                     end_col must be greater than start_col and
 *                     within the \a ncols extent of the \a bagDef
 *  \param  type       Indicates which data surface type to access, element of \a BAG_SURFACE_PARAMS
 *
 *  \return : \li On success, \a bagError is set to \a BAG_SUCCESS.
 *            \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS.
 * 
 ********************************************************************/
bagError bagWriteRegion (bagHandle bagHandle, u32 start_row, u32 start_col, u32 end_row, u32 end_col, s32 type)
{
    return bagAlignRegion (bagHandle, start_row, start_col, end_row, end_col, type, WRITE_BAG, H5P_DEFAULT);
}

/****************************************************************************************/
/*! \brief : bagReadRegionPos
 *
 *  Description :
 *    Same as bagReadRegion, but also populates \a **x and \a **y with the positions.
 *
 *  \param  bag        External reference to the private \a bagHandle object
 *  \param  start_row  Starting Row offset within \a bag to access
 *                     start_row must be less than end_row and greater than or equal to zero.
 *  \param  start_col  Starting Col offset within \a bag to access
                       start_col must be less than end_col and greater than or equal to zero.
 *  \param  end_row    Ending row offset within \a bag to access
 *                     end_row must be greater than start_row and 
 *                     within the \a nrows extent of the \a bagDef
 *  \param  end_col    Ending col offset within \a bag to access
 *                     end_col must be greater than start_col and
 *                     within the \a ncols extent of the \a bagDef
 *  \param  type       Indicates which data surface type to access, element of \a BAG_SURFACE_PARAMS
 *  \param **x         Address of a pointer to a space of f64s. \a *x must be freed when no longer needed.
 *  \param **y         Address of a pointer to a space of f64s. \a *y must be freed when no longer needed.
 *
 *  \return : \li On success, \a bagError is set to \a BAG_SUCCESS.
 *            \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS.
 * 
 ********************************************************************/
bagError bagReadRegionPos (bagHandle bag, u32 start_row, u32 start_col, 
                           u32 end_row, u32 end_col, s32 type, f64 **x, f64 **y)
{
    bagError status;
    
    if ((status = bagFillPos (bag, start_row, start_col, end_row, end_col, x, y)) != BAG_SUCCESS)
        return status;
    return bagAlignRegion (bag, start_row, start_col, end_row, end_col, type, WRITE_BAG, H5P_DEFAULT);
}

/****************************************************************************************/
/*! I made a choice so far in this function - notice that void \a *data is just a temporary
 *  memory is either already allocated to the arrays in \a bagHandle bagHandle,
 *  or gets allocated here for a read.  It is up to the caller to free this memory later!
 *  This is achieved through the \a bagFreeArray function!
 *
 ****************************************************************************************/
bagError bagAlignRegion (bagHandle bagHandle, u32 start_row, u32 start_col, 
                    u32 end_row, u32 end_col, s32 type, s32 read_or_write, hid_t xfer)
{
    u32         srow, scol;
    herr_t      status = 0;

    /* hyperslab selection parameters */
    hsize_t	  count[RANK];
    hssize_t	  offset[RANK];
    hid_t       memspace_id, 
                datatype_id,
                dataset_id,
                filespace_id;

    /* xfer params */
    hssize_t    size;               /* Temp buffer size */
    hid_t       xfer_plist = -1;
    size_t      type_size;

    void       *data;


    if (bagHandle == NULL)
        return BAG_INVALID_BAG_HANDLE;

    if (type > Uncertainty)
    {
        srow=bagHandle->bag.opt[type].nrows;
        scol=bagHandle->bag.opt[type].ncols;
    }
    else
    {
        srow=bagHandle->bag.def.nrows;
        scol=bagHandle->bag.def.ncols;
    }

    if (end_col >= scol ||
        end_row >= srow ||
        start_row > end_row || 
        start_col > end_col)
    {
        fprintf(stderr, "Internal error, bad parameters given to access surface extents! Aborting...\n");
        fprintf(stderr, "\tCannot access region, %d-%d / %d-%d, with surface extents 0-%d / 0-%d\n",
                start_row, start_col, end_row, end_col, srow, scol);
        fflush(stderr);
        return BAG_HDF_ACCESS_EXTENTS_ERROR;
    }


    /*! define the hyperslab parameters */
    count[0] = (end_row - start_row) + 1;
    count[1] = (end_col - start_col) + 1;
    offset[0] = start_row;
    offset[1] = start_col;
    
    /*!
     * Depending on the type:
     *    1) alloc the buffers with bagallocarray
     *    2) point memspace to the correct
     *       surface within the BagDef structure
     */
    switch (type)
    {
        u32 nct = 0;
    case Elevation:

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

        if (bagHandle->elv_memspace_id >= 0)
            nct = (u32) H5Sget_select_npoints (bagHandle->elv_memspace_id);

        if (bagHandle->elv_memspace_id < 0 || nct != (count[0] * count[1]))
        {
            if (bagHandle->elv_memspace_id >= 0)
            {
                status = H5Sclose (bagHandle->elv_memspace_id);
                check_hdf_status();
            }
            
            /*! Prepare the dataspaces */
            bagHandle->elv_memspace_id = H5Screate_simple(RANK, count, NULL);
            if (bagHandle->elv_memspace_id < 0)
                return BAG_HDF_DATASPACE_CORRUPTED;
        }

        datatype_id  = bagHandle->elv_datatype_id;
        memspace_id  = bagHandle->elv_memspace_id;
        filespace_id = bagHandle->elv_filespace_id;
        dataset_id   = bagHandle->elv_dataset_id;
        break;
    case Uncertainty:

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

        if (bagHandle->unc_memspace_id >= 0)
            nct = (u32) H5Sget_select_npoints (bagHandle->unc_memspace_id);

        if (bagHandle->unc_memspace_id < 0 || nct != (count[0] * count[1]))
        {
            if (bagHandle->unc_memspace_id >= 0)
            {
                status = H5Sclose (bagHandle->unc_memspace_id);
                check_hdf_status();
            }
            
            /*! Prepare the dataspaces */
            bagHandle->unc_memspace_id = H5Screate_simple(RANK, count, NULL);
            if (bagHandle->unc_memspace_id < 0)
                return BAG_HDF_DATASPACE_CORRUPTED;
        }

        datatype_id  = bagHandle->unc_datatype_id;
        memspace_id  = bagHandle->unc_memspace_id;
        filespace_id = bagHandle->unc_filespace_id;
        dataset_id   = bagHandle->unc_dataset_id;
        break;
	
    case Nominal_Elevation:
	case Surface_Correction:
	case Node_Group:
	case Elevation_Solution_Group:
        if (read_or_write == READ_BAG)
        {
            if (bagAllocArray (bagHandle,  start_row,  start_col, 
                               end_row,  end_col, type) != BAG_SUCCESS)
                data = NULL;
            else
                data = bagHandle->dataArray[type];
        }
        else
            data = bagHandle->dataArray[type];

        if (bagHandle->opt_memspace_id[type] >= 0)
            nct = (u32) H5Sget_select_npoints (bagHandle->opt_memspace_id[type]);

        if (bagHandle->opt_memspace_id[type] < 0 || nct != (count[0] * count[1]))
        {
            if (bagHandle->opt_memspace_id[type] >= 0)
            {
                status = H5Sclose (bagHandle->opt_memspace_id[type]);
                check_hdf_status();
            }
            
            /*! Prepare the dataspaces */
            bagHandle->opt_memspace_id[type] = H5Screate_simple(RANK, count, NULL);
            if (bagHandle->opt_memspace_id[type] < 0)
                return BAG_HDF_DATASPACE_CORRUPTED;
        }

        datatype_id  = bagHandle->opt_datatype_id[type];
        memspace_id  = bagHandle->opt_memspace_id[type];
        filespace_id = bagHandle->opt_filespace_id[type];
        dataset_id   = bagHandle->opt_dataset_id[type];
        break;
    default:
        return BAG_HDF_TYPE_NOT_FOUND;
        break;
    }

    if (data == NULL)
        return BAG_MEMORY_ALLOCATION_FAILED;

    H5Sselect_hyperslab (filespace_id, H5S_SELECT_SET, (hsize_t *) offset, NULL, count, NULL);

    /*! xfer params */
    if (xfer == DISABLE_STRIP_MINING)
    {
        type_size = H5Tget_size(datatype_id);
        size = H5Sget_simple_extent_npoints(filespace_id) * type_size;
        xfer_plist = H5Pcreate (H5P_DATASET_XFER);
        H5Pset_buffer(xfer_plist, size, NULL, NULL);
        xfer = xfer_plist;
    }

    /*! perform read_or_write on hyperslab */
    if (read_or_write == READ_BAG)
        status = H5Dread (dataset_id, datatype_id, memspace_id, filespace_id, 
                          xfer, data);
    else if (read_or_write == WRITE_BAG)
        status = H5Dwrite (dataset_id, datatype_id, memspace_id, filespace_id, 
                           xfer, data);

    check_hdf_status();

    /*! did what we came to do, now close up */
    if (xfer_plist >= 0)
    {
        status = H5Pclose (xfer_plist);
        check_hdf_status();
    }

    if (status < 0)
        return BAG_HDF_INTERNAL_ERROR;
    else
        return BAG_SUCCESS;
}

/****************************************************************************************/
/*! \brief  bagAllocArray, for 2dimensional access to the surface, this function simplifies allocation of memory structures for the user
 ****************************************************************************************
 *
 * The array functions manage private memory within the \a bagHandle
 * which is used to buffer data from the surface datasets.
 * The user is able to access this data from the \a bagData's
 * 2D \a **elevation and \a **uncertainty pointers.
 * 
 *   Description :
 *       Set up the pointers for the 2d array in \a bagData
 *       to point to the correct offset within the actual
 *       1d array being used in \a bagHandle for contiguous HDF I/O.
 *
 *      Note: \a bagAllocArray is only suitable for Region and complete Dataset access,
 *            whereas Node and Row access functions demand the user to supply their own
 *            allocated external memory for access to the \a hnd bagHandle.
 *
 *  \param  hnd        External reference to the private \a bagHandle object
 *  \param  start_row  Starting Row offset within \a bag to access
 *                     start_row must be less than end_row and greater than or equal to zero.
 *  \param  start_col  Starting Col offset within \a bag to access
                       start_col must be less than end_col and greater than or equal to zero.
 *  \param  end_row    Ending row offset within \a bag to access
 *                     end_row must be greater than start_row and 
 *                     within the \a nrows extent of the \a bagDef
 *  \param  end_col    Ending col offset within \a bag to access
 *                     end_col must be greater than start_col and
 *                     within the \a ncols extent of the \a bagDef
 *  \param  type       Indicates which data surface type to access, element of \a BAG_SURFACE_PARAMS
 *
 *  \return : \li On success, \a bagError is set to \a BAG_SUCCESS.
 *            \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS.
 * 
 ********************************************************************/
bagError bagAllocArray (bagHandle hnd, u32 start_row, u32 start_col, 
                        u32 end_row, u32 end_col, s32 type)
{
    u32 i, srow, scol;
    bagError status;
    u32 elementSize = sizeof(f32);
    u32 elementPointerSize = sizeof(f32*);
    u32 numCol = end_col - start_col + 1;
    u32 numRow = end_row - start_row + 1;
    u32 numElements = numRow * numCol;
    char *dataBuffer = NULL;

    if (hnd == NULL)
        return BAG_INVALID_BAG_HANDLE;

    if (type >= BAG_OPT_SURFACE_LIMIT)
        return  BAG_INVALID_FUNCTION_ARGUMENT;

    if (type > Uncertainty)
    {
        srow=hnd->bag.opt[type].nrows;
        scol=hnd->bag.opt[type].ncols;
    }
    else
    {
        srow=hnd->bag.def.nrows;
        scol=hnd->bag.def.ncols;
    }

    if (end_col >= scol ||
        end_row >= srow ||
        start_row > end_row || 
        start_col > end_col)
    {
        fprintf(stderr, "Internal error, bad parameters given to access surface extents! Aborting...\n");
        fprintf(stderr, "\tCannot access region, %d-%d / %d-%d, with surface extents 0-%d / 0-%d\n",
                start_row, start_col, end_row, end_col, srow, scol);
        fflush(stderr);
        return BAG_HDF_ACCESS_EXTENTS_ERROR;
    }

    /*! free previously allocated buffer if present */
    status = bagFreeArray (hnd, type);
    if (status != BAG_SUCCESS)
        return status;

    switch (type) 
    {
    case Uncertainty:
        
        /* alloc the contiguous 1d array */
        hnd->uncertaintyArray = (f32 *)calloc(numElements, sizeof(f32));
        if (hnd->uncertaintyArray == NULL)
            return BAG_MEMORY_ALLOCATION_FAILED;

        /*! alloc the array of pointers to floats */
        hnd->bag.uncertainty = (f32 **)calloc (numRow, sizeof (f32 *));
        if (hnd->bag.uncertainty == NULL)
            return BAG_MEMORY_ALLOCATION_FAILED;

        /*! now the 2d is tied to the contiguous 1d */
        dataBuffer = (char *)hnd->uncertaintyArray;
        for (i = 0; i < numRow; ++i)
        {
            hnd->bag.uncertainty[i] = dataBuffer + (i * numCol * elementSize);
        }

        /*! init data to NULL values */
        for (i = 0; i < numElements; i++)
            hnd->uncertaintyArray [i] = NULL_UNCERTAINTY;
        break;

    case Elevation:
        
        /*! alloc the contiguous 1d array */
        hnd->elevationArray = (f32 *)calloc(numElements, sizeof(f32));
        if (hnd->elevationArray == NULL)
            return BAG_MEMORY_ALLOCATION_FAILED;

        /*! alloc the array of pointers to floats */
        hnd->bag.elevation = (f32 **)calloc (numRow, sizeof (f32 *));
        if (hnd->bag.elevation == NULL)
            return BAG_MEMORY_ALLOCATION_FAILED;

        /*! now the 2d is tied to the contiguous 1d */
        dataBuffer = (char *)hnd->elevationArray;
        for (i = 0; i < numRow; ++i)
        {
            hnd->bag.elevation[i] = dataBuffer + (i * numCol * elementSize);
        }

        /*! init data to NULL values */
        for (i = 0; i < numElements; i++)
            hnd->elevationArray [i] = NULL_ELEVATION;
        break;

    default:
        if (type == Node_Group)
        {
            elementSize = sizeof(bagOptNodeGroup);
            elementPointerSize = sizeof(bagOptNodeGroup *);
        }
        else if (type == Elevation_Solution_Group)
        {
            elementSize = sizeof(bagOptElevationSolutionGroup);
            elementPointerSize = sizeof(bagOptElevationSolutionGroup *);
        }
        else
        {
            elementSize = sizeof(f32);
            elementPointerSize = sizeof(f32 *);
        }

        /* alloc the contiguous 1d array */
        hnd->dataArray[type] = (f32 *)calloc((numCol)*numRow, elementSize);
        if (hnd->dataArray[type] == NULL)
            return BAG_MEMORY_ALLOCATION_FAILED;

        /*! alloc the array of pointers to floats */
        hnd->bag.opt[type].data = (f32 **)calloc(numRow, elementPointerSize);
        if (hnd->bag.opt[type].data == NULL)
            return BAG_MEMORY_ALLOCATION_FAILED;

        /*! now the 2d is tied to the contiguous 1d */
        dataBuffer = (char *)hnd->dataArray[type];
        for (i = 0; i < numRow; ++i)
        {
            hnd->bag.opt[type].data[i] = dataBuffer + (i * numCol * elementSize);
        }

        /*! init data to NULL values */
        for (i = 0; i < numElements; i++)
        {
            if (type == Node_Group)
            {
                ((bagOptNodeGroup*)&hnd->dataArray[type][i])->hyp_strength = NULL_ELEVATION;
                ((bagOptNodeGroup*)&hnd->dataArray[type][i])->num_hypotheses = NULL_ELEVATION;
            }
            else if (type == Elevation_Solution_Group)
            {
                ((bagOptElevationSolutionGroup*)&hnd->dataArray[type][i])->shoal_elevation = NULL_ELEVATION;
                ((bagOptElevationSolutionGroup*)&hnd->dataArray[type][i])->stddev = NULL_ELEVATION;
                ((bagOptElevationSolutionGroup*)&hnd->dataArray[type][i])->num_soundings = NULL_ELEVATION;
            }
            else
            {
                hnd->dataArray[type][i] = NULL_ELEVATION;
            }
        }
        break;
    }
    
    return BAG_SUCCESS;
}

/****************************************************************************************/
/*! \brief  bagFreeArray frees the memory that may have been allocated by \a bagAllocArray
 *
 *  \param hnd   External reference to the private \a bagHandle object
 *  \param type  Indicates which data surface type to access, element of \a BAG_SURFACE_PARAMS
 *
 *  \return : \li On success, \a bagError is set to \a BAG_SUCCESS.
 *            \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS.
 * 
 ********************************************************************/
bagError bagFreeArray (bagHandle hnd, s32 type)
{
    if (hnd == NULL)
        return BAG_INVALID_BAG_HANDLE;

    if (type >= BAG_OPT_SURFACE_LIMIT)
        return  BAG_INVALID_FUNCTION_ARGUMENT;

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
        if (hnd->bag.opt[type].data != NULL)
        {
            free (hnd->bag.opt[type].data);
            hnd->bag.opt[type].data = NULL;
        
            free (hnd->dataArray[type]);
            hnd->dataArray[type] = NULL;
        }
        break;
    }

    return BAG_SUCCESS;
}

/****************************************************************************************/
/*!
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
    u32  xlen, ylen, i;
    f64  xorig, yorig, xoff, yoff;

    if (bagHandle == NULL)
        return BAG_INVALID_BAG_HANDLE;

    if (c2 >= bagHandle->bag.def.ncols ||
        r2 >= bagHandle->bag.def.nrows ||
        r1 > r2 || 
        c1 > c2)
    {
        fprintf(stderr, "Internal error, bad parameters given to access surface extents! Aborting...\n");
        fprintf(stderr, "\tCannot access region, %d-%d / %d-%d, with surface extents 0-%d / 0-%d\n",
                r1, c1, r2, c2, bagHandle->bag.def.nrows, bagHandle->bag.def.ncols);
        fflush(stderr);
        return BAG_HDF_ACCESS_EXTENTS_ERROR;
    }

    /*! first step is to figure out how much memory to alloc for the arrays */
    ylen  = (r2 -r1) + 1;
    xlen  = (c2 -c1) + 1;
    xorig = bagHandle->bag.def.swCornerX;
    yorig = bagHandle->bag.def.swCornerY;
    xoff  = bagHandle->bag.def.nodeSpacingX;
    yoff  = bagHandle->bag.def.nodeSpacingY;

    /*! alloc */
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


/****************************************************************************************/
/*! \brief bagReadXMLStream populates the \a bagDef metadata field with a string derived from the Metadata dataset
 *
 * \param bagHandle  External reference to the private \a bagHandle object
 * \return \li On success, \a bagError is set to \a BAG_SUCCESS
 *         \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS
 ****************************************************************************************/
bagError bagReadXMLStream (bagHandle bagHandle)
{
    return bagAlignXMLStream (bagHandle, READ_BAG);
}

/****************************************************************************************/
/*! \brief bagWriteXMLStream stores the string at \a bagDef's metadata field into the Metadata dataset
 *
 * \param bagHandle  External reference to the private \a bagHandle object
 * \return \li On success, \a bagError is set to \a BAG_SUCCESS
 *         \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS
 ****************************************************************************************/
bagError bagWriteXMLStream (bagHandle bagHandle)
{
    return bagAlignXMLStream (bagHandle, WRITE_BAG);
}

/****************************************************************************************/
/*! \brief bagAlignXMLStream
 *
 *  Special case had to be made so that metadata is a single dimension - 
 *  and also it has no max size and can be extended as much as we'd like.
 *
 ****************************************************************************************/
bagError bagAlignXMLStream (bagHandle hnd, s32 read_or_write)
{
    herr_t      status;
    s32         rank;
    u32         nct;
    void       *data;
    hsize_t	  extend[1];

    /*! hyperslab selection parameters */
    hsize_t	  count[1];
    hssize_t	  offset[1];


    if (hnd == NULL)
        return BAG_INVALID_BAG_HANDLE;

    rank = H5Sget_simple_extent_dims (hnd->mta_filespace_id, count, NULL);
    
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
      extend[0] = strlen((char *)hnd->bag.metadata);
    }
    
    /*! 
       Adding an extra space for the null terminator to be guaranteed
       on disk with the dataset. Webb McDonald -- Wed Mar  1 12:51:24 2006
    */
    if (read_or_write == WRITE_BAG)
        extend[0]++;

    if (extend[0] > XML_METADATA_MAX_LENGTH)
        return BAG_METADTA_BUFFER_EXCEEDED;

    data = hnd->bag.metadata;

    if (extend[0] > count[0])
    {
        /*! let the metadata grow if it needs to */
        status = H5Dextend (hnd->mta_dataset_id, extend);
        check_hdf_status();
        
        /*! Close and reopen below in case we extend the metadata XML string */
        status = H5Sclose (hnd->mta_filespace_id);
        check_hdf_status();
        
        /*! Prepare the dataspaces */
        if ((hnd->mta_filespace_id = H5Dget_space(hnd->mta_dataset_id)) < 0)
        {
            H5Tclose (hnd->mta_datatype_id);
            H5Dclose (hnd->mta_dataset_id);
            hnd->mta_dataset_id = -1;
            hnd->mta_datatype_id = -1;
            return BAG_HDF_DATASPACE_CORRUPTED;
        }
    }
    rank = H5Sget_simple_extent_dims (hnd->mta_filespace_id, count, NULL);
        
    if (hnd->mta_memspace_id >= 0)
        nct = (u32) H5Sget_select_npoints (hnd->mta_memspace_id);

    if (hnd->mta_memspace_id < 0 || nct != count[0])
    {
        if (hnd->mta_memspace_id >= 0)
        {
            status = H5Sclose (hnd->mta_memspace_id);
            check_hdf_status();
        }

        /*! define the hyperslab parameters */
        if ((hnd->mta_memspace_id = H5Screate_simple (1, count, NULL)) < 0)
            return BAG_HDF_CREATE_DATASPACE_FAILURE;
    }

    offset[0] = 0;

    status = H5Sselect_hyperslab (hnd->mta_filespace_id, H5S_SELECT_SET, (hsize_t *) offset, NULL, count, NULL);
    check_hdf_status();

    /*! perform read_or_write on hyperslab */
    if (read_or_write == READ_BAG)
        status = H5Dread (hnd->mta_dataset_id, hnd->mta_datatype_id, hnd->mta_memspace_id, hnd->mta_filespace_id, 
                          H5P_DEFAULT, data);
    else if (read_or_write == WRITE_BAG)
        status = H5Dwrite (hnd->mta_dataset_id, hnd->mta_datatype_id, hnd->mta_memspace_id, hnd->mta_filespace_id, 
                           H5P_DEFAULT, data);

    check_hdf_status();

    return BAG_SUCCESS;
}


/****************************************************************************************/
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
 ****************************************************************************************/
bagError bagReadSurfaceDims (bagHandle hnd, hsize_t *max_dims)
{
    herr_t   status;
    s32      rank;
    hid_t    dataspace_id;
    
    if (hnd == NULL)
        return BAG_INVALID_BAG_HANDLE;

    /*! This dataspace is a one time throwaway */
    dataspace_id = H5Dget_space(hnd->elv_dataset_id);

    if (H5Sis_simple(dataspace_id))
    {
        rank = H5Sget_simple_extent_ndims(dataspace_id);
        rank = H5Sget_simple_extent_dims(dataspace_id, max_dims, NULL);

        /*! seems like a reasonable requirement for BAG compatibility now? */
        if (rank != RANK)
        {
            fprintf(stderr, "Error - The BAG is corrupted.  The rank of this dataset is said to be = %d, when it should be = %d. \n",
                    rank, RANK);
            fflush(stderr);
            return BAG_HDF_RANK_INCOMPATIBLE;
        }
    }
    else
        return BAG_HDF_DATASPACE_CORRUPTED;
    
    status = H5Sclose(dataspace_id);
    check_hdf_status();
    
    return BAG_SUCCESS;
}

/****************************************************************************************/
/*! \brief  bagGetGridDimensions
 *
 * Description:
 *     This function simply stores grid dims into slot at *rows and *cols.
 * 
 *  \param    hnd    - pointer to the structure which ultimately contains the bag
 *  \param   *rows   - pointer where number of rows of the surfaces will be assigned
 *  \param   *cols   - pointer where number of cols of the surfaces will be assigned
 *
 * \return On success, a value of zero is returned.  On failure a value of -1 is returned.  
 *
 ****************************************************************************************/

bagError bagGetGridDimensions(bagHandle hnd, u32 *rows, u32 *cols) 
{
    if (hnd == NULL)
        return BAG_INVALID_BAG_HANDLE;

    *rows = hnd->bag.def.nrows;
    *cols = hnd->bag.def.ncols;

    return BAG_SUCCESS;
}


/****************************************************************************************/
/*! \brief  bagUpdateSurface
 *
 * Description:
 *     So far this function just calls bagUpdateMinMax on the surface indicated by \a type
 * 
 *  \param    hnd    - pointer to the structure which ultimately contains the bag
 *  \param    type   - Indicates which data surface type to access, element of \a BAG_SURFACE_PARAMS
 *
 * \return On success, a value of zero is returned.  On failure a value of -1 is returned.  
 *
 ****************************************************************************************/
bagError bagUpdateSurface (bagHandle hnd, u32 type)
{
    herr_t   status;

    if (hnd == NULL)
        return BAG_INVALID_BAG_HANDLE;

    status = bagUpdateMinMax (hnd, type);
    check_hdf_status();

    return BAG_SUCCESS;
}

/****************************************************************************************/
/*! \brief  bagUpdateMinMax
 *
 * Description:
 *     Examines all the data in the particular surface and updates the min and max value
 *     attributes with the minimum and maximum values found in the dataset, excluding any
 *     NULL elements.
 * 
 *  \param    hnd    - pointer to the structure which ultimately contains the bag
 *  \param    type   - Indicates which data surface type to access, element of \a BAG_SURFACE_PARAMS
 *
 * \return On success, a value of zero is returned.  On failure a value of -1 is returned.  
 ****************************************************************************************/
bagError bagUpdateMinMax (bagHandle hnd, u32 type)
{
    herr_t status;
    u32    i, j;
    u8    *max_name, *min_name;
    hid_t  dataset_id;
    f32   *min_tmp, *max_tmp, **surface_array, *omax, *omin, null_val;


    if (hnd == NULL)
        return BAG_INVALID_BAG_HANDLE;

    switch (type)
    {
    case Elevation:
        omin          = &hnd->bag.min_elevation;
        omax          = &hnd->bag.max_elevation;
        dataset_id    = hnd->elv_dataset_id;
        max_name      = (u8 *)MAX_ELEVATION_NAME;
        min_name      = (u8 *)MIN_ELEVATION_NAME;
        null_val      = NULL_ELEVATION;
        surface_array = &hnd->elevationArray;
        break;

    case Uncertainty:
        omin          = &hnd->bag.min_uncertainty;
        omax          = &hnd->bag.max_uncertainty;
        dataset_id    = hnd->unc_dataset_id;
        max_name      = (u8 *)MAX_UNCERTAINTY_NAME;
        min_name      = (u8 *)MIN_UNCERTAINTY_NAME;
        null_val      = NULL_UNCERTAINTY;
        surface_array = &hnd->uncertaintyArray;
        break;
    default:
        return BAG_HDF_TYPE_NOT_FOUND;
        break;
    }
    

    if (dataset_id < 0)
        return BAG_HDF_DATASET_OPEN_FAILURE; 

    min_tmp = calloc (1, sizeof (f32));
    max_tmp = calloc (1, sizeof (f32));
    if (min_tmp == NULL || max_tmp == NULL)
        return BAG_MEMORY_ALLOCATION_FAILED;

    *max_tmp = null_val;
    *min_tmp = null_val;

    for (i=0; i < hnd->bag.def.nrows; i++)
    {
		
		bagReadRegion (hnd, i, 0, i, hnd->bag.def.ncols-1, type);
			
		for (j=0; j < hnd->bag.def.ncols; j++)
        {
            if ((* surface_array)[j] != null_val)
            {
                if (*max_tmp == null_val)
                    *max_tmp = (* surface_array)[j];
                if (*min_tmp == null_val)
                    *min_tmp = (* surface_array)[j];
                if ((* surface_array)[j] > *max_tmp)
                {
                    *max_tmp = (* surface_array)[j];
                    /* fprintf(stdout, "ex: %f\n", (* surface_array)[j]); */

                }
                if ((* surface_array)[j] < *min_tmp)
                {
                    /* fprintf(stdout, "ex: %f\n", (* surface_array)[j]); */
                    *min_tmp = (* surface_array)[j];
                }
            }
        }
    }

	if (*max_tmp != null_val)
	{
        *omax = *max_tmp;
		status = bagWriteAttribute (hnd, dataset_id, max_name, max_tmp);
		check_hdf_status();
	}
	if (*min_tmp != null_val)
	{ 
       *omin = *min_tmp;
		status = bagWriteAttribute (hnd, dataset_id, min_name, min_tmp);
		check_hdf_status();
	}
	
    free (min_tmp);
    free (max_tmp);

    /*! free previously allocated buffer if present */
    status = bagFreeArray (hnd, type);
    if (status != BAG_SUCCESS)
        return status;

    return BAG_SUCCESS;
}

