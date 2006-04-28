/* bag_geotrans.c
	purpose:  use the geotrans engine to perform projection and maybe
	tranformation operations for the onswg bag.
	author:  dave fabre, us naval oceanographic office,
	date:  july 2005
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>

#if !defined( M_PI )
#	define M_PI    (3.14159265358979323846)
#endif
#define DEG2RAD   (M_PI/180.0)

#include "bag.h" /* include all necessary public dtcc & engine headers, structs, macros */

/*  function:  bagCoordsys
	purpose: taken from geotrans' engine.h from their Coordinate Type
		Enumeration we have a matching string set.  have included all
		coordinate sys types here although we only are supporting
		0, 5, 6, 17, 18, 25, and 30 initially.

	author:  dave fabre, us naval oceanographic office, jul 2005
*/
struct COORDINATE_SYS_TYPE
{
	char *name;
};

#define COORD_SYS_NAME(k) COORDINATE_SYS_LIST[k].name

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

#define MAX_NCOORD_SYS 32

/******************************************************************************/
Coordinate_Type bagCoordsys( char *str )
{
	long i;
	for(i = 0; i < MAX_NCOORD_SYS; i++)
		if ( strncmp(str, COORD_SYS_NAME(i), strlen(COORD_SYS_NAME(i))) == 0 )
			return i;
	return -1;
} /* bagCoordsys */

struct DATUM_NAME_TYPE
{
	char *name;
};

#define DATUM_NAME(k) DATUM_NAME_LIST[k].name

static struct DATUM_NAME_TYPE DATUM_NAME_LIST[]=
{
  {"WGS84"},
  {"WGS72"},
  {"NAD83"}
};

#define MAX_DATUMS 3

/******************************************************************************/
bagDatum bagDatumID( char *str )
{
	long i;

	for(i = 0; i < MAX_DATUMS; i++)
		if ( strncmp(str, DATUM_NAME(i), strlen(DATUM_NAME(i))) == 0 )
			return i;
	return -1;
} /* bagDatumID */

/* prototype file functions */
static long zero_params();
static long zero_tuples();
static long print_datum( const bagDatum datum );
static long print_geodetic_params( bagGeotransParameters *params );
static long print_geodetic_tuple( bagGeotransTuple *tuple );
static long print_geodetic( bagGeotransTuple *tuple, bagGeotransParameters *params );
static long print_mercator_params( bagGeotransParameters *params );
static long print_mercator_tuple( bagGeotransTuple *tuple );
static long print_mercator( bagGeotransTuple *tuple, bagGeotransParameters *params );
static long print_tranmerc_params( bagGeotransParameters *params );
static long print_tranmerc_tuple( bagGeotransTuple *tuple );
static long print_tranmerc( bagGeotransTuple *tuple, bagGeotransParameters *params );
static long print_utm_params( bagGeotransParameters *params );
static long print_utm_tuple( bagGeotransTuple *tuple );
static long print_utm( bagGeotransTuple *tuple, bagGeotransParameters *params );
static long print_polarst_params( bagGeotransParameters *params );
static long print_polarst_tuple( bagGeotransTuple *tuple );
static long print_polarst( bagGeotransTuple *tuple, bagGeotransParameters *params );
static long print_ups_params( bagGeotransParameters *params );
static long print_ups_tuple( bagGeotransTuple *tuple );
static long print_ups( bagGeotransTuple *tuple, bagGeotransParameters *params );
static long print_lambert_params( bagGeotransParameters *params );
static long print_lambert_tuple( bagGeotransTuple *tuple );
static long print_lambert( bagGeotransTuple *tuple, bagGeotransParameters *params );
static long set_datum( const Input_or_Output io, const bagDatum datum );
static long init_geodetic( const Input_or_Output io, bagGeotransParameters *params );
static long init_mercator( const Input_or_Output io, bagGeotransParameters *params );
static long init_tranmerc( const Input_or_Output io, bagGeotransParameters *params );
static long init_utm( const Input_or_Output io, bagGeotransParameters *params );
static long init_polarst( const Input_or_Output io, bagGeotransParameters *params );
static long init_ups( const Input_or_Output io, bagGeotransParameters *params );
static long init_lambert( const Input_or_Output io, bagGeotransParameters *params );
static long init_inputs( const Coordinate_Type input_sys, bagGeotransParameters *input_params );
static long init_outputs( const Coordinate_Type output_sys, bagGeotransParameters *output_params );
static long set_input_coords( const Coordinate_Type input_sys, bagGeotransTuple *input_coords );
static long get_output_coords( const Coordinate_Type output_sys, bagGeotransTuple *output_coords );
static long bag_geotrans_datastat();
static long bag_env( char *bag_path );

/* 2 unions below for file-only use. */
/* union for parameters (defined in engine.h) of all bag supported horizontal coord. sys. */
typedef union t_GeotransParameters
{
    Geodetic_Parameters geodetic;		/* geographic - longitude,latitude */
    Mercator_Parameters mercator;		/* mercator */
    Transverse_Mercator_Parameters tranmerc;	/* transverse mercator */
    UTM_Parameters utm;				/* universal transverse mercator */
    Polar_Stereo_Parameters polarst;		/* polar stereographic */
    Lambert_Conformal_Conic_Parameters lambert;	/* lambert conformal conic */
} GeotransParameters;

/* union of all the coordinate tuples defined in engine.h needed for bag */
typedef union t_GeotransTuple
{
    Geodetic_Tuple geodetic;			/* longitude, latitude */
    Mercator_Tuple mercator;			/* mercator */
    Transverse_Mercator_Tuple tranmerc;		/* transverse mercator */
    UTM_Tuple utm;				/* universal transverse mercator */
    Polar_Stereo_Tuple polarst;			/* polar stereographic */
    UPS_Tuple ups;				/* universal polar stereographic */
    Lambert_Conformal_Conic_Tuple lambert;	/* lambert conformal conic */
} GeotransTuple;

