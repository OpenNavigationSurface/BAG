# Axiomatic Definitions

## Purpose

A number of components of the BAG file structure are fixed by default. This chapter defines these components, which are used implicitly and axiomatically throughout the file structure.

## Coordinate System Orientation and Geo-Referencing

All valid BAGs shall be represented with a right-handed Cartesian coordinate system. This system shall have the x-axis oriented towards positive eastings (for projected grids), or east (for geographic grids), and y-axis oriented towards positive northings (for projected grids), or north (for geographic grids). These definitions imply that the z-axis for the sounding data is positive away from the center of mass of the earth (i.e., is positive up), rather than the usual hydrographic convention of positive down (i.e., deeper depths are larger numbers and negative depths are above datum). User-level code is free to make this reflection if required, but must write the data using the positive-up convention. To make this distinction clear, the term “elevation” is used for the sounding component of the BAG, rather than “depth”. The uncertainty component of the BAG shall have the same coordinate system as the elevation component, with the exception that the z-axis is unipolar, and therefore the concept of direction of positive increase is irrelevant.

The geo-referencing for a BAG shall be node-based, referenced from the southwestern-most node in a grid. Each sample in a grid represents the value in the grid at a point location at the coordinate specified, rather than an estimate over any area with respect to the coordinate. The reference position included in the metadata shall be given in the coordinates used for the grid, and shall contain sufficient digits of precision to locate the grid with accuracy no worse than a millimeter on the surface of the ellipsoid of rotation of the chosen horizontal datum.

The grid data in a BAG (either elevation or uncertainty, and any other surfaces that may be added in time) shall be organized in row-major order from west to east, and south to north in the file. The first sample of the grid is the node at the southwest corner of the grid with location as specified by the geo-referencing parameters, the second is one grid resolution unit to the east of that position and at the same northing or latitude, and the third is two grid resolution units to the east and at the same northing or latitude. For *C* columns in the grid, the (*C*+1)th sample in the grid is located one grid resolution unit to the north, but on the same easting, or longitude, as the first sample in the grid.

If a datum transformation parameter group are provided in the metadata, the interpretation shall be EPSG 9606 (position vector).

## Units

The units used in all measurements are SI metric units, both in the data representation and in the metadata. Vertical measurements shall be in meters; for projected grids, the horizontal units shall be meters, and for geographic grids, the horizontal units shall be signed decimal degrees. For geographic grids, positive latitude shall be north of the equator, and positive longitude shall be degrees east of Greenwich, measured with respect to the ellipsoid of rotation associated with the horizontal datum declared in the metadata. User-level code shall ensure that geographic coordinates are appropriately mapped into the range [-180.0, 180.0]º for longitude, and [-90, 90]º for latitude.

The units of uncertainty shall be as defined by the metadata associated with the BAG. If the uncertainty can be interpreted as a variance, standard deviation or confidence interval on the elevation data, it shall be expressed as either meters or meters squared, as appropriate for the interpretation.

Units in the metadata shall follow the units used in the grids natively. The metadata shall contain sufficient information in the geo-referencing section to allow this distinction to be determined before the user-level code has to interpret any data in the grid.

Time shall be represented in seconds UTC with respect to the standard UNIX epoch of zero seconds, 1970-01-01/00:00:00.

## Resolution and Precision of Data

All data in the BAG grid, metadata and tracking list shall be represented as IEEE-754 <a href="FSD-References.html#ref7">[7]</a> floating point numbers without rounding or limitation of precision. Data that is fundamentally integer in nature (e.g., counts of elements) may be represented in integer format for compactness.  Grid data shall have single precision (32-bit) representation; metadata shall have at least single precision representation, but may be more if required. All software attempting to manipulate BAGs shall at least preserve the precision of the input data.

All positioning and geo-referencing data within a BAG shall have at least millimeter resolution. All elevation information shall have at least millimeter resolution, although this should not be taken to mean that the fundamental precision of the data is better than the associated uncertainty measurement. All times shall have at least millisecond resolution.

## [Next: BAG Architecture Review Board](FSD-BAGARB.md)
