/*! \file bag_opt_surfaces.c
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
 * Webb McDonald -- Wed Dec 28 11:14:22 2011
 *  -removed bagHandle_opt, all opt contents now are in bagHandle
 *
 * Webb McDonald -- Fri Aug  5 15:05:38 2011
 *  -added bagFileCloseOpt
 *
 * Mike Van Duzee -- Wed Aug 3 15:48:50 2011
 *  -The bagUpdateOptMinMax() function was only processing the last row.
 *
 * Webb McDonald -- Wed Jun 29 15:33:10 2011
 *  -added compression support
 *
 * Webb McDonald -- Tue Jul 27 17:36:08 2010
 *   Added another optional dataset, vertical datum / surface correctors to BAG.
 *   Changed API functions to allow different number of rows/columns in optional
 *   datasets from the mandatory datasets (elevation & uncertainty).
 *
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
 *  \param bag_hnd    Handle of the BAG
 *	\param data       Pointer to memory holding the data for the optional dataset
 *  \param type       Indicates which data surface type to access, element of \a BAG_SURFACE_PARAMS
 *
 *  \return : \li On success, \a bagError is set to \a BAG_SUCCESS.
 *            \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS.
 * 
 ********************************************************************/
bagError bagCreateOptionalDataset (bagHandle bag_hnd, bagData *data, s32 type)
{
	
	hsize_t      chunk_size[RANK] = {0,0};
    hsize_t      dims[RANK];
	hid_t        file_id = bag_hnd->file_id;
    hid_t        dataset_id;
    hid_t        dataspace_id;
    hid_t        datatype_id; 
    hid_t        plist_id;
	herr_t		 status;
    u8           typer;

	f32          null = NULL_ELEVATION;
    bagVerticalCorrector  nullVdat;
    bagVerticalCorrectorNode  nullVdatNode;
    bagOptNodeGroup       nullNodeGroup;
    bagOptElevationSolutionGroup nullElevationSolutionGroup;
	
    /*! init all the HDF structs to -1 */
    ( bag_hnd)->opt_memspace_id[type]  = 
    ( bag_hnd)->opt_dataset_id[type]   =  
    ( bag_hnd)->opt_filespace_id[type] = 
    ( bag_hnd)->opt_datatype_id[type]  = -1;

    ( bag_hnd)->dataArray[type]    = (f32 *) NULL;
    ( bag_hnd)->bag.opt[type].data     = (f32 **) NULL;

	/*! Create the mandatory \a elevation dataset */
    dims[0] = data->opt[type].nrows;
    dims[1] = data->opt[type].ncols;

    bag_hnd->bag.opt[type].nrows = (u32)dims[0];
    bag_hnd->bag.opt[type].ncols = (u32)dims[1];

    if (data->chunkSize > 0)
    {
        chunk_size[0] = chunk_size[1] = data->chunkSize;
    }
    else
    {
        if (dims[0] > 100 && dims[1] > 100)
        {
            chunk_size[0] = chunk_size[1] = 100;
        }        
        else if (dims[0] > 10 && dims[1] > 10)
        {
            chunk_size[0] = chunk_size[1] = 10;
        }
        else
        {
            data->compressionLevel = 0;
        }
    }

    if ((dataspace_id = H5Screate_simple(RANK, dims, NULL)) < 0)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_DATASPACE_FAILURE);
    }

	if ((datatype_id = H5Tcopy(data->opt[type].datatype)) < 0)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_TYPE_COPY_FAILURE);
    }

    if (H5Tget_class(datatype_id) != H5T_COMPOUND)
    {
        if ((status = H5Tset_order(datatype_id, H5T_ORDER_LE)) < 0)
        {
            status = H5Fclose (file_id);
            return (BAG_HDF_SET_PROPERTY_FAILURE);
        }
    }

    if ((plist_id = H5Pcreate(H5P_DATASET_CREATE)) < 0)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_PROPERTY_CLASS_FAILURE);
    }

    if (data->compressionLevel > 0 && data->compressionLevel <= 9)
    {
        status = H5Pset_layout (plist_id, H5D_CHUNKED);
        status = H5Pset_chunk(plist_id, RANK, chunk_size);
        
        if ((status = H5Pset_deflate (plist_id, data->compressionLevel)) < 0)
        {
            status = H5Fclose (file_id);
            return (BAG_HDF_SET_PROPERTY_FAILURE);
        }
    }
    else if (data->compressionLevel) /* if anything other than zero */
    {
        return (BAG_HDF_INVALID_COMPRESSION_LEVEL);
    }

    switch (type)
	{
		case Nominal_Elevation:
            
            status = H5Pset_fill_time  (plist_id, H5D_FILL_TIME_ALLOC);
            status = H5Pset_fill_value (plist_id, datatype_id, &null);
            check_hdf_status();

			if ((dataset_id = H5Dcreate(file_id, NOMINAL_ELEVATION_PATH, datatype_id, dataspace_id, plist_id)) < 0)
			{    
				status = H5Fclose (file_id);
				return (BAG_HDF_CREATE_GROUP_FAILURE);
			}

            /*! Add the attributes to the \a elevation dataset */
            if ((status = bagCreateAttribute (bag_hnd, dataset_id, (u8 *)"min_value", sizeof(f32), BAG_ATTR_F32)) != BAG_SUCCESS)
            {
                status = H5Fclose (file_id);
                return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
            }
            if ((status = bagWriteAttribute (bag_hnd, dataset_id, (u8 *)"min_value", (void *) &(data->opt[type].min) )) != BAG_SUCCESS)
            {
                status = H5Fclose (file_id);
                return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
            }
            if ((status = bagCreateAttribute (bag_hnd, dataset_id, (u8 *)"max_value", sizeof(f32), BAG_ATTR_F32)) != BAG_SUCCESS)
            {
                status = H5Fclose (file_id);
                return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
            }
            if ((status = bagWriteAttribute (bag_hnd, dataset_id, (u8 *)"max_value", (void *) &(data->opt[type].max) )) != BAG_SUCCESS)
            {
                status = H5Fclose (file_id);
                return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
            }

			break;
		
		case Surface_Correction:

            typer = bag_hnd->bag.def.surfaceCorrectionTopography;

            status = H5Pset_fill_time  (plist_id, H5D_FILL_TIME_ALLOC);

            if (BAG_SURFACE_GRID_EXTENTS == typer)
            {
                memset (&nullVdatNode, 0, sizeof (bagVerticalCorrectorNode));
                status = H5Pset_fill_value (plist_id, datatype_id, &nullVdatNode);
            }
            else
            {
                memset (&nullVdat, 0, sizeof (bagVerticalCorrector));
                status = H5Pset_fill_value (plist_id, datatype_id, &nullVdat);
            }
            check_hdf_status();

			if ((dataset_id = H5Dcreate(file_id, VERT_DATUM_CORR_PATH, datatype_id, dataspace_id, plist_id)) < 0)
			{    
				status = H5Fclose (file_id);
				return (BAG_HDF_CREATE_GROUP_FAILURE);
			}
            if ((status = bagCreateAttribute (bag_hnd, dataset_id, (u8 *)"surface_type", sizeof(u8), BAG_ATTR_U8)) != BAG_SUCCESS)
            {
                status = H5Fclose (file_id);
                return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
            }
            if ((status = bagCreateAttribute (bag_hnd, dataset_id, (u8 *)"vertical_datum", MAX_STR, BAG_ATTR_CS1)) != BAG_SUCCESS)
            {
                status = H5Fclose (file_id);
                return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
            }

            bagWriteAttribute (bag_hnd, dataset_id, (u8 *)"surface_type", (u8*) &typer);
            bagWriteAttribute (bag_hnd, dataset_id, (u8 *)"vertical_datum", (u8*)"");
            
			break;
		
		case Node_Group:

            memset (&nullNodeGroup, 0, sizeof (bagOptNodeGroup));
            nullNodeGroup.hyp_strength   = NULL_GENERIC;
            nullNodeGroup.num_hypotheses = NULL_GENERIC;

            status = H5Pset_fill_time  (plist_id, H5D_FILL_TIME_ALLOC);
            status = H5Pset_fill_value (plist_id, datatype_id, &nullNodeGroup);
            check_hdf_status();

			if ((dataset_id = H5Dcreate(file_id, NODE_GROUP_PATH, datatype_id, dataspace_id, plist_id)) < 0)
			{    
				status = H5Fclose (file_id);
				return (BAG_HDF_CREATE_GROUP_FAILURE);
			}
            if ((status = bagCreateAttribute (bag_hnd, dataset_id, (u8 *)"min_hyp_strength", sizeof(f32), BAG_ATTR_F32)) != BAG_SUCCESS)
            {
                status = H5Fclose (file_id);
                return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
            }
            if ((status = bagWriteAttribute (bag_hnd, dataset_id, (u8 *)"min_hyp_strength", (void *) &(data->opt[type].min) )) != BAG_SUCCESS)
            {
                status = H5Fclose (file_id);
                return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
            }
            if ((status = bagCreateAttribute (bag_hnd, dataset_id, (u8 *)"max_hyp_strength", sizeof(f32), BAG_ATTR_F32)) != BAG_SUCCESS)
            {
                status = H5Fclose (file_id);
                return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
            }
            if ((status = bagWriteAttribute (bag_hnd, dataset_id, (u8 *)"max_hyp_strength", (void *) &(data->opt[type].max) )) != BAG_SUCCESS)
            {
                status = H5Fclose (file_id);
                return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
            }
            if ((status = bagCreateAttribute (bag_hnd, dataset_id, (u8 *)"min_num_hypotheses", sizeof(u32), BAG_ATTR_U32)) != BAG_SUCCESS)
            {
                status = H5Fclose (file_id);
                return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
            }
            if ((status = bagWriteAttribute (bag_hnd, dataset_id, (u8 *)"min_num_hypotheses", (void *) &(data->opt[type].min) )) != BAG_SUCCESS)
            {
                status = H5Fclose (file_id);
                return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
            }
            if ((status = bagCreateAttribute (bag_hnd, dataset_id, (u8 *)"max_num_hypotheses", sizeof(u32), BAG_ATTR_U32)) != BAG_SUCCESS)
            {
                status = H5Fclose (file_id);
                return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
            }
            if ((status = bagWriteAttribute (bag_hnd, dataset_id, (u8 *)"max_num_hypotheses", (void *) &(data->opt[type].max) )) != BAG_SUCCESS)
            {
                status = H5Fclose (file_id);
                return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
            }
            

            break;
		
		case Elevation_Solution_Group:

            memset (&nullElevationSolutionGroup, 0, sizeof (bagOptElevationSolutionGroup));
            nullElevationSolutionGroup.shoal_elevation   = NULL_ELEVATION;
            nullElevationSolutionGroup.stddev        = NULL_GENERIC;
            nullElevationSolutionGroup.num_soundings = NULL_GENERIC;

            status = H5Pset_fill_time  (plist_id, H5D_FILL_TIME_ALLOC);
            status = H5Pset_fill_value (plist_id, datatype_id, &nullElevationSolutionGroup);
            check_hdf_status();

			if ((dataset_id = H5Dcreate(file_id, ELEVATION_SOLUTION_GROUP_PATH, datatype_id, dataspace_id, plist_id)) < 0)
			{    
				status = H5Fclose (file_id);
				return (BAG_HDF_CREATE_GROUP_FAILURE);
			}
            
            if ((status = bagCreateAttribute (bag_hnd, dataset_id, (u8 *)"min_shoal_elevation", sizeof(f32), BAG_ATTR_F32)) != BAG_SUCCESS)
            {
                status = H5Fclose (file_id);
                return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
            }
            if ((status = bagWriteAttribute (bag_hnd, dataset_id, (u8 *)"min_shoal_elevation", (void *) &(data->opt[type].min) )) != BAG_SUCCESS)
            {
                status = H5Fclose (file_id);
                return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
            }
            if ((status = bagCreateAttribute (bag_hnd, dataset_id, (u8 *)"max_shoal_elevation", sizeof(f32), BAG_ATTR_F32)) != BAG_SUCCESS)
            {
                status = H5Fclose (file_id);
                return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
            }
            if ((status = bagWriteAttribute (bag_hnd, dataset_id, (u8 *)"max_shoal_elevation", (void *) &(data->opt[type].max) )) != BAG_SUCCESS)
            {
                status = H5Fclose (file_id);
                return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
            }
            if ((status = bagCreateAttribute (bag_hnd, dataset_id, (u8 *)"min_stddev", sizeof(f32), BAG_ATTR_F32)) != BAG_SUCCESS)
            {
                status = H5Fclose (file_id);
                return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
            }
            if ((status = bagWriteAttribute (bag_hnd, dataset_id, (u8 *)"min_stddev", (void *) &(data->opt[type].min) )) != BAG_SUCCESS)
            {
                status = H5Fclose (file_id);
                return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
            }
            if ((status = bagCreateAttribute (bag_hnd, dataset_id, (u8 *)"max_stddev", sizeof(f32), BAG_ATTR_F32)) != BAG_SUCCESS)
            {
                status = H5Fclose (file_id);
                return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
            }
            if ((status = bagWriteAttribute (bag_hnd, dataset_id, (u8 *)"max_stddev", (void *) &(data->opt[type].max) )) != BAG_SUCCESS)
            {
                status = H5Fclose (file_id);
                return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
            }
            if ((status = bagCreateAttribute (bag_hnd, dataset_id, (u8 *)"min_num_soundings", sizeof(u32), BAG_ATTR_U32)) != BAG_SUCCESS)
            {
                status = H5Fclose (file_id);
                return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
            }
            if ((status = bagWriteAttribute (bag_hnd, dataset_id, (u8 *)"min_num_soundings", (void *) &(data->opt[type].min) )) != BAG_SUCCESS)
            {
                status = H5Fclose (file_id);
                return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
            }
            if ((status = bagCreateAttribute (bag_hnd, dataset_id, (u8 *)"max_num_soundings", sizeof(u32), BAG_ATTR_U32)) != BAG_SUCCESS)
            {
                status = H5Fclose (file_id);
                return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
            }
            if ((status = bagWriteAttribute (bag_hnd, dataset_id, (u8 *)"max_num_soundings", (void *) &(data->opt[type].max) )) != BAG_SUCCESS)
            {
                status = H5Fclose (file_id);
                return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
            }
            
            break;
		
		default:
        return BAG_HDF_TYPE_NOT_FOUND;
        break;
	}


    /* get the file space id */
	( bag_hnd)->opt_filespace_id[type] = H5Dget_space(dataset_id);
    if (( bag_hnd)->opt_filespace_id[type] < 0)
    {
        return BAG_HDF_DATASPACE_CORRUPTED;
    }

	(bag_hnd)->opt_dataset_id[type] = dataset_id;
	(bag_hnd)->opt_datatype_id[type] = datatype_id;

    H5Pclose (plist_id);
	return BAG_SUCCESS;
}