/* these data are used global within this file */
static GeotransParameters inParams, outParams;
static GeotransTuple inTuple, outTuple;

/******************************************************************************/
static long zero_params()
{
	memset(&inParams, 0, sizeof(GeotransParameters));
	memset(&outParams, 0, sizeof(GeotransParameters));
    return BAG_SUCCESS;
} /* zero_params */

/******************************************************************************/
static long zero_tuples()
{
	memset(&inTuple, 0, sizeof(GeotransTuple));
	memset(&outTuple, 0, sizeof(GeotransTuple));
    return BAG_SUCCESS;
} /* zero_tuples */

/******************************************************************************/
static long set_datum( const Input_or_Output io, const bagDatum datum )
{
	long stat=0, datum_index;

	switch ( datum )
	{
		case wgs84:
		{
			stat |= Get_Datum_Index( "WGE", &datum_index );
			stat |= Set_Datum( Interactive, io, datum_index );
		}
		break;
		case wgs72:
		{
			stat |= Get_Datum_Index( "WGC", &datum_index );
			stat |= Set_Datum( Interactive, io, datum_index );
		}
		break;
		case nad83:
		{
			stat |= Get_Datum_Index( "NAR-C", &datum_index );
			stat |= Set_Datum( Interactive, io, datum_index );
			/*dhf stat |= Get_Ellipsoid_Index( "RF", &ellip_index ); */
		}
		break;
		default:
		{
			fprintf(stderr, "bag_geotrans:  invalid datum %d\n", datum);
			return 1;
		}
		break;
	}

	return stat;

} /* set_datum */

/*****************************************************************************/
static long print_datum( const bagDatum datum )
{
	switch (datum)
	{
		case wgs84: printf("wgs84"); break;
		case wgs72: printf("wgs72"); break;
		case nad83: printf("nad83"); break;
		default: ; break;
	}
	return 1;
} /* print_datum */

/*****************************************************************************/
static long print_geodetic_params( bagGeotransParameters *params )
{
	printf("Geodetic: Datum: <"); print_datum(params->datum); printf(">\n");
	printf("\tParameters:\n");
	return 1;	
} /* print_geodetic_params */

/*****************************************************************************/
static long print_geodetic_tuple( bagGeotransTuple *tuple )
{
	printf("%lf %lf %lf\n", tuple->longitude, tuple->latitude, tuple->height);
	return 1;
} /* print_geodetic_tuple */

/*****************************************************************************/
static long print_geodetic( bagGeotransTuple *tuple, bagGeotransParameters *params )
{
	print_geodetic_params( params );
	printf("Tuple:  ");
	print_geodetic_tuple( tuple );
	return 1;
} /* print_geodetic */

/*****************************************************************************/
static long init_geodetic( const Input_or_Output io, bagGeotransParameters *params )
{
	long stat=0;

	switch ( io )
	{
		case Input:
		{
			inParams.geodetic.height_type = No_Height; /* for now */
			stat |= Set_Coordinate_System( Interactive, Input, Geodetic );
			stat |= Set_Geodetic_Params( Interactive, Input, inParams.geodetic );
		}
		break;
		case Output:
		{
			outParams.geodetic.height_type = No_Height; /* for now */
			stat |= Set_Coordinate_System( Interactive, Output, Geodetic );
			stat |= Set_Geodetic_Params( Interactive, Output, outParams.geodetic );
		}
		break;
		default:
		{
			fprintf(stderr, "bag_geotrans:  problem setting geodetic i/o params\n");
			return 1;
		}
		break;
	}

	return stat;
	
} /* init_geodetic */

/*****************************************************************************/
static long print_mercator_params( bagGeotransParameters *params )
{
	printf("Mercator:  Datum: <"); print_datum(params->datum); printf(">\n");
	printf("\tParameters:\n");
	printf("\t\torigin_latitude %lf\n", params->origin_latitude);
	printf("\t\tcentral_meridian %lf\n", params->central_meridian);
	printf("\t\tscale_factor %lf\n", params->scale_factor);
	printf("\t\tfalse_easting %lf\n", params->false_easting);
	printf("\t\tfalse_northing %lf\n", params->false_northing);
	return 1;
} /* print_mercator_params */

/*****************************************************************************/
static long print_mercator_tuple( bagGeotransTuple *tuple )
{
	printf("%lf %lf %lf\n", tuple->easting, tuple->northing, tuple->height);
	return 1;
} /* print_mercator_tuple */

/*****************************************************************************/
static long print_mercator( bagGeotransTuple *tuple, bagGeotransParameters *params )
{
	print_mercator_params( params );
	printf("Tuple:  ");
	print_mercator_tuple( tuple );
	return 1;
} /* print_mercator */

/*****************************************************************************/
static long init_mercator( const Input_or_Output io, bagGeotransParameters *params )
{
	long stat=0;

	switch ( io )
	{
		case Input:
		{
			inParams.mercator.origin_latitude = params->origin_latitude*DEG2RAD;
			inParams.mercator.central_meridian = params->central_meridian*DEG2RAD;
			inParams.mercator.scale_factor = params->scale_factor;
			inParams.mercator.false_easting = params->false_easting;
			inParams.mercator.false_northing = params->false_northing;
			stat |= Set_Coordinate_System( Interactive, Input, Mercator );
			stat |= Set_Mercator_Params( Interactive, Input, inParams.mercator );
		}
		break;
		case Output:
		{
			outParams.mercator.origin_latitude = params->origin_latitude*DEG2RAD;
			outParams.mercator.central_meridian = params->central_meridian*DEG2RAD;
			outParams.mercator.scale_factor = params->scale_factor;
			outParams.mercator.false_easting = params->false_easting;
			outParams.mercator.false_northing = params->false_northing;
			stat |= Set_Coordinate_System( Interactive, Output, Mercator );
			stat |= Set_Mercator_Params( Interactive, Output, outParams.mercator );
		}
		break;
		default:
		{
			fprintf(stderr, "bag_geotrans:  problem setting mercator i/o params\n");
			return 1;
		}
		break;
	}

	return stat;
	
} /* init_mercator */

