/***************************************************************************/
/* RSC IDENTIFIER: LAMBERT
 *
 * ABSTRACT
 *
 *    This component provides conversions between Geodetic coordinates
 *    (latitude and longitude in radians) and Lambert Conformal Conic
 *    projection coordinates (easting and northing in meters) defined
 *    by two standard parallels.  When both standard parallel parameters
 *    are set to the same latitude value, the result is a Lambert 
 *    Conformal Conic projection with one standard parallel at the 
 *    specified latitude.
 *
 * ERROR HANDLING
 *
 *    This component checks parameters for valid values.  If an invalid value
 *    is found the error code is combined with the current error code using
 *    the bitwise or.  This combining allows multiple error codes to be
 *    returned. The possible error codes are:
 *
 *       LAMBERT_NO_ERROR           : No errors occurred in function
 *       LAMBERT_LAT_ERROR          : Latitude outside of valid range
 *                                     (-90 to 90 degrees)
 *       LAMBERT_LON_ERROR          : Longitude outside of valid range
 *                                     (-180 to 360 degrees)
 *       LAMBERT_EASTING_ERROR      : Easting outside of valid range
 *                                     (depends on ellipsoid and projection
 *                                     parameters)
 *       LAMBERT_NORTHING_ERROR     : Northing outside of valid range
 *                                     (depends on ellipsoid and projection
 *                                     parameters)
 *       LAMBERT_FIRST_STDP_ERROR   : First standard parallel outside of valid
 *                                     range (-89 59 59.0 to 89 59 59.0 degrees)
 *       LAMBERT_SECOND_STDP_ERROR  : Second standard parallel outside of valid
 *                                     range (-89 59 59.0 to 89 59 59.0 degrees)
 *       LAMBERT_ORIGIN_LAT_ERROR   : Origin latitude outside of valid range
 *                                     (-89 59 59.0 to 89 59 59.0 degrees)
 *       LAMBERT_CENT_MER_ERROR     : Central meridian outside of valid range
 *                                     (-180 to 360 degrees)
 *       LAMBERT_A_ERROR            : Semi-major axis less than or equal to zero
 *       LAMBERT_INV_F_ERROR        : Inverse flattening outside of valid range
 *									                   (250 to 350)
 *       LAMBERT_HEMISPHERE_ERROR   : Standard parallels cannot be opposite latitudes
 *       LAMBERT_FIRST_SECOND_ERROR : The 1st & 2nd standard parallels cannot
 *                                     both be 0
 *
 *
 * REUSE NOTES
 *
 *    LAMBERT is intended for reuse by any application that performs a Lambert
 *    Conformal Conic projection or its inverse.
 *    
 * REFERENCES
 *
 *    Further information on LAMBERT can be found in the Reuse Manual.
 *
 *    LAMBERT originated from:
 *                      U.S. Army Topographic Engineering Center
 *                      Geospatial Information Division
 *                      7701 Telegraph Road
 *                      Alexandria, VA  22310-3864
 *
 * LICENSES
 *
 *    None apply to this component.
 *
 * RESTRICTIONS
 *
 *    LAMBERT has no restrictions.
 *
 * ENVIRONMENT
 *
 *    LAMBERT was tested and certified in the following environments:
 *
 *    1. Solaris 2.5 with GCC, version 2.8.1
 *    2. Windows 95 with MS Visual C++, version 6
 *
 * MODIFICATIONS
 *
 *    Date              Description
 *    ----              -----------
 *    10-02-97          Original Code
 *    08-15-99          Re-engineered Code
 *
 *
 */


/***************************************************************************/
/*
 *                               INCLUDES
 */

#include <math.h>
#include "lambert.h"
/*
 *    math.h     - Standard C math library
 *    lambert.h  - Is for prototype error checking
 */


/***************************************************************************/
/*                               DEFINES
 *
 */

