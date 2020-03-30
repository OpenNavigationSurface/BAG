#ifndef BAG_C_TYPES_H
#define BAG_C_TYPES_H

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

#define BAG_NULL_ELEVATION      1000000
#define BAG_NULL_GENERIC        1000000
#define BAG_NULL_UNCERTAINTY    1000000

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
    VarRes_Metadata = 11,
    VarRes_Refinement = 12,
    VarRes_Node = 13,
    UNKNOWN_LAYER_TYPE,
};

/* Definitions for file open access modes */
enum BAG_OPEN_MODE
{
    BAG_OPEN_READONLY   = 1,
    BAG_OPEN_READ_WRITE = 2,
};

enum BAG_DATA_TYPE
{
    DT_FLOAT32 = 0,
    DT_UINT32,
    DT_UINT8,
    DT_UINT16,
    DT_UINT64,
    DT_BOOLEAN,
    DT_STRING,
    DT_COMPOUND,
    DT_UNKNOWN_DATA_TYPE,
};

//! The type of interleaved group.
enum BAG_GROUP_TYPE
{
    NODE = 0,
    ELEVATION,
    UNKNOWN_GROUP_TYPE,
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

/* VR tracking list structure */
struct BagVRTrackingItem
{
    //! location of the node of the BAG that was modified
    uint32_t row;
    uint32_t col;
    //! location within the refined grid that was modified
    uint32_t sub_row;
    uint32_t sub_col;
    //! original depth before this change
    float depth;
    //! original uncertainty before this change
    float uncertainty;
    //! reason code indicating why the modification was made
    uint8_t track_code;
    //! index number indicating the item in the metadata that describes the modifications
    uint16_t list_series;
};

//! The type found in the variable resolution metadata layer.
struct BagVRMetadataItem
{
    uint32_t index;
    //! Number of nodes in easting
    uint32_t dimensions_x;
    //! Number of nodes in northing
    uint32_t dimensions_y;
    //! Node spacing in easting
    float resolution_x;
    //! Node spacing in northing
    float resolution_y;
    //! Offset east from SW corner of surrounding low-res cell to SW-most node
    float sw_corner_x;
    //! Offset north from SW corner of surrounding low-res cell to SW-most node
    float sw_corner_y;
};

//! The type found in the variable resolution refinement layer.
struct BagVRRefinementItem
{
    float depth;
    float depth_uncrt;
};

//! The type found in the variable resolution node layer.
struct BagVRNodeItem
{
    float hyp_strength;
    uint32_t num_hypotheses;
    uint32_t n_samples;
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

struct BagVerticalCorrectorDef
{
    double nodeSpacingX; /* node spacing in x dimension in units defined by coord system */
    double nodeSpacingY; /* node spacing in y dimension in units defined by coord system */
    double swCornerX;    /* X coordinate of SW corner of vertical corrector surface in BAG_COORDINATES */
    double swCornerY;    /* Y coordinate of SW corner of vertical corrector surface in BAG_COORDINATES */
};

#define BAG_NULL_ELEVATION 1000000

//! A structure to hold the definition of a value.
//! ***NOTE:  This must be POD per the C++11 definition.
//! see:  https://stackoverflow.com/questions/4178175/what-are-aggregates-and-pods-and-how-why-are-they-special/7189821#7189821
struct FieldDefinition
{
    //! The name of the definition.
    const char* name;
    //! The type of the definition; represents a DataType.
    uint8_t type;
};

//! The C version of the BAG::CompoundDataType class
struct BagCompoundDataType
{
    BAG_DATA_TYPE type;
    union Data {
        float f;
        uint32_t ui32;
        bool b;
        char* c;
    } data;
};

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  //BAG_C_TYPES_H

