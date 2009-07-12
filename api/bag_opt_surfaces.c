/*! \file bag_opt_datasets.c
 * \brief This module contains functions for accessing the optional surface datasets and the XML metadata.
 ********************************************************************
 *
 * Module Name : bag_opt_datasets.c
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
 * Michelle Russell -- Mon Jan 28, 2008
 *   Added new file to handle optional datasets being written to the BAG.  
 *
 * Classification : Unclassified
 *
 * References : 
 *
 ********************************************************************/

#include "bag_private.h"

/****************************************************************************************/
/*! \brief bagCreateOptionalDataset creates a dataset for an optional bag surface
 *
 *  \param file_name   A string provides the filesystem name of the BAG
 *	\param bagDataOpt Pointer to memory holding the data for the optional dataset
 *  \param type       Indicates which data surface type to access, element of \a BAG_SURFACE_PARAMS
 *
 *  \return : \li On success, \a bagError is set to \a BAG_SUCCESS.
 *            \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS.
 * 
 ********************************************************************/
bagError bagCreateOptionalDataset (bagHandle bag_hnd, bagHandle_opt *bag_hnd_opt, bagDataOpt *data, s32 type)
{
	
	hsize_t      dims[RANK];
    hsize_t      dim_init[1]; 
    hsize_t      dim_max[1];
	hid_t        file_id = bag_hnd->file_id;
    hid_t        dataset_id;
    hid_t        dataspace_id;
    hid_t        datatype_id; 
    hid_t        bagGroupID; 
    hid_t        plist_id;
	f32          null = data->datanull;
	herr_t		 status;
	
	*bag_hnd_opt = (bagHandle_opt) calloc (1, sizeof (struct _t_bagHandle_opt));
    if (*bag_hnd_opt == (bagHandle_opt) NULL)
    {
        return (BAG_MEMORY_ALLOCATION_FAILED);
    }

	 memcpy (&((*bag_hnd_opt)->bag.def), &data->def, sizeof (bagDef));

	   /*! init all the HDF structs to -1 */
    (* bag_hnd_opt)->memspace_id  = 
    (* bag_hnd_opt)->dataset_id   =  
    (* bag_hnd_opt)->filespace_id = 
    (* bag_hnd_opt)->datatype_id  = -1;

    (* bag_hnd_opt)->dataArray    = 
    (* bag_hnd_opt)->bag.opt_data = NULL;

	/*! Create the mandatory \a elevation dataset */
    dims[0] = data->def.nrows;
    dims[1] = data->def.ncols;

    if ((dataspace_id = H5Screate_simple(RANK, dims, NULL)) < 0)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_DATASPACE_FAILURE);
    }

	if ((datatype_id = H5Tcopy(data->datatype)) < 0)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_TYPE_COPY_FAILURE);
    }

    if ((status = H5Tset_order(datatype_id, H5T_ORDER_LE)) < 0)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_SET_PROPERTY_FAILURE);
    }

    if ((plist_id = H5Pcreate(H5P_DATASET_CREATE)) < 0)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_PROPERTY_CLASS_FAILURE);
    }

    status = H5Pset_fill_time  (plist_id, H5D_FILL_TIME_ALLOC);
	status = H5Pset_fill_value (plist_id, datatype_id, &null);
    check_hdf_status();

	switch (type)
	{
		case Nominal_Elevation:
			if ((dataset_id = H5Dcreate(file_id, NOMINAL_ELEVATION_PATH, datatype_id, dataspace_id, plist_id)) < 0)
			{    
				status = H5Fclose (file_id);
				return (BAG_HDF_CREATE_GROUP_FAILURE);
			}
			break;
		
		default:
        return BAG_HDF_TYPE_NOT_FOUND;
        break;
	}


    /*! Add the attributes to the \a elevation dataset */
    if ((status = bagCreateAttribute (bag_hnd, dataset_id, (u8 *)"min_value", sizeof(f32), BAG_ATTR_F32)) != BAG_SUCCESS)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
    }
	if ((status = bagWriteAttribute (bag_hnd, dataset_id, (u8 *)"min_value", (void *) &(data->min) )) != BAG_SUCCESS)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
    }
    if ((status = bagCreateAttribute (bag_hnd, dataset_id, (u8 *)"max_value", sizeof(f32), BAG_ATTR_F32)) != BAG_SUCCESS)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
    }
    if ((status = bagWriteAttribute (bag_hnd, dataset_id, (u8 *)"max_value", (void *) &(data->max) )) != BAG_SUCCESS)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
    }

	/* get the file space id */
	(* bag_hnd_opt)->filespace_id = H5Dget_space(dataset_id);
    if ((* bag_hnd_opt)->filespace_id < 0)
    {
        return BAG_HDF_DATASPACE_CORRUPTED;
    }

	(*bag_hnd_opt)->file_id = bag_hnd->file_id;
	(*bag_hnd_opt)->dataset_id = dataset_id;
	(*bag_hnd_opt)->datatype_id = datatype_id;
  /*  status = H5Dclose (dataset_id);
    status = H5Sclose (dataspace_id);
    status = H5Tclose (datatype_id);  */

	return BAG_SUCCESS;
}

