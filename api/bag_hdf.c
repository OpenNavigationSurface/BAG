/*! \file bag_hdf.c
 * \brief This is the main module for HDF-related operations of a BAG file.
 ********************************************************************
 *
 * Module Name : bag_hdf.c
 *
 * Author/Date : ONSWG, July 2005
 *
 * Description : 
 *               This is the main module for HDF-related operations of 
 *               a BAG file.
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 * who  when      what
 * ---  ----      ----
 * Webb McDonald -- Wed Jun 29 15:33:10 2011
 *  -added compression support
 *
 * Webb McDonald -- Fri Mar  2 14:13:35 2007
 *   -each "surface" now has a bunch of HDF structs initialized at the
 *    this bag_hdf.c level now instead of temporarily being opened and
 *    closed within every I/O call in bag_surfaces.c module
 *   -dataset, datatype, filespace \a hid_t will be initialized here,
 *    but memspace varies and might be reopened within each call in bag_surfaces.
 *   -bagFileCreate had to be reorganized so that the \a bagWriteXMLStream
 *    is performed at the end, after closing and reopening the entire BAG file
 *   -bagFileClose of course was modified to handle the job of closing all
 *    the HDF structs that were moved into the private \a BagHandle
 *
 * Webb McDonald -- Fri Jan 27 14:35:10 2006
 *   -added hdf property argument for creation of depth and uncertainty datasets to
 *    automatically fill with proper NULL values
 *   -added creation of the tracking list dataset
 *   -added bagGetErrorString() function
 *
 * Classification : Unclassified
 *
 * References : 
 *
 ********************************************************************/

/*! standard c library includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*! bag module specific header files */
#include "bag_private.h" 

/*! Macros definitions for this module */
#define	META_DATA_BLOCK_SIZE           1024           /*! \brief The block size defines the allocation/allocation extension unit size */
#define   BAG_OPEN_CREATE  3 /*! special mode for \a bagFileOpen \a access_mode */

/********************************************************************/
/*! \brief bagFileCreate
 *
 * Description : 
 *
 * *** Per HDF5 Documentation, applications should avoid calling the ***
 * *** create function with the name of an already opened file.      ***
 *   
 * On completion, the file remains open.
 * On completion, the BAG_root group remains open.
 * On completion, all datasets and all attributes are closed.
 *
 * \param *file_name   A string provides the filesystem name of the BAG
 * \param *data        A pointer to properly initialized bagData struct
 * \param *bag_handle  bag_handle will be set to the allocated \a bagHandle
 *                     used by the caller for subsequent external reference 
 *                     to the actual private object used within the library.
 *
 * \return \li On success, \a bagError is set to \a BAG_SUCCESS
 *         \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS
 *
 ********************************************************************/
