#ifndef BAG_LEGACY_CRS_H
#define BAG_LEGACY_CRS_H

#include "bag_config.h"
#include "bag_c_types.h"

#include <cstdint>


/* Legacy BAG datum definition */
enum class BagDatum
{
    wgs84,
    wgs72,
    nad83,
    unknown
};

/* Legacy Coordinate Type Enumeration */
enum class CoordinateType
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
    Van_der_Grinten,
    Unknown
};

/* structure for parameters of all bag supported horizontal coord. sys.
        mercator, tm, utm, ps, ups, lambert, & geodetic. */
struct BagProjectionParameters
{
    BagDatum datum{BagDatum::unknown};            /* wgs84, wgs72, nad83, ...   */
    char ellipsoid[256]{};                        /* ellipsoid                  */
    char vertical_datum[256]{};                   /* vertical datum             */
    double origin_latitude = 0.;                  /* degrees                    */
    double central_meridian = 0.;                 /* degrees                    */
    double std_parallel_1 = 0.;                   /* degrees                    */
    double std_parallel_2 = 0.;                   /* degrees                    */
    double false_easting = 0.;                    /* meters                     */
    double false_northing = 0.;                   /* meters                     */
    double scale_factor = 0.;                     /* unitless                   */
    double latitude_of_true_scale = 0.;           /* degrees                    */
    double longitude_down_from_pole = 0.;         /* degrees                    */
    double latitude_of_centre = 0.;               /* degrees                    */
    double longitude_of_centre = 0.;              /* degrees                    */
    int32_t zone = 0;                             /* utm zone 1-60              */
    int32_t utm_override = 0;                     /* utm: 0=autozone,1=use zone */
};

/* Legacy Reference system definition */
struct BagLegacyReferenceSystem
{
    CoordinateType coordSys{CoordinateType::Unknown}; /* either Geodetic or Mercator Transvers_Mercator,etc */
    BagProjectionParameters geoParameters;            /* Parameters for projection information              */
};

CoordinateType bagCoordsys(const char* str) noexcept;
BagDatum bagDatumID(const char* str) noexcept;

/*! \brief  bagLegacyToWkt
* Description:
*     Utility function used to convert the old reference system definition structures
*     into a WKT (Well Known Text) string.
*
*  \param    system	        The legacy reference system to convert.
*  \param	 hBuffer   	    Modified to contain the horizontal reference system definition as WKT.
*  \param	 hBufferSize    The size of the	horizontal definition buffer passed in.
*  \param	 vBuffer   	    Modified to contain the vertical reference system definition as WKT.
*  \param	 vBufferSize    The size of the	vertical definition buffer passed in.
*
* \return On success, a value of zero is returned.  On failure a value of -1 is returned.
*/
BagError bagLegacyToWkt(const BagLegacyReferenceSystem& system,
    char* hBuffer, size_t hBufferSize, char* vBuffer, size_t vBufferSize);

#endif  // BAG_LEGACY_CRS_H

