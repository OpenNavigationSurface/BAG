/*! \file bag_opt_group.c
 * \brief This module contains functions for accessing the optional surface datasets and the XML metadata.
 ********************************************************************
 *
 * Module Name : bag_opt_group.c
 *
 * Author/Date : ONSWG, September 2011
 *
 * Description : 
 *               Optional surface datasets bagOptNodeGroup & bagOptElevationSolutionGroup.
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 * who  when      what
 * ---  ----      ----
 * Webb McDonald -- Tue Aug 30 10:22:35 2011
 *  -added create and read min/max functions
 *
 * Classification : Unclassified
 *
 * References : 
 *
 ********************************************************************/

#include "bag_private.h"



/****************************************************************************************/
/*! \brief bagCreateNodeGroup initializes the Node Group optional bag surface
 *
 *  \param hnd      BagHandle Pointer
 *  \param data     Data definition for this dataset.
 *
 *  \return : \li On success, \a bagError is set to \a BAG_SUCCESS.
 *            \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS.
 * 
 ********************************************************************/
bagError bagCreateNodeGroup  (bagHandle hnd, bagData *data)
{
    bagError       err;
    herr_t         herr;
    
    if ((data->opt[Node_Group].datatype = H5Tcreate(H5T_COMPOUND, sizeof (bagOptNodeGroup))) < 0)
    {
        return BAG_HDF_TYPE_CREATE_FAILURE;
    }

    herr = H5Tinsert (data->opt[Node_Group].datatype, "hyp_strength", HOFFSET(bagOptNodeGroup, hyp_strength), H5T_NATIVE_FLOAT);
    if (herr < 0)
        return BAG_HDF_TYPE_CREATE_FAILURE;

    herr = H5Tinsert (data->opt[Node_Group].datatype, "num_hypotheses", HOFFSET(bagOptNodeGroup, num_hypotheses), H5T_NATIVE_UINT);
    if (herr < 0)
        return BAG_HDF_TYPE_CREATE_FAILURE;

    err = bagCreateOptionalDataset (hnd, data, Node_Group);

    return err;
}

bagError bagCreateVarResMetadataGroup(bagHandle hnd, bagData *data)
{
	bagError	err;
	herr_t		herr;
	
	if ((data->opt[VarRes_Metadata_Group].datatype = H5Tcreate(H5T_COMPOUND, sizeof(bagVarResMetadataGroup))) < 0)
		return BAG_HDF_TYPE_CREATE_FAILURE;
	
	herr = H5Tinsert(data->opt[VarRes_Metadata_Group].datatype, "index",
                     HOFFSET(bagVarResMetadataGroup, index), H5T_NATIVE_UINT);
	if (herr < 0) return BAG_HDF_TYPE_CREATE_FAILURE;
        
    herr = H5Tinsert(data->opt[VarRes_Metadata_Group].datatype, "dimensions_x",
                     HOFFSET(bagVarResMetadataGroup, dimensions_x), H5T_NATIVE_UINT);
    if (herr < 0) return BAG_HDF_TYPE_CREATE_FAILURE;
    herr = H5Tinsert(data->opt[VarRes_Metadata_Group].datatype, "dimensions_y",
                     HOFFSET(bagVarResMetadataGroup, dimensions_y), H5T_NATIVE_UINT);
    if (herr < 0) return BAG_HDF_TYPE_CREATE_FAILURE;
        
    herr = H5Tinsert(data->opt[VarRes_Metadata_Group].datatype, "resolution_x",
                     HOFFSET(bagVarResMetadataGroup, resolution_x), H5T_NATIVE_FLOAT);
    if (herr < 0) return BAG_HDF_TYPE_CREATE_FAILURE;
    herr = H5Tinsert(data->opt[VarRes_Metadata_Group].datatype, "resolution_y",
                     HOFFSET(bagVarResMetadataGroup, resolution_y), H5T_NATIVE_FLOAT);
    if (herr < 0) return BAG_HDF_TYPE_CREATE_FAILURE;
        
    herr = H5Tinsert(data->opt[VarRes_Metadata_Group].datatype, "sw_corner_x",
                     HOFFSET(bagVarResMetadataGroup, sw_corner_x), H5T_NATIVE_FLOAT);
    if (herr < 0) return BAG_HDF_TYPE_CREATE_FAILURE;
    herr = H5Tinsert(data->opt[VarRes_Metadata_Group].datatype, "sw_corner_y",
                     HOFFSET(bagVarResMetadataGroup, sw_corner_y), H5T_NATIVE_FLOAT);
    if (herr < 0) return BAG_HDF_TYPE_CREATE_FAILURE;
	
	err = bagCreateOptionalDataset(hnd, data, VarRes_Metadata_Group);
	
	return err;
}