#define PI         3.14159265358979323e0   /* PI     */
#define PI_OVER_2  (PI / 2.0)
#define MAX_LAT    (( PI *  89.99972222222222) / 180.0)  /* 89 59 59.0 degrees in radians */
#define TWO_PI     (2.0 * PI)
#define LAMBERT_m(clat,essin)                  (clat / sqrt(1.0 - essin * essin))
#define LAMBERT_t(lat,essin)                   tan(PI_OVER_4 - lat / 2) /				\
										            pow((1.0 - essin) / (1.0 + essin), es_OVER_2)
#define ES_SIN(sinlat)                         (es * sinlat)

/**************************************************************************/
/*                               GLOBAL DECLARATIONS
 *
 */

const double PI_OVER_4 = (PI / 4.0);

/* Ellipsoid Parameters, default to WGS 84  */
static double Lambert_a = 6378137.0;               /* Semi-major axis of ellipsoid, in meters */
static double Lambert_f = 1 / 298.257223563;       /* Flattening of ellipsoid */
static double es = 0.08181919084262188000;         /* Eccentricity of ellipsoid */
static double es2 = 0.0066943799901413800;         /* Eccentricity squared */
static double es_OVER_2 = .040909595421311;        /* Eccentricity / 2.0 */
static double n = .70802074516367;                 /* Ratio of angle between meridians */
static double F = 1.8538379832459;                 /* Flattening of ellipsoid */
static double rho0 = 6356377.2707128;              /* Height above ellipsoid */
static double Lambert_aF = 11824032.632946;        /* Lambert_a * F */

/* Lambert_Conformal_Conic projection Parameters */
static double Lambert_Std_Parallel_1 = (40 * PI / 180); /* Lower std. parallel, in radians */
static double Lambert_Std_Parallel_2 = (50 * PI / 180); /* Upper std. parallel, in radians */
static double Lambert_Origin_Lat = (45 * PI / 180);     /* Latitude of origin, in radians */
static double Lambert_Origin_Long = 0.0;                /* Longitude of origin, in radians */
static double Lambert_False_Northing = 0.0;             /* False northing, in meters */
static double Lambert_False_Easting = 0.0;              /* False easting, in meters */


/* Maximum variance for easting and northing values for WGS 84. */
static double Lambert_Delta_Easting = 40000000.0;
static double Lambert_Delta_Northing = 40000000.0;

/* These state variables are for optimization purposes. The only function
 * that should modify them is Set_Lambert_Parameters.         */



/************************************************************************/
/*                              FUNCTIONS
 *
 */

long Set_Lambert_Parameters(double a,
                            double f,
                            double Origin_Latitude,
                            double Central_Meridian,
                            double Std_Parallel_1,
                            double Std_Parallel_2,
                            double False_Easting,
                            double False_Northing)