/********************************************************************/
/*! \brief bagGetOptDatasets
 *
 * Description : 
 *     This function opens a BAG file and finds the number of optional datasets along with a 
 *		list of those found.
 *
 * \param *bag_handle_opt   bag_handle will be set to the allocated \a bagHandle
 *                          private object for subsequent external reference
 *                          by the caller.
 * \param num_opt_datasets    Number of optional datasets in bag file
 * \param opt_dataset_names	  List of BAG_SURFACE_PARAMS loaded in the BAG.
 *
 * \return \li On success, \a bagError is set to \a BAG_SUCCESS
 *         \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS
 *
 ********************************************************************/
bagError bagGetOptDatasets(bagHandle *bag_handle_opt, s32 *num_opt_datasets, 
                           int opt_dataset_names[BAG_OPT_SURFACE_LIMIT])
{
    hid_t     dataset_id;
    herr_t    status;
     
    
  	*num_opt_datasets = 0;

    /*!  Try to open the Nominal Elevation dataset */
    dataset_id = H5Dopen((* bag_handle_opt)->file_id, NOMINAL_ELEVATION_PATH);
    if (dataset_id > 0)
	{
		opt_dataset_names[*num_opt_datasets] = Nominal_Elevation;
		++(*num_opt_datasets);
        status = H5Dclose (dataset_id);
        check_hdf_status();
	}


	/*!  Try to open the SEP / vertical dataum corrections dataset */
    dataset_id = H5Dopen((* bag_handle_opt)->file_id, VERT_DATUM_CORR_PATH);
    if (dataset_id > 0)
	{
		opt_dataset_names[*num_opt_datasets] = Surface_Correction;
		++(*num_opt_datasets);
        status = H5Dclose (dataset_id);
        check_hdf_status();
	}


	/*!  Try to open the node group dataset */
    dataset_id = H5Dopen((* bag_handle_opt)->file_id, NODE_GROUP_PATH);
    if (dataset_id > 0)
	{
		opt_dataset_names[*num_opt_datasets] = Node_Group;
		++(*num_opt_datasets);
        status = H5Dclose (dataset_id);
        check_hdf_status();
	}


	/*!  Try to open the elevation solution group dataset */
    dataset_id = H5Dopen((* bag_handle_opt)->file_id, ELEVATION_SOLUTION_GROUP_PATH);
    if (dataset_id > 0)
	{
		opt_dataset_names[*num_opt_datasets] = Elevation_Solution_Group;
		++(*num_opt_datasets);
        status = H5Dclose (dataset_id);
        check_hdf_status();
	}


	/*!  Try to open the num hypothesis dataset */
    dataset_id = H5Dopen((* bag_handle_opt)->file_id, NUM_HYPOTHESES_PATH);
    if (dataset_id > 0)
    {
		opt_dataset_names[*num_opt_datasets] = Num_Hypotheses;
		++(*num_opt_datasets);
        status = H5Dclose (dataset_id);
        check_hdf_status();
	}

	/*!  Try to open the average dataset */
    dataset_id = H5Dopen((* bag_handle_opt)->file_id, AVERAGE_PATH);
    if (dataset_id > 0)
    {
		opt_dataset_names[*num_opt_datasets] = Average;
		++(*num_opt_datasets);
        status = H5Dclose (dataset_id);
        check_hdf_status();
	}

	/*!  Try to open the standard deviation dataset */
    dataset_id = H5Dopen((* bag_handle_opt)->file_id, STANDARD_DEV_PATH);
    if (dataset_id > 0)
    {
		opt_dataset_names[*num_opt_datasets] = Standard_Dev;
		++num_opt_datasets;
        status = H5Dclose (dataset_id);
        check_hdf_status();
	}
	
    
    return (BAG_SUCCESS);
}




