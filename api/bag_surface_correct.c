/*! \file bag_surface_correct.c
 * \brief This module contains functions for creating and accessing the Surface Corrector optional surface dataset.
 ********************************************************************
 *
 * Module Name : bag_surface_correct.c
 *
 * Author/Date : SAIC, August 2010
 *
 * Description : 
 *               Surfaces are datasets.
 *               There is a restriction here which requires all the
 *               proper surfaces to have the exact same extents/dimensions.
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 * who  when      what
 * ---  ----      ----
 * Webb McDonald -- Mon Jan  9 17:09:46 2012
 *   Obtain the correctors surface origin and node spacing from new function,
 *   bagVerticalCorrectorDef.
 *
 * Webb McDonald -- Tue Jul 27 17:36:08 2010
 *   Added another optional dataset to BAG: vertical datum / surface correctors.
 *
 * Classification : Unclassified
 *
 * References : 
 *
 ********************************************************************/

#include "bag_private.h"
#include <math.h>

#define SEARCH_RADIUS 3
#define XML_ATTR_MAXSTR 256

static bagError bagReadCorrectedRowBalance (bagHandle bagHandle, u32 row, u32 startcol, u32 endcol,
                                            u32 type, u32 surf, f32 *data);

/****************************************************************************************/
/*! \brief bagCreateCorrectorDataset initializes the surface correctors optional bag surface
 *
 *  \param hnd        BagHandle Pointer
 *  \param data       Data definition for this dataset.
 *  \param numCorrectors  The number of surface corrections per node of the optional dataset
 *  \param type       The coordinate topography type.
 *
 *  \return : \li On success, \a bagError is set to \a BAG_SUCCESS.
 *            \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS.
 * 
 ********************************************************************/
bagError bagCreateCorrectorDataset  (bagHandle hnd, bagData *data, u32 numCorrectors, u8 type)
{
    hsize_t        adims[] = {1, 2}; 
    bagError       err;
    herr_t         herr, status;
    hid_t          dataset_id;
    
    adims[1] = numCorrectors;

    if (BAG_SURFACE_GRID_EXTENTS == type)
    {
        if ((data->opt[Surface_Correction].datatype = H5Tcreate(H5T_COMPOUND, sizeof (bagVerticalCorrectorNode))) < 0)
        {
            return BAG_HDF_TYPE_CREATE_FAILURE;
        }

       /*! Allow user to set the size of the Z array here.  1 x N array to be, with N = numCorrectors */
        herr = H5Tinsert (data->opt[Surface_Correction].datatype, "z", HOFFSET(bagVerticalCorrectorNode, z), H5Tarray_create(H5T_NATIVE_FLOAT, RANK, adims, NULL));
        if (herr < 0)
            return BAG_HDF_TYPE_CREATE_FAILURE;
    }
    else
    {
        if ((data->opt[Surface_Correction].datatype = H5Tcreate(H5T_COMPOUND, sizeof (bagVerticalCorrector))) < 0)
        {
            return BAG_HDF_TYPE_CREATE_FAILURE;
        }

        herr = H5Tinsert (data->opt[Surface_Correction].datatype, "x", HOFFSET(bagVerticalCorrector, x), H5T_NATIVE_DOUBLE);
        if (herr < 0)
            return BAG_HDF_TYPE_CREATE_FAILURE;

        herr = H5Tinsert (data->opt[Surface_Correction].datatype, "y", HOFFSET(bagVerticalCorrector, y), H5T_NATIVE_DOUBLE);
        if (herr < 0)
            return BAG_HDF_TYPE_CREATE_FAILURE;

        /*! Allow user to set the size of the Z array here.  1 x N array to be, with N = numCorrectors */
        herr = H5Tinsert (data->opt[Surface_Correction].datatype, "z", HOFFSET(bagVerticalCorrector, z), H5Tarray_create(H5T_NATIVE_FLOAT, RANK, adims, NULL));
        if (herr < 0)
            return BAG_HDF_TYPE_CREATE_FAILURE;
    }

	err = bagCreateOptionalDataset (hnd, data, Surface_Correction);

    dataset_id = (hnd)->opt_dataset_id[Surface_Correction];   
    if (dataset_id < 0)
        return BAG_HDF_DATASET_OPEN_FAILURE; 

    if (err == BAG_SUCCESS)
    {
        status = bagWriteAttribute (hnd,  dataset_id , (u8 *)"surface_type", (u8 *)&type);
        check_hdf_status();
    }

    if (type == BAG_SURFACE_GRID_EXTENTS)
    {
        f64 junk=0.0;

        /*! Create the mandatory attributes for this level */
        if ((status = bagCreateAttribute (hnd, dataset_id, (u8 *)VERT_DATUM_CORR_SWX, sizeof(f64), BAG_ATTR_F64)) != BAG_SUCCESS)
        {
            return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
        } 
        if ((status = bagWriteAttribute (hnd, dataset_id, (u8 *)VERT_DATUM_CORR_SWX, (void *) &(junk) )) != BAG_SUCCESS)
        {
            return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
        }
        if ((status = bagCreateAttribute (hnd, dataset_id, (u8 *)VERT_DATUM_CORR_SWY, sizeof(f64), BAG_ATTR_F64)) != BAG_SUCCESS)
        {
            return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
        } 
        if ((status = bagWriteAttribute (hnd, dataset_id, (u8 *)VERT_DATUM_CORR_SWY, (void *) &(junk) )) != BAG_SUCCESS)
        {
            return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
        }
        if ((status = bagCreateAttribute (hnd, dataset_id, (u8 *)VERT_DATUM_CORR_NSX, sizeof(f64), BAG_ATTR_F64)) != BAG_SUCCESS)
        {
            return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
        } 
        if ((status = bagWriteAttribute (hnd, dataset_id, (u8 *)VERT_DATUM_CORR_NSX, (void *) &(junk) )) != BAG_SUCCESS)
        {
            return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
        }
        if ((status = bagCreateAttribute (hnd, dataset_id, (u8 *)VERT_DATUM_CORR_NSY, sizeof(f64), BAG_ATTR_F64)) != BAG_SUCCESS)
        {
            return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
        } 
        if ((status = bagWriteAttribute (hnd, dataset_id, (u8 *)VERT_DATUM_CORR_NSY, (void *) &(junk) )) != BAG_SUCCESS)
        {
            return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
        }

    }

    return err;
}

