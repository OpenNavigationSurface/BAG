/*! \file bag_tracking_list.c
 * \brief This module defines the Tracking List dataset API
 *******************************************************************
 *
 * Module Name : bag_tracking_list.c
 *
 * Author/Date : Webb McDonald -- Thu Jan 26 16:41:03 2006
 *
 * Description : 
 *               Reading and writing of tracking list items is
 *               supported.  
 *               Creation of the dataset is performed in bag_hdf,
 *               along with creation of the BAG file itself.
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 * who  when      what
 * ---  ----      ----
 *
 * Classification : Unclassified
 *
 * References : 
 *
 ********************************************************************/


#include "bag_private.h"


/****************************************************************************************/
/*! \brief :     bagReadTrackingListIndex
 * Purpose:     Read the one list item at the index provided. 
 *              This is mostly to allow application to read out of the entire list
 *              one element at a time.
 *
 *  \param       bagHandle    Handle for the Bag file
 *  \param       index        tracking list index
 *  \param      *item         This pointer must point to memory already allocated
 *                             by the caller for a bagTrackingItem.
 *
 *  \return  \li On success, \a bagError is set to \a BAG_SUCCESS
 *           \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS
 *
 ****************************************************************************************/
bagError bagReadTrackingListIndex (bagHandle bagHandle, u16 index, bagTrackingItem *item)
{
    herr_t      status;
    u32         list_len;

    /* hyperslab selection parameters */
    hsize_t	    count[1];
    hssize_t	offset[1];
    hid_t       memspace_id, 
                datatype_id,
                dataset_id,
                filespace_id;
    u8         *dataset_name;

    if (bagHandle == NULL)
        return BAG_INVALID_BAG_HANDLE;

    dataset_name = TRACKING_LIST_PATH;
 
    /*! Open an existing dataset. */
    dataset_id = H5Dopen(bagHandle->file_id, dataset_name);
    if (dataset_id < 0)
        return BAG_HDF_DATASET_OPEN_FAILURE; 

    if ((datatype_id = H5Dget_type(dataset_id)) < 0)
        return BAG_HDF_TYPE_NOT_FOUND;

    /*! Open the filespace */
    filespace_id = H5Dget_space(dataset_id);
    if (filespace_id < 0)
    {
        H5Tclose (datatype_id);
        H5Dclose (dataset_id);
        return BAG_HDF_DATASPACE_CORRUPTED;
    }

    if ((status = bagReadAttribute (bagHandle, dataset_id, TRACKING_LIST_LENGTH_NAME, &list_len)) < 0)
    {
        H5Sclose (filespace_id);
        H5Tclose (datatype_id);
        H5Dclose (dataset_id);
        return (status);
    }
    
    /*! beware~ error conditions if \a index exceeds \a list_len extents or supplied \a *item is NULL */
    if (item == NULL || list_len <= index)
    {
        H5Sclose (filespace_id);
        H5Tclose (datatype_id);
        H5Dclose (dataset_id);
        return BAG_INVALID_FUNCTION_ARGUMENT;
    }

    count[0]     = 1;      /*! chunk size */
    offset[0]    = index;  /*! simply seek to index */

    /*! define the memspace */
    if ((memspace_id = H5Screate_simple (1, count, NULL)) < 0)
    {
        H5Sclose (filespace_id);
        H5Tclose (datatype_id);
        H5Dclose (dataset_id);
        return BAG_HDF_CREATE_DATASPACE_FAILURE;
    }

    status = H5Sselect_hyperslab (filespace_id, H5S_SELECT_SET, offset, NULL, count, NULL);
    status = H5Dread (dataset_id, datatype_id, memspace_id, filespace_id, 
                      H5P_DEFAULT, item);
    check_hdf_status();

    /*! did what we came to do, now close up HDF objects*/
    status = H5Dclose (dataset_id);
    check_hdf_status();
    status = H5Sclose (memspace_id);
    check_hdf_status();
    status = H5Sclose (filespace_id);
    check_hdf_status();
    status = H5Tclose (datatype_id);
    check_hdf_status();

    return BAG_SUCCESS;
}

