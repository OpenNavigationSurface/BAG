import unittest
import pathlib

import xmlrunner

from bagPy import *

import bagMetadataSamples, testUtils


# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "/../examples/sample-data"
chunkSize = 100
compressionLevel = 6


class TestVRMetadataDescriptor(unittest.TestCase):
    def testGetMinMaxDimensions(self):
        tmpFile = testUtils.RandomFileGuard("name")

        metadata = Metadata()
        metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)

        dataset = Dataset.create(tmpFile.getName(), metadata,
                                 chunkSize, compressionLevel)
        self.assertIsNotNone(dataset)

        dataset.createVR(chunkSize, compressionLevel, False)

        vrMetadata = dataset.getVRMetadata()
        self.assertIsNotNone(vrMetadata)

        vrMetadataDescriptor = vrMetadata.getDescriptor()

        # Set some expected minimum dimension values and verify they are set.
        kExpectedMinDimX = 1
        kExpectedMinDimY = 2

        vrMetadataDescriptor.setMinDimensions(kExpectedMinDimX, kExpectedMinDimY)

        # Verify the min X and Y dimensions are expected.
        minDimX, minDimY = vrMetadataDescriptor.getMinDimensions()
        self.assertEqual(minDimX, kExpectedMinDimX)
        self.assertEqual(minDimY, kExpectedMinDimY)

        # Set some expected maximum dimension values and verify they are set.
        kExpectedMaxDimX = 101
        kExpectedMaxDimY = 202

        vrMetadataDescriptor.setMaxDimensions(kExpectedMaxDimX, kExpectedMaxDimY)

        # Verify the max X and Y dimensions are expected.
        maxDimX, maxDimY = vrMetadataDescriptor.getMaxDimensions()
        self.assertEqual(maxDimX, kExpectedMaxDimX)
        self.assertEqual(maxDimY, kExpectedMaxDimY)

        # Force a close.
        del dataset

    def testGetSetMinMaxResolution(self):
        tmpFile = testUtils.RandomFileGuard("name")

        metadata = Metadata()
        metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)

        dataset = Dataset.create(tmpFile.getName(), metadata,
                                 chunkSize, compressionLevel)
        self.assertIsNotNone(dataset)

        dataset.createVR(chunkSize, compressionLevel, False)

        vrMetadata = dataset.getVRMetadata()
        self.assertIsNotNone(vrMetadata)

        vrMetadataDescriptor = vrMetadata.getDescriptor()

        # Set some expected minimum resolution values and verify they are set.
        kExpectedMinResX = 1.1
        kExpectedMinResY = 2.2

        vrMetadataDescriptor.setMinResolution(kExpectedMinResX, kExpectedMinResY)

        # Verify the min X and Y resolution is expected.
        minResX, minResY = vrMetadataDescriptor.getMinResolution()
        self.assertAlmostEqual(kExpectedMinResX, minResX, places=5)
        self.assertAlmostEqual(kExpectedMinResY, minResY, places=5)

        # Set some expected maximum resolution values and verify they are set.
        kExpectedMaxResX = 444.4
        kExpectedMaxResY = 555.5

        vrMetadataDescriptor.setMaxResolution(kExpectedMaxResX, kExpectedMaxResY)

        # Verify the max X and Y resolution is expected.
        maxResX, maxResY = vrMetadataDescriptor.getMaxResolution()
        self.assertAlmostEqual(kExpectedMaxResX, maxResX, places=4)
        self.assertAlmostEqual(kExpectedMaxResY, maxResY, places=5)

        # Force a close.
        del dataset


if __name__ == '__main__':
    unittest.main(
        testRunner=xmlrunner.XMLTestRunner(output='test-reports'),
        failfast=False, buffer=False, catchbreak=False
    )
