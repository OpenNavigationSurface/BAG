/*!
\file bag_legacy.h
\brief Definition of all legacy coordinate system types and functions.
*/
//************************************************************************
//
//      Open Navigation Surface Working Group, 2013
//
//************************************************************************
#ifndef BAG_LEGACY_H
#define BAG_LEGACY_H

#include "bag_config.h"
#include "stdtypes.h"

/* Legacy BAG datum definition */
typedef enum bagDatums
{
        wgs84,
        wgs72,
        nad83
} bagDatum;

/* Legacy Coordinate Type Enumeration */
typedef enum Coordinate_Types
{
  Geodetic,
  GEOREF,
  Geocentric,
  Local_Cartesian,    
  MGRS,
  UTM,
  UPS,
  Albers_Equal_Area_Conic,
  Azimuthal_Equidistant,
  BNG,
  Bonne,
  Cassini,
  Cylindrical_Equal_Area,
  Eckert4,
  Eckert6,
  Equidistant_Cylindrical,
  Gnomonic,
  Lambert_Conformal_Conic,
  Mercator,
  Miller_Cylindrical,
  Mollweide,
  Neys,
  NZMG,
  Oblique_Mercator,
  Orthographic,
  Polar_Stereo,
  Polyconic,
  Sinusoidal,
  Stereographic,
  Transverse_Cylindrical_Equal_Area,
  Transverse_Mercator,
  Van_der_Grinten
} Coordinate_Type;

/* structure for parameters of all bag supported horizontal coord. sys.
        mercator, tm, utm, ps, ups, lambert, & geodetic. */
typedef struct t_bagProjectionParameters
{
        bagDatum datum;                               /* wgs84, wgs72, nad83, ...   */
        u8  ellipsoid[256];                           /* ellipsoid                  */
        u8  vertical_datum[256];                      /* vertical datum             */
        f64 origin_latitude;                          /* degrees                    */
        f64 central_meridian;                         /* degrees                    */
        f64 std_parallel_1;                           /* degrees                    */
        f64 std_parallel_2;                           /* degrees                    */
        f64 false_easting;                            /* meters                     */
        f64 false_northing;                           /* meters                     */
        f64 scale_factor;                             /* unitless                   */
        f64 latitude_of_true_scale;                   /* degrees                    */
        f64 longitude_down_from_pole;                 /* degrees                    */
        f64 latitude_of_centre;                       /* degrees                    */
        f64 longitude_of_centre;                      /* degrees                    */
        s32 zone;                                     /* utm zone 1-60              */
        s32 utm_override;                             /* utm: 0=autozone,1=use zone */
} bagProjectionParameters;

/* Legacy Reference system definition */
typedef struct t_bagLegacyReferenceSystem
{
    Coordinate_Type coordSys;                         /* either Geodetic or Mercator Transvers_Mercator,etc */
    bagProjectionParameters geoParameters;            /* Parameters for projection information                        */
} bagLegacyReferenceSystem;

/* Define convenience data structure for BAG geographic definitions */
enum BAG_COORDINATES {

        BAG_COORDINATES_GEOGRAPHIC = 1,               /* values in XY array will be degrees on the earth */
        BAG_COORDINATES_PROJECTED  = 2                /* values in XY array will distance on projection  */
};
    
enum BAG_COORD_UNITS {
                                       
        BAG_COORD_UNITS_DEGREES =  1,                 /* values in XY array are geographic in degrees    */
        BAG_COORD_UNITS_METERS  =  2                  /* values in XY array are projected in meters      */
};


BAG_EXTERNAL Coordinate_Type bagCoordsys(char *str);
BAG_EXTERNAL bagDatum        bagDatumID(char *str);
BAG_EXTERNAL s32             bagIdentifyEPSG(s32 crd_sys, bagDatum datum, s32 zone, f64 false_northing);

/*! \brief  bagLegacyToWkt
 * Description:
 *     Utility function used to convert the old reference system definition structures
 *     into a WKT (Well Known Text) string.
 * 
 *  \param    system		The legacy reference system to convert.
 *  \param	  hBuffer   	Modified to contain the horizontal reference system definition as WKT.
 *	\param	  hBuffer_size	The size of the	horizontal definition buffer passed in.
 *  \param	  vBuffer   	Modified to contain the vertical reference system definition as WKT.
 *	\param	  vBuffer_size	The size of the	vertical definition buffer passed in.
 *
 * \return On success, a value of zero is returned.  On failure a value of -1 is returned.  
 */
BAG_EXTERNAL bagError bagLegacyToWkt(const bagLegacyReferenceSystem system,
                                     char *hBuffer, u32 hBuffer_size, char *vBuffer, u32 vBuffer_size);

/*! \brief  bagWktToLegacy
 * Description:
 *     Utility function used to convert a WKT (Well Known Text) reference system definition
 *     into the old reference system defiition structures.
 *
 *     Some WKT definitions can not be converted into the old structures.
 * 
 *  \param    horiz_wkt     String buffer containing the horizontal WKT reference system definition.
 *  \param    vert_wkt      String buffer containing the vertical WKT reference system definition.
 *  \param	  system	    Modified to contain the legacy reference system type.
  *
 * \return On success, a value of zero is returned.  On failure a value of -1 is returned.  
 */
BAG_EXTERNAL bagError bagWktToLegacy(const char *horiz_wkt, const char *vert_wkt, bagLegacyReferenceSystem *system);

#endif