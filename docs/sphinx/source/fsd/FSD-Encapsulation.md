# Encapsulation
BAG data are encoded as [HDF5](https://www.ogc.org/standard/HDF5/) files. HDF5 is a hierarchical data format product consisting of a data format specification and a supporting library implementation, it is also an OGC Community Standard. HDF5 provides two primary structures: groups and datasets. They are defined as:

* HDF5 Group: a grouping structure containing instances of zero or more groups or data sets, together with supporting metadata.

* HDF5 Dataset: a multidimensional array of data elements, together with supporting metadata or attributes.

An HDF5 “Group” provides the top-level structure for the data contents of a BAG. The major subcomponents are defined using the HDF5 “Dataset” types, and “Attribute” types.  Within each “Dataset”, further structural decomposition is specified via the DATATYPE and DATASPACE parameters. “Attributes” are included were appropriate to provide “Dataset” specific metadata. The specific HDF5 type definitions that define the BAG encapsulation structure are illustrated in Fig. 2.1. Note that the digital signature is not shown in Fig. 2.1. As described in [Structure of the BAG Certification Block](FSD-BAGStructure.md#structure-of-the-bag-certification-block), the digital signature byte stream is appended to the end of the HDF5 group.

Figure 2.1 shows the structure of BAG data encapsulated using HDF5:
```
$ h5dump -A examples/sample-data/sample.bag
HDF5 "examples/sample-data/sample.bag" {
GROUP "/" {
   GROUP "BAG_root" {
      ATTRIBUTE "Bag Version" {
         DATATYPE  H5T_STRING {
            STRSIZE 32;
            STRPAD H5T_STR_NULLTERM;
            CSET H5T_CSET_ASCII;
            CTYPE H5T_C_S1;
         }
         DATASPACE  SCALAR
         DATA {
         (0): "2.0.1"
         }
      }
      DATASET "elevation" {
         DATATYPE  H5T_IEEE_F32LE
         DATASPACE  SIMPLE { ( 100, 100 ) / ( 100, 100 ) }
         ATTRIBUTE "Maximum Elevation Value" {
            DATATYPE  H5T_IEEE_F32LE
            DATASPACE  SCALAR
            DATA {
            (0): 99.99
            }
         }
         ATTRIBUTE "Minimum Elevation Value" {
            DATATYPE  H5T_IEEE_F32LE
            DATASPACE  SCALAR
            DATA {
            (0): -10
            }
         }
      }
      DATASET "metadata" {
         DATATYPE  H5T_STRING {
            STRSIZE 1;
            STRPAD H5T_STR_NULLTERM;
            CSET H5T_CSET_ASCII;
            CTYPE H5T_C_S1;
         }
         DATASPACE  SIMPLE { ( 11005 ) / ( H5S_UNLIMITED ) }
      }
      DATASET "nominal_elevation" {
         DATATYPE  H5T_IEEE_F32LE
         DATASPACE  SIMPLE { ( 100, 100 ) / ( 100, 100 ) }
         ATTRIBUTE "max_value" {
            DATATYPE  H5T_IEEE_F32LE
            DATASPACE  SCALAR
            DATA {
            (0): 515.05
            }
         }
         ATTRIBUTE "min_value" {
            DATATYPE  H5T_IEEE_F32LE
            DATASPACE  SCALAR
            DATA {
            (0): 1
            }
         }
      }
      DATASET "tracking_list" {
         DATATYPE  H5T_COMPOUND {
            H5T_STD_U32LE "row";
            H5T_STD_U32LE "col";
            H5T_IEEE_F32LE "depth";
            H5T_IEEE_F32LE "uncertainty";
            H5T_STD_U8LE "track_code";
            H5T_STD_I16LE "list_series";
         }
         DATASPACE  SIMPLE { ( 0 ) / ( H5S_UNLIMITED ) }
         ATTRIBUTE "Tracking List Length" {
            DATATYPE  H5T_STD_U32LE
            DATASPACE  SCALAR
            DATA {
            (0): 0
            }
         }
      }
      DATASET "uncertainty" {
         DATATYPE  H5T_IEEE_F32LE
         DATASPACE  SIMPLE { ( 100, 100 ) / ( 100, 100 ) }
         ATTRIBUTE "Maximum Uncertainty Value" {
            DATATYPE  H5T_IEEE_F32LE
            DATASPACE  SCALAR
            DATA {
            (0): 100.01
            }
         }
         ATTRIBUTE "Minimum Uncertainty Value" {
            DATATYPE  H5T_IEEE_F32LE
            DATASPACE  SCALAR
            DATA {
            (0): 0
            }
         }
      }
      DATASET "vertical_datum_corrections" {
         DATATYPE  H5T_COMPOUND {
            H5T_IEEE_F64LE "x";
            H5T_IEEE_F64LE "y";
            H5T_ARRAY { [1][2] H5T_IEEE_F32LE } "z";
         }
         DATASPACE  SIMPLE { ( 3, 3 ) / ( H5S_UNLIMITED, H5S_UNLIMITED ) }
         ATTRIBUTE "surface_type" {
            DATATYPE  H5T_STD_U8LE
            DATASPACE  SCALAR
            DATA {
            (0): 2
            }
         }
         ATTRIBUTE "vertical_datum" {
            DATATYPE  H5T_STRING {
               STRSIZE 1;
               STRPAD H5T_STR_NULLTERM;
               CSET H5T_CSET_ASCII;
               CTYPE H5T_C_S1;
            }
            DATASPACE  SIMPLE { ( 0 ) / ( 256 ) }
            DATA {
            }
         }
      }
   }
}
}
```
**Figure 2.1: Structure of BAG data encapsulated using HDF5.**

Table 2.1 defines the contents of the HDF data elements belonging to the `BAG_root` Group.  

**Table 2.1: Contents of BAG_Root group**

| Entity Name        | Data Type | Domain                              | Required | 
|:-------------------|:----------|:------------------------------------| :-------- |
| BAG Version        | String    | Maximum 32 bytes available          | Yes |
| metadata           | Dataset   | Detailed in table 4                 | Yes |
| elevation          | Dataset   | Detailed in table 5                 | Yes |
| uncertainty        | Dataset   | Detailed in table 6                 | Yes |
| tracking list      | Dataset   | Detailed in table 7, and in table 8 | Yes |
| nominal_elevation  | Dataset   | Detailed in table 9                 | No |
| node               | Dataset | Detailed in table 10                | No |
| elevation_solution | Dataset | Detailed in table 11                | No |
 | georef_metadata    | Dataset | Detailed in table 12                | No |

Table 2.2 defines the metadata items used within the BAG library. These items must be present and properly defined for BAG data to be read. Note that this listing of metadata items does not specify the mandatory metadata items required by the ISO 19115 Standard.

**Table 2.2: Group level metadata - grid parameters.**

| Entity Name | XML Tag Nesting                                                                         | Data Type             | Domain |
| :------------- |:----------------------------------------------------------------------------------------|:----------------------| :-------- |
| **CoordSys** |                                                                                         |                       | |
| Coordinate System code | Reference System Info/projection/Identifier/code                                        | Non-Null String       | Geodetic |
| |                                                                                         |                       | GEOREF |
| |                                                                                         |                       | Geocentric |
| |                                                                                         |                       | Local_Cartesian |
| |                                                                                         |                       | MGRS |
| |                                                                                         |                       | UTM |
| |                                                                                         |                       | UPS |
| |                                                                                         |                       | Albers_Equal_Area_Conic |
| |                                                                                         |                       | Azimuthal_Equidistant |
| |                                                                                         |                       | BNG |
| |                                                                                         |                       | Bonne |
| |                                                                                         |                       | Cassini |
| |                                                                                         |                       | Cylindrical_Equal_Area |
| |                                                                                         |                       | Eckert4 |
| |                                                                                         |                       | Eckert6 |
| |                                                                                         |                       | Equidistant_Cylindrical |
| |                                                                                         |                       | Gnomonic |
| |                                                                                         |                       | Lambert_Conformal_Conic |
| |                                                                                         |                       | Mercator |
| |                                                                                         |                       | Miller_Cylindrical |
| |                                                                                         |                       | Mollweide |
| |                                                                                         |                       | Neys |
| |                                                                                         |                       | NZMG |
| |                                                                                         |                       | Oblique_Mercator |
| |                                                                                         |                       | Orthographic |
| |                                                                                         |                       | Polar_Stereo |
| |                                                                                         |                       | Polyconic |
| |                                                                                         |                       | Sinusoidal |
| |                                                                                         |                       | Stereographic |
| |                                                                                         |                       | Transverse_Cylindrical_Equal_Area |
| |                                                                                         |                       | Transverse_Mercator |
| |                                                                                         |                       | Van_der_Grinten |
| Zone | Reference System Info/projection Parameters/zone                                        | integer               | [-60,-1] U [1,60] |
| Standard Parallel | Reference System Info/ projection Parameters/ standard Parallel                         | Decimal Latitude      | 0 to 2 decimal numbers of range: [-90.0,+90.0] |
| Longitude Of Central Meridian | Reference System Info/ projection Parameters/ longitude Of Central Meridian             | Decimal Longitude     | range: [-180.0, +180.0] |
| Latitude Of Projection Origin | Reference System Info/ projection Parameters/ latitude Of Projection Origin             | Decimal Latitude      | range: [-90.0,+90.0] |
| False Easting | Reference System Info/ projection Parameters/ false Easting                             | Non-Negative Decimal  | [0.0, …), decimal is guaranteed at least 18 digits |
| False Northing | Reference System Info/ projection Parameters/ false Northing                            | Non-Negative Decimal  | [0.0, …), decimal is guaranteed at least 18 digits |
| False Easting Northing Units | Reference System Info/ projection Parameters/ false Easing Northing Units               | Unit Of Measure       | string |
| Scale Factor at Equator | Reference System Info/ projection Parameters/ scale Factor At Equator                   | Positive Decimal      | [0.0, …) |
| Height of Prospective Point Above Surface | Reference System Info/ projection Parameters/ height Of Prospective Point Above Surface | Positive Decimal      | [0.0, …) |
| Longitude of Projection Center | Reference System Info/ projection Parameters/ longitude Of Projection Center            | Decimal Longitude     | range: [-180.0, +180.0] |
| Latitude of Projection Center | Reference System Info/ projection Parameters/ latitude Of Projection Center             | Decimal Latitude      | range: [-90.0,+90.0] |
| Scale Factor at Center Line | Reference System Info/ projection Parameters/ scale Factor At Center Line               | Positive Decimal      | [0.0, …) |
| Straight Vertical Longitude from Pole | Reference System Info/ projection Parameters/ straight Vertical Longitude From Pole     | Decimal Longitude     | range: [-180.0, +180.0] |
| Scale Factor at Projection Origin | Reference System Info/ projection Parameters/ scale Factor At Projection Origin         | Positive Decimal      | [0.0, …) |
| Oblique Line Azimuth Parameter | Reference System Info/ projection Parameters/ oblique Line Azimuth Parameter            | Oblique Line Azimuth  | AzimuthAngle, azimuthMeasurePointLongitude |
| Oblique Line Point Parameter | Reference System Info/ projection Parameters/ oblique Line Point Parameter              | Oblique Line Point    | obliqueLineLatitude, obliqueLineLongitude |
| Semi-Major Axis | Reference System Info/Ellipsoid Parameters/ semi Major Axis                             | Positive Decimal      | [0.0, …) |
| Axis Units | Reference System Info/ Ellipsoid Parameters/ axis Units                                 | Unit Of Measure       | String |
| **Spatial Extent** |                                                                                         |                       | |            
| Horizontal Datum | Reference System Info/ datum/ Identifier/ code                                          | Non-Null String       | NAD83 – North American 1983 |
| |                                                                                         |                       | WGS72 – World Geodetic System 1972 |
| |                                                                                         |                       | WGS84 – World Geodetic System 1984 |
| Number of Dimensions | Spatial Representation Info/ number Of Dimensions                                       | Positive Integer      | [0,1,2,…) |
| Resolution per Spatial Dimension | Spatial Representation Info/ Dimension/ resolution/value                                | Decimal               | (0.0, 1.0e18) Guaranteed 18 digits with optional ‘.’, or leading signs, '+/-'. |
| Size per Dimension | Spatial Representation Info/ Dimension/ dimension Size                                  | nonnegative integer   | [0,1,2,...,2^16-1] |
| Corner Points | Spatial Representation Info/ corner Points/ Point/ coordinates                          | Coordinates           | 1 to 4 nodes of pointPropertyType decimal degrees or meters |
| West Bounding Longitude | Data Identification/ extent/ geographic Element/ west Bound Longitude                   | Approximate Longitude | [-180.00, 180.00], maximum 2 fractional digits |
| East Bounding Longitude | Data Identification/ extent/ geographic Element/ east Bound Longitude                   | Approximate Longitude | [-180.00, 180.00], maximum 2 fractional digits |
| South Bounding Latitude | Data Identification/ extent/ geographic Element/ south Bound Latitude                   | Approximate Latitude  | [-90.00, 90.00], maximum 2 fractional digits |
| North Bounding Latitude | Data Identification/ extent/ geographic Element/ north Bound Latitude                   | Approximate Latitude  | [-90.00, 90.00] , maximum 2 fractional digits |
| **Bag Metadata Extension** |                                                                                         |                       | |            
| Tracking List ID | Data Quality/Lineage/process Step/tracking Id                                           | Positive Integer      | Short (2byte) integer |
| Vertical Uncertainty Type | Data Identification/vertical Uncertainty Type                                           | Character String      | Unknown                  = 0, |
| |                                                                                         |                       | Raw_Std_Dev          = 1, |
| |                                                                                         |                       | CUBE_Std_Dev       = 2, |
| |                                                                                         |                       | Product_Uncert        = 3, |
| |                                                                                         |                       | Historical_Std_Dev  = 4 |
| Depth Correction Type | Data Identification/ depth Correction Type                                              | Character String      | Unknown                  = 0, |
| |                                                                                         |                       | Raw_Std_Dev          = 1, |
| |                                                                                         |                       | CUBE_Std_Dev       = 2, |
| |                                                                                         |                       | Product_Uncert        = 3, |
| |                                                                                         |                       | Historical_Std_Dev  = 4 |


**Table 2.3: Elevation dataset attributes.**

| Entity Name             | Data Type | Domain |
|:------------------------| :----------- | :-------- |
| elevation               | Float 32[][] | (FLT_MIN, FLT_MAX) | 
| Minimum Elevation Value | Float 32 | (FLT_MIN, FLT_MAX) |
| Maximum Elevation Value | Float 32 | (FLT_MIN, FLT_MAX) |


**Table 2.4: Uncertainty dataset attributes.**

| Entity Name               | Data Type | Domain |
|:--------------------------| :----------- | :-------- |
| uncertainty               | Float 32[][] | (FLT_MIN, FLT_MAX) | 
| Minimum Uncertainty Value | Float 32 | (FLT_MIN, FLT_MAX) |
| Maximum Uncertainty Value | Float 32 | (FLT_MIN, FLT_MAX) |


**Table 2.5: Tracking list dataset attributes.**

| Entity Name | Data Type | Domain |
| :------------- | :----------- | :-------- |
| Tracking List Item | Bag Tracking List Item | N/A |
| Tracking List Length | Unsigned Integer 32 | 0, 2^32-1] |


**Table 2.6: Definition of contents of the BAG tracking list item.**

| Entity Name | Data Type | Domain |
|:------------| :----------- | :-------- |
| row         | Unsigned Integer 32 | location of the node of the BAG that was modified |
| col         | Unsigned Integer 32 | location of the node of the BAG that was modified |
| depth       | Float 32 | original depth before this change |
| uncertainty | Float 32 | original uncertainty before this change |
| track_code  | Char | reason code indicating why the modification was made |
| list_series | Unsigned Integer 16 | index number indicating the item in the metadata that describes the modifications |


**Table 2.7: Nominal elevation dataset attributes.**

| Entity Name                     | Data Type | Domain |
|:--------------------------------| :-------- | :----- |
 | nominal_elevation               | Float 32[][] | (FLT_MIN, FLT_MAX) |
| Minimum Nominal Elevation Value | Float 32 | (FLT_MIN, FLT_MAX) |
| Maximum Nominal Elevation Value | Float 32 | (FLT_MIN, FLT_MAX) |


**Table 2.8: Node extension dataset attributes.**

| Entity Name | Data Type               | Domain |
| :---------- |:------------------------| :----- |
 | hyp_strength | Float 32[][]            | (FLT_MIN, FLT_MAX) |
| max_hyp_strength | Float 32                | (FLT_MIN, FLT_MAX) |
 | min_hyp_strength | Float 32                | (FLT_MIN, FLT_MAX) |
 | num_hypotheses | Unsigned Integer 32[][] | (UINT32_MIN, UINT32_MAX) |
 | max_num_hypotheses | Unsigned Integer 32     | (UINT32_MIN, UINT32_MAX) |
 | min_num_hypotheses | Unsigned Integer 32     | (UINT32_MIN, UINT32_MAX) |


**Table 2.9: Elevation solution extension dataset attributes.**

| Entity Name         | Data Type               | Domain |
|:--------------------|:------------------------| :----- |
 | shoal_elevation     | Float 32[][]            | (FLT_MIN, FLT_MAX) |
| max_shoal_elevation | Float 32                | (FLT_MIN, FLT_MAX) |
| min_shoal_elevation | Float 32                | (FLT_MIN, FLT_MAX) |
 | stddev              | Float 32[][]            | (FLT_MIN, FLT_MAX) |
| max_stddev          | Float 32                | (FLT_MIN, FLT_MAX) |
| min_stddev          | Float 32                | (FLT_MIN, FLT_MAX) |
 | num_soundings       | Unsigned Integer 32[][] | (UINT32_MIN, UINT32_MAX) |
| max_num_soundings   | Unsigned Integer 32 | (UINT32_MIN, UINT32_MAX) |
| min_num_soundings   | Unsigned Integer 32 | (UINT32_MIN, UINT32_MAX) |


**Table 2.10: Georef_metadata extension dataset attributes.**

| Entity Name    | Data Type  | Domain                                                                                                      |
|:---------------|:-----------|:------------------------------------------------------------------------------------------------------------|
 | <LAYER_NAME_1> | HDF5 group | HDF5 group whose name corresponds to the BAG layer this georeferenced metadata layer provides metadata for. |
 | ...            | ...        | ...                                                                                                         |
 | <LAYER_NAME_N> | HDF5 group | HDF5 group whose name corresponds to the BAG layer this georeferenced metadata layer provides metadata for. |


Each HDF5 group in `georef_metadata` must correspond to an existing BAG layer of the same name. Each `georef_metadata`
group must consist of two HDF5 compound datasets: (1) "keys"; and (2) "values". The "keys" dataset is described in
Table 2.11, and "values" described in Table 2.13.

**Table 2.11: Required and optional elements of Georef_metadata "keys" dataset.**

| Entity Name           | Data Type                        | Domain                               | Required |
|:----------------------|:---------------------------------|:-------------------------------------| :------- |
 | Record Definition     | Array of Record Definition Items | Definition Item detailed in table 12 | Yes |
 | Metadata Profile Type | Character String                 | 32 characters                        | No |


Metadata Profile Type string should be "Unknown metadata profile" unless the `georef_metadata` Record Definition is
of a known profile (e.g., "NOAA-OCS-2022.10"). See [Appendix A](FSD-Appendices.md) for a description of
known metadata profiles.

**Table 2.12: Georef_metadata Record Definition Item.**

| Entity Name          | Data Type                  | Domain                          |
|:---------------------|:---------------------------|:--------------------------------|
 | name                 | Character String           | The name of the metadata record |
 | type | Unsigned Integer 8 | [UINT8_MIN, UINT8_MAX]          |


**Table 2.13: Required elements of Georef_metadata "values" dataset.**

| Entity Name      | Data Type                                                       | Domain |
|:-----------------|:----------------------------------------------------------------|:------|
| <RECORD_VALUE_1> | Float 32 or Unsigned Integer 32 or Boolean or Character String  |       |
| ...              | ...                                                             | ...   |
 | <RECORD_VALUE_N> | Float 32 or Unsigned Integer 32 or Boolean or Character String  |       |


The raster values of a `georef_metadata` are interpreted to correspond to the entry of the "values" table containing
the metadata to be associated with one or more points in raster space. The `NoData` value is 0, hence the first entry
in the "values" table will always contain 0 or NULL values.

Figure 2.2 shows structure of BAG `georef_metadata` for an elevation layer encapsulated using HDF5.
```
$ h5dump -A examples/sample-data/bag_georefmetadata_layer.bag -g /BAG_root/georef_metadata examples/sample-data/bag_georefmetadata_layer.bag
HDF5 "examples/sample-data/bag_georefmetadata_layer.bag" {
GROUP "/BAG_root/georef_metadata" {
   GROUP "Elevation" {
      DATASET "keys" {
         DATATYPE  H5T_STD_U16LE
         DATASPACE  SIMPLE { ( 100, 100 ) / ( 100, 100 ) }
         DATA {
         (0,0): 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
         (0,20): 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
         (0,40): 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
         (0,60): 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
         (0,80): 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
         (1,0): 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
         (1,20): 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
         (1,40): 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
         (1,60): 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
         (1,80): 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
         ...
         (5,0): 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         (5,20): 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         (5,40): 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         (5,60): 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         (5,80): 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         (6,0): 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         (6,20): 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         (6,40): 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         (6,60): 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         (6,80): 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         ...
         (99,0): 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         (99,20): 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         (99,40): 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         (99,60): 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         (99,80): 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
         }
      }
      DATASET "values" {
         DATATYPE  H5T_COMPOUND {
            H5T_STD_U8LE "significant_features";
            H5T_STD_U8LE "feature_least_depth";
            H5T_IEEE_F32LE "feature_size";
            H5T_IEEE_F32LE "feature_size_var";
            H5T_STD_U8LE "coverage";
            H5T_STD_U8LE "bathy_coverage";
            H5T_IEEE_F32LE "horizontal_uncert_fixed";
            H5T_IEEE_F32LE "horizontal_uncert_var";
            H5T_STRING {
               STRSIZE H5T_VARIABLE;
               STRPAD H5T_STR_NULLTERM;
               CSET H5T_CSET_ASCII;
               CTYPE H5T_C_S1;
            } "survey_date_start";
            H5T_STRING {
               STRSIZE H5T_VARIABLE;
               STRPAD H5T_STR_NULLTERM;
               CSET H5T_CSET_ASCII;
               CTYPE H5T_C_S1;
            } "survey_date_end";
            H5T_STRING {
               STRSIZE H5T_VARIABLE;
               STRPAD H5T_STR_NULLTERM;
               CSET H5T_CSET_ASCII;
               CTYPE H5T_C_S1;
            } "source_institution";
            H5T_STRING {
               STRSIZE H5T_VARIABLE;
               STRPAD H5T_STR_NULLTERM;
               CSET H5T_CSET_ASCII;
               CTYPE H5T_C_S1;
            } "source_survey_id";
            H5T_STD_U32LE "source_survey_index";
            H5T_STRING {
               STRSIZE H5T_VARIABLE;
               STRPAD H5T_STR_NULLTERM;
               CSET H5T_CSET_ASCII;
               CTYPE H5T_C_S1;
            } "license_name";
            H5T_STRING {
               STRSIZE H5T_VARIABLE;
               STRPAD H5T_STR_NULLTERM;
               CSET H5T_CSET_ASCII;
               CTYPE H5T_C_S1;
            } "license_url";
         }
         DATASPACE  SIMPLE { ( 3 ) / ( 65535 ) }
         DATA {
         (0): {
               0,
               0,
               0,
               0,
               0,
               0,
               0,
               0,
               NULL,
               NULL,
               NULL,
               NULL,
               0,
               NULL,
               NULL
            },
         (1): {
               0,
               1,
               1234.57,
               765.432,
               1,
               0,
               9.87,
               1.23,
               "2019-04-01 00:00:00.0Z",
               "2019-04-01 12:00:00.0Z",
               "NOAA",
               "CD71EB77-5812-4735-B728-0DC1AE2A2F3B",
               42,
               "Creative Commons Zero Public Domain Dedication (CC0)",
               "https://creativecommons.org/publicdomain/zero/1.0/"
            },
         (2): {
               1,
               0,
               987.6,
               6.789,
               0,
               1,
               12345.7,
               89,
               "2019-04-02 00:00:00.0Z",
               "2019-04-02 12:00:00.0Z",
               "NOAA",
               "15B46F99-1D94-4669-92D8-AA86F533B097",
               23,
               "Open Data Commons Public Domain Dedication and Licence (PDDL)",
               "http://opendatacommons.org/licenses/pddl/1.0/"
            }
         }
      }
   }
}
}
```
**Figure 2.2: Structure of BAG Georef_metadata for an elevation layer encapsulated using HDF5. Note, `key` datatable is truncated in several places using ellipses ("...").**

## [Next: Axiomatic Definitions](FSD-AxiomaticDefs.md)