bagError bagCreateVarResRefinementGroup(bagHandle hnd, bagData *data, u32 const n_cells)
{
	bagError	err;
	herr_t		herr;
	
	if ((data->opt[VarRes_Refinement_Group].datatype = H5Tcreate(H5T_COMPOUND, sizeof(bagVarResRefinementGroup))) < 0) {
		return BAG_HDF_TYPE_CREATE_FAILURE;
	}
	herr = H5Tinsert(data->opt[VarRes_Refinement_Group].datatype, "depth", HOFFSET(bagVarResRefinementGroup, depth), H5T_NATIVE_FLOAT);
	if (herr < 0) return BAG_HDF_TYPE_CREATE_FAILURE;
	herr = H5Tinsert(data->opt[VarRes_Refinement_Group].datatype, "depth_uncrt", HOFFSET(bagVarResRefinementGroup, depth_uncrt), H5T_NATIVE_FLOAT);
	if (herr < 0) return BAG_HDF_TYPE_CREATE_FAILURE;
	
	data->opt[VarRes_Refinement_Group].nrows = 1;
	data->opt[VarRes_Refinement_Group].ncols = n_cells;
	
	err = bagCreateOptionalDataset(hnd, data, VarRes_Refinement_Group);
	
	return err;
}

bagError bagCreateVarResNodeGroup(bagHandle hnd, bagData *data, u32 const n_cells)
{
	bagError	err;
	herr_t		herr;
	
	if ((data->opt[VarRes_Node_Group].datatype = H5Tcreate(H5T_COMPOUND, sizeof(bagVarResNodeGroup))) < 0) {
		return BAG_HDF_TYPE_CREATE_FAILURE;
	}
	herr = H5Tinsert(data->opt[VarRes_Node_Group].datatype, "hyp_strength", HOFFSET(bagVarResNodeGroup, hyp_strength), H5T_NATIVE_FLOAT);
	if (herr < 0) return BAG_HDF_TYPE_CREATE_FAILURE;
	herr = H5Tinsert(data->opt[VarRes_Node_Group].datatype, "num_hypotheses", HOFFSET(bagVarResNodeGroup, num_hypotheses), H5T_NATIVE_UINT);
	if (herr < 0) return BAG_HDF_TYPE_CREATE_FAILURE;
	herr = H5Tinsert(data->opt[VarRes_Node_Group].datatype, "n_samples", HOFFSET(bagVarResNodeGroup, n_samples), H5T_NATIVE_UINT);
	if (herr < 0) return BAG_HDF_TYPE_CREATE_FAILURE;
	
	data->opt[VarRes_Node_Group].nrows = 1;
	data->opt[VarRes_Node_Group].ncols = n_cells;
	
	err = bagCreateOptionalDataset(hnd, data, VarRes_Node_Group);
	
	return err;
}