bagError bagGetNumSurfaceCorrectors  (bagHandle hnd_opt, u32 *num)
{
    hid_t          typenow;
    hsize_t        adims[] = {0,0};
    bagError       err;
    int            inx;
    
    *num = 0;

    if (hnd_opt == NULL)
        return BAG_INVALID_BAG_HANDLE;

    /*! Access the "z" member of the datatype */
    inx = H5Tget_member_index (hnd_opt->opt_datatype_id[Surface_Correction], "z");
    typenow = H5Tget_member_type (hnd_opt->opt_datatype_id[Surface_Correction], inx );

    if (typenow < 0)
        return BAG_HDF_TYPE_NOT_FOUND;

    /*! Now dimension of the "z" array is obtained */
    err = H5Tget_array_dims( typenow, adims, NULL );
    if (err > 0)
    {
        *num  = (u32)adims[1];
        return BAG_SUCCESS;
    }
    else
    {
        *num = 0;
        return BAG_HDF_TYPE_NOT_FOUND;
    }
}

bagError bagReadCorrectedDataset(bagHandle bagHandle, u32 corrIndex, u32 surfIndex, f32 *data)
{
    bagError err;
    u32 i;

    for (i=0; i <  bagHandle->bag.def.nrows; i++)
    {
        /*! By row at a time, fill the whole buffer */
        err = bagReadCorrectedRow (bagHandle, i, corrIndex, surfIndex,
                                   (data + (bagHandle->bag.def.ncols * i)));
        if (err != BAG_SUCCESS)
            return err;
    }
    return BAG_SUCCESS;

}

