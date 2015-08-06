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
	
	if ((data->opt[VarRes_Metadata_Group].datatype = H5Tcreate(H5T_COMPOUND, sizeof(bagVarResMetadataGroup))) < 0) {
		return BAG_HDF_TYPE_CREATE_FAILURE;
	}
	herr = H5Tinsert(data->opt[VarRes_Metadata_Group].datatype, "index", HOFFSET(bagVarResMetadataGroup, index), H5T_NATIVE_UINT);
	if (herr < 0) return BAG_HDF_TYPE_CREATE_FAILURE;
	herr = H5Tinsert(data->opt[VarRes_Metadata_Group].datatype, "dimensions", HOFFSET(bagVarResMetadataGroup, dimensions), H5T_NATIVE_UINT);
	if (herr < 0) return BAG_HDF_TYPE_CREATE_FAILURE;
	herr = H5Tinsert(data->opt[VarRes_Metadata_Group].datatype, "resolution", HOFFSET(bagVarResMetadataGroup, resolution), H5T_NATIVE_FLOAT);
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
    
    if ((status = bagReadAttribute(hnd, hnd->opt_dataset_id[VarRes_Metadata_Group], (u8*)"min_dimensions", &(minGroup->dimensions))) != BAG_SUCCESS)
        return status;
    if ((status = bagReadAttribute(hnd, hnd->opt_dataset_id[VarRes_Metadata_Group], (u8*)"max_dimensions", &(maxGroup->dimensions))) != BAG_SUCCESS)
        return status;
    if ((status = bagReadAttribute(hnd, hnd->opt_dataset_id[VarRes_Metadata_Group], (u8*)"min_resolution", &(minGroup->resolution))) != BAG_SUCCESS)
        return status;
    if ((status = bagReadAttribute(hnd, hnd->opt_dataset_id[VarRes_Metadata_Group], (u8*)"max_resolution", &(maxGroup->resolution))) != BAG_SUCCESS)
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