/****************************************************************************************/
/*! \brief :     bagReadTrackingListCode
 *
 * Purpose:     Read all tracking list items from a particular node.
 * Comment:     Caller must free the memory at \a *items if length is greater than 0.
 *              Caller must assign \a *items a NULL value before using this function!
 *
 *  \param       bagHandle       Handle for the Bag file
 *  \param       code         \a bagTrackCode for accessing all instances of a
 *                               reason code of \a bagTrackingItem  
 *  \param     **items           Pointer will be set to an allocated array of 
 *                            \a bagTrackingItems, or will be left \a NULL if there
 *                               are none at the node give by row/col. Pointer 
 *                               MUST be set to NULL before calling this function!
 *  \param       *length         The length of the list will be set to the number
 *                               of elements allocated in *items
 *
 *  \return   \li On success, \a bagError is set to \a BAG_SUCCESS
 *            \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS
 *
 ****************************************************************************************/
bagError bagReadTrackingListCode(bagHandle bagHandle, u8 code, bagTrackingItem **items, u32 *length)
{
    return bagReadTrackingList( bagHandle, READ_TRACK_CODE, code, code, items, length);
}

/****************************************************************************************/
/*! \brief :     bagReadTrackingListSeries
 *
 * Purpose:     Read all tracking list items from a specific list series index.
 *              In case the index is not ever changing, then this function can
 *              actually be used to read all tracking list items.
 *
 * Comment:     Caller must free the memory at \a *items if length is greater than 0.
 *              The \a index corresponds to a batch of tracking list updates whose
 *                   lineage will be maintained in the Bag's metadata.
 *              Caller must assign \a *items a \a NULL value before using this function!
 *
 * \param      bagHandle  Handle for the \a Bag file
 * \param      index      tracking list series index
 * \param    **items      pointer will be set to an allocated array of 
 *                        \a bagTrackingItems, or will be left NULL if there
 *                        are none at the node by given \a list_series index. Pointer 
 *                        MUST be set to \a NULL before calling this function!
 * \param       *length   The length of the list will be set to the number
 *                        of elements allocated in \a *items
 *
 * \return   \li On success, \a bagError is set to \a BAG_SUCCESS
 *           \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS
 *
 ****************************************************************************************/
bagError bagReadTrackingListSeries(bagHandle bagHandle, u16 index, bagTrackingItem **items, u32 *length)
{
    return bagReadTrackingList( bagHandle, READ_TRACK_SERIES, index, index, items, length);
}

/***************************************************************************************/
/*! \brief :     bagReadTrackingListNode
 *
 * Purpose:     Read all tracking list items from a particular node.
 *
 * Comment:     Caller must free the memory at items if length is greater than 0.
 *              Caller must assign items a \a NULL value before using this function!
 *
 * \param      bagHandle    Handle for the Bag file
 * \param      row          Coordinate
 * \param      col          Coordinate
 * \param    **items        Pointer will be set to an allocated array of 
 *                          \a bagTrackingItems, or will be left \a NULL if there
 *                          are none at the node given by row/col. Pointer 
 *                          MUST be set to NULL before calling this function!
 *            *length       The length of the list will be set to the number
 *                          of elements allocated in \a *items
 *
 * \return   \li On success, \a bagError is set to \a BAG_SUCCESS
 *           \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS
 *
 ****************************************************************************************/
bagError bagReadTrackingListNode(bagHandle bagHandle, u32 row, u32 col, bagTrackingItem **items, u32 *length)
{
    return bagReadTrackingList( bagHandle, READ_TRACK_RC, row, col, items, length);
}

/***************************************************************************************/
/*! 
 * This one is the single private function that the read by node and the read by index
 * functions go thru to access the tracking list dataset.
 *
 * The detailed prototype comments from those functions apply here.
 ****************************************************************************************/

