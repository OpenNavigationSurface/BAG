import unittest
import pathlib

import xmlrunner

from bagPy import *

import bagMetadataSamples, testUtils


# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "/../examples/sample-data"
chunkSize = 100
compressionLevel = 6


class TestDataset(unittest.TestCase):
    def testReadOnly(self):
        bagFileName = datapath + "/sample.bag"

        dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READONLY)
        self.assertIsNotNone(dataset)

        kNumExpectedLayers = 4
        numLayerTypes = len(dataset.getLayerTypes())

        self.assertEqual(numLayerTypes, kNumExpectedLayers)

    def testReadWrite(self):
        bagFileName = datapath + "/sample.bag"
        tmpFile = testUtils.RandomFileGuard("file", bagFileName)

        dataset = Dataset.openDataset(tmpFile.getName(), BAG_OPEN_READ_WRITE)
        self.assertIsNotNone(dataset)

        kNumExpectedLayers = 4
        numLayerTypes = len(dataset.getLayerTypes())
        self.assertEqual(numLayerTypes, kNumExpectedLayers)

        simpleLayer = dataset.createSimpleLayer(Average_Elevation, chunkSize, compressionLevel)
        self.assertIsNotNone(simpleLayer)

        numLayerTypes2 = len(dataset.getLayerTypes())
        self.assertEqual(numLayerTypes2, (numLayerTypes + 1))

        del dataset #ensure dataset is deleted before tmpFile

    def testCreation(self):
        tmpFile = testUtils.RandomFileGuard("name")
        metadata = Metadata()
        metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)

        dataset = Dataset.create(tmpFile.getName(), metadata, chunkSize, compressionLevel)
        self.assertIsNotNone(dataset)

        numLayerTypes = len(dataset.getLayerTypes())
        kNumExpectedLayers = 2
        self.assertEqual(numLayerTypes, kNumExpectedLayers)

        dataset = Dataset.openDataset(tmpFile.getName(), BAG_OPEN_READONLY)
        self.assertIsNotNone(dataset)

        numLayerTypes = len(dataset.getLayerTypes())
        self.assertEqual(numLayerTypes, kNumExpectedLayers)

        del dataset #ensure dataset is deleted before tmpFile

    def testGetLayerTypes(self):
        bagFileName = datapath + "/NAVO_data/JD211_Public_Release_1-5.bag"
        dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READONLY)
        self.assertIsNotNone(dataset)

        numLayerTypes = len(dataset.getLayerTypes())
        self.assertEqual(numLayerTypes, 2)

    def testAddLayer(self):
        bagFileName = datapath + "/sample.bag"
        tmpFile = testUtils.RandomFileGuard("file", bagFileName)

        dataset = Dataset.openDataset(tmpFile.getName(), BAG_OPEN_READ_WRITE)
        self.assertIsNotNone(dataset)

        kNumExpectedLayers = 4
        numLayerTypes = len(dataset.getLayerTypes())
        self.assertEqual(numLayerTypes, kNumExpectedLayers)

        simpleLayer = dataset.createSimpleLayer(Average_Elevation, chunkSize, compressionLevel)
        self.assertIsNotNone(simpleLayer)

        numLayerTypes = len(dataset.getLayerTypes())
        self.assertEqual(numLayerTypes, (kNumExpectedLayers + 1))

        del dataset #ensure dataset is deleted before tmpFile

    def testGetLayer(self):
        bagFileName = datapath + "/sample.bag"
        dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READ_WRITE)
        self.assertIsNotNone(dataset)

        kNumExpectedLayers = 4
        layers = dataset.getLayers()
        numLayers = len(layers)
        self.assertEqual(numLayers, kNumExpectedLayers)

        for layer in layers:
            layerFromId = dataset.getLayer(layer.getDescriptor().getId())
            self.assertEqual(layerFromId.getDescriptor().getInternalPath(),
                             layer.getDescriptor().getInternalPath())

    def testGetTrackingList(self):
        bagFileName = datapath + "/sample.bag"
        dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READ_WRITE)
        self.assertIsNotNone(dataset)

        trackingList = dataset.getTrackingList()
        self.assertIsNotNone(trackingList)
        self.assertTrue(trackingList.empty())
        self.assertEqual(trackingList.size(), 0)

    def testGetMetadata(self):
        bagFileName = datapath + "/sample.bag"
        dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READ_WRITE)
        self.assertIsNotNone(dataset)

        metadata = dataset.getMetadata()
        self.assertIsNotNone(metadata)

        kExpectedLLcornerX = 687910.0
        kExpectedLLcornerY = 5554620.0
        self.assertEqual(metadata.llCornerX(), kExpectedLLcornerX)
        self.assertEqual(metadata.llCornerY(), kExpectedLLcornerY)

    def testGridToGeo(self):
        bagFileName = datapath + "/sample.bag"
        dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READ_WRITE)
        self.assertIsNotNone(dataset)

        x = 0.0
        y = 0.0
        xy = dataset.gridToGeo(0, 0)
        self.assertEqual(xy[0], 687910.0)
        self.assertEqual(xy[1], 5554620.0)

    def testGeoToGrid(self):
        bagFileName = datapath + "/sample.bag"
        dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READ_WRITE)
        self.assertIsNotNone(dataset)

        row = 123
        column = 456
        rc = dataset.geoToGrid(687910.0, 5554620.0)
        self.assertEqual(rc[0], 0)
        self.assertEqual(rc[1], 0)

    def testGetDescriptor(self):
        bagFileName = datapath + "/sample.bag"
        dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READ_WRITE)
        self.assertIsNotNone(dataset)

        descriptor = dataset.getDescriptor()
        self.assertIsNotNone(descriptor)
        self.assertEqual(descriptor.isReadOnly(), False)


if __name__ == '__main__':
    unittest.main(
        testRunner=xmlrunner.XMLTestRunner(output='test-reports'),
        failfast=False, buffer=False, catchbreak=False
    )
