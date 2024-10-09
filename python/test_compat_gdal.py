import unittest
from pathlib import Path
from typing import Union, Tuple
import os
import logging
import tempfile
import shutil

import xmlrunner
from osgeo import gdal, gdalconst
import numpy as np

import bagPy as BAG
import bagMetadataSamples

logger = logging.getLogger(__file__)


def cmp_bag_compound_rectype_to_gdal_rat_fieldtype(bag_type: int, gdal_type: int) -> bool:
    """
    Map BAG compound data types to GDAL RAT field types
    :param bag_type:
    :param gdal_type:
    :return: True if bag_type maps to gdal_type
    """
    if bag_type == BAG.DT_FLOAT32:
        return gdal_type == gdalconst.GFT_Real
    elif bag_type >= BAG.DT_UINT32 and bag_type <= BAG.DT_BOOLEAN:
        return gdal_type == gdalconst.GFT_Integer
    elif bag_type == BAG.DT_STRING:
        return gdal_type == gdalconst.GFT_String

    logger.warning((f"Unable to map BAG compound record type '{bag_type}' to "
                    f"GDAL RAT field type '{gdal_type}'."))
    return False


def get_bag_compound_data_type_field_value(bag_field: BAG.CompoundDataType) -> Union[int, float, str, bool, None]:
    field_type = bag_field.getType()
    if field_type == BAG.DT_FLOAT32:
        return bag_field.asFloat()
    elif field_type >= BAG.DT_UINT32 and field_type <= BAG.DT_UINT64:
        return bag_field.asUInt32()
    elif field_type == BAG.DT_BOOLEAN:
        return bag_field.asBool()
    elif field_type == BAG.DT_STRING:
        return bag_field.asString()

    logger.warning((f"Unable to get value for BAG compound record type `{field_type}`."))

    return None


def get_gdal_rat_field_value(gdal_rat: gdal.RasterAttributeTable,
                             row: int, col: int) -> Union[int, float, str]:
    # Get type of column
    gdal_type: int = gdal_rat.GetTypeOfCol(col)
    if gdal_type == gdalconst.GFT_Real:
        return gdal_rat.GetValueAsDouble(row, col)
    elif gdal_type == gdalconst.GFT_Integer:
        return gdal_rat.GetValueAsInt(row, col)
    else:
        return gdal_rat.GetValueAsString(row, col)


def cmp_bag_gdal_raster_rows_cols(bag_descriptor: BAG.Descriptor, gdal_band_arr: np.ndarray) -> bool:
    bag_rows, bag_cols = bag_descriptor.getDims()
    gdal_rows, gdal_cols = gdal_band_arr.shape
    return (bag_rows == gdal_rows) and (bag_cols == gdal_cols)


def cmp_bag_gdal_raster_res(bag_descriptor: BAG.Descriptor,
                            gdal_pixel_width: float,
                            gdal_pixel_height: float) -> bool:
    bag_grid_spacing_x, bag_grid_spacing_y = bag_descriptor.getGridSpacing()
    return (bag_grid_spacing_x == gdal_pixel_width) and (bag_grid_spacing_y == gdal_pixel_height)


def get_gdal_resolution(gdal_geo_transform: Tuple) -> Tuple[float, float]:
    """

    :param gdal_geo_transform:
    :return: Tuple of (gdal_pixel_width, abs(gdal_pixel_height)
    """
    gdal_pixel_width = gdal_geo_transform[1]
    # Since GDAL's origin is the upper left (NW) corner, the pixel height is
    # expressed as a negative number, so we convert to an absolute value
    gdal_pixel_height = abs(gdal_geo_transform[5])
    return gdal_pixel_width, gdal_pixel_height


def cmp_bag_gdal_origin_coord(bag_descriptor: BAG.Descriptor, gdal_geo_transform: Tuple, ) -> bool:
    """
    Make sure GDAL origin coordinates match the projected coverage of the BAG
    :param bag_descriptor:
    :param gdal_geo_transform:
    :return:
    """
    # Note: GDAL's origin is at the upper left, while the bag coverage is
    # defined by lower left and upper right corners. So we need use the Y value of
    # upper right corner of the BAG coverage
    gdal_top_left_x = gdal_geo_transform[0]
    gdal_top_left_y = gdal_geo_transform[3]
    bag_proj_cover = bag_descriptor.getProjectedCover()
    bag_lower_left_x = bag_proj_cover[0][0]
    bag_upper_right_y = bag_proj_cover[1][1]

    # Note: Make a half-pixel correction to account for BAG cells being the
    # center of pixels, while the GDAL origin is the upper left corner of
    # the upper left pixel
    gdal_pixel_width, gdal_pixel_height = get_gdal_resolution(gdal_geo_transform)
    # Upper left corner of upper left pixel is less east than center of pixel, so subtract
    #   1/2 the GDAL pixel width (which should always be >0, so no need to take absolute value)
    bag_lower_left_x = bag_lower_left_x - (gdal_pixel_width / 2)
    # Upper left corner of upper left pixel is more north of center, so add the absolute value
    #   of 1/2 the GDAL pixel height
    bag_upper_right_y = bag_upper_right_y + (gdal_pixel_height / 2)

    return (gdal_top_left_x == bag_lower_left_x) and (gdal_top_left_y == bag_upper_right_y)