bagError bagReadCorrectedRegion (bagHandle bagHandle,
                                 u32 startrow, u32 endrow, u32 startcol, u32 endcol,
                                 u32 corrIndex, u32 surfIndex, f32 *data)
{
    bagError err;
    s32 f;
    u32 i, j;
    f32 *sepData;
    
    if (endcol >= bagHandle->bag.def.ncols ||
        endrow >= bagHandle->bag.def.nrows ||
        startrow > endrow || 
        startcol > endcol)
    {
        fprintf(stderr, "Internal error, bad parameters given to access surface extents! Aborting...\n");
        fprintf(stderr, "\tCannot access region, %d-%d / %d-%d, with surface extents 0-%d / 0-%d\n",
                startrow, startcol, endrow, endcol, bagHandle->bag.def.nrows, bagHandle->bag.def.ncols);
        fflush(stderr);
        return BAG_HDF_ACCESS_EXTENTS_ERROR;
    }

    sepData = (f32 *)calloc (endcol-startcol+1, sizeof (f32));
    if (sepData == NULL)
    {
        return BAG_MEMORY_ALLOCATION_FAILED;
    }
    
    for (f=0, i=0; i < bagHandle->bag.def.nrows; i++)
    {
        if (i >= startrow && i <= endrow)
        {
            memset (sepData, NULL_GENERIC, sizeof(f32) * endcol-startcol+1);
            
            /*! By row at a time, fill the whole buffer */
            err = bagReadCorrectedRowBalance (bagHandle,  i, startcol, endcol,
                                              corrIndex, surfIndex, sepData);
            
            if (err != BAG_SUCCESS)
            {
                return err;
            }
            
            /*! For each column in the transfer set */
            for (j=0; j < endcol-startcol+1; j ++)
            {
                data[(endcol - startcol + 1) * f + j] = sepData[j];
            }
            
            f++;
        }
    }
    free (sepData);
    return BAG_SUCCESS;
}

bagError bagReadCorrectedNode   (bagHandle bagHandle, u32 row, u32 col,
                                 u32 corrIndex, u32 surfIndex, f32 *data)
{
    return bagReadCorrectedRowBalance (bagHandle,  row, col, col,
                                       corrIndex, surfIndex, data);
}


bagError bagReadCorrectedRow (bagHandle bagHandle, u32 row,
                              u32 type, u32 surfIndex, f32 *data)
{
    return bagReadCorrectedRowBalance ( bagHandle,  row,
                                        0, bagHandle->bag.def.ncols-1, type, surfIndex, data);
}