/*****************************************************************************/
static long print_tranmerc_params( bagGeotransParameters *params )
{
	printf("Transverse Mercator:  Datum: <"); print_datum(params->datum); printf(">\n");
	printf("\tParameters:\n");
	printf("\t\torigin_latitude %lf\n", params->origin_latitude);
	printf("\t\tcentral_meridian %lf\n", params->central_meridian);
	printf("\t\tscale_factor %lf\n", params->scale_factor);
	printf("\t\tfalse_easting %lf\n", params->false_easting);
	printf("\t\tfalse_northing %lf\n", params->false_northing);
	return 1;
} /* print_tranmerc_params */

/*****************************************************************************/
static long print_tranmerc_tuple( bagGeotransTuple *tuple )
{
	printf("%lf %lf %lf\n", tuple->easting, tuple->northing, tuple->height);
	return 1;
} /* print_tranmerc_tuple */

/*****************************************************************************/
static long print_tranmerc( bagGeotransTuple *tuple, bagGeotransParameters *params )
{
	print_tranmerc_params( params );
	printf("Tuple:  ");
	print_tranmerc_tuple( tuple );
	return 1;
} /* print_tranmerc */

/*****************************************************************************/
static long init_tranmerc( const Input_or_Output io, bagGeotransParameters *params )
{
	long stat=0;

	switch ( io )
	{
		case Input:
		{
			inParams.tranmerc.origin_latitude = params->origin_latitude*DEG2RAD;
			inParams.tranmerc.central_meridian = params->central_meridian*DEG2RAD;
			inParams.tranmerc.scale_factor = params->scale_factor;
			inParams.tranmerc.false_easting = params->false_easting;
			inParams.tranmerc.false_northing = params->false_northing;
			stat |= Set_Coordinate_System( Interactive, Input, Transverse_Mercator );
			stat |= Set_Transverse_Mercator_Params( Interactive, Input, inParams.tranmerc );
		}
		break;
		case Output:
		{
			outParams.tranmerc.origin_latitude = params->origin_latitude*DEG2RAD;
			outParams.tranmerc.central_meridian = params->central_meridian*DEG2RAD;
			outParams.tranmerc.scale_factor = params->scale_factor;
			outParams.tranmerc.false_easting = params->false_easting;
			outParams.tranmerc.false_northing = params->false_northing;
			stat |= Set_Coordinate_System( Interactive, Output, Transverse_Mercator );
			stat |= Set_Transverse_Mercator_Params( Interactive, Output, outParams.tranmerc );
		}
		break;
		default:
		{
			fprintf(stderr, "bag_geotrans:  problem setting tranmerc i/o params\n");
			return 1;
		}
		break;
	}

	return stat;
	
} /* init_tranmerc */

/*****************************************************************************/
static long print_utm_params( bagGeotransParameters *params )
{
	printf("UTM:  Datum: <"); print_datum(params->datum); printf(">\n");
	printf("\tParameters:\n");
	printf("\t\tzone %d\n", params->zone);
	printf("\t\toverride %d\n", params->override);
	return 1;
} /* print_utm_params */

/*****************************************************************************/
static long print_utm_tuple( bagGeotransTuple *tuple )
{
	printf("%lf %lf %lf %d %c\n", tuple->easting, tuple->northing, 
		tuple->height, tuple->zone, tuple->hemisphere);
	return 1;
} /* print_utm_tuple */

/*****************************************************************************/
static long print_utm( bagGeotransTuple *tuple, bagGeotransParameters *params )
{
	print_utm_params( params );
	printf("Tuple:  ");
	print_utm_tuple( tuple );
	return 1;
} /* print_utm */

/*****************************************************************************/
static long init_utm( const Input_or_Output io, bagGeotransParameters *params )
{
	long stat=0;

	switch ( io )
	{
		case Input:
		{
			inParams.utm.zone = params->zone;
			inParams.utm.override = params->override; /* 0=autozone, 1=use zone */
			stat |= Set_Coordinate_System( Interactive, Input, UTM );
			stat |= Set_UTM_Params( Interactive, Input, inParams.utm );
		}
		break;
		case Output:
		{
			outParams.utm.zone = params->zone;
			outParams.utm.override = params->override; /* 0=autozone, 1=use zone */
			stat |= Set_Coordinate_System( Interactive, Output, UTM );
			stat |= Set_UTM_Params( Interactive, Output, outParams.utm );
		}
		break;
		default:
		{
			fprintf(stderr, "bag_geotrans:  problem setting utm i/o params\n");
			return 1;
		}
		break;
	}

	return stat;
	
} /* init_utm */

/*****************************************************************************/
static long print_polarst_params( bagGeotransParameters *params )
{
	printf("Polar Stereographic:  Datum: <"); print_datum(params->datum); printf(">\n");
	printf("\tParameters:\n");
	printf("\t\tlatitude_of_true_scale %lf\n", params->latitude_of_true_scale);
	printf("\t\tlongitude_down_from_pole %lf\n", params->longitude_down_from_pole);
	printf("\t\tfalse_easting %lf\n", params->false_easting);
	printf("\t\tfalse_northing %lf\n", params->false_northing);
	return 1;
} /* print_polarst_params */

/*****************************************************************************/
static long print_polarst_tuple( bagGeotransTuple *tuple )
{
	printf("%lf %lf %lf\n", tuple->easting, tuple->northing, tuple->height);
	return 1;
} /* print_polarst_tuple */

/*****************************************************************************/
static long print_polarst( bagGeotransTuple *tuple, bagGeotransParameters *params )
{
	print_polarst_params( params );
	printf("Tuple:  ");
	print_polarst_tuple( tuple );
	return 1;
} /* print_polarst */

