//************************************************************************
//
//      Open Navigation Surface Working Group, 2013
//
//************************************************************************
#include "bag_legacy.h"
#include <string.h>

#define MAX_NCOORD_SYS 32
#define MAX_DATUMS 3

#define DATUM_NAME(k) DATUM_NAME_LIST[k].name
#define COORD_SYS_NAME(k) COORDINATE_SYS_LIST[k].name

struct COORDINATE_SYS_TYPE
{
	char *name;
};
struct DATUM_NAME_TYPE
{
	char *name;
};

static struct COORDINATE_SYS_TYPE COORDINATE_SYS_LIST[]=
{
  {"Geodetic"},
  {"GEOREF"},
  {"Geocentric"},
  {"Local_Cartesian"},
  {"MGRS"},
  {"UTM"},
  {"UPS"},
  {"Albers_Equal_Area_Conic"},
  {"Azimuthal_Equidistant"},
  {"BNG"},
  {"Bonne"},
  {"Cassini"},
  {"Cylindrical_Equal_Area"},
  {"Eckert4"},
  {"Eckert6"},
  {"Equidistant_Cylindrical"},
  {"Gnomonic"},
  {"Lambert_Conformal_Conic"},
  {"Mercator"},
  {"Miller_Cylindrical"},
  {"Mollweide"},
  {"Neys"},
  {"NZMG"},
  {"Oblique_Mercator"},
  {"Orthographic"},
  {"Polar_Stereo"},
  {"Polyconic"},
  {"Sinusoidal"},
  {"Stereographic"},
  {"Transverse_Cylindrical_Equal_Area"},
  {"Transverse_Mercator"},
  {"Van_der_Grinten"}
};

static struct DATUM_NAME_TYPE DATUM_NAME_LIST[]=
{
  {"WGS84"},
  {"WGS72"},
  {"NAD83"}
};

//************************************************************************
/*!
\brief Convert the given coordinate system name to its enum identifier.

\param str
    \li The coordinate system name to convert.
\return
    \li The corresponding coordinate system identifier.
*/
//************************************************************************
Coordinate_Type bagCoordsys( char *str )
{
	long i;
	for(i = 0; i < MAX_NCOORD_SYS; i++)
		if ( strncmp(str, COORD_SYS_NAME(i), strlen(COORD_SYS_NAME(i))) == 0 )
			return (Coordinate_Type)i;
	return (Coordinate_Type)-1;
}

//************************************************************************
/*!
\brief Convert the given datum name to its enum identifier.

\param str
    \li The datum name to convert.
\return
    \li The corresponding datum identifier.
*/
//************************************************************************
bagDatum bagDatumID( char *str )
{
	long i;

	for(i = 0; i < MAX_DATUMS; i++)
		if ( strncmp(str, DATUM_NAME(i), strlen(DATUM_NAME(i))) == 0 )
			return (bagDatum)i;
	return (bagDatum)-1;
}

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
s32 bagIdentifyEPSG(s32 crd_sys, bagDatum datum, s32 zone, f64 false_northing)
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
