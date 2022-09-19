import unittest
import pathlib

import xmlrunner

from bagPy import *

import bagMetadataSamples, testUtils


# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "/../examples/sample-data"
chunkSize = 100
compressionLevel = 6


class TestSimpleLayer(unittest.TestCase):
    def testGetName(self):
        bagFileName = datapath + "/sample.bag"
        dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READONLY)
        self.assertIsNotNone(dataset)

        layer = dataset.getLayer(Elevation)
        self.assertIsNotNone(layer)
        descriptor = layer.getDescriptor()
        self.assertIsNotNone(descriptor)

        self.assertEqual(descriptor.getName(), getLayerTypeAsString(Elevation))

    def testRead(self):
        bagFileName = datapath + "/NAVO_data/JD211_public_Release_1-4_UTM.bag"
        dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READONLY)
        self.assertIsNotNone(dataset)

        kLayerType = Elevation
        elevLayer = dataset.getLayer(kLayerType)

        rowStart = 288
        rowEnd = 289
        columnStart = 249
        columnEnd = 251

        result = elevLayer.read(rowStart, columnStart, rowEnd, columnEnd) # 2x3
        self.assertIsNotNone(result)

        kExpectedNumNodes = 6

        buffer = result.asFloatItems()
        self.assertEqual(len(buffer), kExpectedNumNodes)

        kExpectedBuffer = (1000000.0, -52.161003, -52.172005,
            1000000.0, -52.177002, -52.174004)

        for actual, expected in zip(buffer, kExpectedBuffer):
            self.assertAlmostEqual(actual, expected, places=5)

    def testWrite(self):
        kLayerType = Elevation
        kExpectedNumNodes = 12
        kFloatValue = 123.456
        tmpFile = testUtils.RandomFileGuard("name")

        metadata = Metadata()
        metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)
        self.assertIsNotNone(metadata)

        # Create the dataset.
        dataset = Dataset.create(tmpFile.getName(), metadata, chunkSize, compressionLevel)
        self.assertIsNotNone(dataset)

        layer = dataset.getLayer(kLayerType)
        self.assertIsNotNone(layer)

        fileDims = dataset.getDescriptor().getDims()

        kExpectedRows = dataset.getMetadata().getStruct().spatialRepresentationInfo.numberOfRows
        kExpectedColumns = dataset.getMetadata().getStruct().spatialRepresentationInfo.numberOfColumns
        self.assertEqual(fileDims, (kExpectedRows, kExpectedColumns))

        # Open the dataset read/write and write to it.
        dataset = Dataset.openDataset(tmpFile.getName(), BAG_OPEN_READ_WRITE)
        self.assertIsNotNone(dataset)

        elevLayer = dataset.getLayer(kLayerType)
        self.assertIsNotNone(elevLayer)

        origBuffer = (kFloatValue,) * kExpectedNumNodes
        buffer = FloatLayerItems(origBuffer)

        # Write the floats to the elevation layer.
        elevLayer.write(1, 2, 3, 5, buffer)

        # Open the dataset and read what was written.
        dataset = Dataset.openDataset(tmpFile.getName(), BAG_OPEN_READONLY)
        self.assertIsNotNone(dataset)

        elevLayer = dataset.getLayer(kLayerType)
        self.assertIsNotNone(elevLayer)

        readBuffer = elevLayer.read(1, 2, 3, 5) # 3x4
        self.assertGreater(readBuffer.size(), 0)

        floatBuffer = readBuffer.asFloatItems()

        for actual, expected in zip(floatBuffer, origBuffer):
            self.assertAlmostEqual(actual, expected, places=5)

        del dataset #ensure dataset is deleted before tmpFile

    def testWriteAttributes(self):
        bagFileName = datapath + "/sample.bag"
        tmpFile = testUtils.RandomFileGuard("file", bagFileName)
        kLayerType = Elevation

        # Open the dataset read/write and write to it.
        dataset = Dataset.openDataset(tmpFile.getName(), BAG_OPEN_READ_WRITE)
        self.assertIsNotNone(dataset)
        elevLayer = dataset.getLayer(kLayerType)
        self.assertIsNotNone(elevLayer)
        descriptor = elevLayer.getDescriptor()
        self.assertIsNotNone(descriptor)

        # change min & max values
        originalMinMax = descriptor.getMinMax()
        kExpectedMin = originalMinMax[0] - 12.34
        kExpectedMax = originalMinMax[1] + 56.789
        descriptor.setMinMax(kExpectedMin, kExpectedMax)
        elevLayer.writeAttributes()

        # check that written values have changed
        dataset = Dataset.openDataset(tmpFile.getName(), BAG_OPEN_READONLY)
        self.assertIsNotNone(dataset)
        elevLayer = dataset.getLayer(kLayerType)
        descriptor = elevLayer.getDescriptor()
        actualMinMax = descriptor.getMinMax()
        self.assertAlmostEqual(actualMinMax[0], kExpectedMin, places=4)
        self.assertAlmostEqual(actualMinMax[1], kExpectedMax, places=4)

        del dataset #ensure dataset is deleted before tmpFile

    def testGetSimpleLayer(self):
        bagFileName = datapath + "/sample.bag"
        dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READONLY)
        self.assertIsNotNone(dataset)

        layer = dataset.getSimpleLayer(Elevation)
        self.assertIsNotNone(layer)
        descriptor = layer.getDescriptor()
        self.assertIsNotNone(descriptor)

        self.assertEqual(descriptor.getName(), getLayerTypeAsString(Elevation))


if __name__ == '__main__':
    unittest.main(
        testRunner=xmlrunner.XMLTestRunner(output='test-reports'),
        failfast=False, buffer=False, catchbreak=False
    )
