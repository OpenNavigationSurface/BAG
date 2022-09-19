# Encapsulation
The BAG structure utilizes HDF-5.  HDF-5 is a hierarchical data format product consisting of a data format specification and a supporting library implementation.  HDF-5 files are organized in a hierarchical structure, with two primary structures; groups and datasets.  They are defined as:

* HDF-5 Group: a grouping structure containing instances of zero or more groups or data sets, together with supporting metadata.

* HDF-5 Data set: a multidimensional array of data elements, together with supporting metadata or attributes.

An HDF-5 “Group” provides the top-level structure for the data contents of a BAG.  The major subcomponents are defined using the HDF-5 “Dataset” types, and “Attribute” types.  Within each “Dataset”, further structural decomposition is specified via the DATATYPE and DATASPACE parameters.  “Attributes” are included were appropriate to provide “Dataset” specific metadata.    Following the high level BAG file structure described in Fig. 3, the specific HDF-5 type definitions that define the BAG encapsulation structure are illustrated in Fig. 8.  Note that the digital signature is not shown in Fig. 8.  As described in Section 2, the digital signature byte stream is appended to the end of the HDF-5 group.

Output of `h5dump -A examples/sample-data/sample-2.0.1.bag`:
```
HDF5 "examples/sample-data/sample-2.0.1.bag" {
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
            (0): 100.99
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

**Figure 8: Structure of BAG data encapsulated using HDF-5.**

Table 3 defines the contents of the HDF data elements belonging to the BAG_root Group.  

| Entity Name | Data Type | Domain                              | Required |
| :------------- |:----------|:------------------------------------| :-------- |
| BAG Version | String    | Maximum 32 bytes available          | Yes |
| metadata | Array     | Detailed in table 4                 | Yes |
| elevation | Array     | Detailed in table 5                 | Yes |
| uncertainty | Array     | Detailed in table 6                 | Yes |
| tracking list | Table     | Detailed in table 7, and in table 8 | Yes |
| nominal_elevation | Array     | Detailed in table 9                 | No |
 | vertical_datum_corrections | Table | Detaild in Table 10 | No |

**Table 3: Contents of BAG_Root group**

Table 4 defines the metadata items used with in the BAG library.  These items must be present and properly defined for BAG I/O operations to succeed.  Note that this listing of metadata items does not specify the mandatory metadata items required by the ISO 19915 standard.

| Entity Name | XML Tag Nesting | Data Type | Domain |
| :------------- | :------------------- | :----------- | :-------- |
| **CoordSys** | | | |
| Coordinate System code | Reference System Info/projection/Identifier/code |  Non Null String | Geodetic |
| | | | GEOREF |
| | | | Geocentric |
| | | | Local_Cartesian |
| | | | MGRS |
| | | | UTM |
| | | | UPS |
| | | | Albers_Equal_Area_Conic |
| | | | Azimuthal_Equidistant |
| | | | BNG |
| | | | Bonne |
| | | | Cassini |
| | | | Cylindrical_Equal_Area |
| | | | Eckert4 |
| | | | Eckert6 |
| | | | Equidistant_Cylindrical |
| | | | Gnomonic |
| | | | Lambert_Conformal_Conic |
| | | | Mercator |
| | | | Miller_Cylindrical |
| | | | Mollweide |
| | | | Neys |
| | | | NZMG |
| | | | Oblique_Mercator |
| | | | Orthographic |
| | | | Polar_Stereo |
| | | | Polyconic |
| | | | Sinusoidal |
| | | | Stereographic |
| | | | Transverse_Cylindrical_Equal_Area |
| | | | Transverse_Mercator |
| | | | Van_der_Grinten |
| Zone | Reference System Info/projection Parameters/zone | integer | [-60,-1] U [1,60] |
| Standard Parallel | Reference System Info/ projection Parameters/ standard Parallel | Decimal Latitude | 0 to 2 decimal numbers of range: [-90.0,+90.0] |
| Longitude Of Central Meridian | Reference System Info/ projection Parameters/ longitude Of Central Meridian | Decimal Longitude | range: [-180.0, +180.0] |
| Latitude Of Projection Origin | Reference System Info/ projection Parameters/ latitude Of Projection Origin | Decimal Latitude | range: [-90.0,+90.0] |
| False Easting | Reference System Info/ projection Parameters/ false Easting | Non Negative Decimal | [0.0, …), decimal is guaranteed at least 18 digits |
| False Northing | Reference System Info/ projection Parameters/ false Northing | Non Negative Decimal | [0.0, …), decimal is guaranteed at least 18 digits |
| False Easting Northing Units | Reference System Info/ projection Parameters/ false Easing Northing Units | Unit Of Measure | string |
| Scale Factor at Equator | Reference System Info/ projection Parameters/ scale Factor At Equator | Positive Decimal | [0.0, …) |
| Height of Prospective Point Above Surface | Reference System Info/ projection Parameters/ height Of Prospective Point Above Surface | Positive Decimal | [0.0, …) |
| Longitude of Projection Center | Reference System Info/ projection Parameters/ longitude Of Projection Center | Decimal Longitude | range: [-180.0, +180.0] |
| Latitude of Projection Center | Reference System Info/ projection Parameters/ latitude Of Projection Center | Decimal Latitude | range: [-90.0,+90.0] |
| Scale Factor at Center Line | Reference System Info/ projection Parameters/ scale Factor At Center Line | Positive Decimal | [0.0, …) |
| Straight Vertical Longitude from Pole | Reference System Info/ projection Parameters/ straight Vertical Longitude From Pole | Decimal Longitude | range: [-180.0, +180.0] |
| Scale Factor at Projection Origin | Reference System Info/ projection Parameters/ scale Factor At Projection Origin | Positive Decimal | [0.0, …) |
| Oblique Line Azimuth Parameter | Reference System Info/ projection Parameters/ oblique Line Azimuth Parameter | Oblique Line Azimuth | AzimuthAngle, azimuthMeasurePointLongitude |
| Oblique Line Point Parameter | Reference System Info/ projection Parameters/ oblique Line Point Parameter | Oblique Line Point | obliqueLineLatitude, obliqueLineLongitude |
| Semi-Major Axis | Reference System Info/ Ellipsoid Parameters/ semi Major Axis | Positive Decimal | [0.0, …) |
| Axis Units | Reference System Info/ Ellipsoid Parameters/ axis Units | Unit Of Measure | String |
| **Spatial Extent** | | | |            
| Horizontal Datum | Reference System Info/ datum/ Identifier/ code | Non Null String | NAD83 – North American 1983 |
| | | | WGS72 – World Geodetic System 1972 |
| | | | WGS84 – World Geodetic System 1984 |
| Number of Dimensions | Spatial Representation Info/ number Of Dimensions | Positive Integer | [0,1,2,…) |
| Resolution per Spatial Dimension | Spatial Representation Info/ Dimension/ resolution/value | Decimal | (0.0, 1.0e18) Guaranteed 18 digits with optional ‘.’, or leading signs, '+/-'. |
| Size per Dimension | Spatial Representation Info/ Dimension/ dimension Size | nonnegative integer | [0,1,2,...,2^16-1] |
| Corner Points | Spatial Representation Info/ corner Points/ Point/ coordinates | Coordinates | 1 to 4 nodes of pointPropertyType decimal degrees or meters |
| West Bounding Longitude | Data Identification/ extent/ geographic Element/ west Bound Longitude | Approximate Longitude | [-180.00, 180.00], maximum 2 fractional digits |
| East Bounding Longitude | Data Identification/ extent/ geographic Element/ east Bound Longitude | Approximate Longitude | [-180.00, 180.00], maximum 2 fractional digits |
| South Bounding Latitude | Data Identification/ extent/ geographic Element/ south Bound Latitude | Approximate Latitude | [-90.00, 90.00], maximum 2 fractional digits |
| North Bounding Latitude | Data Identification/ extent/ geographic Element/ north Bound Latitude | Approximate Latitude | [-90.00, 90.00] , maximum 2 fractional digits |
| **Bag Metadata Extension** | | | |            
| Tracking List ID | Data Quality/Lineage/process Step/tracking Id | Positive Integer | Short (2byte) integer |
| Vertical Uncertainty Type | Data Identification/vertical Uncertainty Type | Character String | Unknown                  = 0, |
| | | | Raw_Std_Dev          = 1, |
| | | | CUBE_Std_Dev       = 2, |
| | | | Product_Uncert        = 3, |
| | | | Historical_Std_Dev  = 4 |
| Depth Correction Type | Data Identification/ depth Correction Type | Character String | Unknown                  = 0, |
| | | | Raw_Std_Dev          = 1, |
| | | | CUBE_Std_Dev       = 2, |
| | | | Product_Uncert        = 3, |
| | | | Historical_Std_Dev  = 4 |

**Table 4: Group level metadata - grid parameters.**

| Entity Name | Data Type | Domain |
| :------------- | :----------- | :-------- |
| Elevation | Float 32[][] | (FLT_MIN, FLT_MAX) | 
| Minimum Elevation Value | Float 32 | (FLT_MIN, FLT_MAX) |
| Maximum Elevation Value | Float 32 | (FLT_MIN, FLT_MAX) |

**Table 5: Elevation dataset attributes.**

| Entity Name | Data Type | Domain |
| :------------- | :----------- | :-------- |
| Uncertainty | Float 32[][] | (FLT_MIN, FLT_MAX) | 
| Minimum Uncertainty Value | Float 32 | (FLT_MIN, FLT_MAX) |
| Maximum Uncertainty Value | Float 32 | (FLT_MIN, FLT_MAX) |

**Table 6: Uncertainty dataset attributes.**

| Entity Name | Data Type | Domain |
| :------------- | :----------- | :-------- |
| Tracking List Item | Bag Tracking List Item | N/A |
| Tracking List Length | Unsigned Integer 32 | 0, 2^32-1] |

**Table 7: Tracking list dataset attributes.**

| Entity Name | Data Type | Domain |
| :------------- | :----------- | :-------- |
| Row | Unsigned Integer 32 | location of the node of the BAG that was modified |
| Col | Unsigned Integer 32 | location of the node of the BAG that was modified |
| Depth | Float 32 | original depth before this change |
| Uncertainty | Float 32 | original uncertainty before this change |
| track_code | Char | reason code indicating why the modification was made |
| list_series | Unsigned Integer 16 | index number indicating the item in the metadata that describes the modifications |

**Table 8: Definition of contents of the BAG tracking list item.**

## [Next: Axiomatic Definitions](FSD-AxiomaticDefs.md)