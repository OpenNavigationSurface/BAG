import unittest
import pathlib

import xmlrunner

from bagPy import *

import bagMetadataSamples, testUtils


# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "/../examples/sample-data"
kExpectedChunkSize = 100
kExpectedCompressionLevel = 6


class TestSurfaceCorrectionsDescriptor(unittest.TestCase):
    def testCreation(self):
        tmpFile = testUtils.RandomFileGuard("name")
        #print(tmpFile.getName())
        metadata = Metadata()
        metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)
        self.assertIsNotNone(metadata)

        # Create the dataset.
        dataset = Dataset.create(tmpFile.getName(), metadata,
                                 kExpectedChunkSize, kExpectedCompressionLevel)
        self.assertIsNotNone(dataset)

        descriptor = SimpleLayerDescriptor.create(dataset, Elevation,
                                                  kExpectedChunkSize, kExpectedCompressionLevel)
        self.assertIsNotNone(descriptor)

        self.assertEqual(descriptor.getLayerType(), Elevation)
        self.assertEqual(descriptor.getChunkSize(), kExpectedChunkSize)
        self.assertEqual(descriptor.getCompressionLevel(), kExpectedCompressionLevel)

        del dataset #ensure dataset is deleted before tmpFile

    def testGetSetName(self):
        tmpFile = testUtils.RandomFileGuard("name")
        metadata = Metadata()
        metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)
        self.assertIsNotNone(metadata)

        # Create the dataset.
        dataset = Dataset.create(tmpFile.getName(), metadata,
                                 kExpectedChunkSize, kExpectedCompressionLevel)
        self.assertIsNotNone(dataset)

        descriptor = SimpleLayerDescriptor.create(dataset, Elevation,
                                                  kExpectedChunkSize, kExpectedCompressionLevel)
        self.assertIsNotNone(descriptor)

        kExpectedName = "Expected Name"
        descriptor.setName(kExpectedName)
        self.assertEqual(descriptor.getName(), kExpectedName)

        del dataset #ensure dataset is deleted before tmpFile

    def testGetDataType(self):
        tmpFile = testUtils.RandomFileGuard("name")
        metadata = Metadata()
        metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)
        self.assertIsNotNone(metadata)

        # Create the dataset.
        dataset = Dataset.create(tmpFile.getName(), metadata,
                                 kExpectedChunkSize, kExpectedCompressionLevel)
        self.assertIsNotNone(dataset)

        descriptor = SimpleLayerDescriptor.create(dataset, Elevation,
                                                  kExpectedChunkSize, kExpectedCompressionLevel)
        self.assertIsNotNone(descriptor)

        self.assertEqual(descriptor.getDataType(), Layer.getDataType(Elevation))

        del dataset #ensure dataset is deleted before tmpFile

    def testGetLayerType(self):
        tmpFile = testUtils.RandomFileGuard("name")
        metadata = Metadata()
        metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)
        self.assertIsNotNone(metadata)

        # Create the dataset.
        dataset = Dataset.create(tmpFile.getName(), metadata,
                                 kExpectedChunkSize, kExpectedCompressionLevel)
        self.assertIsNotNone(dataset)

        descriptor = SimpleLayerDescriptor.create(dataset, Elevation,
                                                  kExpectedChunkSize, kExpectedCompressionLevel)
        self.assertIsNotNone(descriptor)

        self.assertEqual(descriptor.getLayerType(), Elevation)

        descriptor = SimpleLayerDescriptor.create(dataset, Std_Dev,
                                                  kExpectedChunkSize, kExpectedCompressionLevel)
        self.assertIsNotNone(descriptor)

        self.assertEqual(descriptor.getDataType(), Layer.getDataType(Std_Dev))

        del dataset #ensure dataset is deleted before tmpFile
    
    def testGetSetMinMax(self):
        tmpFile = testUtils.RandomFileGuard("name")
        metadata = Metadata()
        metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)
        self.assertIsNotNone(metadata)

        # Create the dataset.
        dataset = Dataset.create(tmpFile.getName(), metadata,
                                 kExpectedChunkSize, kExpectedCompressionLevel)
        self.assertIsNotNone(dataset)

        descriptor = SimpleLayerDescriptor.create(dataset, Elevation,
                                                  kExpectedChunkSize, kExpectedCompressionLevel)
        self.assertIsNotNone(descriptor)

        kExpectedMin = 1.2345
        kExpectedMax = 9876.54321
        descriptor.setMinMax(kExpectedMin, kExpectedMax)

        actualMinMax = descriptor.getMinMax()
        self.assertAlmostEqual(actualMinMax[0], kExpectedMin, places=4)
        self.assertAlmostEqual(actualMinMax[1], kExpectedMax, places=3)

        del dataset #ensure dataset is deleted before tmpFile
  
    def testGetInternalPath(self):
        tmpFile = testUtils.RandomFileGuard("name")
        metadata = Metadata()
        metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)
        self.assertIsNotNone(metadata)

        # Create the dataset.
        dataset = Dataset.create(tmpFile.getName(), metadata,
                                 kExpectedChunkSize, kExpectedCompressionLevel)
        self.assertIsNotNone(dataset)

        descriptor = SimpleLayerDescriptor.create(dataset, Elevation,
                                                  kExpectedChunkSize, kExpectedCompressionLevel)
        self.assertIsNotNone(descriptor)
        self.assertEqual(descriptor.getInternalPath(), Layer.getInternalPath(Elevation))

        descriptor = SimpleLayerDescriptor.create(dataset, Uncertainty,
                                                  kExpectedChunkSize, kExpectedCompressionLevel)
        self.assertIsNotNone(descriptor)
        self.assertEqual(descriptor.getInternalPath(), Layer.getInternalPath(Uncertainty))

        del dataset #ensure dataset is deleted before tmpFile

    def testGetElementSize(self):
        tmpFile = testUtils.RandomFileGuard("name")
        metadata = Metadata()
        metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)
        self.assertIsNotNone(metadata)

        # Create the dataset.
        dataset = Dataset.create(tmpFile.getName(), metadata,
                                 kExpectedChunkSize, kExpectedCompressionLevel)
        self.assertIsNotNone(dataset)

        descriptor = SimpleLayerDescriptor.create(dataset, Elevation,
                                                  kExpectedChunkSize, kExpectedCompressionLevel)
        self.assertIsNotNone(descriptor)
        self.assertEqual(descriptor.getElementSize(),
                         Layer.getElementSize(Layer.getDataType(Elevation)))

        del dataset #ensure dataset is deleted before tmpFile

    def testGetChunkSize(self):
        tmpFile = testUtils.RandomFileGuard("name")
        metadata = Metadata()
        metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)
        self.assertIsNotNone(metadata)

        # Create the dataset.
        dataset = Dataset.create(tmpFile.getName(), metadata,
                                 kExpectedChunkSize, kExpectedCompressionLevel)
        self.assertIsNotNone(dataset)

        descriptor = SimpleLayerDescriptor.create(dataset, Elevation,
                                                  kExpectedChunkSize, kExpectedCompressionLevel)
        self.assertIsNotNone(descriptor)
        self.assertEqual(descriptor.getChunkSize(), kExpectedChunkSize)

        del dataset #ensure dataset is deleted before tmpFile

    def testGetCompressionLevel(self):
        tmpFile = testUtils.RandomFileGuard("name")
        metadata = Metadata()
        metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)
        self.assertIsNotNone(metadata)

        # Create the dataset.
        dataset = Dataset.create(tmpFile.getName(), metadata,
                                 kExpectedChunkSize, kExpectedCompressionLevel)
        self.assertIsNotNone(dataset)

        descriptor = SimpleLayerDescriptor.create(dataset, Elevation,
                                                  kExpectedChunkSize, kExpectedCompressionLevel)
        self.assertIsNotNone(descriptor)
        self.assertEqual(descriptor.getCompressionLevel(), kExpectedCompressionLevel)

        del dataset #ensure dataset is deleted before tmpFile


if __name__ == '__main__':
    unittest.main(
        testRunner=xmlrunner.XMLTestRunner(output='test-reports'),
        failfast=False, buffer=False, catchbreak=False
    )