bagError bagFileCreate(const u8 *file_name, bagData *data, bagHandle *bag_handle) 
{
    u32          i, length;
    herr_t       status;
    hsize_t      dims[RANK];
    hsize_t      dim_init[1]; 
    hsize_t      dim_max[1];
    hid_t        file_id;
    hid_t        dataset_id;
    hid_t        dataspace_id;
    hid_t        datatype_id; 
    hid_t        bagGroupID; 
    hid_t        plist_id;
    f32          null_elv = NULL_ELEVATION;
    f32          null_unc = NULL_UNCERTAINTY;

    /* chunking data block */
    hsize_t      chunk_dims[1];
    hsize_t      chunk_size[RANK] = {0,0};
    hid_t        cparms;

    *bag_handle = (bagHandle) calloc (1, sizeof (struct _t_bagHandle));
    if (*bag_handle == (bagHandle) NULL)
    {
        return (BAG_MEMORY_ALLOCATION_FAILED);
    }

    /*! Make sure \a *data has been properly loaded by the caller */
    if ((data == (bagData *) NULL)         || 
        (data->metadata == (u8 *) NULL)    ||
        (data->def.nrows < 1)              ||
        (data->def.ncols < 1))
    {
        return (BAG_INVALID_FUNCTION_ARGUMENT);
    }


    /*! Copy bagData from the passed argument into the abstracted \a *bag_handle area.
     *  We expect that the calling application has loaded these fields of the 
     *  passed \a *data argument.
     */
    memcpy (&((*bag_handle)->bag.def), &data->def, sizeof (bagDef));
    strncpy ((char *)(* bag_handle)->filename, (char *)file_name, MAX_STR-1);

    /*! init all the HDF structs to -1 */
    (* bag_handle)->unc_memspace_id  = 
    (* bag_handle)->trk_memspace_id  = 
    (* bag_handle)->mta_memspace_id  = 
    (* bag_handle)->elv_memspace_id  = 
    (* bag_handle)->unc_dataset_id   =  
    (* bag_handle)->trk_dataset_id   =  
    (* bag_handle)->mta_dataset_id   =  
    (* bag_handle)->elv_dataset_id   =  
    (* bag_handle)->unc_filespace_id = 
    (* bag_handle)->trk_filespace_id = 
    (* bag_handle)->mta_filespace_id = 
    (* bag_handle)->elv_filespace_id = 
    (* bag_handle)->unc_datatype_id  = 
    (* bag_handle)->trk_datatype_id  = 
    (* bag_handle)->mta_datatype_id  = 
    (* bag_handle)->mta_cparms_id    = 
    (* bag_handle)->elv_datatype_id  = -1;

    for (i=0; i < BAG_OPT_SURFACE_LIMIT; i++)
    {
        (* bag_handle)->opt_memspace_id[i]  = 
        (* bag_handle)->opt_dataset_id [i]  =  
        (* bag_handle)->opt_filespace_id[i] = 
        (* bag_handle)->opt_datatype_id[i]  = -1;

        (*bag_handle)->dataArray[i]         = (f32 *) NULL;
        (*bag_handle)->bag.opt[i].data      = (f32 **) NULL;
    }
    (*bag_handle)->elevationArray    = NULL;
    (*bag_handle)->uncertaintyArray  = NULL;
    (*bag_handle)->cryptoBlock       = NULL;
    (*bag_handle)->bag.elevation     = NULL;
    (*bag_handle)->bag.uncertainty   = NULL;
    (*bag_handle)->bag.tracking_list = NULL;

    /*! Create the file with default HDF5 properties, but only if the file does not already exist */
    if ((file_id = H5Fcreate((char *)file_name, H5F_ACC_EXCL, H5P_DEFAULT, H5P_DEFAULT)) < 0)
    {    
        return (BAG_HDF_CREATE_FILE_FAILURE);
    }

    (*bag_handle)->file_id = file_id;

    /*! Create the BAG_root path */
    if ((bagGroupID = H5Gcreate(file_id, ROOT_PATH, 0)) < 0)
    {    
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_GROUP_FAILURE);
    }

    (*bag_handle)->bagGroupID = bagGroupID;

    /*! Create the mandatory attributes for this level */
    if ((status = bagCreateAttribute (*bag_handle, bagGroupID, (u8 *)BAG_VERSION_NAME, BAG_VERSION_LENGTH, BAG_ATTR_CS1)) != BAG_SUCCESS)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
    } 
    if ((status = bagWriteAttribute (*bag_handle, bagGroupID, (u8 *)BAG_VERSION_NAME, (void *) BAG_VERSION)) != BAG_SUCCESS)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
    }

    /*! Create the dataset for the XML metadata */
    dim_init[0] = strlen((char *)data->metadata);
    dim_max[0]  = H5S_UNLIMITED;

    if ((dataspace_id = H5Screate_simple (1, dim_init, dim_max)) < 0)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_DATASPACE_FAILURE);
    }
    
    if ((cparms = H5Pcreate (H5P_DATASET_CREATE)) < 0)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_PROPERTY_CLASS_FAILURE);        
    }
    
    chunk_dims[0] = META_DATA_BLOCK_SIZE;
    if ((status = H5Pset_chunk (cparms, 1, chunk_dims)) < 0)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_SET_PROPERTY_FAILURE);
    }

    if ((datatype_id = H5Tcopy(H5T_C_S1)) < 0)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_TYPE_COPY_FAILURE);
    }
    
    if ((dataset_id = H5Dcreate(file_id, METADATA_PATH, datatype_id, dataspace_id, cparms)) <0)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_DATASET_FAILURE);
    }
    
    if ((status = H5Dextend (dataset_id, dim_init)) < 0)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_DATASET_EXTEND_FAILURE);
    }

    status = H5Pclose(cparms);
    status = H5Dclose(dataset_id);
    status = H5Sclose(dataspace_id);
    status = H5Tclose(datatype_id);
    check_hdf_status();

    /*! Create the dataset for the tracking list */
    dim_init[0] = 0;
    dim_max[0]  = H5S_UNLIMITED;

    if ((dataspace_id = H5Screate_simple (1, dim_init, dim_max)) < 0)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_DATASPACE_FAILURE);
    }
    
    if ((datatype_id = H5Tcreate(H5T_COMPOUND, sizeof (bagTrackingItem))) < 0)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_TYPE_COPY_FAILURE);
    }
    status = H5Tinsert (datatype_id, "row", HOFFSET(bagTrackingItem, row), H5T_NATIVE_UINT);
    status = H5Tinsert (datatype_id, "col", HOFFSET(bagTrackingItem, col), H5T_NATIVE_UINT);
    status = H5Tinsert (datatype_id, "depth", HOFFSET(bagTrackingItem, depth), H5T_NATIVE_FLOAT);
    status = H5Tinsert (datatype_id, "uncertainty", HOFFSET(bagTrackingItem, uncertainty), H5T_NATIVE_FLOAT);
    status = H5Tinsert (datatype_id, "track_code", HOFFSET(bagTrackingItem, track_code), H5T_NATIVE_UCHAR);
    status = H5Tinsert (datatype_id, "list_series", HOFFSET(bagTrackingItem, list_series), H5T_NATIVE_SHORT);
    check_hdf_status();

   /*  status = H5Pset_fill_time  (plist_id, H5D_FILL_TIME_ALLOC); */