bagError bagCreateVarResTrackingList(bagHandle hnd, bagData *data)
{
    bagError    err;
    herr_t      status;
    
    hsize_t dim_init[1];
    hsize_t dim_max[1];
    hid_t   dataspace_id;
    hid_t   datatype_id;
    hid_t   dataset_id;
    hid_t   cparams;
    hsize_t chunk_dims[1];
    
    u32 length;
    
    dim_init[0] = 0;
    dim_max[0] = H5S_UNLIMITED;
    
    if ((dataspace_id = H5Screate_simple(1, dim_init, dim_max)) < 0) {
        status = H5Fclose(hnd->file_id);
        return BAG_HDF_CREATE_DATASET_FAILURE;
    }
    
    printf("Creating base data type\n");
    if ((datatype_id = H5Tcreate(H5T_COMPOUND, sizeof(bagVarResTrackingItem))) < 0) {
        return BAG_HDF_TYPE_CREATE_FAILURE;
    }
    printf("Creating row\n");
    status = H5Tinsert(datatype_id, "row", HOFFSET(bagVarResTrackingItem, row), H5T_NATIVE_UINT32);
    if (status < 0) return BAG_HDF_TYPE_CREATE_FAILURE;
    printf("Creating col\n");
    status = H5Tinsert(datatype_id, "col", HOFFSET(bagVarResTrackingItem, col), H5T_NATIVE_UINT32);
    if (status < 0) return BAG_HDF_TYPE_CREATE_FAILURE;
    printf("Creating sub_row\n");
    status = H5Tinsert(datatype_id, "sub_row", HOFFSET(bagVarResTrackingItem, sub_row), H5T_NATIVE_UINT32);
    if (status < 0) return BAG_HDF_TYPE_CREATE_FAILURE;
    printf("Creating sub_col\n");
    status = H5Tinsert(datatype_id, "sub_col", HOFFSET(bagVarResTrackingItem, sub_col), H5T_NATIVE_UINT32);
    if (status < 0) return BAG_HDF_TYPE_CREATE_FAILURE;
    printf("Creating depth\n");
    status = H5Tinsert(datatype_id, "depth", HOFFSET(bagVarResTrackingItem, depth), H5T_NATIVE_FLOAT);
    if (status < 0) return BAG_HDF_TYPE_CREATE_FAILURE;
    printf("Creating uncertainty\n");
    status = H5Tinsert(datatype_id, "uncertainty", HOFFSET(bagVarResTrackingItem, uncertainty), H5T_NATIVE_FLOAT);
    if (status < 0) return BAG_HDF_TYPE_CREATE_FAILURE;
    printf("Creating track_code\n");
    status = H5Tinsert(datatype_id, "track_code", HOFFSET(bagVarResTrackingItem, track_code), H5T_NATIVE_UCHAR);
    if (status < 0) return BAG_HDF_TYPE_CREATE_FAILURE;
    printf("Creating list_series\n");
    status = H5Tinsert(datatype_id, "list_series", HOFFSET(bagVarResTrackingItem, list_series), H5T_NATIVE_UINT16);
    if (status < 0) return BAG_HDF_TYPE_CREATE_FAILURE;
    
    data->opt[VarRes_Tracking_List].nrows = 1;
    data->opt[VarRes_Tracking_List].ncols = 0;
    
    if ((cparams = H5Pcreate(H5P_DATASET_CREATE)) < 0) {
        status = H5Fclose(hnd->file_id);
        return BAG_HDF_CREATE_PROPERTY_CLASS_FAILURE;
    }
    chunk_dims[0] = VARRES_TRACKING_LIST_BLOCK_SIZE;
    if ((status = H5Pset_chunk(cparams, 1, chunk_dims)) < 0) {
        status = H5Fclose(hnd->file_id);
        return BAG_HDF_SET_PROPERTY_FAILURE;
    }
    if (data->compressionLevel > 0 && data->compressionLevel <= 9) {
        if ((status = H5Pset_deflate(cparams, data->compressionLevel)) < 0) {
            status = H5Fclose(hnd->file_id);
            return BAG_HDF_SET_PROPERTY_FAILURE;
        }
    } else if (data->compressionLevel != 0) {
        return BAG_HDF_INVALID_COMPRESSION_LEVEL;
    }
    if ((dataset_id = H5Dcreate(hnd->file_id, VARRES_TRACKING_LIST_PATH, datatype_id, dataspace_id, cparams)) < 0) {
        status = H5Fclose(hnd->file_id);
        return BAG_HDF_CREATE_DATASET_FAILURE;
    }
    if ((status = H5Dextend(dataset_id, dim_init)) < 0) {
        status = H5Fclose(hnd->file_id);
        return BAG_HDF_DATASET_EXTEND_FAILURE;
    }
    if ((err = bagCreateAttribute(hnd, dataset_id, (u8*)VARRES_TRACKING_LIST_LENGTH_NAME, sizeof(u32), BAG_ATTR_U32)) != BAG_SUCCESS) {
        status = H5Fclose(hnd->file_id);
        return BAG_HDF_CREATE_ATTRIBUTE_FAILURE;
    }
    length = 0;
    if ((err = bagWriteAttribute(hnd, dataset_id, (u8*)VARRES_TRACKING_LIST_LENGTH_NAME, &length)) != BAG_SUCCESS) {
        status = H5Fclose(hnd->file_id);
        return BAG_HDF_CREATE_ATTRIBUTE_FAILURE;
    }
    status = H5Pclose(cparams);
    
    printf("done\n");
    return err;
}