/********************************************************************/
/*! \brief : bagFreeInfoOpt
 *
 * Description : 
 *   This function closes the HDF handles previously opened via
 *           bagGetOptDatasetInfo.
 *
 * \param bag_handle_opt  External reference to the private object 
 *                                          used within the library.
 *
 * \return : \li On success, \a bagError is set to \a BAG_SUCCESS
 *           \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS
 *
 ********************************************************************/

bagError bagFreeInfoOpt (bagHandle bag_handle_opt)
{
    s32 i;
    herr_t    status;


    if (bag_handle_opt == NULL)
        return BAG_INVALID_BAG_HANDLE;

    for (i=0; i < BAG_OPT_SURFACE_LIMIT; i ++)
    {
        /*! close the \a HDF entities */
        if (bag_handle_opt->opt_memspace_id[i] >= 0)
        {
            status = H5Sclose (bag_handle_opt->opt_memspace_id[i]);
            check_hdf_status();
            bag_handle_opt->opt_memspace_id[i] = -1;
        }
        if (bag_handle_opt->opt_filespace_id[i] >= 0)
        {
            status = H5Sclose (bag_handle_opt->opt_filespace_id[i]);
            check_hdf_status();
            bag_handle_opt->opt_filespace_id[i] = -1;
        }
        if (bag_handle_opt->opt_datatype_id[i] >= 0)
        {
            status = H5Tclose (bag_handle_opt->opt_datatype_id[i]);
            check_hdf_status();
            bag_handle_opt->opt_datatype_id[i] = -1;
        }
        if (bag_handle_opt->opt_dataset_id[i] >= 0)
        {
            status = H5Dclose (bag_handle_opt->opt_dataset_id[i]);
            check_hdf_status();
            bag_handle_opt->opt_dataset_id[i] = -1;
        }
    }

    return (BAG_SUCCESS);   
}