bagError bagReadTrackingList(bagHandle bagHandle, u16 mode, u32 inp1, u32 inp2, bagTrackingItem **items, u32 *rtn_len)
{
    herr_t      status;
    u32         list_len;
    u32         row = inp1, col = inp2;
    u16         code = inp1, series = inp1;

    /* hyperslab selection parameters */
    hsize_t	    count[1];
    hssize_t	offset[1];
    hid_t       memspace_id, 
                datatype_id,
                dataset_id,
                filespace_id;
    u8         *dataset_name;

    /*! tracking item buffers and allocation structs */
    bagTrackingItem readbuf[TRACKING_LIST_BLOCK_SIZE];
    
    if (bagHandle == NULL)
        return BAG_INVALID_BAG_HANDLE;

    /*! beware - \a *items must be \a NULL first~ */
    if ((*items) != NULL)
        return BAG_INVALID_FUNCTION_ARGUMENT;
    (*items) = NULL;

    dataset_name = TRACKING_LIST_PATH;
 
    /*! Open an existing dataset. */
    dataset_id = H5Dopen(bagHandle->file_id, dataset_name);
    if (dataset_id < 0)
        return BAG_HDF_DATASET_OPEN_FAILURE; 

    if ((datatype_id = H5Dget_type(dataset_id)) < 0)
        return BAG_HDF_TYPE_NOT_FOUND;

    /*! Open the filespace */
    filespace_id = H5Dget_space(dataset_id);
    if (filespace_id < 0)
    {
        H5Tclose (datatype_id);
        H5Dclose (dataset_id);
        return BAG_HDF_DATASPACE_CORRUPTED;
    }

    if ((status = bagReadAttribute (bagHandle, dataset_id, TRACKING_LIST_LENGTH_NAME, &list_len)) < 0)
    {
        H5Sclose (filespace_id);
        H5Tclose (datatype_id);
        H5Dclose (dataset_id);
        return (status);
    }

    *rtn_len  = 0;                         /*! nothing to read yet */
    count[0]  = TRACKING_LIST_BLOCK_SIZE;  /* chunk size */
    offset[0] = 0;                         /*! start at head of list */

    /*! define the hyperslab parameters */
    if ((memspace_id = H5Screate_simple (1, count, NULL)) < 0)
    {
        H5Sclose (filespace_id);
        H5Tclose (datatype_id);
        H5Dclose (dataset_id);
        return BAG_HDF_CREATE_DATASPACE_FAILURE;
    }

    while (offset[0] < list_len)
    {
        int i;

        /*! shrink the memspace if we have less than a chunk left to read from
         * the tracking list */
        if (list_len - offset[0] < TRACKING_LIST_BLOCK_SIZE) 
        {
            status = H5Sclose (memspace_id);
            check_hdf_status();
            count[0] = (list_len % TRACKING_LIST_BLOCK_SIZE);

            /* define the hyperslab parameters */
            if ((memspace_id = H5Screate_simple (1, count, NULL)) < 0)
            {
                H5Sclose (filespace_id);
                H5Tclose (datatype_id);
                H5Dclose (dataset_id);
                return BAG_HDF_CREATE_DATASPACE_FAILURE;
            }
        }

        status = H5Sselect_hyperslab (filespace_id, H5S_SELECT_SET, offset, NULL, count, NULL);
        check_hdf_status();
        
        status = H5Dread (dataset_id, datatype_id, memspace_id, filespace_id, 
                          H5P_DEFAULT, readbuf);
        check_hdf_status();

        /*! loop thru what we just read into *readBuf */
        for (i=0; i < count[0]; i++)
        {
            /* found a match? */
            if ((mode == READ_TRACK_SERIES && readbuf[i].list_series == series) ||
                (mode == READ_TRACK_CODE && readbuf[i].track_code == code) ||
                (mode == READ_TRACK_RC   && readbuf[i].col == col && readbuf[i].row == row))
            {
                /*! alloc for new list or expand plus 1 item */
                if ((*items) == NULL)
                {
                    (*items) = calloc (1, sizeof(bagTrackingItem));
                    if ((*items) == NULL)
                    {
                        H5Sclose (memspace_id);
                        H5Sclose (filespace_id);
                        H5Tclose (datatype_id);
                        H5Dclose (dataset_id);
                        return BAG_MEMORY_ALLOCATION_FAILED;
                    }
                }
                else
                {
                    bagTrackingItem *tmp;
                    tmp = realloc ((*items), sizeof(bagTrackingItem) * ((*rtn_len) + 1));
                    if (tmp == NULL)
                    {
                        H5Sclose (memspace_id);
                        H5Sclose (filespace_id);
                        H5Tclose (datatype_id);
                        H5Dclose (dataset_id);
                        free ((*items));
                        return BAG_MEMORY_ALLOCATION_FAILED;
                    }
                    (*items) = tmp; /*! make sure (*items) is set to the newly alloc'd mem */
                }

                /*! copy into next location */
                memcpy (&((*items)[(*rtn_len)]), &readbuf[i], sizeof(bagTrackingItem));
 
                (*rtn_len)++;
            }
        }
        offset[0] += TRACKING_LIST_BLOCK_SIZE;
    }

    /*! did what we came to do, now close up HDF objects */
    status = H5Dclose (dataset_id);
    check_hdf_status();
    status = H5Sclose (memspace_id);
    check_hdf_status();
    status = H5Sclose (filespace_id);
    check_hdf_status();
    status = H5Tclose (datatype_id);
    check_hdf_status();

    return BAG_SUCCESS;
}