/****************************************************************************************/
/*! \brief bagCreateElevationSolutionGroup initializes the Elevation Solution Group optional bag surface
 *
 *  \param hnd      BagHandle Pointer
 *  \param data     Data definition for this dataset.
 *
 *  \return : \li On success, \a bagError is set to \a BAG_SUCCESS.
 *            \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS.
 * 
 ********************************************************************/
bagError bagCreateElevationSolutionGroup  (bagHandle hnd, bagData *data)
{
    bagError       err;
    herr_t         herr;
    
    if ((data->opt[Elevation_Solution_Group].datatype = H5Tcreate(H5T_COMPOUND, sizeof (bagOptElevationSolutionGroup))) < 0)
    {
        return BAG_HDF_TYPE_CREATE_FAILURE;
    }

    herr = H5Tinsert (data->opt[Elevation_Solution_Group].datatype, "shoal_elevation", HOFFSET(bagOptElevationSolutionGroup, shoal_elevation), H5T_NATIVE_FLOAT);
    if (herr < 0)
        return BAG_HDF_TYPE_CREATE_FAILURE;

    herr = H5Tinsert (data->opt[Elevation_Solution_Group].datatype, "stddev", HOFFSET(bagOptElevationSolutionGroup, stddev), H5T_NATIVE_FLOAT);
    if (herr < 0)
        return BAG_HDF_TYPE_CREATE_FAILURE;

    herr = H5Tinsert (data->opt[Elevation_Solution_Group].datatype, "num_soundings", HOFFSET(bagOptElevationSolutionGroup, num_soundings), H5T_NATIVE_UINT);
    if (herr < 0)
        return BAG_HDF_TYPE_CREATE_FAILURE;

    err = bagCreateOptionalDataset (hnd, data, Elevation_Solution_Group);

    return err;
}


/****************************************************************************************/
/*! \brief  bagReadMinMaxElevationSolutionGroup
 *
 * Description:
 *     This function populates minGroup and maxGroup with the min/max attrs for
 *      the optional Elevation Solution Group dataset.
 * 
 *  \param    hnd     - pointer to the structure which ultimately contains the bag
 *  \param    minGroup    - pointer to user allocated struct for min values of Elevation Solution Group
 *  \param    maxGroup    - pointer to user allocated struct for max values of Elevation Solution Group
 *
 * \return On success, a value of zero is returned.  On failure a value of -1 is returned.  
 *
 ****************************************************************************************/
bagError bagReadMinMaxElevationSolutionGroup (bagHandle hnd, bagOptElevationSolutionGroup *minGroup, bagOptElevationSolutionGroup *maxGroup)
{
    herr_t   status;

    if (hnd == NULL)
        return BAG_INVALID_BAG_HANDLE;

    if (minGroup == NULL || maxGroup == NULL)
        return  BAG_INVALID_FUNCTION_ARGUMENT;
    
    if ((status = bagReadAttribute ((bagHandle)(hnd), (hnd)->opt_dataset_id[Elevation_Solution_Group], (u8 *)"min_stddev", &(minGroup->stddev))) != BAG_SUCCESS)
    {
        return (status);
    }
    if ((status = bagReadAttribute ((bagHandle)(hnd), (hnd)->opt_dataset_id[Elevation_Solution_Group], (u8 *)"max_stddev", &(maxGroup->stddev))) != BAG_SUCCESS)
    {
        return (status);
    }
    check_hdf_status();
    if ((status = bagReadAttribute ((bagHandle)(hnd), (hnd)->opt_dataset_id[Elevation_Solution_Group], (u8 *)"min_num_soundings", &(minGroup->num_soundings))) != BAG_SUCCESS)
    {
        return (status);
    }
    if ((status = bagReadAttribute ((bagHandle)(hnd), (hnd)->opt_dataset_id[Elevation_Solution_Group], (u8 *)"max_num_soundings", &(maxGroup->num_soundings))) != BAG_SUCCESS)
    {
        return (status);
    }
    check_hdf_status();
    if ((status = bagReadAttribute ((bagHandle)(hnd), (hnd)->opt_dataset_id[Elevation_Solution_Group], (u8 *)"min_shoal_elevation", &(minGroup->shoal_elevation))) != BAG_SUCCESS)
    {
        return (status);
    }
    if ((status = bagReadAttribute ((bagHandle)(hnd), (hnd)->opt_dataset_id[Elevation_Solution_Group], (u8 *)"max_shoal_elevation", &(maxGroup->shoal_elevation))) != BAG_SUCCESS)
    {
        return (status);
    }
    
    check_hdf_status();
		    

    return BAG_SUCCESS;
}

