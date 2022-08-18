import unittest
import pathlib

import xmlrunner

from bagPy import *

import bagMetadataSamples, testUtils


# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "/../examples/sample-data"
chunkSize = 100
compressionLevel = 6


class TestInterleavedLegacyLayerDescriptor(unittest.TestCase):
    def testCreate(self):
        tmpFile = testUtils.RandomFileGuard("name")
        metadata = Metadata()
        metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)
        self.assertIsNotNone(metadata)

        dataset = Dataset.create(tmpFile.getName(), metadata, chunkSize, compressionLevel)
        self.assertIsNotNone(dataset)

        descriptor = InterleavedLegacyLayerDescriptor.create(dataset, Hypothesis_Strength,
            NODE)
        self.assertIsNotNone(descriptor)

        del dataset #ensure dataset is deleted before tmpFile

    def testGetGroupType(self):
        tmpFile = testUtils.RandomFileGuard("name")
        metadata = Metadata()
        metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)
        self.assertIsNotNone(metadata)

        dataset = Dataset.create(tmpFile.getName(), metadata, chunkSize, compressionLevel)
        self.assertIsNotNone(dataset)

        kExpectedGroup = NODE
        descriptor = InterleavedLegacyLayerDescriptor.create(dataset, Hypothesis_Strength,
            kExpectedGroup)
        self.assertIsNotNone(descriptor)
        self.assertEqual(descriptor.getGroupType(), kExpectedGroup)

        kExpectedGroup = ELEVATION
        descriptor = InterleavedLegacyLayerDescriptor.create(dataset, Num_Soundings,
            kExpectedGroup)
        self.assertIsNotNone(descriptor)
        self.assertEqual(descriptor.getGroupType(), kExpectedGroup)

        del dataset #ensure dataset is deleted before tmpFile

    def testElementSize(self):
        tmpFile = testUtils.RandomFileGuard("name")
        metadata = Metadata()
        metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)
        self.assertIsNotNone(metadata)

        dataset = Dataset.create(tmpFile.getName(), metadata, chunkSize, compressionLevel)
        self.assertIsNotNone(dataset)

        kExpectedLayerType = Num_Hypotheses
        kExpectedGroupType = NODE
        descriptor = InterleavedLegacyLayerDescriptor.create(dataset, kExpectedLayerType,
            kExpectedGroupType)
        self.assertIsNotNone(descriptor)
        self.assertEqual(descriptor.getElementSize(),
                         Layer.getElementSize(Layer.getDataType(kExpectedLayerType)))

        kExpectedLayerType = Std_Dev
        kExpectedGroupType = ELEVATION
        descriptor = InterleavedLegacyLayerDescriptor.create(dataset, kExpectedLayerType,
            kExpectedGroupType)
        self.assertIsNotNone(descriptor)
        self.assertEqual(descriptor.getElementSize(),
                         Layer.getElementSize(Layer.getDataType(kExpectedLayerType)))

        del dataset #ensure dataset is deleted before tmpFile


if __name__ == '__main__':
    unittest.main(
        testRunner=xmlrunner.XMLTestRunner(output='test-reports'),
        failfast=False, buffer=False, catchbreak=False
    )