/***************************************************************************************/
/*! \brief :     bagWriteTrackingListItem
 *
 * Purpose:     Write a single \a bagTrackingItem into the tracking_list dataset.
 *
 * \param       bagHandle    Handle for the \a Bag file
 * \param      *item         pointer to tracking list item
 * \return      bagError     Will be set if there is an error accessing the 
 *                           bagHandle, or if item is \a NULL
 *
 ****************************************************************************************/
bagError bagWriteTrackingListItem(bagHandle bagHandle, bagTrackingItem *item)
{
    herr_t      status;
    u32         list_len;

    /* hyperslab selection parameters */
    hsize_t	    count[1];
    hssize_t	offset[1];
    hid_t       memspace_id, 
                datatype_id,
                dataset_id,
                filespace_id;
    u8         *dataset_name;
    hsize_t	    extend[1];
    
    if (bagHandle == NULL)
        return BAG_INVALID_BAG_HANDLE;

    dataset_name = TRACKING_LIST_PATH;
 
    /*! Open an existing dataset. */
    dataset_id = H5Dopen(bagHandle->file_id, dataset_name);
    if (dataset_id < 0)
    {
        H5Dclose (dataset_id);
        return BAG_HDF_DATASET_OPEN_FAILURE; 
    }

    if ((datatype_id = H5Dget_type(dataset_id)) < 0)
    {
        H5Dclose (dataset_id);
        return BAG_HDF_TYPE_NOT_FOUND;
    }

    /*! Open the filespace */
    filespace_id = H5Dget_space(dataset_id);
    if (filespace_id < 0)
    {
        H5Tclose (datatype_id);
        H5Dclose (dataset_id);
        return BAG_HDF_DATASPACE_CORRUPTED;
    }

    if ((status = bagReadAttribute (bagHandle, dataset_id, TRACKING_LIST_LENGTH_NAME, &list_len)) < 0)
    {
        H5Tclose (datatype_id);
        H5Dclose (dataset_id);
        return (status);
    }

    count[0]  = 1;          /*! adding 1 item */
    offset[0] = list_len;   /*! add it to end of list */
    extend[0] = ++list_len; /*! increase extents by 1 */

    /*! let the tracking_list grow */
    status = H5Dextend (dataset_id, extend);
    check_hdf_status();

    /*! Close and reopen below  */
    status = H5Sclose (filespace_id);
    check_hdf_status();

    /*! Prepare the dataspaces */
    if ((filespace_id = H5Dget_space(dataset_id)) < 0)
    {
        H5Tclose (datatype_id);
        H5Dclose (dataset_id);
        return BAG_HDF_DATASPACE_CORRUPTED;
    }

    /*! define the hyperslab parameters */
    if ((memspace_id = H5Screate_simple (1, count, NULL)) < 0)
    {
        H5Sclose (filespace_id);
        H5Tclose (datatype_id);
        H5Dclose (dataset_id);
        return BAG_HDF_CREATE_DATASPACE_FAILURE;
    }

    status = H5Sselect_hyperslab (filespace_id, H5S_SELECT_SET, offset, NULL, count, NULL);
    check_hdf_status();
  
    status = H5Dwrite (dataset_id, datatype_id, memspace_id, filespace_id, 
                       H5P_DEFAULT, item);
    check_hdf_status();

    /*! definitely should update the list length attribute of the dataset */
    if ((status = bagWriteAttribute (bagHandle, dataset_id, TRACKING_LIST_LENGTH_NAME, &list_len)) < 0)
    {        
        H5Sclose (memspace_id);
        H5Sclose (filespace_id);
        H5Tclose (datatype_id);
        H5Dclose (dataset_id);
        return (status);
    }

    /*! did what we came to do, now close up */
    status = H5Dclose (dataset_id);
    check_hdf_status();
    status = H5Sclose (memspace_id);
    check_hdf_status();
    status = H5Sclose (filespace_id);
    check_hdf_status();
    status = H5Tclose (datatype_id);
    check_hdf_status();

    return BAG_SUCCESS;
}

/***************************************************************************************/
/*! \brief      bagTrackingListLength
 *
 * Purpose:     Read the tracking list length attribute. This is the total 
 *              length for the entire list, comprising all series of edits
 *              for this bag's surfaces.
 *
 * \param     bagHandle    Handle for the Bag file
 * \param    *len          Will be assigned the length of the list.
 * \return   \li On success, \a bagError is set to \a BAG_SUCCESS
 *           \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS
 *
 ****************************************************************************************/