/*****************************************************************************/
static long init_polarst( const Input_or_Output io, bagGeotransParameters *params )
{
	long stat=0;

	switch ( io )
	{
		case Input:
		{
			inParams.polarst.latitude_of_true_scale = params->latitude_of_true_scale*DEG2RAD;
			inParams.polarst.longitude_down_from_pole = params->longitude_down_from_pole*DEG2RAD;
			inParams.polarst.false_easting = params->false_easting;
			inParams.polarst.false_northing = params->false_northing;
			stat |= Set_Coordinate_System( Interactive, Input, Polar_Stereo );
			stat |= Set_Polar_Stereo_Params( Interactive, Input, inParams.polarst );
		}
		break;
		case Output:
		{
			outParams.polarst.latitude_of_true_scale = params->latitude_of_true_scale*DEG2RAD;
			outParams.polarst.longitude_down_from_pole = params->longitude_down_from_pole*DEG2RAD;
			outParams.polarst.false_easting = params->false_easting;
			outParams.polarst.false_northing = params->false_northing;
			stat |= Set_Coordinate_System( Interactive, Output, Polar_Stereo );
			stat |= Set_Polar_Stereo_Params( Interactive, Output, outParams.polarst );
		}
		break;
		default:
		{
			fprintf(stderr, "bag_geotrans:  problem setting polarst i/o params\n");
			return 1;
		}
		break;
	}

	return stat;
	
} /* init_polarst */

/*****************************************************************************/
static long print_ups_params( bagGeotransParameters *params )
{
	printf("UPS:  Datum: <"); print_datum(params->datum); printf(">\n");
	printf("\tParameters:\n");
	return 1;
} /* print_ups_params */

/*****************************************************************************/
static long print_ups_tuple( bagGeotransTuple *tuple )
{
	printf("%lf %lf %lf %c\n", tuple->easting, tuple->northing,
		tuple->height, tuple->hemisphere);
	return 1;
} /* print_ups_tuple */

/*****************************************************************************/
static long print_ups( bagGeotransTuple *tuple, bagGeotransParameters *params )
{
	print_ups_params( params );
	printf("Tuple:  ");
	print_ups_tuple( tuple );
	return 1;
} /* print_ups */

/*****************************************************************************/
static long init_ups( const Input_or_Output io, bagGeotransParameters *params )
{
	long stat=0;

	switch ( io )
	{
		case Input:
		{
			stat |= Set_Coordinate_System( Interactive, Input, UPS );
		}
		break;
		case Output:
		{
			stat |= Set_Coordinate_System( Interactive, Output, UPS );
		}
		break;
		default:
		{
			fprintf(stderr, "bag_geotrans:  problem setting ups i/o params\n");
			return 1;
		}
		break;
	}

	return stat;
	
} /* init_ups */

/*****************************************************************************/
static long print_lambert_params( bagGeotransParameters *params )
{
	printf("Lambert Conformal Conic:  Datum: <"); print_datum(params->datum); printf(">\n");
	printf("\tParameters:\n");
	printf("\t\torigin_latitude %lf\n", params->origin_latitude);
	printf("\t\tcentral_meridian %lf\n", params->central_meridian);
	printf("\t\tstd_parallel_1 %lf\n", params->std_parallel_1);
	printf("\t\tstd_parallel_2 %lf\n", params->std_parallel_2);
	printf("\t\tfalse_easting %lf\n", params->false_easting);
	printf("\t\tfalse_northing %lf\n", params->false_northing);
	return 1;
} /* print_lambert_params */

/*****************************************************************************/
static long print_lambert_tuple( bagGeotransTuple *tuple )
{
	printf("%lf %lf %lf\n", tuple->easting, tuple->northing, tuple->height);
	return 1;
} /* print_lambert_tuple */

/*****************************************************************************/
static long print_lambert( bagGeotransTuple *tuple, bagGeotransParameters *params )
{
	print_lambert_params( params );
	printf("Tuple:  ");
	print_lambert_tuple( tuple );
	return 1;
} /* print_lambert */

/*****************************************************************************/
static long init_lambert( const Input_or_Output io, bagGeotransParameters *params )
{
	long stat=0;

	switch ( io )
	{
		case Input:
		{
			inParams.lambert.origin_latitude = params->origin_latitude*DEG2RAD;
			inParams.lambert.central_meridian = params->central_meridian*DEG2RAD;
			inParams.lambert.std_parallel_1 = params->std_parallel_1*DEG2RAD;
			inParams.lambert.std_parallel_2 = params->std_parallel_2*DEG2RAD;
			inParams.lambert.false_easting = params->false_easting;
			inParams.lambert.false_northing = params->false_northing;
			stat |= Set_Coordinate_System( Interactive, Input, Lambert_Conformal_Conic );
			stat |= Set_Lambert_Conformal_Conic_Params( Interactive, Input, inParams.lambert );
		}
		break;
		case Output:
		{
			outParams.lambert.origin_latitude = params->origin_latitude*DEG2RAD;
			outParams.lambert.central_meridian = params->central_meridian*DEG2RAD;
			outParams.lambert.std_parallel_1 = params->std_parallel_1*DEG2RAD;
			outParams.lambert.std_parallel_2 = params->std_parallel_2*DEG2RAD;
			outParams.lambert.false_easting = params->false_easting;
			outParams.lambert.false_northing = params->false_northing;
			stat |= Set_Coordinate_System( Interactive, Output, Lambert_Conformal_Conic );
			stat |= Set_Lambert_Conformal_Conic_Params( Interactive, Output, outParams.lambert );
		}
		break;
		default:
		{
			fprintf(stderr, "bag_geotrans:  problem setting lambert i/o params\n");
			return 1;
		}
		break;
	}

	return stat;
	
} /* init_lambert */

