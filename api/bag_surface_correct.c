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

static bagError bagReadCorrectedRowBalance (bagHandle bagHandle, bagHandle_opt bagOptHandle, u32 row, u32 startcol, u32 endcol,
                                            u32 type, u32 surf, f32 *data);

/****************************************************************************************/
/*! \brief bagCreateCorrectorDataset initializes the surface correctors optional bag surface
 *
 *  \param hnd        BagHandle Pointer
 *	\param bagDataOpt Pointer to memory holding the data for the optional dataset
 *  \param opt_data   Data definition for this dataset.
 *  \param numCorrectors  The number of surface corrections per node of the optional dataset
 *
 *  \return : \li On success, \a bagError is set to \a BAG_SUCCESS.
 *            \li On failure, \a bagError is set to a proper code from \a BAG_ERRORS.
 * 
 ********************************************************************/
bagError bagCreateCorrectorDataset  (bagHandle hnd, bagHandle_opt *hnd_opt, bagDataOpt *opt_data,
                                     u32 numCorrectors, u8 type)
{
    hsize_t        adims[] = {1, 2}; 
    bagError       err;
    herr_t         herr, status;
    
    if ((opt_data->datatype = H5Tcreate(H5T_COMPOUND, sizeof (bagVerticalCorrector))) < 0)
    {
        return BAG_HDF_TYPE_CREATE_FAILURE;
    }

    adims[1] = numCorrectors;

    herr = H5Tinsert (opt_data->datatype, "x", HOFFSET(bagVerticalCorrector, x), H5T_NATIVE_DOUBLE);
    if (herr < 0)
        return BAG_HDF_TYPE_CREATE_FAILURE;

    herr = H5Tinsert (opt_data->datatype, "y", HOFFSET(bagVerticalCorrector, y), H5T_NATIVE_DOUBLE);
    if (herr < 0)
        return BAG_HDF_TYPE_CREATE_FAILURE;

    /*! Allow user to set the size of the Z array here.  1 x N array to be, with N = numCorrectors */
    herr = H5Tinsert (opt_data->datatype, "z", HOFFSET(bagVerticalCorrector, z), H5Tarray_create(H5T_NATIVE_FLOAT, RANK, adims, NULL));
    if (herr < 0)
        return BAG_HDF_TYPE_CREATE_FAILURE;

	err = bagCreateOptionalDataset (hnd, hnd_opt, opt_data, Surface_Correction);

    if (err == BAG_SUCCESS)
    {
        status = bagWriteAttribute (hnd, (*hnd_opt)->dataset_id, (u8 *)"surface_type", (u8 *)&type);
        check_hdf_status();
    }

    return err;
}

bagError bagGetNumSurfaceCorrectors  (bagHandle_opt hnd_opt, u32 *num)
{
    hid_t          typenow;
    hsize_t        adims[] = {0,0};
    bagError       err;
    int            inx;
    
    *num = 0;

    if (hnd_opt == NULL)
        return BAG_INVALID_BAG_HANDLE;

    /*! Access the "z" member of the datatype */
    inx = H5Tget_member_index (hnd_opt->datatype_id, "z");
    typenow = H5Tget_member_type (hnd_opt->datatype_id, inx );

    if (typenow < 0)
        return BAG_HDF_TYPE_NOT_FOUND;

    /*! Now dimension of the "z" array is obtained */
    err = H5Tget_array_dims( typenow, adims, NULL );
    if (err > 0)
    {
        *num  = adims[1];
        return BAG_SUCCESS;
    }
    else
    {
        *num = 0;
        return BAG_HDF_TYPE_NOT_FOUND;
    }
}

bagError bagReadCorrectedDataset(bagHandle bagHandle, bagHandle_opt bagOptHandle, u32 corrIndex, u32 surfIndex, f32 *data)
{
    bagError err;
    u32 i;

    for (i=0; i < bagHandle->bag.def.nrows; i++)
    {
        /*! By row at a time, fill the whole buffer */
        err = bagReadCorrectedRow (bagHandle,  bagOptHandle,  i,
                                   corrIndex, surfIndex, (data + (bagHandle->bag.def.ncols * i)));
        if (err != BAG_SUCCESS)
            return err;
    }
    return BAG_SUCCESS;

}