{ /* BEGIN Set_Lambert_Parameters */
/*
 * The function Set_Lambert_Parameters receives the ellipsoid parameters and
 * Lambert Conformal Conic projection parameters as inputs, and sets the
 * corresponding state variables.  If any errors occur, the error code(s)
 * are returned by the function, otherwise LAMBERT_NO_ERROR is returned.
 *
 *   a                   : Semi-major axis of ellipsoid, in meters   (input)
 *   f                   : Flattening of ellipsoid						       (input)
 *   Origin_Latitude     : Latitude of origin, in radians            (input)
 *   Central_Meridian    : Longitude of origin, in radians           (input)
 *   Std_Parallel_1      : First standard parallel, in radians       (input)
 *   Std_Parallel_2      : Second standard parallel, in radians      (input)
 *   False_Easting       : False easting, in meters                  (input)
 *   False_Northing      : False northing, in meters                 (input)
 *
 *   Note that when the two standard parallel parameters are both set to the 
 *   same latitude value, the result is a Lambert Conformal Conic projection 
 *   with one standard parallel at the specified latitude.
 */

  double slat, slat1, clat;
  double es_sin;
  double t0, t1, t2;
  double m1, m2;
  double inv_f = 1 / f;
  long Error_Code = LAMBERT_NO_ERROR;

  if (a <= 0.0)
  { /* Semi-major axis must be greater than zero */
    Error_Code |= LAMBERT_A_ERROR;
  }
  if ((inv_f < 250) || (inv_f > 350))
  { /* Inverse flattening must be between 250 and 350 */
    Error_Code |= LAMBERT_INV_F_ERROR;
  }
  if ((Origin_Latitude < -MAX_LAT) || (Origin_Latitude > MAX_LAT))
  { /* Origin Latitude out of range */
    Error_Code |= LAMBERT_ORIGIN_LAT_ERROR;
  }
  if ((Std_Parallel_1 < -MAX_LAT) || (Std_Parallel_1 > MAX_LAT))
  { /* First Standard Parallel out of range */
    Error_Code |= LAMBERT_FIRST_STDP_ERROR;
  }
  if ((Std_Parallel_2 < -MAX_LAT) || (Std_Parallel_2 > MAX_LAT))
  { /* Second Standard Parallel out of range */
    Error_Code |= LAMBERT_SECOND_STDP_ERROR;
  }
  if ((Std_Parallel_1 == 0) && (Std_Parallel_2 == 0))
  { /* First & Second Standard Parallels are both 0 */
    Error_Code |= LAMBERT_FIRST_SECOND_ERROR;
  }
  if (Std_Parallel_1 == -Std_Parallel_2)
  { /* Parallels are the negation of each other */
    Error_Code |= LAMBERT_HEMISPHERE_ERROR;
  }
  if ((Central_Meridian < -PI) || (Central_Meridian > TWO_PI))
  { /* Origin Longitude out of range */
    Error_Code |= LAMBERT_CENT_MER_ERROR;
  }

  if (!Error_Code)
  { /* no errors */

    Lambert_a = a;
    Lambert_f = f;
    Lambert_Origin_Lat = Origin_Latitude;
    Lambert_Std_Parallel_1 = Std_Parallel_1;
    Lambert_Std_Parallel_2 = Std_Parallel_2;
    if (Central_Meridian > PI)
      Central_Meridian -= TWO_PI;
    Lambert_Origin_Long = Central_Meridian;
    Lambert_False_Easting = False_Easting;
    Lambert_False_Northing = False_Northing;

    es2 = 2 * Lambert_f - Lambert_f * Lambert_f;
    es = sqrt(es2);
    es_OVER_2 = es / 2.0;

    slat = sin(Lambert_Origin_Lat);
    es_sin = ES_SIN(slat);
    t0 = LAMBERT_t(Lambert_Origin_Lat, es_sin);

    slat1 = sin(Lambert_Std_Parallel_1);
    clat = cos(Lambert_Std_Parallel_1);
    es_sin = ES_SIN(slat1);
    m1 = LAMBERT_m(clat, es_sin);
    t1 = LAMBERT_t(Lambert_Std_Parallel_1, es_sin);


    if (fabs(Lambert_Std_Parallel_1 - Lambert_Std_Parallel_2) > 1.0e-10)
    {
      slat = sin(Lambert_Std_Parallel_2);
      clat = cos(Lambert_Std_Parallel_2);
      es_sin = ES_SIN(slat);
      m2 = LAMBERT_m(clat, es_sin);
      t2 = LAMBERT_t(Lambert_Std_Parallel_2, es_sin);
      n = log(m1 / m2) / log(t1 / t2);
    }
    else
      n = slat1;
    F = m1 / (n * pow(t1, n));
    Lambert_aF = Lambert_a * F;
    if ((t0 == 0) && (n < 0))
      rho0 = 0.0;
    else
      rho0 = Lambert_aF * pow(t0, n);

  }
  return (Error_Code);
} /* END OF Set_Lambert_Parameters */


void Get_Lambert_Parameters(double *a,
                            double *f,
                            double *Origin_Latitude,
                            double *Central_Meridian,
                            double *Std_Parallel_1,
                            double *Std_Parallel_2,
                            double *False_Easting,
                            double *False_Northing)

