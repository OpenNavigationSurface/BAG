import unittest
import pathlib

import xmlrunner

from bagPy import *

import bagMetadataSamples, testUtils


# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "/../examples/sample-data"
chunkSize = 100
compressionLevel = 6


print("Testing VRMetadata")

class TestVRMetadata(unittest.TestCase):
    def testCreateOpen(self):
        tmpBagFile = testUtils.RandomFileGuard("name")

        # test create
        metadata = Metadata()
        self.assertIsNotNone(metadata)

        metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)

        dataset = Dataset.create(tmpBagFile.getName(), metadata,
                                 chunkSize, compressionLevel)
        self.assertIsNotNone(dataset)

        dataset.createVR(chunkSize, compressionLevel, False)

        # Check that the optional variable resolution metadata layer exists.
        vrMetadata = dataset.getVRMetadata()
        self.assertIsNotNone(vrMetadata)

        vrMetadataDescriptor = vrMetadata.getDescriptor()
        self.assertIsNotNone(vrMetadataDescriptor)

        vrMetadata.writeAttributes()

        # Set some new attribute values.
        kExpectedMinDimX = 1
        kExpectedMinDimY = 2
        vrMetadataDescriptor.setMinDimensions(kExpectedMinDimX, kExpectedMinDimY)

        kExpectedMaxDimX = 100
        kExpectedMaxDimY = 200
        vrMetadataDescriptor.setMaxDimensions(kExpectedMaxDimX, kExpectedMaxDimY)

        kExpectedMinResX = 10.1
        kExpectedMinResY = 20.2
        vrMetadataDescriptor.setMinResolution(kExpectedMinResX, kExpectedMinResY)

        kExpectedMaxResX = 111.1
        kExpectedMaxResY = 222.2
        vrMetadataDescriptor.setMaxResolution(kExpectedMaxResX, kExpectedMaxResY)

        # Read the attributes back from memory.
        minDimX, minDimY = vrMetadataDescriptor.getMinDimensions()
        self.assertEqual(kExpectedMinDimX, minDimX)
        self.assertEqual(kExpectedMinDimY, minDimY)

        maxDimX, maxDimY = vrMetadataDescriptor.getMaxDimensions()
        self.assertEqual(kExpectedMaxDimX, maxDimX)
        self.assertEqual(kExpectedMaxDimY, maxDimY)

        minResX, minResY = vrMetadataDescriptor.getMinResolution()
        self.assertAlmostEqual(kExpectedMinResX, minResX, places=5)
        self.assertAlmostEqual(kExpectedMinResY, minResY, places=5)

        maxResX, maxResY = vrMetadataDescriptor.getMaxResolution()
        self.assertAlmostEqual(kExpectedMaxResX, maxResX, places=5)
        self.assertAlmostEqual(kExpectedMaxResY, maxResY, places=5)

        # Save the new attributes.
        vrMetadata.writeAttributes()

        # Force a close
        del dataset

        # Test opening
        dataset = Dataset.openDataset(tmpBagFile.getName(), BAG_OPEN_READONLY)

        # Check that the optional variable resolution metadata layer exists.
        vrMetadata = dataset.getVRMetadata()
        self.assertIsNotNone(vrMetadata)

        vrMetadataDescriptor = vrMetadata.getDescriptor()

        # Read the attributes from the file.
        minDimX, minDimY = vrMetadataDescriptor.getMinDimensions()
        self.assertEqual(kExpectedMinDimX, minDimX)
        self.assertEqual(kExpectedMinDimY, minDimY)

        maxDimX, maxDimY = vrMetadataDescriptor.getMaxDimensions()
        self.assertEqual(kExpectedMaxDimX, maxDimX)
        self.assertEqual(kExpectedMaxDimY, maxDimY)

        minResX, minResY = vrMetadataDescriptor.getMinResolution()
        self.assertAlmostEqual(kExpectedMinResX, minResX, places=5)
        self.assertAlmostEqual(kExpectedMinResY, minResY, places=5)

        maxResX, maxResY = vrMetadataDescriptor.getMaxResolution()
        self.assertAlmostEqual(kExpectedMaxResX, maxResX, places=5)
        self.assertAlmostEqual(kExpectedMaxResY, maxResY, places=5)

        # Force a close.
        del dataset

    def testWriteRead(self):
        tmpBagFile = testUtils.RandomFileGuard("name")

        # test create
        metadata = Metadata()
        self.assertIsNotNone(metadata)

        metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)

        dataset = Dataset.create(tmpBagFile.getName(), metadata, chunkSize, compressionLevel)
        self.assertIsNotNone(dataset)

        dataset.createVR(chunkSize, compressionLevel, False)

        # Check that the optional variable resolution metadata layer exists.
        vrMetadata = dataset.getVRMetadata()
        self.assertIsNotNone(vrMetadata)

        vrMetadataDescriptor = vrMetadata.getDescriptor()
        self.assertIsNotNone(vrMetadataDescriptor)

        # Write one record.
        kExpectedItem0 = BagVRMetadataItem(0, 1, 2, 3.45, 6.789, 1001.01, 4004.004)

        kRowStart = 0
        kColumnStart = 0
        kRowEnd = 0
        kColumnEnd = 0

        buffer = VRMetadataLayerItems((kExpectedItem0,))
        vrMetadata.write(kRowStart, kColumnStart, kRowEnd, kColumnEnd, buffer)

        # Force a close.
        del vrMetadata
        del dataset

        # Re-open read-only
        dataset = Dataset.openDataset(tmpBagFile.getName(), BAG_OPEN_READONLY)
        vrMetadata = dataset.getVRMetadata()

        # Read the record back.
        buffer = vrMetadata.read(kRowStart, kColumnStart, kRowEnd, kColumnEnd)
        self.assertIsNotNone(buffer)

        result = buffer.asVRMetadataItems()
        self.assertEqual(len(result), 1)

        # Check the expected value of VRMetadataItem::index.
        self.assertEqual(result[0].index, kExpectedItem0.index)

        # Check the expected value of VRMetadataItem::dimensions_x.
        self.assertEqual(result[0].dimensions_x, kExpectedItem0.dimensions_x)

        # Check the expected value of VRMetadataItem::dimensions_y.
        self.assertEqual(result[0].dimensions_y, kExpectedItem0.dimensions_y)

        # Check the expected value of VRMetadataItem::resolution_x.
        self.assertAlmostEqual(result[0].resolution_x, kExpectedItem0.resolution_x, places=5)

        # Check the expected value of VRMetadataItem::resolution_y.
        self.assertAlmostEqual(result[0].resolution_y, kExpectedItem0.resolution_y, places=5)

        # Check the expected value of VRMetadataItem::sw_corner_x.".
        self.assertAlmostEqual(result[0].sw_corner_x, kExpectedItem0.sw_corner_x, places=5)

        # Check the expected value of VRMetadataItem::sw_corner_y."
        self.assertAlmostEqual(result[0].sw_corner_y, kExpectedItem0.sw_corner_y, places=5)

        # Force a close.
        del dataset


if __name__ == '__main__':
    unittest.main(
        testRunner=xmlrunner.XMLTestRunner(output='test-reports'),
        failfast=False, buffer=False, catchbreak=False
    )