static bagError bagReadCorrectedRowBalance (bagHandle bagHandle, u32 row, u32 startcol, u32 endcol,
                                            u32 type, u32 surfIndex, f32 *data)
{
    bagError err;
    u8 topography;
    u32 j,q,u;
    s32 lastP[2] = {-1,-1};
    bagVerticalCorrectorNode *   readbuf;
    bagVerticalCorrectorDef      vddef;

    if (bagHandle == NULL)
        return BAG_INVALID_BAG_HANDLE;

    if (data == NULL ||
        type > BAG_SURFACE_CORRECTOR_LIMIT || type < 1)
    {
        return  BAG_INVALID_FUNCTION_ARGUMENT;
    }

    if (endcol >= bagHandle->bag.def.ncols ||
        row >= bagHandle->bag.def.nrows || 
        startcol > endcol)
    {
        fprintf(stderr, "Internal error, bad parameters given to access surface extents! Aborting...\n");
        fprintf(stderr, "\tCannot access region, R: %d / C: %d-%d, with surface extents 0-%d / 0-%d\n",
                row, startcol, endcol, bagHandle->bag.def.nrows, bagHandle->bag.def.ncols);
        fflush(stderr);
        return BAG_HDF_ACCESS_EXTENTS_ERROR;
    }

    readbuf = (bagVerticalCorrectorNode *)calloc (bagHandle->bag.opt[Surface_Correction].ncols, sizeof (bagVerticalCorrectorNode));

    if (readbuf == NULL)
    {
        return BAG_MEMORY_ALLOCATION_FAILED;
    }
    
    /*!  Read in this row of the desired surface data being corrected */
    err = bagReadRow ( bagHandle, row, startcol, endcol, surfIndex, data);
    if (err != BAG_SUCCESS )
    {
        free (readbuf);
        return err;
    }

    topography = bagHandle->bag.def.surfaceCorrectionTopography;

    if (BAG_SURFACE_GRID_EXTENTS == topography)
    {
        /*! Obtain cell resolution and SW origin (0,1,1,0) */
        bagReadCorrectorDefinition (bagHandle, &vddef);
    }
    else
    {
        return BAG_INVALID_BAG_HANDLE;
    }

    /*! loop through every cell in the row and compute a SEP */
    for (j=startcol; j <= endcol; j++)
    {
        f64 nodeXY[2];
        f64 sum_sep = 0.0, sum = 0.0,
            leastDistSq = DBL_MAX;
        u32 rowRange[2] = {0, 0}, colRange[2] = {0, 0};
        u32 indx = j-startcol;
        u8  zeroDist=0;

        if (data[indx] == NULL_GENERIC || data[indx] == NULL_ELEVATION || data[indx] == NULL_UNCERTAINTY)
            continue;

        /*! determine the XY value of given cell */
        nodeXY[0] = bagHandle->bag.def.swCornerX + j * bagHandle->bag.def.nodeSpacingX;
        nodeXY[1] = bagHandle->bag.def.swCornerY + row * bagHandle->bag.def.nodeSpacingY;


        if (topography == BAG_SURFACE_IRREGULARLY_SPACED)
        {
            if (lastP[0] == -1 || lastP[1] == -1)
            {
                colRange[0] = 0;
                colRange[1] = bagHandle->bag.opt[Surface_Correction].ncols -1;
                rowRange[0] = 0;
                rowRange[1] = bagHandle->bag.opt[Surface_Correction].nrows -1;
            }
            else
            {
                colRange[0] = lastP[0] - SEARCH_RADIUS;
                colRange[1] = lastP[0] + SEARCH_RADIUS;
                rowRange[0] = lastP[1] - SEARCH_RADIUS;
                rowRange[1] = lastP[1] + SEARCH_RADIUS;
            }
        }
        else if (BAG_SURFACE_GRID_EXTENTS == topography)
        {

            /*! An simple calculation for 4 nearest corrector nodes */
            colRange[0] = (s32)fabs(floor ((vddef.swCornerX - nodeXY[0]) / vddef.nodeSpacingX));
            colRange[1] = (s32)fabs(ceil  ((vddef.swCornerX - nodeXY[0]) / vddef.nodeSpacingX));
            rowRange[0] = (s32)fabs(floor ((vddef.swCornerY - nodeXY[1]) / vddef.nodeSpacingY));
            rowRange[1] = (s32)fabs(ceil  ((vddef.swCornerY - nodeXY[1]) / vddef.nodeSpacingY));
        }

        /*! Enforce dataset limits */
        if (colRange[0] > colRange[1])
        {
            s32 c = colRange[0];
            colRange[0] = colRange[1];
            colRange[1] = c;
        }
        if (colRange[0] < 0)
            colRange[0] = 0;
        if (colRange[0] >= bagHandle->bag.opt[Surface_Correction].ncols)
            colRange[0] = bagHandle->bag.opt[Surface_Correction].ncols -1;
        if (colRange[1] < 0)
            colRange[1] = 0;
        if (colRange[1] >= bagHandle->bag.opt[Surface_Correction].ncols)
            colRange[1] = bagHandle->bag.opt[Surface_Correction].ncols -1;
        if (rowRange[0] > rowRange[1])
        {
            s32 c = rowRange[0];
            rowRange[0] = rowRange[1];
            rowRange[1] = c;
        }
        if (rowRange[0] < 0)
            rowRange[0] = 0;
        if (rowRange[0] >= bagHandle->bag.opt[Surface_Correction].nrows)
            rowRange[0] = bagHandle->bag.opt[Surface_Correction].nrows -1;
        if (rowRange[1] < 0)
            rowRange[1] = 0;
        if (rowRange[1] >= bagHandle->bag.opt[Surface_Correction].nrows)
            rowRange[1] = bagHandle->bag.opt[Surface_Correction].nrows -1;

        if (colRange[1] == colRange[0])
        {
            if (colRange[0] > 0)
                colRange[0]--;
            if ((colRange[1] + 1) < bagHandle->bag.opt[Surface_Correction].ncols)
                colRange[1]++;
        }
        if (rowRange[1] == rowRange[0])
        {

            if (rowRange[0] > 0)
                rowRange[0]--;
            if ((rowRange[1] + 1) < bagHandle->bag.opt[Surface_Correction].nrows)
                rowRange[1]++;
        }

/*         fprintf(stderr, "INDX: %d Row: %d RC:  %d  / %d\n", indx, row,  rowRange[0], rowRange[1]); */

        /*! look through the SEPs and calculate the weighted average between them and this position  */
        for (q=rowRange[0]; q <= rowRange[1]; q++)
        {
            if (zeroDist)
                break;

            /*!  now the SEP should be accessed, up to entire row of all columns of Surface_Correction */
            err = bagReadRow ( bagHandle, q, colRange[0], colRange[1], Surface_Correction, readbuf);
            if ( err != BAG_SUCCESS )
            {
                free (readbuf);
                return err;
            }

            for (u=colRange[0]; u <= colRange[1]; u++)
            {
                bagVerticalCorrectorNode  *vertCorr;
                f64 x1, y1, resratio, distSq;
                f32 z1;

                vertCorr =  readbuf + (u - colRange[0]);

                z1 = vertCorr->z[type-1];
                x1 = vddef.swCornerX + u * vddef.nodeSpacingX;
                y1 = vddef.swCornerY + q * vddef.nodeSpacingY;

                resratio = vddef.nodeSpacingX / vddef.nodeSpacingY;

                if (nodeXY[0] ==  x1 && nodeXY[1] == y1)
                {
                    zeroDist = 1;
                    distSq = 1.0;
                    data[indx] += z1;
                    break;
                }
                else
                {
                    /*! calculate distance weight between nodeXY and y1/x1 */
                    distSq  = (f64)pow (fabs((f64)(nodeXY[0] - x1)), 2.0) +
                        (f64)pow (resratio * fabs((f64)(nodeXY[1] - y1)), 2.0);
                }

                if (leastDistSq > distSq)
                {
                    leastDistSq = distSq;
                    lastP[0] = u;
                    lastP[1] = q;
                }
                    
                /*! inverse distance calculation */
                sum_sep += z1  / distSq;
                sum     += 1.0 / distSq;


            } /* for u cols */

        } /* for q rows */
        
/*         fprintf(stderr, "sum sum %f / %f =  %f : %f\n", sum_sep, sum, sum_sep / sum, data[indx]); */

        if (!zeroDist)
        {
            /*! is not a constant SEP with one point? */
            if (sum_sep != 0.0 && sum != 0.0)
            {
                data[indx] += sum_sep / sum;
            }
            else 
            {
                data[indx] = NULL_GENERIC;
            }
        }

    } /* for j cols */
    
    free (readbuf);

    return BAG_SUCCESS;
}