/*     status = H5Pset_fill_value (plist_id, datatype_id, &null_elv); */
/*     check_hdf_status(); */
    
    if ((cparms = H5Pcreate (H5P_DATASET_CREATE)) < 0)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_PROPERTY_CLASS_FAILURE);        
    }
    
    /*! tracking_list will access chunks of items at a time for faster performance (hopefully) */
    chunk_dims[0] = TRACKING_LIST_BLOCK_SIZE;
    if ((status = H5Pset_chunk (cparms, 1, chunk_dims)) < 0)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_SET_PROPERTY_FAILURE);
    }


    if (data->compressionLevel > 0 && data->compressionLevel <= 9)
    {
        if ((status = H5Pset_deflate (cparms, data->compressionLevel)) < 0)
        {
            status = H5Fclose (file_id);
            return (BAG_HDF_SET_PROPERTY_FAILURE);
        }
    }
    else if (data->compressionLevel) /* if anything other than zero */
    {
        return (BAG_HDF_INVALID_COMPRESSION_LEVEL);
    }

    if ((dataset_id = H5Dcreate(file_id, TRACKING_LIST_PATH, datatype_id, dataspace_id, cparms)) < 0)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_DATASET_FAILURE);
    }
    
    if ((status = H5Dextend (dataset_id, dim_init)) < 0)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_DATASET_EXTEND_FAILURE);
    }

    /*! Add the attributes to the \a tracking list dataset */
    if ((status = bagCreateAttribute (*bag_handle, dataset_id, (u8 *)TRACKING_LIST_LENGTH_NAME, sizeof(u32), BAG_ATTR_U32)) != BAG_SUCCESS)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
    }
    length = 0;
    if ((status = bagWriteAttribute (*bag_handle, dataset_id, (u8 *)TRACKING_LIST_LENGTH_NAME, (void *) &length)) != BAG_SUCCESS)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
    }

    status = H5Pclose(cparms);
    status = H5Dclose(dataset_id);
    status = H5Sclose(dataspace_id);
    status = H5Tclose(datatype_id);

    dims[0] = data->def.nrows;
    dims[1] = data->def.ncols;

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

    /*! Create the mandatory \a elevation dataset */
    if ((dataspace_id = H5Screate_simple(RANK, dims, NULL)) < 0)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_DATASPACE_FAILURE);
    }

    if ((datatype_id = H5Tcopy(H5T_NATIVE_FLOAT)) < 0)
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
    status = H5Pset_fill_value (plist_id, datatype_id, &null_elv);
    check_hdf_status();


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

    if ((dataset_id = H5Dcreate(file_id, ELEVATION_PATH, datatype_id, dataspace_id, plist_id)) < 0)
    {    
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_GROUP_FAILURE);
    }

    /*! Add the attributes to the \a elevation dataset */
    if ((status = bagCreateAttribute (*bag_handle, dataset_id, (u8 *)MIN_ELEVATION_NAME, sizeof(f32), BAG_ATTR_F32)) != BAG_SUCCESS)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
    }
    if ((status = bagWriteAttribute (*bag_handle, dataset_id, (u8 *)MIN_ELEVATION_NAME, (void *) &(data->min_elevation) )) != BAG_SUCCESS)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
    }
    if ((status = bagCreateAttribute (*bag_handle, dataset_id, (u8 *)MAX_ELEVATION_NAME, sizeof(f32), BAG_ATTR_F32)) != BAG_SUCCESS)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
    }
    if ((status = bagWriteAttribute (*bag_handle, dataset_id, (u8 *)MAX_ELEVATION_NAME, (void *) &(data->max_elevation) )) != BAG_SUCCESS)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
    }

    status = H5Dclose (dataset_id);
    status = H5Sclose (dataspace_id);
    status = H5Tclose (datatype_id);  
   
    /*! Create the mandatory \a uncertainty dataset */
    dims[0] = data->def.nrows;
    dims[1] = data->def.ncols;
    if ((dataspace_id = H5Screate_simple(RANK, dims, NULL)) < 0)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_DATASPACE_FAILURE);
    }

    if ((datatype_id = H5Tcopy(H5T_NATIVE_FLOAT)) < 0)
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
    status = H5Pset_fill_value (plist_id, datatype_id, &null_unc);
    check_hdf_status();


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

    if ((dataset_id = H5Dcreate(file_id, UNCERTAINTY_PATH, datatype_id, dataspace_id, plist_id)) < 0)
    {    
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_GROUP_FAILURE);
    }

    /*! Add the attributes to the \a elevation dataset */
    if ((status = bagCreateAttribute (*bag_handle, dataset_id, (u8 *)MIN_UNCERTAINTY_NAME, sizeof(f32), BAG_ATTR_F32)) != BAG_SUCCESS)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
    }
    if ((status = bagWriteAttribute (*bag_handle, dataset_id, (u8 *)MIN_UNCERTAINTY_NAME, (void *) &(data->min_uncertainty) )) != BAG_SUCCESS)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
    }
    if ((status = bagCreateAttribute (*bag_handle, dataset_id, (u8 *)MAX_UNCERTAINTY_NAME, sizeof(f32), BAG_ATTR_F32)) != BAG_SUCCESS)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
    }
    if ((status = bagWriteAttribute (*bag_handle, dataset_id, (u8 *)MAX_UNCERTAINTY_NAME, (void *) &(data->max_uncertainty) )) != BAG_SUCCESS)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
    }

    status = H5Dclose (dataset_id);
    status = H5Sclose (dataspace_id);
    status = H5Tclose (datatype_id);
    status = H5Pclose (plist_id);
    check_hdf_status();

    if ((status = bagFileClose (*bag_handle)) != BAG_SUCCESS)
    {
        return BAG_HDF_INTERNAL_ERROR;
    }

    /*! reopen the \a bag, but without trying to parse the non-existent \a metadata yet */
    if ((status = bagFileOpen (bag_handle, BAG_OPEN_CREATE, file_name)) != BAG_SUCCESS)
    {
        return status;
    }

    length = (u32)strlen((char *)data->metadata);
    if (length < XML_METADATA_MIN_LENGTH)
    {
        return (BAG_INVALID_FUNCTION_ARGUMENT);
    }

    if ((*bag_handle)->bag.metadata == (u8 *) NULL)
    {
        return (BAG_MEMORY_ALLOCATION_FAILED);
    }
    memcpy ((*bag_handle)->bag.metadata, data->metadata, length);

    /*! Write the XML metadata string into its dataset */
    if ((status = bagWriteXMLStream (*bag_handle)) != BAG_SUCCESS)
    {
        H5Fclose (file_id);
        return status;
    }

    return (status >= 0) ? BAG_SUCCESS : BAG_HDF_INTERNAL_ERROR;
}

/********************************************************************/
/*! \brief bagFileOpen
 *
 * Description : 
 *     This function opens a BAG file stored in HDF5.  The library supports 
 *     access for up to 20 separate BAG files at a time.
 *
 * \param *bag_handle   bag_handle will be set to the allocated \a bagHandle
 *                      private object for subsequent external reference
 *                      by the caller.
 * \param  access_mode  Entity \a BAG_OPEN_MODE
 * \param *file_name    A string provides the filesystem name of the BAG
 *
 * \return \li On success, \a bagError is set to \a BAG_SUCCESS
 *         \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS
 *
 ********************************************************************/