/****************************************************************************************/
/*! \brief  bagReadMinMaxNodeGroup
 *
 * Description:
 *     This function populates minGroup and maxGroup with the min/max attrs for
 *      the optional Elevation Solution Group dataset.
 * 
 *  \param    hnd     - pointer to the structure which ultimately contains the bag
 *  \param    minGroup    - pointer to user allocated struct for min values of Elevation Solution Group
 *  \param    maxGroup    - pointer to user allocated struct for max values of Elevation Solution Group
 *
 * \return On success, a value of zero is returned.  On failure a value of -1 is returned.  
 *
 ****************************************************************************************/
bagError bagReadMinMaxNodeGroup (bagHandle hnd, bagOptNodeGroup *minGroup, bagOptNodeGroup *maxGroup)
{
    herr_t   status;

    if (hnd == NULL)
        return BAG_INVALID_BAG_HANDLE;

    if (minGroup == NULL || maxGroup == NULL)
        return  BAG_INVALID_FUNCTION_ARGUMENT;
    

    if ((status = bagReadAttribute ((bagHandle)(hnd), (hnd)->opt_dataset_id[Node_Group], (u8 *)"min_hyp_strength", &(minGroup->hyp_strength))) != BAG_SUCCESS)
    {
        return (status);
    }
    if ((status = bagReadAttribute ((bagHandle)(hnd), (hnd)->opt_dataset_id[Node_Group], (u8 *)"max_hyp_strength", &(maxGroup->hyp_strength))) != BAG_SUCCESS)
    {
        return (status);
    }
    check_hdf_status();
    if ((status = bagReadAttribute ((bagHandle)(hnd), (hnd)->opt_dataset_id[Node_Group], (u8 *)"min_num_hypotheses", &(minGroup->num_hypotheses))) != BAG_SUCCESS)
    {
        return (status);
    }
    if ((status = bagReadAttribute ((bagHandle)(hnd), (hnd)->opt_dataset_id[Node_Group], (u8 *)"max_num_hypotheses", &(maxGroup->num_hypotheses))) != BAG_SUCCESS)
    {
        return (status);
    }
    check_hdf_status();

    return BAG_SUCCESS;
}

bagError bagReadMinMaxVarResMetadataGroup(bagHandle hnd, bagVarResMetadataGroup *minGroup, bagVarResMetadataGroup *maxGroup)
{
    herr_t status;
    
    if (hnd == NULL) return BAG_INVALID_BAG_HANDLE;
    if (minGroup == NULL || maxGroup == NULL) return BAG_INVALID_FUNCTION_ARGUMENT;
    
    if ((status = bagReadAttribute(hnd, hnd->opt_dataset_id[VarRes_Metadata_Group],
                                   (u8*)"min_dimensions_x", &(minGroup->dimensions_x))) != BAG_SUCCESS)
        return status;
    if ((status = bagReadAttribute(hnd, hnd->opt_dataset_id[VarRes_Metadata_Group],
                                   (u8*)"max_dimensions_x", &(maxGroup->dimensions_x))) != BAG_SUCCESS)
        return status;
    if ((status = bagReadAttribute(hnd, hnd->opt_dataset_id[VarRes_Metadata_Group],
                                   (u8*)"min_dimensions_y", &(minGroup->dimensions_y))) != BAG_SUCCESS)
        return status;
    if ((status = bagReadAttribute(hnd, hnd->opt_dataset_id[VarRes_Metadata_Group],
                                   (u8*)"max_dimensions_y", &(maxGroup->dimensions_y))) != BAG_SUCCESS)
        return status;
    if ((status = bagReadAttribute(hnd, hnd->opt_dataset_id[VarRes_Metadata_Group],
                                   (u8*)"min_resolution_x", &(minGroup->resolution_x))) != BAG_SUCCESS)
        return status;
    if ((status = bagReadAttribute(hnd, hnd->opt_dataset_id[VarRes_Metadata_Group],
                                   (u8*)"max_resolution_x", &(maxGroup->resolution_x))) != BAG_SUCCESS)
        return status;
    if ((status = bagReadAttribute(hnd, hnd->opt_dataset_id[VarRes_Metadata_Group],
                                   (u8*)"min_resolution_y", &(minGroup->resolution_y))) != BAG_SUCCESS)
        return status;
    if ((status = bagReadAttribute(hnd, hnd->opt_dataset_id[VarRes_Metadata_Group],
                                   (u8*)"max_resolution_y", &(maxGroup->resolution_y))) != BAG_SUCCESS)
        return status;
    check_hdf_status();
    
    /* We don't store index max/min since they're defined by the data structure.  We can provide min/max values that are possible,
     * however, in order to make this uniform.
     */
    minGroup->index = 0;
    maxGroup->index = BAG_NULL_VARRES_INDEX - 1;
    
    return BAG_SUCCESS;
}

