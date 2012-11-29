/*! \file bag_epsg.c
 * \brief This module contains a function to convert a BAG file's georeferencing parameters to an EPSG authority code.
 ********************************************************************
 *
 * Module Name : epsg.c
 *
 * Author/Date : IVS3D, June 2010
 *
 * Description : 
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 *
 * Classification : Unclassified
 *
 * References : 
 *
 ********************************************************************/

#include "bag_private.h"

/****************************************************************************************/
/*! \brief IdentifyEPSG This function converts bag parameters to an EPSG authority code. 
 *               UTM and geodetic systems of the BAG defined datums are covered. 
 *               The return value for other georeferencing systems, other than UTM and geodetic, is undefined.
 *
 *  \param  crdsys          The georeferencing system - BAG_COORDINATES_GEOGRAPHIC or BAG_COORDINATES_PROJECTED (indicates UTM).
 *  \param  datum           The datum.
 *  \param  zone            The UTM zone, if any.
 *  \param  false_northing  The false northing, if any.
 *  \return : \li On success, \a Integer value containing the EPSG authority code.
 *            \li On failure, \a Zero.
 * 
 ********************************************************************/

s32        bagIdentifyEPSG(s32 crd_sys, bagDatum datum, s32 zone, f64 false_northing)
{
  s32 is_north = ( false_northing == 0. ? 1 : 0 ), EPSG = 0;  /* if the false northing is 0, it is in the northern hemisphere */

  if( crd_sys == Geodetic ) /* If it is a geodetic unprojected system, return the geographic code for that datum */
    {
      if ( datum == wgs84 ) /* WGS84 geodetic */
        {
            EPSG = 4326;
        }
      else if ( datum == nad83 ) /* NAD83 geodetic */
        {
            EPSG = 4269;
        }
      else if ( datum == wgs72 ) /* WGS72 geodetic */
        {
            EPSG = 4322;
        }
    }
    else if( crd_sys == Mercator )
    {
        EPSG = 3395;
    }
    else if( crd_sys == UTM )
    {
        if ( datum == wgs84 || datum == nad83 )
        {
          if( datum == nad83 && zone >= 1 && zone <= 23 && is_north ) /* NAD83 UTM */
            {
                EPSG = 26900 + zone;
            }
          else /* if this is a NAD83 - use the wgs84 codes outside of the specified N.A. range - WGS84 UTM */
            {
                if ( is_north ) 
                {
                    EPSG = 32600 + zone;
                }
                else
                {
                    EPSG = 32700 + zone;
                }
            }
        }
        else if ( datum == wgs72 ) /* WGS72 UTM */
        {
            if ( is_north )
            {
                EPSG = 32200 + zone;
            }
            else
            {
                EPSG = 32300 + zone;
            }
        }
    }

    return EPSG;
}