{ /* BEGIN Get_Lambert_Parameters */
/*                         
 * The function Get_Lambert_Parameters returns the current ellipsoid
 * parameters and Lambert Conformal Conic projection parameters.
 *
 *   a                   : Semi-major axis of ellipsoid, in meters   (output)
 *   f                   : Flattening of ellipsoid					         (output)
 *   Origin_Latitude     : Latitude of origin, in radians            (output)
 *   Central_Meridian    : Longitude of origin, in radians           (output)
 *   Std_Parallel_1      : First standard parallel, in radians       (output)
 *   Std_Parallel_2      : Second standard parallel, in radians      (output)
 *   False_Easting       : False easting, in meters                  (output)
 *   False_Northing      : False northing, in meters                 (output)
 */


  *a = Lambert_a;
  *f = Lambert_f;
  *Std_Parallel_1 = Lambert_Std_Parallel_1;
  *Std_Parallel_2 = Lambert_Std_Parallel_2;
  *Origin_Latitude = Lambert_Origin_Lat;
  *Central_Meridian = Lambert_Origin_Long;
  *False_Easting = Lambert_False_Easting;
  *False_Northing = Lambert_False_Northing;
  return;
} /* END OF Get_Lambert_Parameters */


long Convert_Geodetic_To_Lambert (double Latitude,
                                  double Longitude,
                                  double *Easting,
                                  double *Northing)

{ /* BEGIN Convert_Geodetic_To_Lambert */
/*
 * The function Convert_Geodetic_To_Lambert converts Geodetic (latitude and
 * longitude) coordinates to Lambert Conformal Conic projection (easting
 * and northing) coordinates, according to the current ellipsoid and
 * Lambert Conformal Conic projection parameters.  If any errors occur, the
 * error code(s) are returned by the function, otherwise LAMBERT_NO_ERROR is
 * returned.
 *
 *    Latitude         : Latitude, in radians                         (input)
 *    Longitude        : Longitude, in radians                        (input)
 *    Easting          : Easting (X), in meters                       (output)
 *    Northing         : Northing (Y), in meters                      (output)
 */

  double slat;
  double es_sin;
  double t;
  double rho;
  double dlam;
  double theta;
  long  Error_Code = LAMBERT_NO_ERROR;

  if ((Latitude < -PI_OVER_2) || (Latitude > PI_OVER_2))
  {  /* Latitude out of range */
    Error_Code|= LAMBERT_LAT_ERROR;
  }
  if ((Longitude < -PI) || (Longitude > TWO_PI))
  {  /* Longitude out of range */
    Error_Code|= LAMBERT_LON_ERROR;
  }

  if (!Error_Code)
  { /* no errors */

    if (fabs(fabs(Latitude) - PI_OVER_2) > 1.0e-10)
    {
      slat = sin(Latitude);
      es_sin = ES_SIN(slat);
      t = LAMBERT_t(Latitude, es_sin);
      rho = Lambert_aF * pow(t, n);
    }
    else
    {
      if ((Latitude * n) <= 0)
      { /* Point can not be projected */
        Error_Code |= LAMBERT_LAT_ERROR;
        return (Error_Code);
      }
      rho = 0.0;
    }

    dlam = Longitude - Lambert_Origin_Long;

    if (dlam > PI)
    {
      dlam -= TWO_PI;
    }
    if (dlam < -PI)
    {
      dlam += TWO_PI;
    }

    theta = n * dlam;

    *Easting = rho * sin(theta) + Lambert_False_Easting;
    *Northing = rho0 - rho * cos(theta) + Lambert_False_Northing;

  }
  return (Error_Code);
} /* END OF Convert_Geodetic_To_Lambert */



long Convert_Lambert_To_Geodetic (double Easting,
                                  double Northing,
                                  double *Latitude,
                                  double *Longitude)