/********************************************************************/
/*! \brief : bagGetOptDataPointer
 *
 * Description : 
 *        This function is used to access the \a bagData structure from
 *        within the private object referenced by \a *bag_handle_opt.
 * 
 * \param  bag_handle_opt   External reference to the private \a bagHandle object
 *
 * \return : \li On success, \a bagError is set to \a BAG_SUCCESS
 *           \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS
 * 
 ********************************************************************/
bagDataOpt *bagGetOptDataPointer(bagHandle bag_handle_opt)
{
    if (bag_handle_opt == NULL)
        return NULL;
    else
        return (&bag_handle_opt->bag);
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
bagError bagReadOptDatasetPos (bagHandle bagHandle, s32 type, f64 **x, f64 **y)
{
    bagError status;
    hid_t xfer = (type >= Surface_Correction) ? H5P_DEFAULT : DISABLE_STRIP_MINING;
        
    
    
    if ((status = bagFillPos (bagHandle, 0, 0, bagHandle->bag.opt[type].nrows - 1, 
                              bagHandle->bag.opt[type].ncols - 1, x, y)) != BAG_SUCCESS)
        return status;
    return bagAlignRegion (bagHandle, 0, 0, bagHandle->bag.opt[type].nrows - 1, 
                              bagHandle->bag.opt[type].ncols - 1, type, READ_BAG, xfer);
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
bagError bagReadOptDataset (bagHandle bagHandle, s32 type)
{
    hid_t xfer = (type >= Surface_Correction) ? H5P_DEFAULT : DISABLE_STRIP_MINING;
        
    return bagAlignRegion (bagHandle, 0, 0, bagHandle->bag.opt[type].nrows - 1, 
                              bagHandle->bag.opt[type].ncols - 1, type, READ_BAG, xfer);
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
bagError bagWriteOptDataset (bagHandle bagHandle, s32 type)
{
    hid_t xfer = (type >= Surface_Correction) ? H5P_DEFAULT : DISABLE_STRIP_MINING;

    return bagAlignRegion (bagHandle, 0, 0, bagHandle->bag.opt[type].nrows - 1, 
                              bagHandle->bag.opt[type].ncols - 1, type, WRITE_BAG, xfer);
}



bagError bagReadOptSurfaceDims (bagHandle hnd, s32 type)
{
    herr_t   status;
    s32      rank;
    hid_t    dataspace_id;
    hsize_t  max_dims[RANK];
    
    if (hnd == NULL)
        return BAG_INVALID_BAG_HANDLE;

    /*! This dataspace is a one time throwaway */
    dataspace_id = H5Dget_space(hnd->opt_dataset_id[type]);

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

    hnd->bag.opt[type].nrows = (u32)max_dims[0];
    hnd->bag.opt[type].ncols = (u32)max_dims[1];
    
    return BAG_SUCCESS;
}

/********************************************************************/
/*! \brief bagGetOptDatasetInfo
 *
 * Description : 
 *     This function opens a BAG file and finds the optional dataset HDF information
 *
 * \param *bag_handle_opt   bag_handle_opt will be set to the allocated \a bagHandle
 *                          private object for subsequent external reference
 *                          by the caller.
 * \param type		    Dataset name from BAG_SURFACE_PARAMS
 *
 * \return \li On success, \a bagError is set to \a BAG_SUCCESS
 *         \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS
 *
 ********************************************************************/
bagError bagGetOptDatasetInfo(bagHandle *bag_handle_opt, s32 type)
{
    bagError     status;

	/* set the memspace id to -1 */
	(*bag_handle_opt)->opt_memspace_id[type] = -1;
       
    /*!  Try to open the optional dataset */
	switch(type)
	{
	case Nominal_Elevation:
		/*!  Open the Nominal Elevation dataset and then the supporting HDF structures */

		(* bag_handle_opt)->opt_dataset_id[type] = H5Dopen((* bag_handle_opt)->file_id, NOMINAL_ELEVATION_PATH);
		if ((* bag_handle_opt)->opt_dataset_id[type] < 0)
				return BAG_HDF_DATASET_OPEN_FAILURE; 
		if ((status = bagReadAttribute ((bagHandle)(* bag_handle_opt), (*bag_handle_opt)->opt_dataset_id[type], (u8 *)"min_value", &(* bag_handle_opt)->bag.opt[type].min)) != BAG_SUCCESS)
		{
			return (status);
		}
		if ((status = bagReadAttribute ((bagHandle)(* bag_handle_opt), (*bag_handle_opt)->opt_dataset_id[type], (u8 *)"max_value", &(* bag_handle_opt)->bag.opt[type].max)) != BAG_SUCCESS)
		{
			return (status);
		}

		/*! Obtain Nominal Elevation datatype */
		if (((* bag_handle_opt)->opt_datatype_id[type] = H5Dget_type((*bag_handle_opt)->opt_dataset_id[type])) < 0)
			return BAG_HDF_TYPE_NOT_FOUND;

		/*! Obtain Nominal Elevation file space  */
		(* bag_handle_opt)->opt_filespace_id[type] = H5Dget_space((*bag_handle_opt)->opt_dataset_id[type]);
		if ((* bag_handle_opt)->opt_filespace_id[type] < 0)
		{
			return BAG_HDF_DATASPACE_CORRUPTED;
		}

		/*!  Obtain surface dimensions */
        /*!  With the HDF structs in place, we're ready to read the max_dims and nrows/ncols */
        if ((status = bagReadOptSurfaceDims ((* bag_handle_opt), type)) != BAG_SUCCESS)
        {
            return status;
        }
        
		break;
	
	case Surface_Correction:
		/*!  Open the SEP dataset and then the supporting HDF structures */

		(*bag_handle_opt)->opt_dataset_id[type] = H5Dopen((* bag_handle_opt)->file_id, VERT_DATUM_CORR_PATH);
		if ((*bag_handle_opt)->opt_dataset_id[type] < 0)
				return BAG_HDF_DATASET_OPEN_FAILURE; 
		if ((status = bagReadAttribute ((bagHandle)(* bag_handle_opt), (*bag_handle_opt)->opt_dataset_id[type], (u8 *)"surface_type", &(* bag_handle_opt)->bag.def.surfaceCorrectionTopography)) != BAG_SUCCESS)
		{
            (* bag_handle_opt)->bag.def.surfaceCorrectionTopography = BAG_SURFACE_UNKNOWN;
		}

		/*! Obtain SEP datatype */
		if (((* bag_handle_opt)->opt_datatype_id[type] = H5Dget_type((*bag_handle_opt)->opt_dataset_id[type])) < 0)
			return BAG_HDF_TYPE_NOT_FOUND;

		/*! Obtain  SEP file space  */
		(* bag_handle_opt)->opt_filespace_id[type] = H5Dget_space((*bag_handle_opt)->opt_dataset_id[type]);
		if ((* bag_handle_opt)->opt_filespace_id[type] < 0)
		{
			return BAG_HDF_DATASPACE_CORRUPTED;
		}

        /*!  Obtain surface dimensions */
        /*!  With the HDF structs in place, we're ready to read the max_dims and nrows/ncols */
        if ((status = bagReadOptSurfaceDims ((* bag_handle_opt), type)) != BAG_SUCCESS)
        {
            return status;
        }
        
		break;
	
	case Elevation_Solution_Group:
		/*!  Open the Elevation Solution dataset and then the supporting HDF structures */

		(*bag_handle_opt)->opt_dataset_id[type] = H5Dopen((* bag_handle_opt)->file_id, ELEVATION_SOLUTION_GROUP_PATH);
		if ((*bag_handle_opt)->opt_dataset_id[type] < 0)
				return BAG_HDF_DATASET_OPEN_FAILURE; 

		/*! Obtain datatype */
		if (((* bag_handle_opt)->opt_datatype_id[type] = H5Dget_type((*bag_handle_opt)->opt_dataset_id[type])) < 0)
			return BAG_HDF_TYPE_NOT_FOUND;

		/*! Obtain file space  */
		(* bag_handle_opt)->opt_filespace_id[type] = H5Dget_space((*bag_handle_opt)->opt_dataset_id[type]);
		if ((* bag_handle_opt)->opt_filespace_id[type] < 0)
		{
			return BAG_HDF_DATASPACE_CORRUPTED;
		}

        /*!  Obtain surface dimensions */
        /*!  With the HDF structs in place, we're ready to read the max_dims and nrows/ncols */
        if ((status = bagReadOptSurfaceDims ((* bag_handle_opt), type)) != BAG_SUCCESS)
        {
            return status;
        }
		break;
	
	case Node_Group:
		/*!  Open the NODE GROUP dataset and then the supporting HDF structures */

		(*bag_handle_opt)->opt_dataset_id[type] = H5Dopen((* bag_handle_opt)->file_id, NODE_GROUP_PATH);
		if ((*bag_handle_opt)->opt_dataset_id[type] < 0)
				return BAG_HDF_DATASET_OPEN_FAILURE; 

		/*! Obtain datatype */
		if (((* bag_handle_opt)->opt_datatype_id[type] = H5Dget_type((*bag_handle_opt)->opt_dataset_id[type])) < 0)
			return BAG_HDF_TYPE_NOT_FOUND;

		/*! Obtain file space  */
		(* bag_handle_opt)->opt_filespace_id[type] = H5Dget_space((*bag_handle_opt)->opt_dataset_id[type]);
		if ((* bag_handle_opt)->opt_filespace_id[type] < 0)
		{
			return BAG_HDF_DATASPACE_CORRUPTED;
		}

        /*!  Obtain surface dimensions */
        /*!  With the HDF structs in place, we're ready to read the max_dims and nrows/ncols */
        if ((status = bagReadOptSurfaceDims ((* bag_handle_opt), type)) != BAG_SUCCESS)
        {
            return status;
        }
		break;
	
	case Num_Hypotheses:
		/*!  Open the number of hypotheses dataset and then the supporting HDF structures */

		(*bag_handle_opt)->opt_dataset_id[type] = H5Dopen((* bag_handle_opt)->file_id, NUM_HYPOTHESES_PATH);
		if ((*bag_handle_opt)->opt_dataset_id[type] < 0)
				return BAG_HDF_DATASET_OPEN_FAILURE; 
		if ((status = bagReadAttribute ((bagHandle)(* bag_handle_opt), (*bag_handle_opt)->opt_dataset_id[type], (u8 *)"min_value", &(* bag_handle_opt)->bag.opt[type].min)) != BAG_SUCCESS)
		{
			return (status);
		}
		if ((status = bagReadAttribute ((bagHandle)(* bag_handle_opt), (*bag_handle_opt)->opt_dataset_id[type], (u8 *)"max_value", &(* bag_handle_opt)->bag.opt[type].max)) != BAG_SUCCESS)
		{
			return (status);
		}

		/*! Obtain number of hypotheses datatype */
		if (((* bag_handle_opt)->opt_datatype_id[type] = H5Dget_type((*bag_handle_opt)->opt_dataset_id[type])) < 0)
			return BAG_HDF_TYPE_NOT_FOUND;

		/*! Obtain number of hypotheses file space  */
		(* bag_handle_opt)->opt_filespace_id[type] = H5Dget_space((*bag_handle_opt)->opt_dataset_id[type]);
		if ((* bag_handle_opt)->opt_filespace_id[type] < 0)
		{
			return BAG_HDF_DATASPACE_CORRUPTED;
		}
		/*!  Obtain surface dimensions */
        /*!  With the HDF structs in place, we're ready to read the max_dims and nrows/ncols */
        if ((status = bagReadOptSurfaceDims ((* bag_handle_opt), type)) != BAG_SUCCESS)
        {
            return status;
        }
        
		break;
	case Average:
		/*!  Open the number of hypotheses dataset and then the supporting HDF structures */

		(*bag_handle_opt)->opt_dataset_id[type] = H5Dopen((* bag_handle_opt)->file_id, AVERAGE_PATH);
		if ((*bag_handle_opt)->opt_dataset_id[type] < 0)
				return BAG_HDF_DATASET_OPEN_FAILURE; 
		if ((status = bagReadAttribute ((bagHandle)(* bag_handle_opt), (*bag_handle_opt)->opt_dataset_id[type], (u8 *)"min_value", &(* bag_handle_opt)->bag.opt[type].min)) != BAG_SUCCESS)
		{
			return (status);
		}
		if ((status = bagReadAttribute ((bagHandle)(* bag_handle_opt), (*bag_handle_opt)->opt_dataset_id[type], (u8 *)"max_value", &(* bag_handle_opt)->bag.opt[type].max)) != BAG_SUCCESS)
		{
			return (status);
		}

		/*! Obtain number of hypotheses datatype */
		if (((* bag_handle_opt)->opt_datatype_id[type] = H5Dget_type((*bag_handle_opt)->opt_dataset_id[type])) < 0)
			return BAG_HDF_TYPE_NOT_FOUND;

		/*! Obtain number of hypotheses file space  */
		(* bag_handle_opt)->opt_filespace_id[type] = H5Dget_space((*bag_handle_opt)->opt_dataset_id[type]);
		if ((* bag_handle_opt)->opt_filespace_id[type] < 0)
		{
			return BAG_HDF_DATASPACE_CORRUPTED;
		}
		/*!  Obtain surface dimensions */
        /*!  With the HDF structs in place, we're ready to read the max_dims and nrows/ncols */
        if ((status = bagReadOptSurfaceDims ((* bag_handle_opt), type)) != BAG_SUCCESS)
        {
            return status;
        }
		break;
	case Standard_Dev: 
		/*!  Open the number of hypotheses dataset and then the supporting HDF structures */

		(*bag_handle_opt)->opt_dataset_id[type] = H5Dopen((* bag_handle_opt)->file_id, STANDARD_DEV_PATH);
		if ((*bag_handle_opt)->opt_dataset_id[type] < 0)
				return BAG_HDF_DATASET_OPEN_FAILURE; 
		if ((status = bagReadAttribute ((bagHandle)(* bag_handle_opt), (*bag_handle_opt)->opt_dataset_id[type],
                                        (u8 *)"min_value", &(* bag_handle_opt)->bag.opt[type].min)) != BAG_SUCCESS)
		{
			return (status);
		}
		if ((status = bagReadAttribute ((bagHandle)(* bag_handle_opt), (*bag_handle_opt)->opt_dataset_id[type],
                                        (u8 *)"max_value", &(* bag_handle_opt)->bag.opt[type].max)) != BAG_SUCCESS)
		{
			return (status);
		}

		/*! Obtain number of hypotheses datatype */
		if (((* bag_handle_opt)->opt_datatype_id[type] = H5Dget_type((*bag_handle_opt)->opt_dataset_id[type])) < 0)
			return BAG_HDF_TYPE_NOT_FOUND;

		/*! Obtain number of hypotheses file space  */
		(* bag_handle_opt)->opt_filespace_id[type] = H5Dget_space((*bag_handle_opt)->opt_dataset_id[type]);
		if ((* bag_handle_opt)->opt_filespace_id[type] < 0)
		{
			return BAG_HDF_DATASPACE_CORRUPTED;
		}
		/*!  Obtain surface dimensions */
        /*!  With the HDF structs in place, we're ready to read the max_dims and nrows/ncols */
        if ((status = bagReadOptSurfaceDims ((* bag_handle_opt), type)) != BAG_SUCCESS)
        {
            return status;
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
bagError bagUpdateOptMinMax (bagHandle hnd, u32 type)
{
    herr_t status;
    u32    i, j;
    u8    *max_name, *min_name;
    hid_t  dataset_id;
    f32   *min_tmp, *max_tmp, **surface_array, *omax, *omin, null_val;
    bagOptNodeGroup minNode, maxNode, *nodeSurf;
    bagOptElevationSolutionGroup minElevationSolution, maxElevationSolution, *elevationSolutionSurf;


    if (hnd == NULL)
        return BAG_INVALID_BAG_HANDLE;

    dataset_id    = hnd->opt_dataset_id[type];
    null_val      = NULL_GENERIC;

    if (dataset_id < 0)
        return BAG_HDF_DATASET_OPEN_FAILURE; 
    
    switch (type)
    {
    case Surface_Correction:
        /* N/A */
        break;
    case Node_Group:
        minNode.hyp_strength = null_val;
        maxNode.hyp_strength = null_val;
        minNode.num_hypotheses = null_val;
        maxNode.num_hypotheses = null_val;

        nodeSurf = (bagOptNodeGroup *) calloc (hnd->bag.opt[type].ncols, sizeof (bagOptNodeGroup));
        if (nodeSurf == NULL)
            return BAG_MEMORY_ALLOCATION_FAILED;

        fprintf(stdout, "Computing mins and maxes for the Node Group Surface\n");

        for (i=0; i < hnd->bag.opt[type].nrows; i++)
        {
            bagReadRow (hnd, i, 0, hnd->bag.opt[type].ncols-1, type, (void *)nodeSurf);
			
            for (j=0; j < hnd->bag.opt[type].ncols-1; j++)
            {
                if (nodeSurf[j].hyp_strength != null_val)
                {
                    if (maxNode.hyp_strength == null_val)
                        maxNode.hyp_strength = nodeSurf[j].hyp_strength;
                    if (minNode.hyp_strength == null_val)
                        minNode.hyp_strength = nodeSurf[j].hyp_strength;
                    if (nodeSurf[j].hyp_strength > maxNode.hyp_strength)
                        maxNode.hyp_strength = nodeSurf[j].hyp_strength;
                    if (nodeSurf[j].hyp_strength < minNode.hyp_strength)
                        minNode.hyp_strength = nodeSurf[j].hyp_strength;
                }
                if (nodeSurf[j].num_hypotheses != null_val)
                {
                    if (maxNode.num_hypotheses == null_val)
                        maxNode.num_hypotheses = nodeSurf[j].num_hypotheses;
                    if (minNode.num_hypotheses == null_val)
                        minNode.num_hypotheses = nodeSurf[j].num_hypotheses;
                    if (nodeSurf[j].num_hypotheses > maxNode.num_hypotheses)
                        maxNode.num_hypotheses = nodeSurf[j].num_hypotheses;
                    if (nodeSurf[j].num_hypotheses < minNode.num_hypotheses)
                        minNode.num_hypotheses = nodeSurf[j].num_hypotheses;
                }
            }
        }
    
        if (maxNode.hyp_strength != null_val)
        {
            status = bagWriteAttribute (hnd, dataset_id, "max_hyp_strength", &maxNode.hyp_strength);
            check_hdf_status();
        }
        if (minNode.hyp_strength != null_val)
        {
            status = bagWriteAttribute (hnd, dataset_id, "min_hyp_strength", &minNode.hyp_strength);
            check_hdf_status();
        }
        if (maxNode.num_hypotheses != null_val)
        {
            status = bagWriteAttribute (hnd, dataset_id, "max_num_hypotheses", &maxNode.num_hypotheses);
            check_hdf_status();
        }
        if (minNode.num_hypotheses != null_val)
        {
            status = bagWriteAttribute (hnd, dataset_id, "min_num_hypotheses", &minNode.num_hypotheses);
            check_hdf_status();
        }
        
        free (nodeSurf);
        break;

    case Elevation_Solution_Group:
        minElevationSolution.shoal_elevation = null_val;
        maxElevationSolution.shoal_elevation = null_val;
        minElevationSolution.stddev = null_val;
        maxElevationSolution.stddev = null_val;
        minElevationSolution.num_soundings = (u32)null_val;
        maxElevationSolution.num_soundings = (u32)null_val;
        elevationSolutionSurf = (bagOptElevationSolutionGroup *) calloc (hnd->bag.opt[type].ncols, sizeof (bagOptElevationSolutionGroup));
        if (elevationSolutionSurf == NULL)
            return BAG_MEMORY_ALLOCATION_FAILED;

        fprintf(stdout, "Computing mins and maxes for the Elevation Solution Group Surface\n");


        for (i=0; i < hnd->bag.opt[type].nrows; i++)
        {
            bagReadRow (hnd, i, 0, hnd->bag.opt[type].ncols-1, type, (void *)elevationSolutionSurf);
			
            for (j=0; j < hnd->bag.opt[type].ncols-1; j++)
            {
                if (elevationSolutionSurf[j].shoal_elevation != null_val)
                {
                    if (maxElevationSolution.shoal_elevation == null_val)
                        maxElevationSolution.shoal_elevation = elevationSolutionSurf[j].shoal_elevation;
                    if (minElevationSolution.shoal_elevation == null_val)
                        minElevationSolution.shoal_elevation = elevationSolutionSurf[j].shoal_elevation;
                    if (elevationSolutionSurf[j].shoal_elevation > maxElevationSolution.shoal_elevation)
                        maxElevationSolution.shoal_elevation = elevationSolutionSurf[j].shoal_elevation;
                    if (elevationSolutionSurf[j].shoal_elevation < minElevationSolution.shoal_elevation)
                        minElevationSolution.shoal_elevation = elevationSolutionSurf[j].shoal_elevation;
                }
                if (elevationSolutionSurf[j].num_soundings != null_val)
                {
                    if (maxElevationSolution.num_soundings == null_val)
                        maxElevationSolution.num_soundings = elevationSolutionSurf[j].num_soundings;
                    if (minElevationSolution.num_soundings == null_val)
                        minElevationSolution.num_soundings = elevationSolutionSurf[j].num_soundings;
                    if (elevationSolutionSurf[j].num_soundings > maxElevationSolution.num_soundings)
                        maxElevationSolution.num_soundings = elevationSolutionSurf[j].num_soundings;
                    if (elevationSolutionSurf[j].num_soundings < minElevationSolution.num_soundings)
                        minElevationSolution.num_soundings = elevationSolutionSurf[j].num_soundings;
                }
                if (elevationSolutionSurf[j].stddev != null_val)
                {
                    if (maxElevationSolution.stddev == null_val)
                        maxElevationSolution.stddev = elevationSolutionSurf[j].stddev;
                    if (minElevationSolution.stddev == null_val)
                        minElevationSolution.stddev = elevationSolutionSurf[j].stddev;
                    if (elevationSolutionSurf[j].stddev > maxElevationSolution.stddev)
                        maxElevationSolution.stddev = elevationSolutionSurf[j].stddev;
                    if (elevationSolutionSurf[j].stddev < minElevationSolution.stddev)
                        minElevationSolution.stddev = elevationSolutionSurf[j].stddev;
                }
            }
        }

        if (maxElevationSolution.stddev != null_val)
        {
            status = bagWriteAttribute (hnd, dataset_id, "max_stddev", &maxElevationSolution.stddev);
            check_hdf_status();
        }
        if (minElevationSolution.stddev != null_val)
        {
            status = bagWriteAttribute (hnd, dataset_id, "min_stddev", &minElevationSolution.stddev);
            check_hdf_status();
        }
        if (maxElevationSolution.shoal_elevation != null_val)
        {
            status = bagWriteAttribute (hnd, dataset_id, "max_shoal_elevation", &maxElevationSolution.shoal_elevation);
            check_hdf_status();
        }
        if (minElevationSolution.shoal_elevation != null_val)
        {
            status = bagWriteAttribute (hnd, dataset_id, "min_shoal_elevation", &minElevationSolution.shoal_elevation);
            check_hdf_status();
        }
        if (maxElevationSolution.num_soundings != null_val)
        {
            status = bagWriteAttribute (hnd, dataset_id, "max_num_soundings", &maxElevationSolution.num_soundings);
            check_hdf_status();
        }
        if (minElevationSolution.num_soundings != null_val)
        {
            status = bagWriteAttribute (hnd, dataset_id, "min_num_soundings", &minElevationSolution.num_soundings);
            check_hdf_status();
        }
        
        free (elevationSolutionSurf);
        break;

    default:
        
        min_tmp = calloc (1, sizeof (f32));
        max_tmp = calloc (1, sizeof (f32));
        if (min_tmp == NULL || max_tmp == NULL)
            return BAG_MEMORY_ALLOCATION_FAILED;
        
        omin          = &hnd->bag.opt[type].min;
        omax          = &hnd->bag.opt[type].max;
        
        *max_tmp = null_val;
        *min_tmp = null_val;
        
        max_name      = (u8 *)"max_value";
        min_name      = (u8 *)"min_value";
        surface_array = &hnd->dataArray[type];
      

        for (i=0; i < hnd->bag.opt[type].nrows; i++)
        {
            bagReadRegion (hnd, i, 0, i, hnd->bag.opt[type].ncols-1, type);
			
            for (j=0; j < hnd->bag.opt[type].ncols-1; j++)
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
        break;
	}

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
bagError bagUpdateOptSurface (bagHandle hnd, u32 type)
{
    herr_t   status;

    if (hnd == NULL)
        return BAG_INVALID_BAG_HANDLE;

    status = bagUpdateOptMinMax (hnd, type);
    check_hdf_status();

    return BAG_SUCCESS;
}
