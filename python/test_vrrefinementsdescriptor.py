import unittest
import pathlib

import xmlrunner

from bagPy import *

import bagMetadataSamples, testUtils


# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "/../examples/sample-data"
chunkSize = 100
compressionLevel = 6


class TestVRRefinementsDescriptor(unittest.TestCase):
    def testGetSetMinMaxDepth(self):
        tmpBagFile = testUtils.RandomFileGuard("name")

        metadata = Metadata()

        metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)

        dataset = Dataset.create(tmpBagFile.getName(), metadata,
                                 chunkSize, compressionLevel)
        self.assertIsNotNone(dataset)

        dataset.createVR(chunkSize, compressionLevel, False)

        vrRefinements = dataset.getVRRefinements()
        self.assertIsNotNone(vrRefinements)

        vrRefinementDescriptor = vrRefinements.getDescriptor()

        # Set some expected minimum depth values and verify they are set.
        # Verify setting the min/max depth does not throw.
        kExpectedMinDepth = 1.1
        kExpectedMaxDepth = 2.2

        vrRefinementDescriptor.setMinMaxDepth(kExpectedMinDepth, kExpectedMaxDepth)

        # Verify the min/max depth is expected.
        minDepth, maxDepth = vrRefinementDescriptor.getMinMaxDepth()
        self.assertAlmostEqual(kExpectedMinDepth, minDepth, places=5)
        self.assertAlmostEqual(kExpectedMaxDepth, maxDepth, places=5)

        # Force a close.
        del dataset

    def testGetSetMinMaxUncertainty(self):
        tmpBagFile = testUtils.RandomFileGuard("name")

        metadata = Metadata()

        metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)

        dataset = Dataset.create(tmpBagFile.getName(), metadata,
                                 chunkSize, compressionLevel)
        self.assertIsNotNone(dataset)

        dataset.createVR(chunkSize, compressionLevel, False)

        vrRefinements = dataset.getVRRefinements()
        self.assertIsNotNone(vrRefinements)

        vrRefinementDescriptor = vrRefinements.getDescriptor()

        # Set some expected min/max uncertainty values and verify they are set.
        # Verify setting the min/max uncertainty does not throw.
        kExpectedMinUncert = 101.01
        kExpectedMaxUncert = 202.02

        vrRefinementDescriptor.setMinMaxUncertainty(kExpectedMinUncert, kExpectedMaxUncert)

        # Verify the min/max uncertainty is expected.
        minUncert, maxUncert = vrRefinementDescriptor.getMinMaxUncertainty()
        self.assertAlmostEqual(kExpectedMinUncert, minUncert, places=5)
        self.assertAlmostEqual(kExpectedMaxUncert, maxUncert, places=5)

        # Force a close.
        del dataset


if __name__ == '__main__':
    unittest.main(
        testRunner=xmlrunner.XMLTestRunner(output='test-reports'),
        failfast=False, buffer=False, catchbreak=False
    )
