import unittest
import pathlib

import xmlrunner

from bagPy import *

import bagMetadataSamples, testUtils


# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "/../examples/sample-data"
chunkSize = 100
compressionLevel = 6


class TestTrackingList(unittest.TestCase):
    def testConstructOpen(self):
        tmpFile = testUtils.RandomFileGuard("name")
        metadata = Metadata()
        metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)
        self.assertIsNotNone(metadata)

        # Create the dataset.
        dataset = Dataset.create(tmpFile.getName(), metadata,
                                 chunkSize, compressionLevel)
        self.assertIsNotNone(dataset)

        trackingList = dataset.getTrackingList()

        # Add 1 record and save.  Is there one record with expected values?
        kExpectedItem0 = BagTrackingItem(1, 2, 3.4, 5.6, 7, 8) #createTrackingItem
        trackingList.push_back(kExpectedItem0)
        self.assertEqual(trackingList.size(), 1)

        kExpectedItem1 = BagTrackingItem(11, 22, 33.44, 55.66, 77, 88) #createTrackingItem
        trackingList.push_back(kExpectedItem1)
        self.assertEqual(trackingList.size(), 2)

        trackingList.write()

        dataset = Dataset.openDataset(tmpFile.getName(), BAG_OPEN_READONLY)
        self.assertIsNotNone(dataset)

        trackingList = dataset.getTrackingList()
        kExpectedNumItems = 2
        self.assertEqual(trackingList.size(), kExpectedNumItems)

        del dataset #ensure dataset is deleted before tmpFile

    def testConstructCreate(self):
        tmpFile = testUtils.RandomFileGuard("name")

        # Create the dataset.
        dataset = Dataset.create(tmpFile.getName(), Metadata(),
                                 chunkSize, compressionLevel)
        self.assertIsNotNone(dataset)

        trackingList = dataset.getTrackingList()
        self.assertEqual(trackingList.size(), 0)

        # Add 1 record and save.  Is there one record with expected values?
        kExpectedItem0 = BagTrackingItem(1, 2, 3.4, 5.6, 7, 8) #createTrackingItem
        trackingList.push_back(kExpectedItem0)
        self.assertEqual(trackingList.size(), 1)

        trackingList.write()

        trackingList = dataset.getTrackingList()
        self.assertEqual(trackingList.size(), 1)

        item0 = trackingList.front()
        self.assertEqual(kExpectedItem0.row, item0.row)
        self.assertEqual(kExpectedItem0.col, item0.col)
        self.assertEqual(kExpectedItem0.depth, item0.depth)
        self.assertEqual(kExpectedItem0.uncertainty, item0.uncertainty)
        self.assertEqual(kExpectedItem0.track_code, item0.track_code)
        self.assertEqual(kExpectedItem0.list_series, item0.list_series)

        kExpectedItem1 = BagTrackingItem(9, 8, 7.6, 5.4, 3, 2)
        trackingList.push_back(kExpectedItem1)
        self.assertEqual(trackingList.size(), 2)

        trackingList.write()

        item0 = trackingList.at(0)
        self.assertEqual(kExpectedItem0.row, item0.row)
        self.assertEqual(kExpectedItem0.col, item0.col)
        self.assertEqual(kExpectedItem0.depth, item0.depth)
        self.assertEqual(kExpectedItem0.uncertainty, item0.uncertainty)
        self.assertEqual(kExpectedItem0.track_code, item0.track_code)
        self.assertEqual(kExpectedItem0.list_series, item0.list_series)

        item1 = trackingList.at(1)
        self.assertEqual(kExpectedItem1.row, item1.row)
        self.assertEqual(kExpectedItem1.col, item1.col)
        self.assertEqual(kExpectedItem1.depth, item1.depth)
        self.assertEqual(kExpectedItem1.uncertainty, item1.uncertainty)
        self.assertEqual(kExpectedItem1.track_code, item1.track_code)
        self.assertEqual(kExpectedItem1.list_series, item1.list_series)

        del dataset #ensure dataset is deleted before tmpFile


if __name__ == '__main__':
    unittest.main(
        testRunner=xmlrunner.XMLTestRunner(output='test-reports'),
        failfast=False, buffer=False, catchbreak=False
    )