def get_bag_layer_as_array(bag_descriptor: BAG.Descriptor, bag_layer: BAG.Layer) -> np.ndarray:
    """
    Someday we may be able to do zero-copy exports of BAG data to numpy arrays, until then...
    :param bag_descriptor:
    :param bag_layer:
    :return:
    """
    bag_dtype = bag_layer.getDescriptor().getDataType()
    bag_rows, bag_cols = bag_descriptor.getDims()
    bag_layer_items = bag_layer.read(0, 0, bag_rows-1, bag_cols-1)
    if bag_dtype == BAG.DT_FLOAT32:
        bag_layer_data = bag_layer_items.asFloatItems()
        np_dtype = np.float32
    elif bag_dtype == BAG.DT_UINT32:
        bag_layer_data = bag_layer_items.asUInt32Items()
        np_dtype = np.uint32
    elif bag_dtype == BAG.DT_UINT16:
        bag_layer_data = bag_layer_items.asUInt16Items()
        np_dtype = np.uint16
    else:
        # Assume UInt8
        bag_layer_data = bag_layer_items.asUInt8Items()
        np_dtype = np.uint8

    return np.reshape(np.array(bag_layer_data, dtype=np_dtype), (bag_rows, bag_cols))


def get_gdal_band_as_array(gdal_band: gdal.Band) -> np.ndarray:
    """
    Note: The origin of BAG files is the lower-left corner, while GDAL uses the upper-left
    corner as the origin, so we need to flip the GDAL data about the Y axis (rows)
    :param gdal_band:
    :return:
    """
    return np.flip(gdal_band.ReadAsArray(), axis=0)


def get_bag_path(bag_path: Path) -> str:
    if os.name == 'nt' and bag_path.drive != '':
        # On Windows we strip the drive letter from the path because having it there
        # causes GDAL to be unable to open subdatasets of the BAG using "BAG:$FILENAME:georef_metadata:Elevation"
        # syntax
        return str(Path('/', *bag_path.parts[1:]))
    else:
        return str(bag_path)


