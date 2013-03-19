/*! \file bag_attr.c
 * \brief Attribute functions create, write, read 
 ****************************************************************************************
 *
 * Author/Date : ONSWG, July 2005
 *
 * Description : 
 *                This file just deals with attributes. 
 *      In HDF, these are best suited to be single element key-value
 *      pairs.  Currently the only arrayed datatype available is a
 *      limited sized string type.  Anything else should be stored
 *      as a proper "dataset", or in the BAG vernacular, a proper "surface".
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


/****************************************************************************************
 *
 * Attribute functions create, write, read
 *
 ****************************************************************************************/
/*! \brief bagCreateAttribute
 * 
 *         This function will try to create a new attribute.
 *         If the \a type is a character string, then a maximum string length argument
 *         must be specified as \a max_len.
 * 
 * \param bag        \li external reference to the Bag file descriptor
 * \param loc_id     \li location ID for the HDF object possessing this attribute
 * \param *attr_name  \li string name describing this attribute
 * \param max_len    \li a maximum length for string \a type attribtues
 * \param type       \li datatype, defined in \a bagAttrTypes
 * \return           \li On success, \a bagError is set to \a BAG_SUCCESS.
 *                   \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS.
 */
bagError bagCreateAttribute (bagHandle bag, hid_t loc_id, u8 *attr_name, u32 max_len, u32 type)
{
    herr_t   status;
    hid_t    attribute_id, dataspace_id, datatype_id;

    dataspace_id = H5Screate (H5S_SCALAR);
    if (dataspace_id < 0)
        return BAG_HDF_DATASPACE_CORRUPTED;

    switch (type)
    {
    case BAG_ATTR_F32:
        datatype_id  = H5Tcopy (H5T_NATIVE_FLOAT);     
        break;
    case BAG_ATTR_F64:
        datatype_id  = H5Tcopy (H5T_NATIVE_DOUBLE);     
        break;
    case BAG_ATTR_U8:
        datatype_id  = H5Tcopy (H5T_NATIVE_UCHAR);     
        break;
    case BAG_ATTR_CS1:
        datatype_id  = H5Tcopy (H5T_C_S1);     
        break;
    case BAG_ATTR_S32:
        datatype_id  = H5Tcopy (H5T_NATIVE_INT);     
        break;
    case BAG_ATTR_U32:
        datatype_id  = H5Tcopy (H5T_NATIVE_UINT);     
        break;
    default:
        return BAG_HDF_TYPE_NOT_FOUND;
        break;
    }
    
    if (datatype_id < 0)
    {
        H5Sclose (dataspace_id);
        return BAG_HDF_TYPE_COPY_FAILURE;
    }

    /*! \brief resizing of the attribute datatype is only supported for u8's
     * Take note of this constraint!
     *                   Webb McDonald -- Fri Jul 22 17:56:06 2005
     */
    if (type == BAG_ATTR_CS1)
    {
        status = H5Tset_size(datatype_id, max_len);
        check_hdf_status();
    }

    /*! Writing out the object metadata */
    attribute_id= H5Acreate(loc_id, (char *)attr_name, 
                            datatype_id, dataspace_id, H5P_DEFAULT);
    if (attribute_id < 0)
    {
        H5Sclose (dataspace_id);
        H5Tclose (datatype_id);
        return BAG_HDF_CREATE_ATTRIBUTE_FAILURE;
    }

    status = H5Aclose(attribute_id);
    check_hdf_status();
    status = H5Sclose(dataspace_id);
    check_hdf_status();
    status = H5Tclose(datatype_id);
    check_hdf_status();

    return BAG_SUCCESS;
}

/*! \brief bagWriteAttribute
 * 
 *         This function writes the attribute.
 * 
 * \param bag        \li external reference to the Bag file descriptor
 * \param loc_id     \li location ID for the HDF object possessing this attribute
 * \param *attr_name  \li string name describing this attribute
 * \param *value      \li User-supplied pointer to some data that is expected to be the same 
 *                        \a type as specified upon the creation of this attribute.
 * \return           \li On success, \a bagError is set to \a BAG_SUCCESS.
 *                   \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS.
 */
bagError bagWriteAttribute (bagHandle bag, hid_t loc_id, u8 *attr_name, void *value)
{
    return bagAlignAttribute (bag, loc_id, attr_name, value, WRITE_BAG);
}

/*! \brief bagReadAttribute
 * 
 *         This function reads the attribute.
 * 
 * \param bag        \li external reference to the Bag file descriptor
 * \param loc_id     \li location ID for the HDF object possessing this attribute
 * \param *attr_name  \li string name describing this attribute
 * \param *value      \li User-supplied pointer to some data that is expected to be the same 
 *                        \a type as specified upon the creation of this attribute.
 * \return           \li On success, \a bagError is set to \a BAG_SUCCESS.
 *                   \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS.
 */
bagError bagReadAttribute (bagHandle bag, hid_t loc_id, u8 *attr_name, void *value)
{
    return bagAlignAttribute (bag, loc_id, attr_name, value, READ_BAG);
}

/****************************************************************************************/
/*! \brief bagAlignAttribute - memory for \a *data must be allocated and maintained by caller
 * 
 * \param bag        \li external reference to the Bag file descriptor
 * \param loc_id     \li location ID for the HDF object possessing this attribute
 * \param *attr_name  \li string name describing this attribute
 * \param *data      \li User-supplied pointer to some data that is expected to be the same 
 *                        \a type as specified upon the creation of this attribute.
 * \param read_or_write \li boolean value used to switch between reading or writing of the bag,
 *                         \a READ_WRITE_BAG enum
 * \return           \li On success, \a bagError is set to \a BAG_SUCCESS.
 *                   \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS.
 */
bagError bagAlignAttribute (bagHandle bag, hid_t loc_id, u8 *attr_name, 
                            void *data, u32 read_or_write)
{    
    herr_t   status = 0;
    hid_t    attribute_id, datatype_id;

    if (data == NULL)
        return BAG_HDF_CANNOT_WRITE_NULL_DATA;

    attribute_id = H5Aopen_name (loc_id, (char *)attr_name);
    if (attribute_id < 0)
        return BAG_HDF_ATTRIBUTE_OPEN_FAILURE;

    datatype_id  = H5Aget_type (attribute_id);
    if (datatype_id < 0)
    {
        H5Aclose (attribute_id);
        return BAG_HDF_INTERNAL_ERROR;
    }

    if (read_or_write == READ_BAG)
        status = H5Aread(attribute_id, datatype_id, data);
    else if (read_or_write == WRITE_BAG)
        status = H5Awrite(attribute_id, datatype_id, data);
    check_hdf_status();

    status = H5Aclose(attribute_id);
    check_hdf_status();
    status = H5Tclose(datatype_id);
    check_hdf_status();

    return BAG_SUCCESS;
}