/****************************************************************************************/
/*! \brief  bagWriteCorrectorVerticalDatum
 *
 * Description:
 *     Write the vertical datum code to an attribute on the Vertical Datum Correctors dataset
 * 
 *  \param    hnd    - pointer to the structure which ultimately contains the bag
 *  \param    type   - The corrector type to write.
 *  \param    datum  - string to store in attr
 *
 * \return On success, a value of zero is returned.  On failure a value of -1 is returned.  
 *
 ****************************************************************************************/
bagError bagWriteCorrectorVerticalDatum (bagHandle hnd, u32 type, u8 *datum)
{
    herr_t status;
    hid_t  dataset_id;
    u8     *tok, buff[XML_ATTR_MAXSTR];
    u8     datum_array [BAG_SURFACE_CORRECTOR_LIMIT][XML_ATTR_MAXSTR];
    u32    i;

    for (i = 0 ; i < BAG_SURFACE_CORRECTOR_LIMIT; i++)
        datum_array[i][0] = '\0';

    if (hnd == NULL)
        return BAG_INVALID_BAG_HANDLE;

    if (datum == NULL)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    if (type > BAG_SURFACE_CORRECTOR_LIMIT || type < 1)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    dataset_id = hnd->opt_dataset_id[Surface_Correction];
      
    if (dataset_id < 0)
        return BAG_HDF_DATASET_OPEN_FAILURE; 


    status = bagReadAttribute (hnd, dataset_id, (u8 *)"vertical_datum", buff);

    if (status == BAG_SUCCESS && strlen((char *) buff) > 0)
    {
        i = 0;
        tok = (u8 *) strtok ((char *) buff, ",");
        do
        {
          strncpy ((char *) (datum_array + i++), (char *) tok, XML_ATTR_MAXSTR -1);

        } while (i < BAG_SURFACE_CORRECTOR_LIMIT && 
                 (tok = (u8 *) strtok (NULL, ",")) != NULL);
    }
    strncpy ((char *) datum_array[type-1], (char *) datum, XML_ATTR_MAXSTR-1);
    sprintf((char *) buff, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s",
            datum_array[0], datum_array[1],datum_array[2],datum_array[3],datum_array[4],
            datum_array[5],datum_array[6],datum_array[7],datum_array[8],datum_array[9]);

    status = bagWriteAttribute (hnd, dataset_id, (u8 *)"vertical_datum", buff);
    check_hdf_status();

	
    return BAG_SUCCESS;
}

/****************************************************************************************/
/*! \brief  bagReadCorrectorVerticalDatum
 *
 * Description:
 *     Read the vertical datum code attribute from the Vertical Datum Correctors dataset
 * 
 *  \param    hnd    - pointer to the structure which ultimately contains the bag
 *  \param    type   - The corrector type to read.
 *  \param    datum  - buffer to read the attribute string into
 *
 * \return On success, a value of zero is returned.  On failure a value of -1 is returned.  
 *
 ****************************************************************************************/