/********************************************************************/
/*! \brief bagGetOptDatasets
 *
 * Description : 
 *     This function opens a BAG file and finds the number of optional datasets along with a 
 *		list of those found.
 *
 * \param *bag_handle   bag_handle will be set to the allocated \a bagHandle
 *                      private object for subsequent external reference
 *                      by the caller.
 * \param *file_name    A string provides the filesystem name of the BAG
 * \param num_opt_datasets    Number of optional datasets in bag file
 * \param opt_dataset_names	  List of BAG_SURFACE_PARAMS loaded in the BAG.
 *
 * \return \li On success, \a bagError is set to \a BAG_SUCCESS
 *         \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS
 *
 ********************************************************************/
bagError bagGetOptDatasets(bagHandle_opt *bag_handle_opt,const u8 *file_name, s32 *num_opt_datasets, 
							int opt_dataset_names[10])
{
    bagError     status;
    hid_t        dataset_id;
       
    *bag_handle_opt = (bagHandle_opt) calloc (1, sizeof (struct _t_bagHandle_opt));
    if (*bag_handle_opt == (bagHandle_opt) NULL)
    {
        return (BAG_MEMORY_ALLOCATION_FAILED);
    }

    /* attempt to open the bag */
    if (((* bag_handle_opt)->file_id = H5Fopen((u8 *)file_name, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0)
    {
       return (BAG_HDF_FILE_OPEN_FAILURE);
    }
   
     
    if (((* bag_handle_opt)->bagGroupID = H5Gopen ((* bag_handle_opt)->file_id, ROOT_PATH)) < 0)
    {
        H5Fclose ((* bag_handle_opt)->file_id);
        return (BAG_HDF_GROUP_OPEN_FAILURE);
    }
    
  	*num_opt_datasets = 0;

    /*!  Try to open the Nominal Elevation dataset */
    dataset_id = H5Dopen((* bag_handle_opt)->file_id, NOMINAL_ELEVATION_PATH);
    if (dataset_id > 0)
	{
		opt_dataset_names[*num_opt_datasets] = Nominal_Elevation;
		++(*num_opt_datasets);
	}


	/*!  Try to open the num hypothesis dataset */
    dataset_id = H5Dopen((* bag_handle_opt)->file_id, NUM_HYPOTHESES_PATH);
    if (dataset_id > 0)
    {
		opt_dataset_names[*num_opt_datasets] = Num_Hypotheses;
		++(*num_opt_datasets);
	}

	/*!  Try to open the average dataset */
    dataset_id = H5Dopen((* bag_handle_opt)->file_id, AVERAGE_PATH);
    if (dataset_id > 0)
    {
		opt_dataset_names[*num_opt_datasets] = Average;
		++(*num_opt_datasets);
	}

	/*!  Try to open the standard deviation dataset */
    dataset_id = H5Dopen((* bag_handle_opt)->file_id, STANDARD_DEV_PATH);
    if (dataset_id > 0)
    {
		opt_dataset_names[*num_opt_datasets] = Standard_Dev;
		++num_opt_datasets;
	}
	
    
    return (BAG_SUCCESS);
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
 *       1d array being used in \a bagHandle_opt for contiguous HDF I/O.
 *
 *      Note: \a bagAllocOptArray is only suitable for Region and complete Dataset access,
 *            whereas Node and Row access functions demand the user to supply their own
 *            allocated external memory for access to the \a hnd bagHandle_opt.
 *
 *  \param  hnd        External reference to the private \a bagHandle_opt object
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
 *
 *  \return : \li On success, \a bagError is set to \a BAG_SUCCESS.
 *            \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS.
 * 
 ********************************************************************/
bagError bagAllocOptArray (bagHandle_opt hnd, u32 start_row, u32 start_col, 
                        u32 end_row, u32 end_col)
{
    u32 i;
    bagError status;

    if (hnd == NULL)
        return BAG_INVALID_BAG_HANDLE;

    if (start_col < 0 || end_col < 0 || end_col >= hnd->bag.def.ncols ||
        start_row < 0 || end_row < 0 || end_row >= hnd->bag.def.nrows ||
        start_row > end_row || 
        start_col > end_col)
    {
        fprintf(stderr, "Internal error, bad parameters given to access surface extents! Aborting...\n");
        fprintf(stderr, "\tCannot access region, %d-%d / %d-%d, with surface extents 0-%d / 0-%d\n",
                start_row, start_col, end_row, end_col, hnd->bag.def.nrows, hnd->bag.def.ncols);
        fflush(stderr);
        return BAG_HDF_ACCESS_EXTENTS_ERROR;
    }

    /*! free previously allocated buffer if present */
    status = bagFreeOptArray (hnd);
    if (status != BAG_SUCCESS)
        return status;

    
        /* alloc the contiguous 1d array */
        hnd->dataArray = (f32 *)calloc ((end_col-start_col+1)*(end_row-start_row+1), sizeof(f32));
        if (hnd->dataArray == NULL)
            return BAG_MEMORY_ALLOCATION_FAILED;

        /*! alloc the array of pointers to floats */
        hnd->bag.opt_data = (f32 **)calloc ((end_row-start_row+1), sizeof (f32 *));
        if (hnd->bag.opt_data == NULL)
            return BAG_MEMORY_ALLOCATION_FAILED;

        /*! now the 2d is tied to the contiguous 1d */
        hnd->bag.opt_data[0] = hnd->dataArray;

        /*! set the rest of the pointers */
        for (i=1; i < (end_row-start_row+1); i++)
        {
            hnd->bag.opt_data[i] = hnd->bag.opt_data[i-1] + (end_col-start_col+1);
        }

        /*! init data to NULL values */
        for (i=0; i < ((end_col-start_col+1)*(end_row-start_row+1)); i++)
            hnd->dataArray [i] = hnd->bag.datanull;
       
    
    return BAG_SUCCESS;
}

/****************************************************************************************
 *
 * \brief This is only slightly different from bagAlignRegion. 
 *  -The out of range conditions are based on a point, not a region
 *  -The dataspaces are just 1dimension
 *  -Uses element select(prone to memory leaks though), instead of hyperslab 
 *
 ****************************************************************************************/
bagError bagAlignOptNode (bagHandle bagHandle, bagHandle_opt bagHandle_opt, u32 row, u32 col, s32 type, void *data, s32 read_or_write)
{
    herr_t         status;
    hsize_t        snode[2] = {1,1};
    hssize_t	     offset[1][2];
    hid_t          dataset_id,
                   memspace_id,
                   datatype_id,
                   filespace_id;


    if (bagHandle == NULL)
        return BAG_INVALID_BAG_HANDLE;

    /*! some error checking on surfaces extents */
    if (row < 0 || row >= bagHandle->bag.def.nrows || col >= bagHandle->bag.def.ncols || col < 0)
    {
        fprintf(stderr, "Fail to access out of bounds row/col = %d/%d out of possible size: %d/%d. Aborting\n",
                row, col, bagHandle->bag.def.nrows, bagHandle->bag.def.ncols);
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
   
	case Nominal_Elevation:
        if (bagHandle_opt->memspace_id >= 0)
        {
            nct = (u32)H5Sget_select_npoints (bagHandle_opt->memspace_id);
        }

        if (bagHandle_opt->memspace_id < 0 || nct != snode[1])
        {
            if (bagHandle_opt->memspace_id >= 0)
            {
                 status = H5Sclose (bagHandle_opt->memspace_id);
                 check_hdf_status();
            }

            /*! Create space in memory */
            bagHandle_opt->memspace_id = H5Screate_simple(RANK, snode, NULL);
            if (bagHandle_opt->memspace_id < 0)
            {
                return BAG_HDF_DATASPACE_CORRUPTED;
            }
        }

        datatype_id  = bagHandle_opt->datatype_id;
        memspace_id  = bagHandle_opt->memspace_id;
        filespace_id = bagHandle_opt->filespace_id;
        dataset_id   = bagHandle_opt->dataset_id;
        break;
    default:
        return BAG_HDF_TYPE_NOT_FOUND;
        break;
    }

    /*! Select grid cell within dataset file space. */
    status = H5Sselect_elements (filespace_id, H5S_SELECT_SET, 1, (const hsize_t **)offset);
    check_hdf_status();
    
    /*!  perform read_or_write on element */
    if (read_or_write == READ_BAG)
        status = H5Dread (dataset_id, datatype_id, memspace_id, filespace_id, 
                          H5P_DEFAULT, data);
    else if (read_or_write == WRITE_BAG)
        status = H5Dwrite (dataset_id, datatype_id, memspace_id, filespace_id, 
                           H5P_DEFAULT, data);
    else
        ; /*!  error? */
    check_hdf_status();

    if (status < 0)
        return BAG_HDF_INTERNAL_ERROR;
    else
        return BAG_SUCCESS;
}

/****************************************************************************************/
bagError bagAlignOptRow (bagHandle bagHandle, bagHandle_opt bagHandle_opt, u32 row, u32 start_col, 
                      u32 end_col, s32 type, s32 read_or_write, void *data)
{
    herr_t      status = 0;

    /* hyperslab selection parameters */
    hsize_t	  count[10];
    hssize_t	  offset[10];
    hid_t       memspace_id, 
                datatype_id,
                dataset_id,
                filespace_id;


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
   
       
	case Nominal_Elevation: 
        if (bagHandle_opt->memspace_id >= 0)
        {
            nct = (u32) H5Sget_select_npoints (bagHandle_opt->memspace_id);
        }
        
        if (bagHandle_opt->memspace_id < 0 || nct != count[1])
        {
            if (bagHandle_opt->memspace_id >= 0)
            {
                status = H5Sclose (bagHandle_opt->memspace_id);
                check_hdf_status();
            }
            
            /*! Create space in memory */
            bagHandle_opt->memspace_id = H5Screate_simple(RANK, count, NULL);
            if (bagHandle_opt->memspace_id < 0)
                return BAG_HDF_DATASPACE_CORRUPTED;
        }
        
        datatype_id  = bagHandle_opt->datatype_id;
        memspace_id  = bagHandle_opt->memspace_id;
        filespace_id = bagHandle_opt->filespace_id;
        dataset_id   = bagHandle_opt->dataset_id;
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
    else
        ; /* error? */
    check_hdf_status();

    if (status < 0)
        return BAG_HDF_INTERNAL_ERROR;
    else
        return BAG_SUCCESS;
}


/****************************************************************************************/
/*! I made a choice so far in this function - notice that void \a *data is just a temporary
 *  memory is either already allocated to the arrays in \a bagHandle bagHandle,
 *  or gets allocated here for a read.  It is up to the caller to free this memory later!
 *  This is achieved through the \a bagFreeArray function!
 *
 ****************************************************************************************/
bagError bagAlignOptRegion (bagHandle bagHandle, bagHandle_opt bagOptHandle, u32 start_row, u32 start_col, 
                    u32 end_row, u32 end_col, s32 type, s32 read_or_write, hid_t xfer)
{
    herr_t      status = 0;

    /* hyperslab selection parameters */
    hsize_t	  count[10];
    hssize_t	  offset[10];
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
    

	case Nominal_Elevation:
        if (read_or_write == READ_BAG)
        {
			bagOptHandle->bag.def.nrows = bagHandle->bag.def.nrows;
			bagOptHandle->bag.def.ncols = bagHandle->bag.def.ncols;
            if (bagAllocOptArray ( bagOptHandle,  start_row,  start_col, 
                                end_row,  end_col) != BAG_SUCCESS)
                data = NULL;
            else
                data = bagOptHandle->dataArray;
        }
        else
            data = bagOptHandle->dataArray;

        if (bagOptHandle->memspace_id >= 0)
            nct = (u32) H5Sget_select_npoints (bagOptHandle->memspace_id);

        if (bagOptHandle->memspace_id < 0 || nct != (count[0] * count[1]))
        {
            if (bagOptHandle->memspace_id >= 0)
            {
                status = H5Sclose (bagOptHandle->memspace_id);
                check_hdf_status();
            }
            
            /*! Prepare the dataspaces */
            bagOptHandle->memspace_id = H5Screate_simple(RANK, count, NULL);
            if (bagOptHandle->memspace_id < 0)
                return BAG_HDF_DATASPACE_CORRUPTED;
        }

        datatype_id  = bagOptHandle->datatype_id;
        memspace_id  = bagOptHandle->memspace_id;
        filespace_id = bagOptHandle->filespace_id;
        dataset_id   = bagOptHandle->dataset_id;
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
    else
        ; /*  error? */
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

/********************************************************************/
/*! \brief : bagGetOptDataPointer
 *
 * Description : 
 *        This function is used to access the \a bagData structure from
 *        within the private object referenced by \a *bag_handle.
 * 
 * \param  bag_handle   External reference to the private \a bagHandle object
 *
 * \return : \li On success, \a bagError is set to \a BAG_SUCCESS
 *           \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS
 * 
 ********************************************************************/
bagDataOpt *bagGetOptDataPointer(bagHandle_opt bag_handle_opt)
{
    if (bag_handle_opt == NULL)
        return NULL;
    else
        return (&bag_handle_opt->bag);
}

/****************************************************************************************/
/*! \brief  bagFreeOptArray frees the memory that may have been allocated by \a bagAllocOptArray
 *
 *  \param hnd   External reference to the private \a bagHandle_opt object
 *
 *  \return : \li On success, \a bagError is set to \a BAG_SUCCESS.
 *            \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS.
 * 
 ********************************************************************/
bagError bagFreeOptArray (bagHandle_opt hnd)
{
    if (hnd == NULL)
        return BAG_INVALID_BAG_HANDLE;

    
        if (hnd->bag.opt_data != NULL)
        {
            free (hnd->bag.opt_data);
            hnd->bag.opt_data = NULL;

            free (hnd->dataArray);
            hnd->dataArray = NULL;
        }
      

    return BAG_SUCCESS;
}

/****************************************************************************************/
/*! \brief : bagWriteOptRow
 *
 * Description : 
 *     This function writes the \a row of data values for the surface parameter 
 *     specified by \a type to the specified \a row for the BAG specified by \a bagHandle.
 *		Data to be written is for an optional dataset indicated by \a bagHandle_opt.
 *     The intended usage of this function is for initial load of data into a BAG.
 *     \a *data must have sufficient space allocated externally to fulfill the request.
 * 
 * \param  bagHandle		External reference to the private \a bagHandle object
 * \param  bagHandle_opt    External reference to the private \a bagHandle_opt object
 * \param  k				Row offset within \a bag to access
 * \param  start_col		Starting col offset within \a bag row, 
 *							cannot be less than zero or greater than \a end_col
 * \param  end_col			Ending col offset within \a bag row,
 *							cannot be less than \a start_col or greater than the extent of the row.
 * \param  type				Indicates which data surface type to access, defined by enum \a BAG_SURFACE_PARAMS
 * \param *data				Pointer to memory for reading from or writing to the \a bag. Cannot be NULL!
 *
 * \return : \li On success, \a bagError is set to \a BAG_SUCCESS.
 *           \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS.
 * 
 ********************************************************************/
bagError bagWriteOptRow (bagHandle bagHandle, bagHandle_opt bagHandle_opt, u32 k, u32 start_col, u32 end_col, s32 type, void *data)
{
    return bagAlignOptRow (bagHandle, bagHandle_opt, k, start_col, end_col, type, WRITE_BAG, data);
}

/****************************************************************************************/
/*! \brief bagWriteOptRegion writes an entire buffer of data, 
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
bagError bagWriteOptRegion (bagHandle bagHandle, bagHandle_opt bagHandle_opt, u32 start_row, u32 start_col, u32 end_row, u32 end_col, s32 type)
{
    return bagAlignOptRegion (bagHandle, bagHandle_opt, start_row, start_col, end_row, end_col, type, WRITE_BAG, H5P_DEFAULT);
}

/****************************************************************************************/
/*! \brief : bagReadOptRow
 *
 * Description : 
 *     This function reads the \a row of data values for the surface parameter 
 *     specified by \a type to the specified \a row for the BAG specified by \a bagHandle.
 *		Data to be read is coming from an optional dataset indicated by \a bagHandle_opt.
 *     The calling application is required to manage the memory for the data array.
 *     \a *data must have sufficient space allocated externally to fulfill the request.
 * 
 * \param  bagHandle		External reference to the private \a bagHandle object
 * \param  bagHandle_opt	External reference to the private \a bagHandle_opt object
 * \param  k				Row offset within \a bag to access
 * \param  start_col		Starting col offset within \a bag row, 
 *							cannot be less than zero or greater than \a end_col
 * \param  end_col			Ending col offset within \a bag row,
 *							cannot be less than \a start_col 
 *							or greater than the \a ncols extent of the \a bagDef
 * \param  type				Indicates which data surface type to access, defined by enum \a BAG_SURFACE_PARAMS
 * \param *data				Pointer to memory for reading from or writing to the \a bag. Cannot be NULL!
 *
 * \return : \li On success, \a bagError is set to \a BAG_SUCCESS.
 *           \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS.
 * 
 ********************************************************************/
bagError bagReadOptRow (bagHandle bagHandle, bagHandle_opt bagHandle_opt, u32 k, u32 start_col, u32 end_col, s32 type, void *data)
{
    return bagAlignOptRow (bagHandle, bagHandle_opt, k, start_col, end_col, type, READ_BAG, data);
}

/****************************************************************************************/
/*! \brief bagReadOptRegion reads an entire buffer of data, 
 *                       defined by starting and ending coordinates, from a bag surface
 *
 *  \param bagHandle   External reference to the private \a bagHandle object
 *  \param  bagHandle_opt	External reference to the private \a bagHandle_opt object
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
bagError bagReadOptRegion (bagHandle bagHandle, bagHandle_opt bagHandle_opt, u32 start_row, u32 start_col, u32 end_row, u32 end_col, s32 type)
{
    return bagAlignOptRegion (bagHandle, bagHandle_opt, start_row, start_col,  end_row,  end_col, type, READ_BAG, H5P_DEFAULT);
}

/*! \brief : bagReadRegionPos
 *
 *  Description :
 *    Same as bagReadOptRegion, but also populates \a **x and \a **y with the positions.
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
bagError bagReadOptRegionPos (bagHandle bag, bagHandle_opt bag_opt, u32 start_row, u32 start_col, 
                           u32 end_row, u32 end_col, s32 type, f64 **x, f64 **y)
{
    bagError status;
    
    if ((status = bagFillPos (bag, start_row, start_col, end_row, end_col, x, y)) != BAG_SUCCESS)
        return status;
    return bagAlignOptRegion (bag, bag_opt, start_row, start_col, end_row, end_col, type, WRITE_BAG, H5P_DEFAULT);
}

/****************************************************************************************/
/*! \brief : bagReadOptDatasetPos
 *
 *  Description :
 *    Same as \a bagReadOptDataset, but also populates \a **x and \a **y with the positions.
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
bagError bagReadOptDatasetPos (bagHandle bagHandle, bagHandle_opt bagHandle_opt, s32 type, f64 **x, f64 **y)
{
    bagError status;
    
    if ((status = bagFillPos (bagHandle, 0, 0, bagHandle->bag.def.nrows - 1, 
                              bagHandle->bag.def.ncols - 1, x, y)) != BAG_SUCCESS)
        return status;
    return bagAlignOptRegion (bagHandle, bagHandle_opt, 0, 0, bagHandle->bag.def.nrows - 1, 
                           bagHandle->bag.def.ncols - 1, type, READ_BAG, DISABLE_STRIP_MINING);
}

/****************************************************************************************/
/*! \brief bagReadOptDataset reads an entire buffer of data from a bag surface
 *
 *  \param bagHandle  External reference to the private \a bagHandle object
 *  \param type       Indicates which data surface type to access, element of \a BAG_SURFACE_PARAMS
 *
 *  \return : \li On success, \a bagError is set to \a BAG_SUCCESS.
 *            \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS.
 * 
 ********************************************************************/
bagError bagReadOptDataset (bagHandle bagHandle,  bagHandle_opt bagHandle_opt, s32 type)
{
    return bagAlignOptRegion (bagHandle, bagHandle_opt, 0, 0, bagHandle->bag.def.nrows - 1, 
                           bagHandle->bag.def.ncols - 1, type, READ_BAG, DISABLE_STRIP_MINING);
}

/****************************************************************************************/
/*! \brief bagWriteOptDataset writes an entire buffer of data to a bag surface
 *
 *  \param bagHandle  External reference to the private \a bagHandle object
 *  \param type       Indicates which data surface type to access, element of \a BAG_SURFACE_PARAMS
 *
 *  \return : \li On success, \a bagError is set to \a BAG_SUCCESS.
 *            \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS.
 * 
 ********************************************************************/
bagError bagWriteOptDataset (bagHandle bagHandle, bagHandle_opt bagHandle_opt, s32 type)
{
    return bagAlignOptRegion (bagHandle, bagHandle_opt, 0, 0, bagHandle->bag.def.nrows - 1, 
                           bagHandle->bag.def.ncols - 1, type, WRITE_BAG, DISABLE_STRIP_MINING);
}


/****************************************************************************************/
/*! \brief : bagReadOptRowPos
 *
 *  Description :
 *    Same as bagReadOptRow, but also populates \a **x and \a **y with the positions,
 *    based on the coordinate system of the \a bag. Y will just have one trivial value, of course.
 *
 * \param  bag         External reference to the private \a bagHandle object
 * \param  bagHandle_opt	External reference to the private \a bagHandle_opt object
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
bagError bagReadOptRowPos (bagHandle bag, bagHandle_opt bagHandle_opt, u32 row, u32 start_col, u32 end_col, s32 type, 
                        void *data, f64 **x, f64 **y)
{
    bagError status;
    
    if ((status = bagFillPos (bag, row, start_col, row, end_col, x, y)) != BAG_SUCCESS)
        return status;
    return bagAlignOptRow (bag, bagHandle_opt, row, start_col, end_col, type, READ_BAG, data);
}

/****************************************************************************************
 * 
 * Read and write indiviidual nodes of a surface
 *
 ****************************************************************************************/
/*! \brief : bagWriteOptNode
 *
 * Description : 
 *        Write individual nodes of a surface
 * 
 * \param  bag   External reference to the private \a bagHandle object
 * \param  bagHandle_opt External reference to the private \a bagHandle_opt object
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

bagError bagWriteOptNode (bagHandle bag, bagHandle_opt bagHandle_opt, u32 row, u32 col, s32 type, void *data)
{
    return bagAlignOptNode (bag, bagHandle_opt, row, col, type, data, WRITE_BAG);
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
bagError bagReadOptNode (bagHandle bag, bagHandle_opt bagHandle_opt, u32 row, u32 col, s32 type, void *data)
{
    return bagAlignOptNode (bag, bagHandle_opt, row, col, type, data, READ_BAG);
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
bagError bagReadOptNodePos (bagHandle bag, bagHandle_opt bag_opt, u32 row, u32 col, s32 type, void *data, f64 **x, f64 **y)
{
    bagError status;
    
    if ((status = bagFillPos (bag, row, col, row, col, x, y)) != BAG_SUCCESS)
        return status;
    return bagAlignOptNode (bag, bag_opt, row, col, type, data, READ_BAG);
}


/********************************************************************/
/*! \brief bagGetOptDatasetInfo
 *
 * Description : 
 *     This function opens a BAG file and finds the optional dataset HDF information
 *
 * \param *bag_handle   bag_handle will be set to the allocated \a bagHandle
 *                      private object for subsequent external reference
 *                      by the caller.
 * \param type		    Dataset name from BAG_SURFACE_PARAMS
 * \param num_opt_datasets    Number of optional datasets in bag file
 * \param opt_dataset_names	  List of BAG_SURFACE_PARAMS loaded in the BAG.
 *
 * \return \li On success, \a bagError is set to \a BAG_SUCCESS
 *         \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS
 *
 ********************************************************************/
bagError bagGetOptDatasetInfo(bagHandle_opt *bag_handle_opt, s32 type)
{
    bagError     status;
    hid_t        dataset_id;

	/* set the memspace id to -1 */
	(*bag_handle_opt)->memspace_id = -1;
       
    /*!  Try to open the optional dataset */
	switch(type)
	{
	case Nominal_Elevation:
		/*!  Open the Elevation dataset and then the supporting HDF structures */

		(* bag_handle_opt)->dataset_id = H5Dopen((* bag_handle_opt)->file_id, NOMINAL_ELEVATION_PATH);
		if ((* bag_handle_opt)->dataset_id < 0)
				return BAG_HDF_DATASET_OPEN_FAILURE; 
		if ((status = bagReadAttribute ((* bag_handle_opt), (* bag_handle_opt)->dataset_id, (u8 *)"min_value", &(* bag_handle_opt)->bag.min)) != BAG_SUCCESS)
		{
			return (status);
		}
		if ((status = bagReadAttribute ((* bag_handle_opt), (* bag_handle_opt)->dataset_id, (u8 *)"max_value", &(* bag_handle_opt)->bag.max)) != BAG_SUCCESS)
		{
			return (status);
		}

		/*! Obtain Nominal Elevation datatype */
		if (((* bag_handle_opt)->datatype_id = H5Dget_type((* bag_handle_opt)->dataset_id)) < 0)
			return BAG_HDF_TYPE_NOT_FOUND;

		/*! Obtain Nominal Elevation file space  */
		(* bag_handle_opt)->filespace_id = H5Dget_space((* bag_handle_opt)->dataset_id);
		if ((* bag_handle_opt)->filespace_id < 0)
		{
			return BAG_HDF_DATASPACE_CORRUPTED;
		}

		break;
	
	case Num_Hypotheses:
		/*!  Open the number of hypotheses dataset and then the supporting HDF structures */

		(* bag_handle_opt)->dataset_id = H5Dopen((* bag_handle_opt)->file_id, NUM_HYPOTHESES_PATH);
		if ((* bag_handle_opt)->dataset_id < 0)
				return BAG_HDF_DATASET_OPEN_FAILURE; 
		if ((status = bagReadAttribute ((* bag_handle_opt), (* bag_handle_opt)->dataset_id, (u8 *)"min_value", &(* bag_handle_opt)->bag.min)) != BAG_SUCCESS)
		{
			return (status);
		}
		if ((status = bagReadAttribute ((* bag_handle_opt), (* bag_handle_opt)->dataset_id, (u8 *)"max_value", &(* bag_handle_opt)->bag.max)) != BAG_SUCCESS)
		{
			return (status);
		}

		/*! Obtain number of hypotheses datatype */
		if (((* bag_handle_opt)->datatype_id = H5Dget_type((* bag_handle_opt)->dataset_id)) < 0)
			return BAG_HDF_TYPE_NOT_FOUND;

		/*! Obtain number of hypotheses file space  */
		(* bag_handle_opt)->filespace_id = H5Dget_space((* bag_handle_opt)->dataset_id);
		if ((* bag_handle_opt)->filespace_id < 0)
		{
			return BAG_HDF_DATASPACE_CORRUPTED;
		}
		break;
	case Average:
		/*!  Open the number of hypotheses dataset and then the supporting HDF structures */

		(* bag_handle_opt)->dataset_id = H5Dopen((* bag_handle_opt)->file_id, AVERAGE_PATH);
		if ((* bag_handle_opt)->dataset_id < 0)
				return BAG_HDF_DATASET_OPEN_FAILURE; 
		if ((status = bagReadAttribute ((* bag_handle_opt), (* bag_handle_opt)->dataset_id, (u8 *)"min_value", &(* bag_handle_opt)->bag.min)) != BAG_SUCCESS)
		{
			return (status);
		}
		if ((status = bagReadAttribute ((* bag_handle_opt), (* bag_handle_opt)->dataset_id, (u8 *)"max_value", &(* bag_handle_opt)->bag.max)) != BAG_SUCCESS)
		{
			return (status);
		}

		/*! Obtain number of hypotheses datatype */
		if (((* bag_handle_opt)->datatype_id = H5Dget_type((* bag_handle_opt)->dataset_id)) < 0)
			return BAG_HDF_TYPE_NOT_FOUND;

		/*! Obtain number of hypotheses file space  */
		(* bag_handle_opt)->filespace_id = H5Dget_space((* bag_handle_opt)->dataset_id);
		if ((* bag_handle_opt)->filespace_id < 0)
		{
			return BAG_HDF_DATASPACE_CORRUPTED;
		}
		break;
	case Standard_Dev: 
		/*!  Open the number of hypotheses dataset and then the supporting HDF structures */

		(* bag_handle_opt)->dataset_id = H5Dopen((* bag_handle_opt)->file_id, STANDARD_DEV_PATH);
		if ((* bag_handle_opt)->dataset_id < 0)
				return BAG_HDF_DATASET_OPEN_FAILURE; 
		if ((status = bagReadAttribute ((* bag_handle_opt), (* bag_handle_opt)->dataset_id, (u8 *)"min_value", &(* bag_handle_opt)->bag.min)) != BAG_SUCCESS)
		{
			return (status);
		}
		if ((status = bagReadAttribute ((* bag_handle_opt), (* bag_handle_opt)->dataset_id, (u8 *)"max_value", &(* bag_handle_opt)->bag.max)) != BAG_SUCCESS)
		{
			return (status);
		}

		/*! Obtain number of hypotheses datatype */
		if (((* bag_handle_opt)->datatype_id = H5Dget_type((* bag_handle_opt)->dataset_id)) < 0)
			return BAG_HDF_TYPE_NOT_FOUND;

		/*! Obtain number of hypotheses file space  */
		(* bag_handle_opt)->filespace_id = H5Dget_space((* bag_handle_opt)->dataset_id);
		if ((* bag_handle_opt)->filespace_id < 0)
		{
			return BAG_HDF_DATASPACE_CORRUPTED;
		}
		break;
		}

    
    return (BAG_SUCCESS);
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
bagError bagUpdateOptMinMax (bagHandle hnd, bagHandle_opt hnd_opt, u32 type)
{
    herr_t status;
    u32    i, j;
    u8    *max_name, *min_name;
    hid_t  dataset_id;
    f32   *min_tmp, *max_tmp, **surface_array, *omax, *omin, null_val;


    if (hnd == NULL)
        return BAG_INVALID_BAG_HANDLE;

    min_tmp = calloc (1, sizeof (f32));
    max_tmp = calloc (1, sizeof (f32));
    if (min_tmp == NULL || max_tmp == NULL)
        return BAG_MEMORY_ALLOCATION_FAILED;
	
    omin          = &hnd_opt->bag.min;
    omax          = &hnd_opt->bag.max;
    dataset_id    = hnd_opt->dataset_id;
    max_name      = (u8 *)"max_value";
    min_name      = (u8 *)"min_value";
    null_val      = NULL_GENERIC;
    surface_array = &hnd_opt->dataArray;
      
    if (dataset_id < 0)
        return BAG_HDF_DATASET_OPEN_FAILURE; 

    *max_tmp = null_val;
    *min_tmp = null_val;

    for (i=0; i < hnd->bag.def.nrows; i++)
    {
		bagReadOptRegion (hnd, hnd_opt, i, 0, i, hnd->bag.def.ncols-1, type);
	}
			

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
    

    /*! update the original bagData values */
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
bagError bagUpdateOptSurface (bagHandle hnd, bagHandle_opt hnd_opt, u32 type)
{
    herr_t   status;

    if (hnd_opt == NULL)
        return BAG_INVALID_BAG_HANDLE;

    status = bagUpdateOptMinMax (hnd, hnd_opt, type);
    check_hdf_status();

    return BAG_SUCCESS;
}