bagError bagFileOpen(bagHandle *bag_handle, s32 access_mode, const u8 *file_name)
{
    bagError     status;
    u8           version[BAG_VERSION_LENGTH+16];
    hsize_t      max_dims[RANK];
    hid_t        plist_id;

    /*! chunking data block */
    hsize_t     chunk_dimsr[1];
    hsize_t	    count[1];
    s32         i, rank_chunk;

    
    *bag_handle = (bagHandle) calloc (1, sizeof (struct _t_bagHandle));
    if (*bag_handle == (bagHandle) NULL)
    {
        return (BAG_MEMORY_ALLOCATION_FAILED);
    }

    (* bag_handle)->cryptoBlock = NULL;
    strncpy ((char *)(* bag_handle)->filename, (char *)file_name, MAX_STR-1);

    /*! \brief open the BAG */
    if (BAG_OPEN_READ_WRITE == access_mode ||
        BAG_OPEN_CREATE == access_mode)
    {
        /*! 
         * Before opening for RW, store the \a ONSCryptoBlock, which apparently gets
         * destroyed whenever you open the BAG for writing.  There are many cases 
         * when we want it preserved.
         */
        (* bag_handle)->cryptoBlock = calloc (DEFAULT_KEY_LEN, sizeof(u8));
        status = bagReadCertification ((char *)file_name,
                                       (* bag_handle)->cryptoBlock,
                                       DEFAULT_KEY_LEN,
                                       &((* bag_handle)->cryptoID));

        if (status != BAG_CRYPTO_SIGNATURE_OK)
        {
            /*!
               nothing to do here because there is not guarantee of there being
               a \a cryptoBlock anyway, right?
            */
            free ((* bag_handle)->cryptoBlock);
            (* bag_handle)->cryptoBlock = NULL;
        }


        if (((* bag_handle)->file_id = H5Fopen((char *)file_name, H5F_ACC_RDWR, H5P_DEFAULT)) < 0)
        {
           return (BAG_HDF_FILE_OPEN_FAILURE);
        }
    }
    else
    {
      if (((* bag_handle)->file_id = H5Fopen((char *)file_name, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0)
        {
           return (BAG_HDF_FILE_OPEN_FAILURE);
        }
    }
    
    /*! init all the HDF structs to -1 */
    (* bag_handle)->unc_memspace_id  = 
    (* bag_handle)->trk_memspace_id  = 
    (* bag_handle)->mta_memspace_id  = 
    (* bag_handle)->elv_memspace_id  = 
    (* bag_handle)->unc_dataset_id   =  
    (* bag_handle)->trk_dataset_id   =  
    (* bag_handle)->mta_dataset_id   =  
    (* bag_handle)->elv_dataset_id   =  
    (* bag_handle)->unc_filespace_id = 
    (* bag_handle)->trk_filespace_id = 
    (* bag_handle)->mta_filespace_id = 
    (* bag_handle)->elv_filespace_id = 
    (* bag_handle)->unc_datatype_id  = 
    (* bag_handle)->trk_datatype_id  = 
    (* bag_handle)->mta_datatype_id  = 
    (* bag_handle)->mta_cparms_id    = 
    (* bag_handle)->elv_datatype_id  = -1;

    for (i=0; i < BAG_OPT_SURFACE_LIMIT; i++)
    {
        (* bag_handle)->opt_memspace_id[i]  = 
        (* bag_handle)->opt_dataset_id [i]  =  
        (* bag_handle)->opt_filespace_id[i] = 
        (* bag_handle)->opt_datatype_id[i]  = -1;

        (*bag_handle)->dataArray[i]         = (f32 *) NULL;
        (*bag_handle)->bag.opt[i].data      = (f32 **) NULL;
    }
    (*bag_handle)->elevationArray    = NULL;
    (*bag_handle)->uncertaintyArray  = NULL;
    (*bag_handle)->cryptoBlock       = NULL;
    (*bag_handle)->bag.elevation     = NULL;
    (*bag_handle)->bag.uncertainty   = NULL;
    (*bag_handle)->bag.tracking_list = NULL;

    if (((* bag_handle)->bagGroupID = H5Gopen ((* bag_handle)->file_id, ROOT_PATH)) < 0)
    {
        H5Fclose ((* bag_handle)->file_id);
        return (BAG_HDF_GROUP_OPEN_FAILURE);
    }
    
    /*! Read out the \a attributes */ 
    memset (version, 0, sizeof(version));
    if ((status = bagReadAttribute ((* bag_handle), (* bag_handle)->bagGroupID, (u8 *)BAG_VERSION_NAME, version)) != BAG_SUCCESS)
    {
        return (status);
    }

    memset ((* bag_handle)->bag.version, 0, BAG_VERSION_LENGTH);
    strncpy ((char *)(* bag_handle)->bag.version, (char *)version, sizeof((* bag_handle)->bag.version));

    /*!  Open the Elevation dataset and then the supporting HDF structures */

    (* bag_handle)->elv_dataset_id = H5Dopen((* bag_handle)->file_id, ELEVATION_PATH);
    if ((* bag_handle)->elv_dataset_id < 0)
            return BAG_HDF_DATASET_OPEN_FAILURE; 
    if ((status = bagReadAttribute ((* bag_handle), (* bag_handle)->elv_dataset_id, (u8 *)MIN_ELEVATION_NAME, &(* bag_handle)->bag.min_elevation)) != BAG_SUCCESS)
    {
        return (status);
    }
    if ((status = bagReadAttribute ((* bag_handle), (* bag_handle)->elv_dataset_id, (u8 *)MAX_ELEVATION_NAME, &(* bag_handle)->bag.max_elevation)) != BAG_SUCCESS)
    {
        return (status);
    }

    /*! Obtain Elevation datatype */
    if (((* bag_handle)->elv_datatype_id = H5Dget_type((* bag_handle)->elv_dataset_id)) < 0)
        return BAG_HDF_TYPE_NOT_FOUND;

    /*! Obtain Elevation file space  */
    (* bag_handle)->elv_filespace_id = H5Dget_space((* bag_handle)->elv_dataset_id);
    if ((* bag_handle)->elv_filespace_id < 0)
    {
        return BAG_HDF_DATASPACE_CORRUPTED;
    }


    /*!  Open the Uncertainty dataset and then the supporting HDF structures */

    (* bag_handle)->unc_dataset_id = H5Dopen((* bag_handle)->file_id, UNCERTAINTY_PATH);
    if ((* bag_handle)->unc_dataset_id < 0)
            return BAG_HDF_DATASET_OPEN_FAILURE; 
    if ((status = bagReadAttribute ((* bag_handle), (* bag_handle)->unc_dataset_id, (u8 *)MIN_UNCERTAINTY_NAME, &(* bag_handle)->bag.min_uncertainty)) != BAG_SUCCESS)
    {
        return (status);
    }
    if ((status = bagReadAttribute ((* bag_handle), (* bag_handle)->unc_dataset_id, (u8 *)MAX_UNCERTAINTY_NAME, &(* bag_handle)->bag.max_uncertainty)) != BAG_SUCCESS)
    {
        return (status);
    }

    /*! Obtain Uncertainty datatype */
    if (((* bag_handle)->unc_datatype_id = H5Dget_type((* bag_handle)->unc_dataset_id)) < 0)
        return BAG_HDF_TYPE_NOT_FOUND;

    /*! Obtain Uncertainty file space  */
    (* bag_handle)->unc_filespace_id = H5Dget_space((* bag_handle)->unc_dataset_id);
    if ((* bag_handle)->unc_filespace_id < 0)
    {
        return BAG_HDF_DATASPACE_CORRUPTED;
    }

    (* bag_handle)->bag.compressionLevel = 0;
    /*! Obtain the compression level from the dataset property list if set */
    if ((plist_id = H5Dget_create_plist((* bag_handle)->unc_dataset_id)) >= 0)
    {
        s32 nfilt = H5Pget_nfilters(plist_id);
        for (i=0; i < nfilt; i++)
        {
            u32 flags;
            size_t cd_nelmts=10,  name_len=64;
            u32  cd_values[10];
            char name[64];

            if (H5Z_FILTER_DEFLATE == H5Pget_filter(plist_id, i, &flags, &cd_nelmts, cd_values, name_len, name ))
            {
                if (cd_nelmts >= 1)
                {
                    (* bag_handle)->bag.compressionLevel = cd_values[0];
                }
                break;
            }
        }
        
        H5Pclose(plist_id);
    }

    /*! Open the Tracking list dataset. */
    (* bag_handle)->trk_dataset_id = H5Dopen((* bag_handle)->file_id, (char *)TRACKING_LIST_PATH);
    if ((* bag_handle)->trk_dataset_id < 0)
        return BAG_HDF_DATASET_OPEN_FAILURE; 

    if (((* bag_handle)->trk_datatype_id = H5Dget_type((* bag_handle)->trk_dataset_id)) < 0)
        return BAG_HDF_TYPE_NOT_FOUND;

    /*! Open the Tracking list filespace */
    (* bag_handle)->trk_filespace_id = H5Dget_space((* bag_handle)->trk_dataset_id);
    if ((* bag_handle)->trk_filespace_id < 0)
    {
        return BAG_HDF_DATASPACE_CORRUPTED;
    }
    
    count[0]  = TRACKING_LIST_BLOCK_SIZE;  /* chunk size */

    /*! define the Tracking list memspace */
    if (((* bag_handle)->trk_memspace_id = H5Screate_simple (1, count, NULL)) < 0)
    {
        return BAG_HDF_CREATE_DATASPACE_FAILURE;
    }

    /*! Open the \a metadata dataset */
    (* bag_handle)->mta_dataset_id = H5Dopen((* bag_handle)->file_id, (char *)METADATA_PATH);
    if ((* bag_handle)->mta_dataset_id < 0)
        return BAG_HDF_DATASET_OPEN_FAILURE; 

    if (((* bag_handle)->mta_datatype_id = H5Dget_type((* bag_handle)->mta_dataset_id)) < 0)
        return BAG_HDF_TYPE_NOT_FOUND;

    /*! Open the filespace for the case where we're reading into a NULL \a metadata */
    (* bag_handle)->mta_filespace_id = H5Dget_space((* bag_handle)->mta_dataset_id);
    if ((* bag_handle)->mta_filespace_id < 0)
    {
        return BAG_HDF_DATASPACE_CORRUPTED;
    }
    if (((* bag_handle)->mta_cparms_id = H5Dget_create_plist ((* bag_handle)->mta_dataset_id)) < 0)
        return BAG_HDF_CREATE_PROPERTY_CLASS_FAILURE;
    
    if (H5D_CHUNKED == H5Pget_layout ((* bag_handle)->mta_cparms_id))
    {
        rank_chunk = H5Pget_chunk ((* bag_handle)->mta_cparms_id, 1, chunk_dimsr);
    }

    (* bag_handle)->bag.metadata = (u8 *)calloc (XML_METADATA_MAX_LENGTH, sizeof (u8));
	
    if ((* bag_handle)->bag.metadata == (u8 *) NULL)
    {
        return (BAG_MEMORY_ALLOCATION_FAILED);
    }

    /*!  With the HDF structs in place, we're ready to read the max_dims and nrows/ncols */
    if ((status = bagReadSurfaceDims ((* bag_handle), max_dims)) != BAG_SUCCESS)
    {
        H5Fclose ((* bag_handle)->file_id);
        return status;
    }
    (* bag_handle)->bag.def.nrows = (u32)max_dims[0];
    (* bag_handle)->bag.def.ncols = (u32)max_dims[1];

    if (access_mode != BAG_OPEN_CREATE)
    {
        /*!
          MP - Removed a simple direct read of the XML data and called the full XML parsing function.
          Previously this wasn't being called so parts of the bag definition structure were not 
          being properly filled.  
          
          WXM - Unfortunately the Xerces parsing leaves a slight one-time memory leak...
                One should keep \a bagFileOpen calls to a minimum.
        */
        if( (status = bagInitDefinitionFromBag( *bag_handle )) != BAG_SUCCESS )
        {
            return status;
        }
    }

    /*! Diables the HDF5-Diag Error messages once the inital bagFileOpen has completed */
    H5Eset_auto (NULL, NULL);

    return (BAG_SUCCESS);
}

/********************************************************************/
/*! \brief : bagFileClose
 *
 * Description : 
 *   This function closes a BAG file previously opened via bagFileOpen.
 *
 * \param bag_handle  External reference to the private object 
 *                      used within the library.
 *
 * \return : \li On success, \a bagError is set to \a BAG_SUCCESS
 *           \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS
 *
 ********************************************************************/

bagError bagFileClose (bagHandle bag_handle)
{
    herr_t    status;

    if (bag_handle == NULL)
        return BAG_INVALID_BAG_HANDLE;

    /*! close any optional surface structures */
    if ((status = bagFreeInfoOpt(bag_handle)) != BAG_SUCCESS)
    {
        return status;
    }

    if (bagGetDataPointer(bag_handle)->metadata != NULL)
        free (bagGetDataPointer(bag_handle)->metadata);

    if (bagGetDataPointer(bag_handle)->metadataDef != NULL)
    {
        bagFreeMetadata(bagGetDataPointer(bag_handle)->metadataDef);
        free(bagGetDataPointer(bag_handle)->metadataDef);
    }

    if ((status = H5Gclose (bag_handle->bagGroupID)) < 0)
    {
        return(BAG_HDF_GROUP_CLOSE_FAILURE);
    }

    /*! close the \a HDF entities */
    if (bag_handle->trk_memspace_id >= 0)
    {
        status = H5Sclose (bag_handle->trk_memspace_id);
        check_hdf_status();
    }
    if (bag_handle->trk_filespace_id >= 0)
    {
        status = H5Sclose (bag_handle->trk_filespace_id);
        check_hdf_status();
    }
    if (bag_handle->trk_datatype_id >= 0)
    {
        status = H5Tclose (bag_handle->trk_datatype_id);
        check_hdf_status();
    }
    if (bag_handle->trk_dataset_id >= 0)
    {
        status = H5Dclose (bag_handle->trk_dataset_id);
        check_hdf_status();
    }
    if (bag_handle->mta_memspace_id >= 0)
    {
        status = H5Sclose (bag_handle->mta_memspace_id);
        check_hdf_status();
    }
    if (bag_handle->mta_cparms_id >= 0)
    {
        status = H5Pclose (bag_handle->mta_cparms_id);
        check_hdf_status();
    }
    if (bag_handle->mta_filespace_id >= 0)
    {
        status = H5Sclose (bag_handle->mta_filespace_id);
        check_hdf_status();
    }
    if (bag_handle->mta_datatype_id >= 0)
    {
        status = H5Tclose (bag_handle->mta_datatype_id);
        check_hdf_status();
    }
    if (bag_handle->mta_dataset_id >= 0)
    {
        status = H5Dclose (bag_handle->mta_dataset_id);
        check_hdf_status();
    }
    if (bag_handle->unc_memspace_id >= 0)
    {
        status = H5Sclose (bag_handle->unc_memspace_id);
        check_hdf_status();
    }
    if (bag_handle->unc_filespace_id >= 0)
    {
        status = H5Sclose (bag_handle->unc_filespace_id);
        check_hdf_status();
    }
    if (bag_handle->unc_datatype_id >= 0)
    {
        status = H5Tclose (bag_handle->unc_datatype_id);
        check_hdf_status();
    }
    if (bag_handle->unc_dataset_id >= 0)
    {
        status = H5Dclose (bag_handle->unc_dataset_id);
        check_hdf_status();
    }
    if (bag_handle->elv_memspace_id >= 0)
    {
        status = H5Sclose (bag_handle->elv_memspace_id);
        check_hdf_status();
    }
    if (bag_handle->elv_filespace_id >= 0)
    {
        status = H5Sclose (bag_handle->elv_filespace_id);
        check_hdf_status();
    }
    if (bag_handle->elv_datatype_id >= 0)
    {
        status = H5Tclose (bag_handle->elv_datatype_id);
        check_hdf_status();
    }
    if (bag_handle->elv_dataset_id >= 0)
    {
        status = H5Dclose (bag_handle->elv_dataset_id);
        check_hdf_status();
    }
    
    /*! close the main file */
    if ((status = H5Fclose (bag_handle->file_id)) < 0)
    {
        return(BAG_HDF_FILE_CLOSE_FAILURE);
    }
     
    /*! if BAG was opened for a \a READ_WRITE, restore the \a ONSCryptoBlock */
    if (bag_handle->cryptoBlock != NULL)
    {
        status  = bagWriteCertification ((char *)bag_handle->filename,
                                         bag_handle->cryptoBlock,
                                         bag_handle->cryptoID);
        
        free (bag_handle->cryptoBlock);
        bag_handle->cryptoBlock = NULL;
        if (status != BAG_CRYPTO_SIGNATURE_OK)
            return status;
    }
    /*! free previously calloc'd bagHandle */
    free (bag_handle);
    return (BAG_SUCCESS);   
}

/********************************************************************/
/*! \brief : bagGetDataPointer
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
bagData *bagGetDataPointer(bagHandle bag_handle)
{
    if (bag_handle == NULL)
        return NULL;
    else
        return (&bag_handle->bag);
}


/****************************************************************************************/
/*! \brief bagGetErrorString
 *
 * Description:
 *     This function provides a short text description for the last error that 
 *     occurred on the BAG specified by bagHandle. Memory for the text string 
 *     is maintained within the library and calling applications do not free this memory.
 *
 * \param    code     Error string will be assigned based on the \a bagError argument.
 * \param  **error    The address of a string pointer which will be updated 
 *                    to point to a short text string describing the error condition.
 *
 * \return            On success the function returns \a BAG_SUCCESS You can always 
 *           call this function again to see what the error was from the results of the
 *           original call to \a bagGetErrorString.
 *
 ****************************************************************************************/

bagError bagGetErrorString(bagError code, u8 **error)
{
    static char str[MAX_STR];

    str[0] = '\0';

    if (error == NULL)
      return BAG_INVALID_FUNCTION_ARGUMENT;
    *error = (u8 *) str;

    switch (code) 
    {
    case BAG_SUCCESS:
        strncpy (str, "Bag returned a successful completion", MAX_STR-1);
        break;
    case BAG_BAD_FILE_IO_OPERATION:
        strncpy (str, "Bad status with file IO operation", MAX_STR-1);
        break;
    case BAG_NO_FILE_FOUND:
        strncpy (str, "Filename not found", MAX_STR-1);
        break;
    case BAG_NO_ACCESS_PERMISSION:
        strncpy (str, "Failed to access Bag because of permissions", MAX_STR-1);
        break;
    case BAG_MEMORY_ALLOCATION_FAILED:
        strncpy (str, "Memory allocation failed", MAX_STR-1);
        break;
    case BAG_INVALID_BAG_HANDLE:
        strncpy (str, "Invalid or NULL bagHandle", MAX_STR-1);
        break;
    case BAG_INVALID_FUNCTION_ARGUMENT:
        strncpy (str, "Invalid function argument or illegal value passed to Bag", MAX_STR-1);
        break;
    case BAG_METADTA_NO_HOME:
        strncpy (str, "The BAG_HOME environment variable must be set to the configdata directory of the openns distribution", MAX_STR-1);
        break;
    case BAG_METADTA_SCHEMA_FILE_MISSING:
        strncpy (str, "Metadata schema file(s) (*.xsd) was missing, they should be within configdata under smXML. Check BAG_HOME path", MAX_STR-1);
        break;
    case BAG_METADTA_PARSE_MEM_EXCEPTION:
        strncpy (str, "Metadata exceeded memory limit during parsing", MAX_STR-1);
        break;
    case BAG_METADTA_PARSE_EXCEPTION:
        strncpy (str, "Metadata parsing encountered an fatal exception", MAX_STR-1);
        break;
    case BAG_METADTA_PARSE_DOM_EXCEPTION:
        strncpy (str, "Metadata parsing encountered a DOM (document object model) exception, impossible to complete operation", MAX_STR-1);
        break;
    case BAG_METADTA_PARSE_UNK_EXCEPTION:
        strncpy (str, "Metadata parsing encountered an unknown error", MAX_STR-1);
        break;
    case BAG_METADTA_PARSE_FAILED:
        strncpy (str, "Metadata parsing failed, unable to parse input file", MAX_STR-1);
        break;
    case BAG_METADTA_PARSE_FAILED_MEM:
        strncpy (str, "Metadata parsing failed, unable to parse specified input buffer memory overflow", MAX_STR-1);
        break;
    case BAG_METADTA_VALIDATE_FAILED:
        strncpy (str, "Metadata XML validation failed", MAX_STR-1);
        break;
    case BAG_METADTA_INVALID_HANDLE:
        strncpy (str, "Metadata Invalid (NULL) bagHandle supplied to an accessor method", MAX_STR-1);
        break;
    case BAG_METADTA_INIT_FAILED:
        strncpy (str, "Metadata Initialization of the low level XML support system failed", MAX_STR-1);
        break;
    case BAG_METADTA_NO_PROJECTION_INFO:
        strncpy (str, "Metadata No projection information was found in the XML supplied", MAX_STR-1);
        break;
    case BAG_METADTA_INSUFFICIENT_BUFFER:
        strncpy (str, "Metadata supplied buffer is not large enough to hold the extracted contents from XML", MAX_STR-1);
        break;
    case BAG_METADTA_UNCRT_MISSING:
        strncpy (str, "Metadata 'uncertaintyType' information is missing from the XML structure", MAX_STR-1);
        break;
    case BAG_METADTA_INCOMPLETE_COVER:
        strncpy (str, "Metadata One or more elements of the requested coverage are missing from the XML file", MAX_STR-1);
        break;
    case BAG_METADTA_INVLID_DIMENSIONS:
        sprintf (str, "Metadata The number of dimensions is incorrect (not equal to %d)", RANK);
        break;
    case BAG_METADTA_BUFFER_EXCEEDED:
        strncpy (str, "Metadata supplied buffer is too large to be stored in the internal array", MAX_STR-1);
        break;
	case BAG_METADTA_DPTHCORR_MISSING:
		strncpy (str, "The 'depthCorrectionType' information is missing from the XML structure", MAX_STR-1);
		break;
    case BAG_METADTA_RESOLUTION_MISSING:
        strncpy (str, "Metadata resolution information is missing from the XML structure", MAX_STR-1);
        break;
    case BAG_METADTA_INVALID_PROJECTION:
        strncpy (str, "Metadata unsupported projection being used", MAX_STR-1);
        break;
    case BAG_METADTA_INVALID_DATUM:
        strncpy (str, "Metadata unsupported datum being used", MAX_STR-1);
        break;
    case BAG_METADTA_INVALID_HREF:
        strncpy (str, "Metadata horizontal reference system is invalid", MAX_STR-1);
        break;
    case BAG_METADTA_INVALID_VREF:
        strncpy (str, "Metadata vertical reference system is invalid", MAX_STR-1);
        break;
	case BAG_METADTA_SCHEMA_SETUP_FAILED:
		strncpy (str, "Failed to setup the xml schema", MAX_STR-1);
        break;
	case BAG_METADTA_SCHEMA_VALIDATION_SETUP_FAILED:
		strncpy (str, "Failed to setup the xml schema validation", MAX_STR-1);
        break;
	case BAG_METADTA_EMPTY_DOCUMENT:
		strncpy (str, "The metadata document is emtpy", MAX_STR-1);
        break;
	case BAG_METADTA_MISSING_MANDATORY_ITEM:
		strncpy (str, "The metadata is missing a mandatory item", MAX_STR-1);
        break;
    case BAG_METADTA_NOT_INITIALIZED:
        strncpy (str, "The metadata has not been initialized correctly", MAX_STR-1);
        break;
    case BAG_NOT_HDF5_FILE:
        strncpy (str, "HDF Bag is not an HDF5 File", MAX_STR-1);
        break;
    case BAG_HDF_RANK_INCOMPATIBLE:
        sprintf (str, "HDF Bag's rank is incompatible with expected Rank of the Datasets: %d", RANK);
        break;
    case BAG_HDF_TYPE_NOT_FOUND:
        strncpy (str, "HDF Bag surface Datatype parameter not available", MAX_STR-1);
        break;
    case BAG_HDF_DATASPACE_CORRUPTED:
        strncpy (str, "HDF Dataspace for a bag surface is corrupted or could not be read", MAX_STR-1);
        break;
    case BAG_HDF_ACCESS_EXTENTS_ERROR:
        strncpy (str, "HDF Failure in request for access outside the extents of a bag surface's Dataset", MAX_STR-1);
        break;
    case BAG_HDF_CANNOT_WRITE_NULL_DATA:
        strncpy (str, "HDF Cannot write NULL or uninitialized data to Dataset", MAX_STR-1);
        break;
    case BAG_HDF_INTERNAL_ERROR:
        strncpy (str, "HDF There was an internal HDF error detected", MAX_STR-1);
        break;
    case BAG_HDF_CREATE_FILE_FAILURE:
        strncpy (str, "HDF Unable to create new HDF Bag File", MAX_STR-1);
        break;
    case BAG_HDF_CREATE_DATASPACE_FAILURE:
        strncpy (str, "HDF Unable to create the Dataspace", MAX_STR-1);
        break;
    case BAG_HDF_CREATE_PROPERTY_CLASS_FAILURE:
        strncpy (str, "HDF Unable to create the Property class", MAX_STR-1);
        break;
    case BAG_HDF_SET_PROPERTY_FAILURE:
        strncpy (str, "HDF Unable to set value of Property class", MAX_STR-1);
        break;
    case BAG_HDF_TYPE_COPY_FAILURE:
        strncpy (str, "HDF Failed to copy Datatype parameter for Dataset access", MAX_STR-1);
        break;
    case BAG_HDF_CREATE_DATASET_FAILURE:
        strncpy (str, "HDF Unable to create the Dataset", MAX_STR-1);
        break;
    case BAG_HDF_DATASET_EXTEND_FAILURE:
        strncpy (str, "HDF Cannot extend Dataset extents", MAX_STR-1);
        break;
    case BAG_HDF_CREATE_ATTRIBUTE_FAILURE:
        strncpy (str, "HDF Unable to create Attribute", MAX_STR-1);
        break;
    case BAG_HDF_CREATE_GROUP_FAILURE:
        strncpy (str, "HDF Unable to create Group", MAX_STR-1);
        break;
    case BAG_HDF_WRITE_FAILURE:
        strncpy (str, "HDF Failure writing to Dataset", MAX_STR-1);
        break;
    case BAG_HDF_READ_FAILURE:
        strncpy (str, "HDF Failure reading from Dataset", MAX_STR-1);
        break;
    case BAG_HDF_GROUP_CLOSE_FAILURE:
        strncpy (str, "HDF Failure closing Group", MAX_STR-1);
        break;
    case BAG_HDF_FILE_CLOSE_FAILURE:
        strncpy (str, "HDF Failure closing File", MAX_STR-1);
        break;
    case BAG_HDF_FILE_OPEN_FAILURE:
        strncpy (str, "HDF Unable to open File", MAX_STR-1);
        break;
    case BAG_HDF_GROUP_OPEN_FAILURE:
        strncpy (str, "HDF Unable to open Group", MAX_STR-1);
        break;
    case BAG_HDF_ATTRIBUTE_OPEN_FAILURE:
        strncpy (str, "HDF Unable to open Attribute", MAX_STR-1);
        break;
    case BAG_HDF_ATTRIBUTE_CLOSE_FAILURE:
        strncpy (str, "HDF Failure closing Attribute", MAX_STR-1);
        break;
    case BAG_HDF_DATASET_CLOSE_FAILURE:
        strncpy (str, "HDF Failure closing Dataset", MAX_STR-1);
        break;
    case BAG_HDF_DATASET_OPEN_FAILURE:
        strncpy (str, "HDF Unable to open Dataset", MAX_STR-1);
        break;
    case BAG_HDF_TYPE_CREATE_FAILURE:
        strncpy (str, "HDF Unable to create Datatype", MAX_STR-1);
        break;
    case BAG_HDF_INVALID_COMPRESSION_LEVEL:
        strncpy (str, "HDF compression level not in acceptable range of 0 to 9", MAX_STR-1);
        break;
    case BAG_CRYPTO_SIGNATURE_OK:
        strncpy (str, "Crypto Signature is OK", MAX_STR-1);
        break;
    case BAG_CRYPTO_NO_SIGNATURE_FOUND:
        strncpy (str, "Crypto No signature was found", MAX_STR-1);
        break;
    case BAG_CRYPTO_BAD_SIGNATURE_BLOCK:
        strncpy (str, "Crypto Bad signature block", MAX_STR-1);
        break;
    case BAG_CRYPTO_BAD_KEY:
        strncpy (str, "Crypto Bad Key", MAX_STR-1);
        break;
    case BAG_CRYPTO_WRONG_KEY:
        strncpy (str, "Crypto Wrong key", MAX_STR-1);
        break;
    case BAG_CRYPTO_GENERAL_ERROR:
        strncpy (str, "Crypto General error was detected", MAX_STR-1);
        break;
    case BAG_CRYPTO_INTERNAL_ERROR:
        strncpy (str, "Crypto Internal error was detected", MAX_STR-1);
        break;
    case BAG_INVALID_ERROR_CODE:
    default:
        strncpy (str, "An undefined bagError code was encountered", MAX_STR-1);
        return BAG_INVALID_ERROR_CODE;
    }
    return BAG_SUCCESS;
}