bagError bagReadCorrectorVerticalDatum (bagHandle hnd, u32 type, u8 * datum)
{
    herr_t status;
    hid_t  dataset_id;
    u8     buff [XML_ATTR_MAXSTR];
    u8     d [BAG_SURFACE_CORRECTOR_LIMIT][XML_ATTR_MAXSTR];
    u32    i;

    for (i = 0 ; i < BAG_SURFACE_CORRECTOR_LIMIT; i ++)
        memset (d[i], 0, sizeof(u8) *XML_ATTR_MAXSTR );

    if (hnd == NULL)
        return BAG_INVALID_BAG_HANDLE;

    if (datum == NULL)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    if (type > BAG_SURFACE_CORRECTOR_LIMIT || type < 1)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    dataset_id    = hnd->opt_dataset_id[Surface_Correction];
      
    datum[0] = '\0';
    
    if (dataset_id < 0)
        return BAG_HDF_DATASET_OPEN_FAILURE; 

    status = bagReadAttribute (hnd, dataset_id, (u8 *)"vertical_datum", buff);

    check_hdf_status();

    if (strlen((char *) buff) > 0)
    {
        u8 * tok;
        i = 0;
        tok = (u8 *) strtok ((char *) buff, ",");
        do
        {
          strncpy ((char *) (d + i++), (char *) tok, XML_ATTR_MAXSTR -1);

        } while (i < BAG_SURFACE_CORRECTOR_LIMIT && 
                 (tok = (u8 *) strtok (NULL, ",")) != NULL);

        if (i > type-1)
          strncpy ((char *) datum, (char *) d[type-1], XML_ATTR_MAXSTR-1);
        else
            return BAG_HDF_ATTRIBUTE_OPEN_FAILURE;
    }
	
    return BAG_SUCCESS;
}

bagError bagWriteCorrectorDefinition (bagHandle hnd,  bagVerticalCorrectorDef *def)
{
    herr_t status;
    hid_t  dataset_id;
    
    if (def == NULL)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    if (hnd == NULL)
        return BAG_INVALID_BAG_HANDLE;
    
    dataset_id = hnd->opt_dataset_id[Surface_Correction];
    
    if (dataset_id < 0)
        return BAG_HDF_DATASET_OPEN_FAILURE; 

    status = bagWriteAttribute (hnd, dataset_id, (u8 *)VERT_DATUM_CORR_SWX, &def->swCornerX);
    check_hdf_status();        
    status = bagWriteAttribute (hnd, dataset_id, (u8 *)VERT_DATUM_CORR_SWY, &def->swCornerY);
    check_hdf_status();        
    status = bagWriteAttribute (hnd, dataset_id, (u8 *)VERT_DATUM_CORR_NSY, &def->nodeSpacingY);
    check_hdf_status();        
    status = bagWriteAttribute (hnd, dataset_id, (u8 *)VERT_DATUM_CORR_NSX, &def->nodeSpacingX);
    check_hdf_status();

    return BAG_SUCCESS;

}

bagError bagReadCorrectorDefinition (bagHandle hnd,  bagVerticalCorrectorDef *def)
{
    herr_t status;
    hid_t  dataset_id;
    
    if (def == NULL)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    if (hnd == NULL)
        return BAG_INVALID_BAG_HANDLE;
    
    memset (def, 0, sizeof(def));

    dataset_id = hnd->opt_dataset_id[Surface_Correction];
    
    if (dataset_id < 0)
        return BAG_HDF_DATASET_OPEN_FAILURE; 

    status = bagReadAttribute (hnd, dataset_id, (u8 *)VERT_DATUM_CORR_SWX, &def->swCornerX);
    check_hdf_status();        
    status = bagReadAttribute (hnd, dataset_id, (u8 *)VERT_DATUM_CORR_SWY, &def->swCornerY);
    check_hdf_status();        
    status = bagReadAttribute (hnd, dataset_id, (u8 *)VERT_DATUM_CORR_NSY, &def->nodeSpacingY);
    check_hdf_status();        
    status = bagReadAttribute (hnd, dataset_id, (u8 *)VERT_DATUM_CORR_NSX, &def->nodeSpacingX);
    check_hdf_status();

    return BAG_SUCCESS;
}