class TestCompatGDAL(unittest.TestCase):
    def setUp(self) -> None:
        self.datapath: Path = Path(Path(__file__).parent.parent, 'examples', 'sample-data')

        self.tmp_dir = tempfile.mkdtemp()
        logger.info(f"tmp_dir: {self.tmp_dir}")

    def tearDown(self) -> None:
        shutil.rmtree(self.tmp_dir)

    def test_simple_layer(self) -> None:
        bag_filename = get_bag_path(Path(self.datapath, 'sample.bag'))

        # Open in BAG library
        bd = BAG.Dataset.openDataset(bag_filename, BAG.BAG_OPEN_READONLY)
        self.assertIsNotNone(bd)
        bag_elev = bd.getLayer(BAG.Elevation)
        self.assertIsNotNone(bag_elev)

        # Open in GDAL
        gd = gdal.Open(bag_filename, gdal.GA_ReadOnly)
        self.assertIsNotNone(gd)
        self.assertEqual('BAG', gd.GetDriver().ShortName)
        gdal_elev = gd.GetRasterBand(1)
        self.assertIsNotNone(gdal_elev)

        # Make sure rows and columns are the same in BAG and GDAL representations
        bag_descriptor = bd.getDescriptor()
        gdal_array = get_gdal_band_as_array(gdal_elev)
        self.assertTrue(cmp_bag_gdal_raster_rows_cols(bag_descriptor, gdal_array))

        # Compare BAG and GDAL resolutions are the same
        gdal_gt = gd.GetGeoTransform()
        gdal_pixel_width, gdal_pixel_height = get_gdal_resolution(gdal_gt)
        self.assertTrue(cmp_bag_gdal_raster_res(bag_descriptor, gdal_pixel_width, gdal_pixel_height))

        # Make sure GDAL origin coordinates match the projected coverage of the BAG
        self.assertTrue(cmp_bag_gdal_origin_coord(bag_descriptor, gdal_gt))

        # Make sure raster data types are the same
        bag_elev_desc = bag_elev.getDescriptor()
        self.assertEqual(BAG.DT_FLOAT32, bag_elev_desc.getDataType())
        self.assertEqual(gdalconst.GDT_Float32, gdal_elev.DataType)

        # Get BAG data as a 2D numpy array
        bag_array = get_bag_layer_as_array(bag_descriptor, bag_elev)

        # Compare bag_array to gdal_array to make sure all data are identical
        self.assertTrue(np.array_equiv(bag_array, gdal_array))

    def test_georefmetadata_layer(self) -> None:
        """
        This test is disabled until GDAL is updated to read georeferenced metadata layer from the HDF5 file from
        `/georef_metadata` instead of `/Georef_metadata`.
        :return:
        """
        bag_filename = get_bag_path(Path(self.datapath, 'bag_georefmetadata_layer.bag'))

        # Open in BAG library
        bd = BAG.Dataset.openDataset(bag_filename, BAG.BAG_OPEN_READONLY)
        self.assertIsNotNone(bd)

        bag_georef_elev: BAG.GeorefMetadataLayer = bd.getGeorefMetadataLayer("Elevation")
        self.assertIsNotNone(bag_georef_elev)
        # Verify metadata profile
        bag_georef_elev_desc: BAG.GeorefMetadataLayerDescriptor = bag_georef_elev.getDescriptor()
        self.assertEqual(BAG.NOAA_OCS_2022_10_METADATA_PROFILE, bag_georef_elev_desc.getProfile())

        # Open in GDAL
        gd = gdal.Open(bag_filename, gdal.GA_ReadOnly)
        self.assertEqual('BAG', gd.GetDriver().ShortName)

        # Okay, we can open the main file. Let's open the georeferenced metadata layer subdataset
        compound_layer_gdal_name = f"BAG:{bag_filename}:georef_metadata:Elevation"
        gdal_georef_elev = gdal.Open(compound_layer_gdal_name)
        self.assertIsNotNone(gdal_georef_elev)
        # GDAL doesn't know about ATTRIBUTE "Metadata Profile Type" under DATASET "keys"
        #   of georef_metadata sub-groups, so don't look for it, but we want to make sure
        #   their presence doesn't trip up GDAL

        # Get BAG georef_metadata record definition from the layer descriptor
        bag_rec_def = bag_georef_elev_desc.getDefinition()
        bag_num_cols = len(bag_rec_def)

        # Get GDAL representation of the georef metadata table, which is a raster attribute table
        gdal_band = gdal_georef_elev.GetRasterBand(1)
        gdal_rat = gdal_band.GetDefaultRAT()
        gdal_num_cols = gdal_rat.GetColumnCount()

        # Make sure there are the same number of georef_metadata table columns
        self.assertEqual(bag_num_cols, gdal_num_cols)

        # Compare names and types between BAG record definition and GDAL RAT columns
        for i in range(gdal_num_cols):
            self.assertEqual(bag_rec_def[i].name, gdal_rat.GetNameOfCol(i))
            self.assertTrue(cmp_bag_compound_rectype_to_gdal_rat_fieldtype(bag_rec_def[i].type,
                                                                           gdal_rat.GetTypeOfCol(i)))

        # Compare BAG georef_metadata value table contents to GDAL RAT rows
        bag_val_table = bag_georef_elev.getValueTable()
        bag_records = bag_val_table.getRecords()
        bag_num_rows = len(bag_records)
        gdal_num_rows = gdal_rat.GetRowCount()
        self.assertEqual(bag_num_rows, gdal_num_rows)

        for row in range(bag_num_rows):
            for col in range(bag_num_cols):
                bag_field: BAG.CompoundDataType = bag_records[row][col]
                bag_field_val = get_bag_compound_data_type_field_value(bag_field)
                gdal_field_val = get_gdal_rat_field_value(gdal_rat, row, col)
                self.assertEqual(bag_field_val, gdal_field_val)

        # Make sure rows and columns are the same in BAG and GDAL representations
        bag_descriptor = bd.getDescriptor()
        gdal_array = get_gdal_band_as_array(gdal_band)
        self.assertTrue(cmp_bag_gdal_raster_rows_cols(bag_descriptor, gdal_array))

        # Compare BAG and GDAL resolutions are the same
        gdal_gt = gd.GetGeoTransform()
        gdal_pixel_width, gdal_pixel_height = get_gdal_resolution(gdal_gt)
        self.assertTrue(cmp_bag_gdal_raster_res(bag_descriptor, gdal_pixel_width, gdal_pixel_height))

        # Make sure GDAL origin coordinates match the projected coverage of the BAG
        self.assertTrue(cmp_bag_gdal_origin_coord(bag_descriptor, gdal_gt))

        # Make sure raster data types are the same
        self.assertEqual(BAG.DT_UINT16, bag_georef_elev_desc.getDataType())
        self.assertEqual(gdalconst.GDT_UInt16, gdal_band.DataType)

        # Get BAG data as a 2D numpy array
        bag_array = get_bag_layer_as_array(bag_descriptor, bag_georef_elev)

        # Compare bag_array to gdal_array to make sure all data are identical
        self.assertTrue(np.array_equiv(bag_array, gdal_array))

    def test_gdal_create_simple(self):
        bag_filename = get_bag_path(Path(self.tmp_dir, 'created_by_gdal.bag'))
        bag_rows = 100
        bag_cols = 100
        gd = gdal.GetDriverByName('BAG').Create(bag_filename,
                                                xsize=bag_cols, ysize=bag_rows, bands=2, eType=gdalconst.GDT_Float32)
        self.assertIsNotNone(gd)
        gd.SetGeoTransform([687905.0, 10.0, 0.0, 5555615.0, 0.0, -10.0])
        gd.SetProjection(bagMetadataSamples.kBAG_CRS_WKT)
        # Write elevation data
        elev_array = np.random.default_rng(12345).random((bag_rows, bag_cols)) * 100
        gdal_elev = gd.GetRasterBand(1)
        gdal_elev.WriteArray(elev_array)
        # Write uncertainty data
        uncrt_array = np.random.default_rng(54321).random((bag_rows, bag_cols))
        gdal_uncrt = gd.GetRasterBand(2)
        gdal_uncrt.WriteArray(uncrt_array)
        # Close GDAL dataset
        gd.Close()

        # Re-open in GDAL, but read-only
        gd = gdal.Open(bag_filename, gdal.GA_ReadOnly)
        self.assertIsNotNone(gd)
        self.assertEqual('BAG', gd.GetDriver().ShortName)
        gdal_elev = gd.GetRasterBand(1)
        self.assertIsNotNone(gdal_elev)
        gdal_uncrt = gd.GetRasterBand(2)
        self.assertIsNotNone(gdal_uncrt)
        gd.Close()

        # Open BAG with BAG library
        bd = BAG.Dataset.openDataset(bag_filename, BAG.BAG_OPEN_READONLY)
        self.assertIsNotNone(bd)
        bag_elev = bd.getLayer(BAG.Elevation)
        self.assertIsNotNone(bag_elev)
        bag_uncert = bd.getLayer(BAG.Uncertainty)
        self.assertIsNotNone(bag_uncert)
        bag_descriptor = bd.getDescriptor()
        self.assertIsNotNone(bag_descriptor)
        bd.close()

        # Check BAG version
        self.assertEqual('1.6.2', bag_descriptor.getVersion())

        # Make sure rows and columns are the same in BAG and GDAL representations
        #   Note: re-fetch array and flip it so that subsequent comparisons to BAG data succeed
        gdal_elev_array = get_gdal_band_as_array(gdal_elev)
        self.assertTrue(cmp_bag_gdal_raster_rows_cols(bag_descriptor, gdal_elev_array))

        # Compare BAG and GDAL resolutions are the same
        gdal_gt = gd.GetGeoTransform()
        gdal_pixel_width, gdal_pixel_height = get_gdal_resolution(gdal_gt)
        self.assertTrue(cmp_bag_gdal_raster_res(bag_descriptor, gdal_pixel_width, gdal_pixel_height))

        # Make sure GDAL origin coordinates match the projected coverage of the BAG
        self.assertTrue(cmp_bag_gdal_origin_coord(bag_descriptor, gdal_gt))

        # Make sure raster data types are the same
        bag_elev_desc = bag_elev.getDescriptor()
        self.assertEqual(BAG.DT_FLOAT32, bag_elev_desc.getDataType())
        self.assertEqual(gdalconst.GDT_Float32, gdal_elev.DataType)

        # Get BAG elevation data as a 2D numpy array
        bag_elev_array = get_bag_layer_as_array(bag_descriptor, bag_elev)

        # Compare BAG elevation array to GDAL elevation array to make sure all data are identical
        self.assertTrue(np.array_equiv(bag_elev_array, gdal_elev_array))

        # Get BAG uncertainty data as a 2D numpy array
        bag_uncrt_array = get_bag_layer_as_array(bag_descriptor, bag_uncert)

        # Compare BAG uncertainty array to GDAL uncertainty array to make sure all data are identical
        gdal_uncrt_array = get_gdal_band_as_array(gdal_uncrt)
        self.assertTrue(np.array_equiv(bag_uncrt_array, gdal_uncrt_array))


if __name__ == '__main__':
    unittest.main(
        testRunner=xmlrunner.XMLTestRunner(output='test-reports'),
        failfast=False, buffer=False, catchbreak=False
    )