bagError bagReadCorrectedRegion (bagHandle bagHandle, bagHandle_opt bagOptHandle,
                                 u32 startrow, u32 endrow, u32 startcol, u32 endcol,
                                 u32 corrIndex, u32 surfIndex, f32 *data)
{
    bagError err;
    s32 i, j, f;
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
            err = bagReadCorrectedRowBalance (bagHandle,  bagOptHandle,  i, startcol, endcol,
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

bagError bagReadCorrectedNode   (bagHandle bagHandle, bagHandle_opt bagOptHandle, u32 row, u32 col,
                                 u32 corrIndex, u32 surfIndex, f32 *data)
{
    return bagReadCorrectedRowBalance (bagHandle,  bagOptHandle,  row, col, col,
                                       corrIndex, surfIndex, data);
}


bagError bagReadCorrectedRow (bagHandle bagHandle, bagHandle_opt bagOptHandle, u32 row,
                              u32 type, u32 surfIndex, f32 *data)
{
    return bagReadCorrectedRowBalance ( bagHandle,  bagOptHandle,  row,
                                        0, bagHandle->bag.def.ncols-1, type, surfIndex, data);
}


static bagError bagReadCorrectedRowBalance (bagHandle bagHandle, bagHandle_opt bagOptHandle, u32 row, u32 startcol, u32 endcol,
                                            u32 type, u32 surfIndex, f32 *data)
{
    bagError err;
    u32 j,q,u;
    f64 xres = 0, yres = 0;
    f32 * surfbuf;
    s32 lastP[2] = {-1,-1};
    bagVerticalCorrector * readbuf, originbuf[4];
    u8 topography;

    if (bagHandle == NULL)
        return BAG_INVALID_BAG_HANDLE;

    if (bagOptHandle == NULL)
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

    readbuf = (bagVerticalCorrector *)calloc (bagOptHandle->bag.def.ncols, sizeof (bagVerticalCorrector));
    surfbuf = (f32 *)calloc (bagHandle->bag.def.ncols, sizeof (f32));

    if (surfbuf == NULL || readbuf == NULL)
    {
        return BAG_MEMORY_ALLOCATION_FAILED;
    }
    
    /*!  Read in this row of the desired surface data being corrected */
    err = bagReadRow ( bagHandle, row, startcol, endcol, surfIndex, surfbuf);
    if ( err != BAG_SUCCESS )
    {
        free (readbuf);
        free (surfbuf);
        return err;
    }

    topography = bagOptHandle->bag.def.surfaceCorrectionTopography;

    if (BAG_SURFACE_GRID_EXTENTS == topography)
    {
        /*! Calculate the cell resolution between the neighboring points at origin (0,1,1,0) */
        err = bagReadOptRow ( bagHandle,  bagOptHandle, 0, 0, 1, Surface_Correction, originbuf);
        if ( err != BAG_SUCCESS )
        {
            free (readbuf);
            free (surfbuf);
            return err;
        }
        xres = originbuf[0].x - originbuf[1].x;
        err = bagReadOptRow ( bagHandle,  bagOptHandle, 1, 0, 1, Surface_Correction, originbuf+2);
        if ( err != BAG_SUCCESS )
        {
            free (readbuf);
            free (surfbuf);
            return err;
        }
        yres = originbuf[0].y - originbuf[2].y;
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
            leastDistSq = FLT_MAX;
        s32 rowRange[2] = {0, 0}, colRange[2] = {0, 0};
        u32 indx = j-startcol;
        u8  zeroDist=0;


        /*! determine the XY value of given cell */
        nodeXY[0] = bagHandle->bag.def.swCornerX + j * bagHandle->bag.def.nodeSpacingX;
        nodeXY[1] = bagHandle->bag.def.swCornerY + row * bagHandle->bag.def.nodeSpacingY;


        if (topography == BAG_SURFACE_IRREGULARLY_SPACED)
        {
            if (lastP[0] == -1 || lastP[1] == -1)
            {
                colRange[0] = 0;
                colRange[1] = bagOptHandle->bag.def.ncols -1;
                rowRange[0] = 0;
                rowRange[1] = bagOptHandle->bag.def.nrows -1;
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
            colRange[0] = (s32)floor ((originbuf[0].x - nodeXY[0]) / xres);
            colRange[1] = (s32)ceil  ((originbuf[0].x - nodeXY[0]) / xres);
            rowRange[0] = (s32)floor ((originbuf[0].y - nodeXY[1]) / yres);
            rowRange[1] = (s32)ceil  ((originbuf[0].y - nodeXY[1]) / yres);
        }

        /*! Enforce dataset limits */
        if (colRange[0] < 0)
            colRange[0] = 0;
        if (colRange[0] >= bagOptHandle->bag.def.ncols)
            colRange[0] = bagOptHandle->bag.def.ncols -1;
        if (colRange[1] < 0)
            colRange[1] = 0;
        if (colRange[1] >= bagOptHandle->bag.def.ncols)
            colRange[1] = bagOptHandle->bag.def.ncols -1;
        if (rowRange[0] < 0)
            rowRange[0] = 0;
        if (rowRange[0] >= bagOptHandle->bag.def.nrows)
            rowRange[0] = bagOptHandle->bag.def.nrows -1;
        if (rowRange[1] < 0)
            rowRange[1] = 0;
        if (rowRange[1] >= bagOptHandle->bag.def.nrows)
            rowRange[1] = bagOptHandle->bag.def.nrows -1;

        if (colRange[1] == colRange[0])
        {
            if (colRange[0] > 0)
                colRange[0]--;
            if ((colRange[1] + 1) < bagOptHandle->bag.def.ncols)
                colRange[1]++;
        }
        if (rowRange[1] == rowRange[0])
        {

            if (rowRange[0] > 0)
                rowRange[0]--;
            if ((rowRange[1] + 1) < bagOptHandle->bag.def.nrows)
                rowRange[1]++;
        }

       /*  fprintf(stderr, "RC:   %d  %d  - %d  %d\n", colRange[0], rowRange[0], colRange[1], rowRange[1]); */
/* fflush(stderr); */

        /*! look through the SEPs and calculate the weighted average between them and this position  */
        for (q=rowRange[0]; q <= rowRange[1]; q++)
        {
            if (zeroDist)
                break;

            /*!  now the SEP should be accessed, up to entire row of all columns of Surface_Correction */
            err = bagReadOptRow ( bagHandle,  bagOptHandle, q, colRange[0], colRange[1], Surface_Correction, readbuf);
            if ( err != BAG_SUCCESS )
            {
                 free (readbuf);
                 free (surfbuf);
                 return err;
            }

            for (u=colRange[0]; u <= colRange[1]; u++)
            {
                bagVerticalCorrector  *vertCorr;
                f64 z1, resratio, distSq;

                vertCorr =  readbuf + (u - colRange[0]);
                z1 = vertCorr->z[type-1];

                /*! is a constant SEP with one point? */
                if (vertCorr->y == NULL_GENERIC && vertCorr->x == NULL_GENERIC)
                {
                  /* skip null points */
                    if (z1 == NULL_GENERIC)
                    {
                        continue;
                    }
                    else
                    {
                        data[indx] = surfbuf[indx] + (f32)z1;
                    }
                }
                else
                {
                    /*! scaled latitude degrees, si vous plait. */
                    if ( bagOptHandle->bag.def.coordSys == Geodetic )
                    {
                        resratio = bagHandle->bag.def.nodeSpacingX / bagHandle->bag.def.nodeSpacingY;
                    }
                    else
                    {
                        resratio = 1.0;
                    }

                    /*! calculate distance weight between nodeXY and y1/x1 */
                    distSq  = powl ((nodeXY[0] - vertCorr->x), 2.0L) +
                        powl (resratio * (nodeXY[1] - vertCorr->y), 2.0L);

                    if (distSq == 0)
                    {
                        zeroDist = 1;
                        distSq = 1.0;
                        data[indx] = surfbuf[indx] + z1;
                        break;
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
                }
    
            } /* for u cols */

        } /* for q rows */
        
        /*! is not a constant SEP with one point? */
        if (sum_sep != 0.0 && sum != 0.0 && surfbuf[indx] != NULL_GENERIC)
        {
            data[indx] = surfbuf[indx] + sum_sep / sum;
        }
        else
        {
            data[indx] = NULL_GENERIC;
        }
    }
    
    free (readbuf);
    free (surfbuf);

    return BAG_SUCCESS;
}


/****************************************************************************************/
/*! \brief  bagWriteCorrectorVerticalDatum
 *
 * Description:
 *     Write the vertical datum code to an attribute on the Vertical Datum Correctors dataset
 * 
 *  \param    hnd    - pointer to the structure which ultimately contains the bag
 *  \param    datum  - string to store in attr
 *
 * \return On success, a value of zero is returned.  On failure a value of -1 is returned.  
 *
 ****************************************************************************************/
bagError bagWriteCorrectorVerticalDatum (bagHandle hnd, bagHandle_opt hnd_opt, u32 type, u8 *datum)
{
    herr_t status;
    hid_t  dataset_id;
    u8     *tok, buff[XML_ATTR_MAXSTR];
    u8     datum_array [BAG_SURFACE_CORRECTOR_LIMIT][XML_ATTR_MAXSTR];
    u32    i;

    for (i = 0 ; i < BAG_SURFACE_CORRECTOR_LIMIT; i++)
        datum_array[i][0] = '\0';

    if (hnd == NULL || hnd_opt == NULL)
        return BAG_INVALID_BAG_HANDLE;

    if (datum == NULL)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    if (type > BAG_SURFACE_CORRECTOR_LIMIT || type < 1)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    dataset_id = hnd_opt->dataset_id;
      
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
 *  \param    datum  - buffer to read the attribute string into
 *
 * \return On success, a value of zero is returned.  On failure a value of -1 is returned.  
 *
 ****************************************************************************************/
bagError bagReadCorrectorVerticalDatum (bagHandle hnd, bagHandle_opt hnd_opt, u32 type, u8 * datum)
{
    herr_t status;
    hid_t  dataset_id;
    u8     buff [XML_ATTR_MAXSTR];
    u8     d [BAG_SURFACE_CORRECTOR_LIMIT][XML_ATTR_MAXSTR];
    u32    i;

    for (i = 0 ; i < BAG_SURFACE_CORRECTOR_LIMIT; i ++)
        memset (d[i], 0, sizeof(u8) *XML_ATTR_MAXSTR );

    if (hnd == NULL || hnd_opt == NULL)
        return BAG_INVALID_BAG_HANDLE;

    if (datum == NULL)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    if (type > BAG_SURFACE_CORRECTOR_LIMIT || type < 1)
        return BAG_INVALID_FUNCTION_ARGUMENT;

    dataset_id    = hnd_opt->dataset_id;
      
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