/*****************************************************************************/
static long init_inputs( const Coordinate_Type input_sys, bagGeotransParameters *input_params )
{
	long stat=0;

	if ( set_datum( Input, input_params->datum ) != 0 )
	{
		fprintf(stderr, "bagGeotrans:  problem with input datum %d\n",
			input_params->datum);
		return 1;
	}
	switch ( input_sys )
	{
		case Geodetic:
		{
			stat |= init_geodetic( Input, input_params );
		}
		break;
		case Mercator:
		{
			stat |= init_mercator( Input, input_params );
		}
		break;
		case Transverse_Mercator:
		{
			stat |= init_tranmerc( Input, input_params );
		}
		break;
		case UTM:
		{
			stat |= init_utm( Input, input_params );
		}
		break;
		case Polar_Stereo:
		{
			stat |= init_polarst( Input, input_params );
		}
		break;
		case UPS:
		{
			stat |= init_ups( Input, input_params );
		}
		break;
		case Lambert_Conformal_Conic:
		{
			stat |= init_lambert( Input, input_params );
		}
		break;
		default:
		{
			fprintf(stderr, "bag_geotrans:  unknown input type %d\n", input_sys);
			return 1;
		}
		break;
	} /* input switch */

	return stat;

} /* init_inputs */

/*****************************************************************************/
static long init_outputs( const Coordinate_Type output_sys, bagGeotransParameters *output_params )
{
	long stat=0;

	if ( set_datum( Output, output_params->datum ) != 0 )
	{
		fprintf(stderr, "bagGeotrans:  problem with output datum %d\n",
			output_params->datum);
		return 1;
	}

	switch ( output_sys )
	{
		case Geodetic:
		{
			stat |= init_geodetic( Output, output_params );
		}
		break;
		case Mercator:
		{
			stat |= init_mercator( Output, output_params );
		}
		break;
		case Transverse_Mercator:
		{
			stat |= init_tranmerc( Output, output_params );
		}
		break;
		case UTM:
		{
			stat |= init_utm( Output, output_params );
		}
		break;
		case Polar_Stereo:
		{
			stat |= init_polarst( Output, output_params );
		}
		break;
		case UPS:
		{
			stat |= init_ups( Output, output_params );
		}
		break;
		case Lambert_Conformal_Conic:
		{
			stat |= init_lambert( Output, output_params );
		}
		break;
		default:
		{
			fprintf(stderr, "bag_geotrans:  unknown output type %d\n", output_sys);
			return 1;
		}
		break;
	} /* output switch */

	return stat;

} /* init_outputs */

/*****************************************************************************/
static long set_input_coords( const Coordinate_Type input_sys, bagGeotransTuple *input_coords )
{
	switch ( input_sys )
	{
		case Geodetic:
		{
			inTuple.geodetic.longitude = input_coords->longitude*DEG2RAD;
			inTuple.geodetic.latitude = input_coords->latitude*DEG2RAD;
			inTuple.geodetic.height = input_coords->height;
			Set_Geodetic_Coordinates( Interactive, Input, inTuple.geodetic );
		}
		break;
		case Mercator:
		{
			inTuple.mercator.easting = input_coords->easting;
			inTuple.mercator.northing = input_coords->northing;
			Set_Mercator_Coordinates( Interactive, Input, inTuple.mercator );
		}
		break;
		case Transverse_Mercator:
		{
			inTuple.tranmerc.easting = input_coords->easting;
			inTuple.tranmerc.northing = input_coords->northing;
			Set_Transverse_Mercator_Coordinates( Interactive, Input, inTuple.tranmerc );
		}
		break;
		case UTM:
		{
			inTuple.utm.easting = input_coords->easting;
			inTuple.utm.northing = input_coords->northing;
			inTuple.utm.zone = input_coords->zone;
			inTuple.utm.hemisphere = input_coords->hemisphere;
			Set_UTM_Coordinates( Interactive, Input, inTuple.utm );
		}
		break;
		case Polar_Stereo:
		{
			inTuple.polarst.easting = input_coords->easting;
			inTuple.polarst.northing = input_coords->northing;
			Set_Polar_Stereo_Coordinates( Interactive, Input, inTuple.polarst );
		}
		break;
		case UPS:
		{
			inTuple.ups.easting = input_coords->easting;
			inTuple.ups.northing = input_coords->northing;
			inTuple.ups.hemisphere = input_coords->hemisphere;
			Set_UPS_Coordinates( Interactive, Input, inTuple.ups );
		}
		break;
		case Lambert_Conformal_Conic:
		{
			inTuple.lambert.easting = input_coords->easting;
			inTuple.lambert.northing = input_coords->northing;
			Set_Lambert_Conformal_Conic_Coordinates( Interactive, Input, inTuple.lambert );
		}
		break;
		default:
		{
			fprintf(stderr, "bag_geotrans:  problem setting input coords\n");
			return 1;
		}
		break;
	}

	return 0;

} /* set_input_coords */

