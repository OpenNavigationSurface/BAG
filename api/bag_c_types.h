#ifndef BAG_C_TYPES_H
#define BAG_C_TYPES_H

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t BagError;

enum BAG_LAYER_TYPE
{
    Elevation = 0,
    Uncertainty = 1,
    Hypothesis_Strength = 2,
	Num_Hypotheses = 3,
	Shoal_Elevation = 4,
	Std_Dev	= 5,
	Num_Soundings = 6,
	Average_Elevation = 7,
	Nominal_Elevation = 8,
    Surface_Correction = 9,
    Compound = 10,
    UNKNOWN_LAYER_TYPE
};

/* Definitions for file open access modes */
enum BAG_OPEN_MODE
{
    BAG_OPEN_READONLY   = 1,
    BAG_OPEN_READ_WRITE = 2
};

enum BAG_DATA_TYPE
{
    FLOAT32 = 0,
    UINT32,
    BOOL,
    STRING,
    COMPOUND,  // More than a single type; the layer will need to figure it out itself.
    UNKNOWN_DATA_TYPE
};

//! The type of interleaved group.
enum BAG_GROUP_TYPE
{
    NODE = 0,
    ELEVATION,
    UNKNOWN_GROUP_TYPE
};

/* tracking list structure */
struct BagTrackingItem
{
    //! location of the node of the BAG that was modified
    uint32_t row;
    uint32_t col;
    //! original depth before this change
    float depth;
    //! original uncertainty before this change
    float uncertainty;
    //! reason code indicating why the modification was made
    uint8_t track_code;
    //! index number indicating the item in the metadata that describes the modifications
    uint16_t list_series;
};

enum BAG_SURFACE_CORRECTION_TOPOGRAPHY {
    BAG_SURFACE_UNKNOWN = 0,        /* Unknown */
    BAG_SURFACE_GRID_EXTENTS,       /* Optional corrector dataset grid coordinates, spanning the required BAG surface extents */
    BAG_SURFACE_IRREGULARLY_SPACED, /* Irregularly spaced corrector values in optional corrector dataset */
};

#define BAG_SURFACE_CORRECTOR_LIMIT 10  // The maximum number of datum correctors per BagVerticalDatumCorrections

struct BagVerticalDatumCorrections
{
    double x;
    double y;
    float z[BAG_SURFACE_CORRECTOR_LIMIT];
};

struct BagVerticalDatumCorrectionsGridded
{
    float z[BAG_SURFACE_CORRECTOR_LIMIT];
};

#define BAG_NULL_ELEVATION 1000000

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  //BAG_C_TYPES_H

