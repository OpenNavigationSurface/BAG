import unittest
import pathlib

import xmlrunner

from bagPy import *

import bagMetadataSamples, testUtils


# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "/../examples/sample-data"
chunkSize = 100
compressionLevel = 6


class TestVRNodeDescriptor(unittest.TestCase):
    def testGetSetminMaxHypStr(self):
        tmpBagFile = testUtils.RandomFileGuard("name")

        metadata = Metadata()

        metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)

        dataset = Dataset.create(tmpBagFile.getName(), metadata,
                                 chunkSize, compressionLevel)
        self.assertIsNotNone(dataset)

        dataset.createVR(chunkSize, compressionLevel, True)

        vrNode = dataset.getVRNode()
        self.assertIsNotNone(vrNode)

        vrNodeDescriptor = vrNode.getDescriptor()

        # Set some expected minimum hyp strength values and verify they are set.
        # Verify setting the min/max hyp strength does not throw.
        kExpectedMinHypStr = 1.1
        kExpectedMaxHypStr = 2.2

        vrNodeDescriptor.setMinMaxHypStrength(kExpectedMinHypStr, kExpectedMaxHypStr)

        # Verify the min/max hyp strength is expected.
        minHypStr, maxHypStr = vrNodeDescriptor.getMinMaxHypStrength()
        self.assertAlmostEqual(kExpectedMinHypStr, minHypStr, places=5)
        self.assertAlmostEqual(kExpectedMaxHypStr, maxHypStr, places=5)

        # Force a close.
        del dataset

    def testGetSetNumHypotheses(self):
        tmpBagFile = testUtils.RandomFileGuard("name")

        metadata = Metadata()

        metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)

        dataset = Dataset.create(tmpBagFile.getName(), metadata,
                                 chunkSize, compressionLevel)
        self.assertIsNotNone(dataset)

        dataset.createVR(chunkSize, compressionLevel, True)

        vrNode = dataset.getVRNode()
        self.assertIsNotNone(vrNode)

        vrNodeDescriptor = vrNode.getDescriptor()

        # Set some expected min/max num hypotheses values and verify they are set.
        # Verify setting the min/max num hypotheses does not throw.
        kExpectedMinNumHyp = 10
        kExpectedMaxNumHyp = 20

        vrNodeDescriptor.setMinMaxNumHypotheses(kExpectedMinNumHyp, kExpectedMaxNumHyp)

        # Verify the min/max num hypotheses is expected.
        minNumHyp, maxNumHyp = vrNodeDescriptor.getMinMaxNumHypotheses()
        self.assertEqual(kExpectedMinNumHyp, minNumHyp)
        self.assertEqual(kExpectedMaxNumHyp, maxNumHyp)

        # Force a close.
        del dataset

    def testGetSetMinMaxNSamples(self):
        tmpBagFile = testUtils.RandomFileGuard("name")

        metadata = Metadata()

        metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)

        dataset = Dataset.create(tmpBagFile.getName(), metadata,
                                 chunkSize, compressionLevel)
        self.assertIsNotNone(dataset)

        dataset.createVR(chunkSize, compressionLevel, True)

        vrNode = dataset.getVRNode()
        self.assertIsNotNone(vrNode)

        vrNodeDescriptor = vrNode.getDescriptor()

        # Set some expected min/max n samples values and verify they are set.
        # Verify setting the min/max n samples does not throw.
        kExpectedMinNSamples = 100
        kExpectedMaxNSamples = 200

        vrNodeDescriptor.setMinMaxNSamples(kExpectedMinNSamples, kExpectedMaxNSamples)

        # Verify the min/max n samples is expected.
        minNSamples, maxNSamples = vrNodeDescriptor.getMinMaxNSamples()
        self.assertEqual(kExpectedMinNSamples, minNSamples)
        self.assertEqual(kExpectedMaxNSamples, maxNSamples)

        # Force a close.
        del dataset


if __name__ == '__main__':
    unittest.main(
        testRunner=xmlrunner.XMLTestRunner(output='test-reports'),
        failfast=False, buffer=False, catchbreak=False
    )