{ /* BEGIN Convert_Lambert_To_Geodetic */
/*
 * The function Convert_Lambert_To_Geodetic converts Lambert Conformal
 * Conic projection (easting and northing) coordinates to Geodetic
 * (latitude and longitude) coordinates, according to the current ellipsoid
 * and Lambert Conformal Conic projection parameters.  If any errors occur,
 * the error code(s) are returned by the function, otherwise LAMBERT_NO_ERROR
 * is returned.
 *
 *    Easting          : Easting (X), in meters                       (input)
 *    Northing         : Northing (Y), in meters                      (input)
 *    Latitude         : Latitude, in radians                         (output)
 *    Longitude        : Longitude, in radians                        (output)
 */


  double dy, dx;
  double rho, rho0_MINUS_dy;
  double t;
  double PHI;
  double tempPHI = 0.0;
  double sin_PHI;
  double es_sin;
  double theta = 0.0;
  double tolerance = 4.85e-10;
  long Error_Code = LAMBERT_NO_ERROR;

  if ((Easting < (Lambert_False_Easting - Lambert_Delta_Easting))
      ||(Easting > (Lambert_False_Easting + Lambert_Delta_Easting)))
  { /* Easting out of range  */
    Error_Code |= LAMBERT_EASTING_ERROR;
  }
  if ((Northing < (Lambert_False_Northing - Lambert_Delta_Northing))
      || (Northing > (Lambert_False_Northing + Lambert_Delta_Northing)))
  { /* Northing out of range */
    Error_Code |= LAMBERT_NORTHING_ERROR;
  }

  if (!Error_Code)
  { /* no errors */

    dy = Northing - Lambert_False_Northing;
    dx = Easting - Lambert_False_Easting;
    rho0_MINUS_dy = rho0 - dy;
    rho = sqrt(dx * dx + (rho0_MINUS_dy) * (rho0_MINUS_dy));

    if (n < 0.0)
    {
      rho *= -1.0;
      dy *= -1.0;
      dx *= -1.0;
      rho0_MINUS_dy *= -1.0;
    }

    if (rho != 0.0)
    {
      theta = atan2(dx, rho0_MINUS_dy);
      t = pow(rho / (Lambert_aF) , 1.0 / n);
      PHI = PI_OVER_2 - 2.0 * atan(t);
      while (fabs(PHI - tempPHI) > tolerance)
      {
        tempPHI = PHI;
        sin_PHI = sin(PHI);
        es_sin = ES_SIN(sin_PHI);
        PHI = PI_OVER_2 - 2.0 * atan(t * pow((1.0 - es_sin) / (1.0 + es_sin), es_OVER_2));
      }
      *Latitude = PHI;
      *Longitude = theta / n + Lambert_Origin_Long;

      if (fabs(*Latitude) < 2.0e-7)  /* force lat to 0 to avoid -0 degrees */
        *Latitude = 0.0;
      if (*Latitude > PI_OVER_2)  /* force distorted values to 90, -90 degrees */
        *Latitude = PI_OVER_2;
      else if (*Latitude < -PI_OVER_2)
        *Latitude = -PI_OVER_2;

      if (*Longitude > PI)
      {
        if (*Longitude - PI < 3.5e-6) 
          *Longitude = PI;
        else
          *Longitude -= TWO_PI;
      }
      if (*Longitude < -PI)
      {
        if (fabs(*Longitude + PI) < 3.5e-6)
          *Longitude = -PI;
        else
          *Longitude += TWO_PI;
      }

      if (fabs(*Longitude) < 2.0e-7)  /* force lon to 0 to avoid -0 degrees */
        *Longitude = 0.0;
      if (*Longitude > PI)  /* force distorted values to 180, -180 degrees */
        *Longitude = PI;
      else if (*Longitude < -PI)
        *Longitude = -PI;
    }
    else
    {
      if (n > 0.0)
        *Latitude = PI_OVER_2;
      else
        *Latitude = -PI_OVER_2;
      *Longitude = Lambert_Origin_Long;
    }
  }
  return (Error_Code);
} /* END OF Convert_Lambert_To_Geodetic */



