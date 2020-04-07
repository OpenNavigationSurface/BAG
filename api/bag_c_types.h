#ifndef BAG_C_TYPES_H
#define BAG_C_TYPES_H

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

//! The bag error.
typedef uint32_t BagError;

//! The types of layers.
enum BAG_LAYER_TYPE
{
    Elevation = 0,  //!< The mandatory elevation layer.
    Uncertainty = 1,  //!< The mandatory uncertainty layer.
    Hypothesis_Strength = 2,  //!< The optional hypothesis strength layer.
	Num_Hypotheses = 3,  //!< The optional number of hypothesis layer.
	Shoal_Elevation = 4,  //!< The optional shoal elevation layer.
	Std_Dev	= 5,  //!< The optional standard deviation layer.
	Num_Soundings = 6,  //!< The optional number of soundings layer.
	Average_Elevation = 7,  //!< The optional average elevation layer.
	Nominal_Elevation = 8,  //!< The optional nominal elevation layer.
    Surface_Correction = 9,  //!< The optional surface correction layer.
    Compound = 10,  //!< The optional compound layer (there can be many; the name must be unique).
    VarRes_Metadata = 11,  //!< The optional variable resolution metadata layer.
    VarRes_Refinement = 12,  //!< The optional variable resolution refinement layer.
    VarRes_Node = 13,  //!< The optional variable resolution node layer.
    UNKNOWN_LAYER_TYPE,
};

//! Definitions for file open access modes
enum BAG_OPEN_MODE
{
    BAG_OPEN_READONLY   = 1,  //!< Open the BAG read only.  It cannot be written to.
    BAG_OPEN_READ_WRITE = 2,  //!< Open the BAG for reading and writing.
};

//! The types of data known to BAG.
enum BAG_DATA_TYPE
{
    DT_FLOAT32 = 0,  //!< A 32 bit floating point value.
    DT_UINT32,  //!< A 32 bit unsigned integer.
    DT_UINT8,  //!< An 8 bit unsigned integer.
    DT_UINT16,  //!< A 16 bit unsigned integer.
    DT_UINT64,  //!< A 64 bit unsigned integer.
    DT_BOOLEAN,  //!< A boolean value.
    DT_STRING,  //!< A string value.
    DT_COMPOUND,  //!< A compound type of one or more of the above.
    DT_UNKNOWN_DATA_TYPE,
};

//! The type of interleaved group.
enum BAG_GROUP_TYPE
{
    NODE = 0,  //!< The NODE group type.
    ELEVATION,  //!< The ELEVATION group type.
    UNKNOWN_GROUP_TYPE,  //!< Unknown group type.
};

//! An item in the Tracking List.
struct BagTrackingItem
{
    //! The row of the node of the BAG that was modified.
    uint32_t row;
    //! The column of the node of the BAG that was modified.
    uint32_t col;
    //! The original depth before this change.
    float depth;
    //! The original uncertainty before this change.
    float uncertainty;
    //! The reason code indicating why the modification was made.
    uint8_t track_code;
    //! The index number indicating the item in the metadata that describes the modifications.
    uint16_t list_series;
};

//! An item in the variable resolution tracking list.
struct BagVRTrackingItem
{
    //! The row of the node of the BAG that was modified.
    uint32_t row;
    //! The column of the node of the BAG that was modified.
    uint32_t col;
    //! The row within the refined grid that was modified.
    uint32_t sub_row;
    //! The column within the refined grid that was modified.
    uint32_t sub_col;
    //! The original depth before this change.
    float depth;
    //! The original uncertainty before this change.
    float uncertainty;
    //! The reason code indicating why the modification was made.
    uint8_t track_code;
    //! The index number indicating the item in the metadata that describes the modifications.
    uint16_t list_series;
};

//! An item in the variable resolution metadata layer.
struct BagVRMetadataItem
{
    //! The index.
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

//! An item in the variable resolution refinements layer.
struct BagVRRefinementsItem
{
    //! The depth.
    float depth;
    //! The uncertainty.
    float depth_uncrt;
};

//! An item in the variable resolution node layer.
struct BagVRNodeItem
{
    //! Hypotheses strength.
    float hyp_strength;
    //! Number of hypotheses.
    uint32_t num_hypotheses;
    //! Number of samples.
    uint32_t n_samples;
};

//! The surface topography.
enum BAG_SURFACE_CORRECTION_TOPOGRAPHY {
    BAG_SURFACE_UNKNOWN = 0,        //!< Unknown
    BAG_SURFACE_GRID_EXTENTS,       //!< Optional corrector dataset grid coordinates, spanning the required BAG surface extents
    BAG_SURFACE_IRREGULARLY_SPACED, //!< Irregularly spaced corrector values in optional corrector dataset
};

#define BAG_SURFACE_CORRECTOR_LIMIT 10  // The maximum number of datum correctors per BagVerticalDatumCorrections

//! An item in a surface corrections layer when the type is BAG_SURFACE_IRREGULARLY_SPACED.
struct BagVerticalDatumCorrections
{
    //! The X correction.
    double x;
    //! The Y correction.
    double y;
    //! The correctors.
    float z[BAG_SURFACE_CORRECTOR_LIMIT];
};

//! An item in a surface corrections layer when the type is BAG_SURFACE_GRID_EXTENTS.
struct BagVerticalDatumCorrectionsGridded
{
    //! The correctors.
    float z[BAG_SURFACE_CORRECTOR_LIMIT];
};

//! The attributes of a correction surface.  Only used in the C interface.
struct BagVerticalCorrectorDef
{
    double nodeSpacingX; //! node spacing in x dimension in units defined by coord system
    double nodeSpacingY; //! node spacing in y dimension in units defined by coord system
    double swCornerX;    //! X coordinate of SW corner of vertical corrector surface in BAG_COORDINATES
    double swCornerY;    //! Y coordinate of SW corner of vertical corrector surface in BAG_COORDINATES
};


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

#define BAG_NULL_ELEVATION      1000000  // A "null" elevation value.
#define BAG_NULL_GENERIC        1000000  // A "null" generic value.
#define BAG_NULL_UNCERTAINTY    1000000  // A "null" uncertainty value.

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  //BAG_C_TYPES_H