bagError bagTrackingListLength (bagHandle bagHandle, u32 *len)
{
    bagError    status;
    hid_t       dataset_id;
    u8         *dataset_name;

    *len = 0;

    if (bagHandle == NULL)
        return BAG_INVALID_BAG_HANDLE;
    
    dataset_name = TRACKING_LIST_PATH;
 
    /*! Open an existing dataset. */
    dataset_id = H5Dopen(bagHandle->file_id, dataset_name);
    if (dataset_id < 0)
        return BAG_HDF_DATASET_OPEN_FAILURE; 

    if ((status = bagReadAttribute (bagHandle, dataset_id, TRACKING_LIST_LENGTH_NAME, len)) < 0)
    {
        H5Dclose (dataset_id);
        return (status);
    }
    status = H5Dclose (dataset_id);
    check_hdf_status();

    return BAG_SUCCESS;
}


/***************************************************************************************/
/*! \brief :     bagSortTrackingList
 * Purpose:     Reads the entire tracking list into memory. This is the total 
 *              length for the entire list, comprising all series of edits
 *              for this bag's surfaces. Sorts either according to row/col 
 *              combination (for spatial locality) or list_series index.
 *
 * Comment:     Edits to BAG, resulting in tracking list items,
 *              could be appended somewhat in a chaotic fashion.  
 *              The user will later want them accessed usually in
 *              a logical ordering though, so these sorting routines are 
 *              offered for assistance and speed of future access.
 *
 * \param       bagHandle    Handle for the Bag file
 * \param       mode         Used to decide btwn \a list_series or R/C sorting.
 *
 * \return   \li On success, \a bagError is set to \a BAG_SUCCESS
 *           \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS
 *
 ****************************************************************************************/
bagError bagSortTrackingList(bagHandle bagHandle, u16 mode)
{
    herr_t      status;
    u32         list_len;

    /*! hyperslab selection parameters */
    hsize_t	    count[1];
    hssize_t	offset[1];
    hid_t       memspace_id, 
                datatype_id,
                dataset_id,
                filespace_id;
    u8         *dataset_name;

    /*! tracking item buffers and allocation structs */
    bagTrackingItem *readbuf;
    
    if (bagHandle == NULL)
        return BAG_INVALID_BAG_HANDLE;

    dataset_name = TRACKING_LIST_PATH;
 
    /*! Open an existing dataset. */
    dataset_id = H5Dopen(bagHandle->file_id, dataset_name);
    if (dataset_id < 0)
        return BAG_HDF_DATASET_OPEN_FAILURE; 

    if ((datatype_id = H5Dget_type(dataset_id)) < 0)
        return BAG_HDF_TYPE_NOT_FOUND;

    /*! Open the filespace */
    filespace_id = H5Dget_space(dataset_id);
    if (filespace_id < 0)
    {
        H5Tclose (datatype_id);
        H5Dclose (dataset_id);
        return BAG_HDF_DATASPACE_CORRUPTED;
    }

    if ((status = bagReadAttribute (bagHandle, dataset_id, TRACKING_LIST_LENGTH_NAME, &list_len)) < 0)
    {
        H5Sclose (filespace_id);
        H5Tclose (datatype_id);
        H5Dclose (dataset_id);
        return (status);
    }

    /*!
     * We're going to try and just read the entire list into memory here.
     * Tested with over 40000 items and performance was... inspirational.
     *   Right now I don't see a need to break this down any further.
     * Webb McDonald -- Thu Jan 26 10:44:45 2006
     */
    readbuf = calloc (list_len, sizeof(bagTrackingItem));
    if (readbuf == NULL)
    {
        H5Sclose (filespace_id);
        H5Tclose (datatype_id);
        H5Dclose (dataset_id);
        return BAG_MEMORY_ALLOCATION_FAILED;
    }

    count[0]  = list_len;  /*! chunk size */
    offset[0] = 0;         /*! start at head of list */

    /*! define the hyperslab parameters */
    if ((memspace_id = H5Screate_simple (1, count, NULL)) < 0)
    {
        H5Sclose (filespace_id);
        H5Tclose (datatype_id);
        H5Dclose (dataset_id);
        return BAG_HDF_CREATE_DATASPACE_FAILURE;
    }

    status = H5Sselect_hyperslab (filespace_id, H5S_SELECT_SET, offset, NULL, count, NULL);
    check_hdf_status();
    
    fprintf(stdout, "Reading entire tracking list dataset into memory...\n");
    fflush(stdout);
    status = H5Dread (dataset_id, datatype_id, memspace_id, filespace_id, 
                      H5P_DEFAULT, readbuf);
    check_hdf_status();

    fprintf(stdout, "Starting sort of %d items of the tracking list...\n", list_len);
    fflush(stdout);
    /*! \a READ_TRACK_MODE, mode indicates the type of sort */
    switch (mode)
    {
    case READ_TRACK_SERIES:
        qsort (readbuf, list_len, sizeof (bagTrackingItem), &bagCompareTrackIndices);
        break;
    case READ_TRACK_CODE:
        qsort (readbuf, list_len, sizeof (bagTrackingItem), &bagCompareTrackCodes);
        break;
    case READ_TRACK_RC:
    default:
        qsort (readbuf, list_len, sizeof (bagTrackingItem), &bagCompareTrackNodes);
        break;
    }

    fprintf(stdout, "Write entire tracking list dataset from memory back into the Bag...\n");
    fflush(stdout);
    status = H5Dwrite (dataset_id, datatype_id, memspace_id, filespace_id,
                       H5P_DEFAULT, readbuf);
    check_hdf_status();
    
    /*! did what we came to do, now close up */
    status = H5Dclose (dataset_id);
    check_hdf_status();
    status = H5Sclose (memspace_id);
    check_hdf_status();
    status = H5Sclose (filespace_id);
    check_hdf_status();
    status = H5Tclose (datatype_id);
    check_hdf_status();

    fprintf(stdout, "Sorting process completed\n");
    fflush(stdout);

    return BAG_SUCCESS;
}