/*****************************************************************************/
static long get_output_coords( const Coordinate_Type output_sys, bagGeotransTuple *output_coords )
{
	long stat=0;

	switch ( output_sys )
	{
		case Geodetic:
		{
			stat |= Get_Geodetic_Coordinates( Interactive, Output, &outTuple.geodetic );
			output_coords->longitude = outTuple.geodetic.longitude/DEG2RAD;
			output_coords->latitude = outTuple.geodetic.latitude/DEG2RAD;
			output_coords->height = outTuple.geodetic.height;
		}
		break;
		case Mercator:
		{
			stat |= Get_Mercator_Coordinates( Interactive, Output, &outTuple.mercator );
			output_coords->easting = outTuple.mercator.easting;
			output_coords->northing = outTuple.mercator.northing;
		}
		break;
		case Transverse_Mercator:
		{
			stat |= Get_Transverse_Mercator_Coordinates( Interactive, Output, &outTuple.tranmerc );
			output_coords->easting = outTuple.tranmerc.easting;
			output_coords->northing = outTuple.tranmerc.northing;
		}
		break;
		case UTM:
		{
			stat |= Get_UTM_Coordinates( Interactive, Output, &outTuple.utm );
			output_coords->easting = outTuple.utm.easting;
			output_coords->northing = outTuple.utm.northing;
			output_coords->zone = outTuple.utm.zone;
			output_coords->hemisphere = outTuple.utm.hemisphere;
		}
		break;
		case Polar_Stereo:
		{
			stat |= Get_Polar_Stereo_Coordinates( Interactive, Output, &outTuple.polarst );
			output_coords->easting = outTuple.polarst.easting;
			output_coords->northing = outTuple.polarst.northing;
		}
		break;
		case UPS:
		{
			stat |= Get_UPS_Coordinates( Interactive, Output, &outTuple.ups );
			output_coords->easting = outTuple.ups.easting;
			output_coords->northing = outTuple.ups.northing;
			output_coords->hemisphere = outTuple.ups.hemisphere;
		}
		break;
		case Lambert_Conformal_Conic:
		{
			stat |= Get_Lambert_Conformal_Conic_Coordinates( Interactive, Output, &outTuple.lambert );
			output_coords->easting = outTuple.lambert.easting;
			output_coords->northing = outTuple.lambert.northing;
		}
		break;
		default:
		{
			fprintf(stderr, "bag_geotrans:  problem getting output coords\n");
			return 1;
		}
		break;
	}

	return stat;

} /* get_output_coords */

/* bag_env module
	purpose:  read/set the necessary environment variables for the ons bag.
		BAG_HOME should be the directory to get to the schema files
		for XML and the GEOTRANS data file for datums, ellipses, and
		the geoid.
	author:  dave fabre, us naval oceanographic office, july 2005
*/
/******************************************************************************/
static long bag_geotrans_datastat()
{
	char *ptr1=getenv("DATUM_DATA");
	char *ptr2=getenv("ELLIPSOID_DATA");
	char *ptr3=getenv("GEOID_DATA");
	char str1[2048], str2[2048], str3[2048], str4[2048];
	struct stat buf1, buf2, buf3, buf4;

	sprintf(str1, "%s/3_param.dat", ptr1);
	sprintf(str2, "%s/7_param.dat", ptr1);
	sprintf(str3, "%s/ellips.dat", ptr2);
	sprintf(str4, "%s/egm96.grd", ptr3);

	/* do a test status of the necessary geotrans data files */
	if ( stat(str1, &buf1) != 0 ) return 0;
	else if ( stat(str2, &buf2) != 0 ) return 0;
	else if ( stat(str3, &buf3) != 0 ) return 0;
	else if ( stat(str4, &buf4) != 0 ) return 0;
	else return 1;

} /* bag_geotrans_datastat */

/******************************************************************************/
static long bag_env( char *bag_path )
{
	static char datum_path[2048];
	static char ellipsoid_path[2048];
	static char geoid_path[2048];
	char *ptr;
	long stat1, stat2, stat3;

	if ( getenv("BAG_HOME") && bag_geotrans_datastat() )
	{
		/*dhf printf("everything seems ok ... \n"); */
		return 1; /* everything ok */
	}

	ptr = getenv("BAG_HOME");

	if (ptr != NULL)
	{
		strcpy(bag_path, ptr);

		/* DATUM_DATA is the dir for 3_param.dat & 7_param.dat */
		sprintf(datum_path, "DATUM_DATA=%s", bag_path);
		stat1 = putenv(datum_path);

		/* ELLIPSOID_DATA is the dir that has ellips.dat */
		sprintf(ellipsoid_path, "ELLIPSOID_DATA=%s", bag_path);
		stat2 = putenv(ellipsoid_path);

		/* GEOID_DATA is dir path to egm96.grd */
		sprintf(geoid_path, "GEOID_DATA=%s", bag_path);
		stat3 = putenv(geoid_path);


#if defined( DEBUG_BAGENV )
		printf("BAG_HOME=%s\n", bag_path);
		printf("%s\n", datum_path);
		printf("%s\n", ellipsoid_path);
		printf("%s\n", geoid_path);
#endif
		if ( !(stat1+stat2+stat3) && bag_geotrans_datastat() )
			return 1;
		else
			return !(stat1 + stat2 + stat3);
	}
	else
	{
		fprintf(stderr, "bag_geotrans:  environment variable BAG_HOME not set\n");
		return 0;
	}
} /* bag_env */

/*****************************************************************************/
bagError bagGeotransInit( const Coordinate_Type input_sys,
	bagGeotransParameters *input_params,
	const Coordinate_Type output_sys,
	bagGeotransParameters *output_params )
{
	long stat=0, input_status=0, output_status=0;
	char path[2048];

	/* internal access functions to zero out internal data structs */
	zero_params();
	zero_tuples();

	if ( !bag_env(path) )
	{
		fprintf(stderr, "bag_geotrans:  problem with BAG_HOME environment variable\n");
		return BAG_GEOTRANS_DATAFILE_INIT_ERROR;
	}

	if ( (stat=(long)Initialize_Engine()) != 0 )
	{
		fprintf(stderr, "bag_geotrans:  problem with Initialize_Engine (%ld)\n", stat);
		return BAG_GEOTRANS_ENGINE_INIT_ERROR;
	}

	Set_Precision( Thousandth_of_Second ); /* 0.01m */
	input_status = init_inputs( input_sys, input_params );
	output_status = init_outputs( output_sys, output_params );

	if ( input_status | output_status ) return BAG_GEOTRANS_IO_INIT_ERROR;
	else return 0;

} /* bagGeotransInit */

