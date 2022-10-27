import unittest
from pathlib import Path
from typing import Union
import logging

import xmlrunner
from osgeo import gdal, gdalconst

import bagPy as BAG

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


# define constants used in multiple tests
datapath: Path = Path(Path(__file__).parent.parent, 'examples', 'sample-data')


class TestCompatGDAL(unittest.TestCase):
    def setUp(self) -> None:
        pass

    def tearDown(self) -> None:
        pass

    def test_compound_layer(self) -> None:
        bag_filename = str(Path(datapath, 'bag_compound_layer.bag'))

        # Open in BAG library
        bd = BAG.Dataset.openDataset(bag_filename, BAG.BAG_OPEN_READONLY)
        self.assertIsNotNone(bd)

        cl: BAG.CompoundLayer = bd.getCompoundLayer("Elevation")
        self.assertIsNotNone(cl)
        # Verfiy metadata profile
        cl_desc: BAG.CompoundLayerDescriptor = cl.getDescriptor()
        self.assertEqual(BAG.NOAA_NBS_2022_06_METADATA_PROFILE, cl_desc.getProfile())

        # Open in GDAL
        gd = gdal.Open(bag_filename, gdal.GA_ReadOnly)
        self.assertEqual('BAG', gd.GetDriver().ShortName)

        # Okay, we can open the main file. Let's open the compound layer subdataset
        compound_layer_gdal_name = f"BAG:{bag_filename}:georef_metadata:Elevation"
        gd_cl = gdal.Open(compound_layer_gdal_name)
        self.assertIsNotNone(gd_cl)
        # GDAL doesn't know about ATTRIBUTE "Metadata Profile Type" under DATASET "keys"
        #   of Georef_metadata sub-groups, so don't look for it, but we want to make sure
        #   their presence doesn't trip up GDAL

        # Get BAG Georef_metadata record definition from the layer descriptor
        bag_rec_def = cl_desc.getDefinition()
        bag_num_cols = len(bag_rec_def)

        # Get GDAL representation of the georef metadata table, which is a raster attribute table
        gdal_band = gd_cl.GetRasterBand(1)
        gdal_rat = gdal_band.GetDefaultRAT()
        gdal_num_cols = gdal_rat.GetColumnCount()

        # Make sure there are the same number of Georef_metadata table columns
        self.assertEqual(bag_num_cols, gdal_num_cols)

        # Compare names and types between BAG record definition and GDAL RAT columns
        for i in range(gdal_num_cols):
            self.assertEqual(bag_rec_def[i].name, gdal_rat.GetNameOfCol(i))
            self.assertTrue(cmp_bag_compound_rectype_to_gdal_rat_fieldtype(bag_rec_def[i].type,
                                                                           gdal_rat.GetTypeOfCol(i)))

        # Compare BAG Georef_metadata value table contents to GDAL RAT rows
        vt = cl.getValueTable()
        bag_records = vt.getRecords()
        bag_num_rows = len(bag_records)
        gdal_num_rows = gdal_rat.GetRowCount()
        self.assertEqual(bag_num_rows, gdal_num_rows)

        for row in range(bag_num_rows):
            for col in range(bag_num_cols):
                bag_field: BAG.CompoundDataType = bag_records[row][col]
                bag_field_val = get_bag_compound_data_type_field_value(bag_field)
                gdal_field_val = get_gdal_rat_field_value(gdal_rat, row, col)
                self.assertEqual(bag_field_val, gdal_field_val)

        # TODO: Read some random values from Georef_data raster and compare to
        #   values in BAG


if __name__ == '__main__':
    unittest.main(
        testRunner=xmlrunner.XMLTestRunner(output='test-reports'),
        failfast=False, buffer=False, catchbreak=False
    )