/* see comments for common function above */
bagError bagSortTrackingListByNode (bagHandle bagHandle)
{
    return bagSortTrackingList( bagHandle, READ_TRACK_RC);
}
/* see comments for common function above */
bagError bagSortTrackingListBySeries (bagHandle bagHandle)
{
    return bagSortTrackingList( bagHandle, READ_TRACK_SERIES);
}
/* see comments for common function above */
bagError bagSortTrackingListByCode (bagHandle bagHandle)
{
    return bagSortTrackingList( bagHandle, READ_TRACK_CODE);
}


/********************************************************************
 *
 * Function Name : bagCompareTrackIndices
 *
 * Description : This is the alpha sort function for qsort.
 *
 * Inputs : void pointers a,b
 *
 * Returns :  a's index greater than b's index, rtn  1
 *            b's index greater than or equal to a's index, rtn 0
 *
 * Error Conditions : if a or b are uninitialized
 *
 ********************************************************************/
s32 bagCompareTrackIndices (const void *a, const void *b)
{
    bagTrackingItem *sa = (bagTrackingItem *)(a);
    bagTrackingItem *sb = (bagTrackingItem *)(b);

    if (sa == NULL || sb == NULL)
        return 0;
    else
        return (sa->list_series > sb->list_series);
}

/********************************************************************
 *
 * Function Name : bagCompareTrackNodes
 *
 * Description : This is the alpha sort function for qsort.
 *
 * Inputs : void pointers a,b
 *
 * Returns : a's index greater than b's index, rtn  1
 *           b's index greater than or equal to a's index, rtn 0
 *
 * Error Conditions : if a or b are uninitialized
 *
 ********************************************************************/
s32 bagCompareTrackNodes (const void *a, const void *b)
{
    bagTrackingItem *sa = (bagTrackingItem *)(a);
    bagTrackingItem *sb = (bagTrackingItem *)(b);

    if (sa == NULL || sb == NULL)
        return 0;
    else
        return (sa->row >= sb->row && sa->col >= sb->col);
}
    
/********************************************************************
 *
 * Function Name : bagCompareTrackCodes
 *
 * Description : This is the alpha sort function for qsort.
 *
 * Inputs : void pointers a,b
 *
 * Returns : a's index greater than b's index, rtn  1
 *           b's index greater than or equal to a's index, rtn 0
 *
 * Error Conditions : if a or b are uninitialized
 *
 ********************************************************************/
s32 bagCompareTrackCodes (const void *a, const void *b)
{
    bagTrackingItem *sa = (bagTrackingItem *)(a);
    bagTrackingItem *sb = (bagTrackingItem *)(b);

    if (sa == NULL || sb == NULL)
        return 0;
    else
        return (sa->track_code > sb->track_code);
}