/*****************************************************************************/
bagError bagGeotransConvert( const Coordinate_Type input_sys,
                bagGeotransTuple *input_coords,
                const Coordinate_Type output_sys,
                bagGeotransTuple *output_coords )
{
	long stat=0;

	stat |= set_input_coords( input_sys, input_coords );
/*dhf printf("set_input_coords stat = %#8.8x\n", stat); */

	stat |= Convert( Interactive );
/*dhf printf("Convert stat = %#8.8x\n", stat); */

	stat |= get_output_coords( output_sys, output_coords );
/*dhf printf("get_output_coords stat = %#8.8x\n", stat); */

	if (stat) return BAG_GEOTRANS_CONVERSION_ERROR;
	else return BAG_SUCCESS;
	
} /* bagGeotransConvert */

#if defined( _TEST_BAG_GEOTRANS )
/*****************************************************************************/
int main(int argc, char **argv)
{
	bagGeotransParameters input_params, output_params;
	bagGeotransTuple input_coords, output_coords;
	long init_stat, conv_stat;
	char str[2048];

printf("\n***********************************************************************************************\n");
	printf("nad83 - Geodetic <-> Mercator: ---------------------------------------------\n");

	input_params.datum = bagDatumID("NAD83");
	input_coords.longitude = 75;
	input_coords.latitude = -35;
	input_coords.height = 0;

	output_params.datum = bagDatumID("NAD83");
	output_params.origin_latitude = 0;
	output_params.central_meridian = 0;
	output_params.scale_factor = 1;
	output_params.false_easting = 0;
	output_params.false_northing = 0;

	init_stat = bagGeotransInit( Geodetic, &input_params, Mercator, &output_params );
	conv_stat = bagGeotransConvert( Geodetic, &input_coords, Mercator, &output_coords );
	printf("stati(init,convert):  %#8.8x %#8.8x\n", init_stat, conv_stat);

	print_geodetic( &input_coords, &input_params );
	print_mercator( &output_coords, &output_params );

Get_Conversion_Status_String( Interactive, Input, ";", str ); printf("<%s>\n", str);
Get_Conversion_Status_String( Interactive, Output, ";", str ); printf("<%s>\n", str);
printf("----------------------------------------------------------------------\n");

	input_params = output_params;
	input_coords = output_coords;

	init_stat = bagGeotransInit( Mercator, &input_params, Geodetic, &output_params );
	conv_stat = bagGeotransConvert( Mercator, &input_coords, Geodetic, &output_coords );
	printf("stati(init,convert):  %#8.8x %#8.8x\n", init_stat, conv_stat);

	print_mercator( &input_coords, &input_params );
	print_geodetic( &output_coords, &output_params );

Get_Conversion_Status_String( Interactive, Input, ";", str ); printf("<%s>\n", str);
Get_Conversion_Status_String( Interactive, Output, ";", str ); printf("<%s>\n", str);

printf("\n***********************************************************************************************\n");
	printf("wgs72 - Geodetic <-> Transverse Mercator: ---------------------------------------------\n");

	input_params.datum = bagDatumID("WGS72");
	input_coords.longitude = -5;
	input_coords.latitude = -35;

	output_params.datum = bagDatumID("WGS72");
	output_params.origin_latitude = 0;
	output_params.central_meridian = 0;
	output_params.scale_factor = 1;
	output_params.false_easting = 0;
	output_params.false_northing = 0;

	init_stat = bagGeotransInit( Geodetic, &input_params, Transverse_Mercator, &output_params );
	conv_stat = bagGeotransConvert( Geodetic, &input_coords, Transverse_Mercator, &output_coords );
	printf("stati(init,convert):  %#8.8x %#8.8x\n", init_stat, conv_stat);

	print_geodetic( &input_coords, &input_params );
	print_tranmerc( &output_coords, &output_params );

Get_Conversion_Status_String( Interactive, Input, ";", str ); printf("<%s>\n", str);
Get_Conversion_Status_String( Interactive, Output, ";", str ); printf("<%s>\n", str);
printf("----------------------------------------------------------------------\n");

	input_params = output_params;
	input_coords = output_coords;

	init_stat = bagGeotransInit( Transverse_Mercator, &input_params, Geodetic, &output_params );
	conv_stat = bagGeotransConvert( Transverse_Mercator, &input_coords, Geodetic, &output_coords );
	printf("stati(init,convert):  %#8.8x %#8.8x\n", init_stat, conv_stat);

	print_tranmerc( &input_coords, &input_params );
	print_geodetic( &output_coords, &output_params );

Get_Conversion_Status_String( Interactive, Input, ";", str ); printf("<%s>\n", str);
Get_Conversion_Status_String( Interactive, Output, ";", str ); printf("<%s>\n", str);

printf("\n***********************************************************************************************\n");
	printf("wgs84 - Geodetic <-> UTM:  ---------------------------------------------\n");

	input_params.datum = bagDatumID("WGS84");
	/* example from mgrs.dat */
	input_coords.longitude = -79.86861111;
	input_coords.latitude = -2.238055555;
	input_coords.height = 0;

	output_params.datum = bagDatumID("WGS84");
	output_params.zone = 1;
	output_params.override = 0;

	init_stat = bagGeotransInit( Geodetic, &input_params, UTM, &output_params );
	conv_stat = bagGeotransConvert( Geodetic, &input_coords, UTM, &output_coords );
	printf("stati(init,convert):  %#8.8x %#8.8x\n", init_stat, conv_stat);

	print_geodetic( &input_coords, &input_params );
	print_utm( &output_coords, &output_params );

Get_Conversion_Status_String( Interactive, Input, ";", str ); printf("<%s>\n", str);
Get_Conversion_Status_String( Interactive, Output, ";", str ); printf("<%s>\n", str);
printf("----------------------------------------------------------------------\n");

	input_params = output_params;
	input_coords = output_coords;

	init_stat = bagGeotransInit( UTM, &input_params, Geodetic, &output_params );
	conv_stat = bagGeotransConvert( UTM, &input_coords, Geodetic, &output_coords );
	printf("stati(init,convert):  %#8.8x %#8.8x\n", init_stat, conv_stat);

	print_utm( &input_coords, &input_params );
	print_geodetic( &output_coords, &output_params );

Get_Conversion_Status_String( Interactive, Input, ";", str ); printf("<%s>\n", str);
Get_Conversion_Status_String( Interactive, Output, ";", str ); printf("<%s>\n", str);

printf("\n***********************************************************************************************\n");
	printf("nad83 - Geodetic <-> Polar Stereographic:  ---------------------------------------------\n");

	input_params.datum = bagDatumID("WGS84");
	input_coords.longitude = 85;
	input_coords.latitude = 15;
	input_coords.height = 0;

	output_params.datum = bagDatumID("WGS84");
	output_params.latitude_of_true_scale = 0;
	output_params.longitude_down_from_pole = 5;
	output_params.false_easting = 0;
	output_params.false_northing = 0;

	init_stat = bagGeotransInit( Geodetic, &input_params, Polar_Stereo, &output_params );
	conv_stat = bagGeotransConvert( Geodetic, &input_coords, Polar_Stereo, &output_coords );
	printf("stati(init,convert):  %#8.8x %#8.8x\n", init_stat, conv_stat);

	print_geodetic( &input_coords, &input_params );
	print_polarst( &output_coords, &output_params );

Get_Conversion_Status_String( Interactive, Input, ";", str ); printf("<%s>\n", str);
Get_Conversion_Status_String( Interactive, Output, ";", str ); printf("<%s>\n", str);
printf("----------------------------------------------------------------------\n");

	input_params = output_params;
	input_coords = output_coords;

	init_stat = bagGeotransInit( Polar_Stereo, &input_params, Geodetic, &output_params );
	conv_stat = bagGeotransConvert( Polar_Stereo, &input_coords, Geodetic, &output_coords );
	printf("stati(init,convert):  %#8.8x %#8.8x\n", init_stat, conv_stat);

	print_polarst( &input_coords, &input_params );
	print_geodetic( &output_coords, &output_params );

Get_Conversion_Status_String( Interactive, Input, ";", str ); printf("<%s>\n", str);
Get_Conversion_Status_String( Interactive, Output, ";", str ); printf("<%s>\n", str);

printf("\n***********************************************************************************************\n");
	printf("wgs84 - Geodetic <-> UPS Stereographic:  ---------------------------------------------\n");

	input_params.datum = bagDatumID("WGS84");
	input_coords.longitude = 60;
	input_coords.latitude = 87;
	input_coords.height = 0;

	output_params.datum = bagDatumID("WGS84");

	init_stat = bagGeotransInit( Geodetic, &input_params, UPS, &output_params );
	conv_stat = bagGeotransConvert( Geodetic, &input_coords, UPS, &output_coords );
	printf("stati(init,convert):  %#8.8x %#8.8x\n", init_stat, conv_stat);

	print_geodetic( &input_coords, &input_params );
	print_ups( &output_coords, &output_params );

Get_Conversion_Status_String( Interactive, Input, ";", str ); printf("<%s>\n", str);
Get_Conversion_Status_String( Interactive, Output, ";", str ); printf("<%s>\n", str);
printf("----------------------------------------------------------------------\n");

	input_params = output_params;
	input_coords = output_coords;

	init_stat = bagGeotransInit( UPS, &input_params, Geodetic, &output_params );
	conv_stat = bagGeotransConvert( UPS, &input_coords, Geodetic, &output_coords );
	printf("stati(init,convert):  %#8.8x %#8.8x\n", init_stat, conv_stat);

	print_ups( &input_coords, &input_params );
	print_geodetic( &output_coords, &output_params );

Get_Conversion_Status_String( Interactive, Input, ";", str ); printf("<%s>\n", str);
Get_Conversion_Status_String( Interactive, Output, ";", str ); printf("<%s>\n", str);

printf("\n***********************************************************************************************\n");
	printf("wgs84 - Geodetic <-> Lambert Conformal Conic:  ---------------------------------------------\n");

	input_params.datum = bagDatumID("WGS84");
	input_coords.longitude = -75;
	input_coords.latitude = 35;
	input_coords.height = 0;

	output_params.datum = bagDatumID("WGS84");
	output_params.origin_latitude = 23;
	output_params.central_meridian = -96;
	output_params.std_parallel_1 = 33;
	output_params.std_parallel_2 = 45;
	output_params.false_easting = 0;
	output_params.false_northing = 0;

	init_stat = bagGeotransInit( bagCoordsys("Geodetic"), &input_params, bagCoordsys("Lambert_Conformal_Conic"), &output_params );
	conv_stat = bagGeotransConvert( bagCoordsys("Geodetic"), &input_coords, bagCoordsys("Lambert_Conformal_Conic"), &output_coords );
	printf("stati(init,convert):  %#8.8x %#8.8x\n", init_stat, conv_stat);

	print_geodetic( &input_coords, &input_params );
	print_lambert( &output_coords, &output_params );

Get_Conversion_Status_String( Interactive, Input, ";", str ); printf("<%s>\n", str);
Get_Conversion_Status_String( Interactive, Output, ";", str ); printf("<%s>\n", str);
printf("----------------------------------------------------------------------\n");

	input_params = output_params;
	input_coords = output_coords;

	init_stat = bagGeotransInit( Lambert_Conformal_Conic, &input_params, Geodetic, &output_params );
	conv_stat = bagGeotransConvert( Lambert_Conformal_Conic, &input_coords, Geodetic, &output_coords );
	printf("stati(init,convert):  %#8.8x %#8.8x\n", init_stat, conv_stat);

	print_lambert( &input_coords, &input_params );
	print_geodetic( &output_coords, &output_params );

Get_Conversion_Status_String( Interactive, Input, ";", str ); printf("<%s>\n", str);
Get_Conversion_Status_String( Interactive, Output, ";", str ); printf("<%s>\n", str);

	return 0;
} /* main */
#endif
