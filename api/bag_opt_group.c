/*! \file bag_opt_group.c
 * \brief This module contains functions for accessing the optional surface datasets and the XML metadata.
 ********************************************************************
 *
 * Module Name : bag_opt_group.c
 *
 * Author/Date : ONSWG, September 2011
 *
 * Description : 
 *               Optiona surface datasets bagOptNodeGroup & bagOptElevationSolutionGroup.
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
