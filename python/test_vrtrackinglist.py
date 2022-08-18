import unittest
import pathlib

import xmlrunner

from bagPy import *

import bagMetadataSamples, testUtils


# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "/../examples/sample-data"
chunkSize = 100
compressionLevel = 6


class TestVRTrackingList(unittest.TestCase):
    def testCreateOpen(self):
        tmpBagFile = testUtils.RandomFileGuard("name")

        metadata = Metadata()

        metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)

        dataset = Dataset.create(tmpBagFile.getName(), metadata,
                                 chunkSize, compressionLevel)
        self.assertIsNotNone(dataset)

        # Check no VR tracking list exists.
        vrTrackingList = dataset.getVRTrackingList()
        self.assertIsNone(vrTrackingList)

        # Check VR tracking list was created successfully.
        dataset.createVR(chunkSize, compressionLevel, False)
        vrTrackingList = dataset.getVRTrackingList()
        self.assertIsNotNone(vrTrackingList)

        # Check no items are in the tracking list after creation.
        self.assertTrue(vrTrackingList.empty())

        # Add 1 record.
        kExpectedItem0 = BagVRTrackingItem(1, 2, 11, 22, 3.4, 5.6, 7, 8)

        vrTrackingList.push_back(kExpectedItem0)

        # Check one item was stored.
        self.assertFalse(vrTrackingList.empty())
        self.assertEqual(vrTrackingList.size(), 1)

        vrTrackingList.write()

        # Read the expected record
        vrTrackingList1 = dataset.getVRTrackingList()
        self.assertIsNotNone(vrTrackingList1)

        # Check one item was read.
        self.assertEqual(vrTrackingList1.size(), 1)

        # Check the expected values are in the item.
        actual = vrTrackingList1.front()
        self.assertEqual(kExpectedItem0.row, actual.row)
        self.assertEqual(kExpectedItem0.col, actual.col)
        self.assertEqual(kExpectedItem0.sub_row, actual.sub_row)
        self.assertEqual(kExpectedItem0.sub_col, actual.sub_col)
        self.assertAlmostEqual(kExpectedItem0.depth, actual.depth, places=5)
        self.assertAlmostEqual(kExpectedItem0.uncertainty, actual.uncertainty, places=5)
        self.assertEqual(kExpectedItem0.track_code, actual.track_code)
        self.assertEqual(kExpectedItem0.list_series, actual.list_series)

        # Add a second record to an existing vrTrackingList.
        vrTrackingList1 = dataset.getVRTrackingList()
        self.assertIsNotNone(vrTrackingList1)

        # Check one item was read.
        self.assertEqual(vrTrackingList1.size(), 1)

        kExpectedItem1 = BagVRTrackingItem(9, 8, 101, 202, 7.6, 5.4, 3, 2)

        vrTrackingList1.push_back(kExpectedItem1)
        self.assertEqual(vrTrackingList1.size(), 2)

        vrTrackingList1.write()

        vrTrackingList2 = dataset.getVRTrackingList()

        # Check the expected values are in the item.
        self.assertEqual(vrTrackingList2.size(), 2)

        # Check the first expected values are exactly the same.
        actual = vrTrackingList2.at(0)
        self.assertEqual(kExpectedItem0.row, actual.row)
        self.assertEqual(kExpectedItem0.col, actual.col)
        self.assertEqual(kExpectedItem0.sub_row, actual.sub_row)
        self.assertEqual(kExpectedItem0.sub_col, actual.sub_col)
        self.assertAlmostEqual(kExpectedItem0.depth, actual.depth, places=5)
        self.assertAlmostEqual(kExpectedItem0.uncertainty, actual.uncertainty, places=5)
        self.assertEqual(kExpectedItem0.track_code, actual.track_code)
        self.assertEqual(kExpectedItem0.list_series, actual.list_series)

        # Check the second expected values are exactly the same.
        actual = vrTrackingList2.at(1)
        self.assertEqual(kExpectedItem1.row, actual.row)
        self.assertEqual(kExpectedItem1.col, actual.col)
        self.assertEqual(kExpectedItem1.sub_row, actual.sub_row)
        self.assertEqual(kExpectedItem1.sub_col, actual.sub_col)
        self.assertAlmostEqual(kExpectedItem1.depth, actual.depth, places=5)
        self.assertAlmostEqual(kExpectedItem1.uncertainty, actual.uncertainty, places=5)
        self.assertEqual(kExpectedItem1.track_code, actual.track_code)
        self.assertEqual(kExpectedItem1.list_series, actual.list_series)

        # Force a close
        del dataset

        # Read the BAG.
        dataset = Dataset.openDataset(tmpBagFile.getName(), BAG_OPEN_READONLY)

        # Check dataset was read successfully.
        self.assertIsNotNone(dataset)

        vrTrackingList3 = dataset.getVRTrackingList()

        # Check the expected number of items in the tracking list are there.
        kExpectedNumItems = 2
        self.assertEqual(vrTrackingList3.size(), kExpectedNumItems)

        # Check the first expected values are exactly the same.
        actual = vrTrackingList3.at(0)
        self.assertEqual(kExpectedItem0.row, actual.row)
        self.assertEqual(kExpectedItem0.col, actual.col)
        self.assertEqual(kExpectedItem0.sub_row, actual.sub_row)
        self.assertEqual(kExpectedItem0.sub_col, actual.sub_col)
        self.assertAlmostEqual(kExpectedItem0.depth, actual.depth, places=5)
        self.assertAlmostEqual(kExpectedItem0.uncertainty, actual.uncertainty, places=5)
        self.assertEqual(kExpectedItem0.track_code, actual.track_code)
        self.assertEqual(kExpectedItem0.list_series, actual.list_series)

        # Check the second expected values are exactly the same.
        actual = vrTrackingList3.at(1)
        self.assertEqual(kExpectedItem1.row, actual.row)
        self.assertEqual(kExpectedItem1.col, actual.col)
        self.assertEqual(kExpectedItem1.sub_row, actual.sub_row)
        self.assertEqual(kExpectedItem1.sub_col, actual.sub_col)
        self.assertAlmostEqual(kExpectedItem1.depth, actual.depth, places=5)
        self.assertAlmostEqual(kExpectedItem1.uncertainty, actual.uncertainty, places=5)
        self.assertEqual(kExpectedItem1.track_code, actual.track_code)
        self.assertEqual(kExpectedItem1.list_series, actual.list_series)

        # Force a close
        del dataset


if __name__ == '__main__':
    unittest.main(
        testRunner=xmlrunner.XMLTestRunner(output='test-reports'),
        failfast=False, buffer=False, catchbreak=False
    )