bagError bagReadMinMaxVarResRefinementGroup(bagHandle hnd, bagVarResRefinementGroup *minGroup, bagVarResRefinementGroup *maxGroup)
{
    herr_t status;
    
    if (hnd == NULL) return BAG_INVALID_BAG_HANDLE;
    if (minGroup == NULL || maxGroup == NULL) return BAG_INVALID_FUNCTION_ARGUMENT;
    
    if ((status = bagReadAttribute(hnd, hnd->opt_dataset_id[VarRes_Refinement_Group], (u8*)"min_depth", &(minGroup->depth))) != BAG_SUCCESS)
        return status;
    if ((status = bagReadAttribute(hnd, hnd->opt_dataset_id[VarRes_Refinement_Group], (u8*)"max_depth", &(maxGroup->depth))) != BAG_SUCCESS)
        return status;
    if ((status = bagReadAttribute(hnd, hnd->opt_dataset_id[VarRes_Refinement_Group], (u8*)"min_uncrt", &(minGroup->depth_uncrt))) != BAG_SUCCESS)
        return status;
    if ((status = bagReadAttribute(hnd, hnd->opt_dataset_id[VarRes_Refinement_Group], (u8*)"max_uncrt", &(maxGroup->depth_uncrt))) != BAG_SUCCESS)
        return status;
    
    return BAG_SUCCESS;
}

bagError bagReadMinMaxVarResNodeGroup(bagHandle hnd, bagVarResNodeGroup *minGroup, bagVarResNodeGroup *maxGroup)
{
    herr_t status;
    
    if (hnd == NULL) return BAG_INVALID_BAG_HANDLE;
    if (minGroup == NULL || maxGroup == NULL) return BAG_INVALID_FUNCTION_ARGUMENT;
    
    if ((status = bagReadAttribute(hnd, hnd->opt_dataset_id[VarRes_Node_Group], (u8*)"min_hyp_strength", &(minGroup->hyp_strength))) != BAG_SUCCESS)
        return status;
    if ((status = bagReadAttribute(hnd, hnd->opt_dataset_id[VarRes_Node_Group], (u8*)"max_hyp_strength", &(maxGroup->hyp_strength))) != BAG_SUCCESS)
        return status;
    if ((status = bagReadAttribute(hnd, hnd->opt_dataset_id[VarRes_Node_Group], (u8*)"min_num_hypotheses", &(minGroup->num_hypotheses))) != BAG_SUCCESS)
        return status;
    if ((status = bagReadAttribute(hnd, hnd->opt_dataset_id[VarRes_Node_Group], (u8*)"max_num_hypotheses", &(maxGroup->num_hypotheses))) != BAG_SUCCESS)
        return status;
    if ((status = bagReadAttribute(hnd, hnd->opt_dataset_id[VarRes_Node_Group], (u8*)"min_n_samples", &(minGroup->n_samples))) != BAG_SUCCESS)
        return status;
    if ((status = bagReadAttribute(hnd, hnd->opt_dataset_id[VarRes_Node_Group], (u8*)"max_n_samples", &(maxGroup->n_samples))) != BAG_SUCCESS)
        return status;

    return BAG_SUCCESS;
}
