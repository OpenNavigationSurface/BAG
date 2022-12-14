import unittest
import pathlib

import xmlrunner

from bagPy import *

import bagMetadataSamples, testUtils


# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "/../examples/sample-data"
chunkSize = 100
compressionLevel = 6


class TestVRNode(unittest.TestCase):
    def testCreateOpen(self):
        tmpBagFile = testUtils.RandomFileGuard("name")

        # Check dataset was created successfully.
        metadata = Metadata()

        metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)

        dataset = Dataset.create(tmpBagFile.getName(), metadata,
                                 chunkSize, compressionLevel)
        self.assertIsNotNone(dataset)

        dataset.createVR(chunkSize, compressionLevel, True)

        # Check that the optional variable resolution node layer exists.
        vrNode = dataset.getVRNode()
        self.assertIsNotNone(vrNode)

        vrNodeDescriptor = vrNode.getDescriptor()

        vrNode.writeAttributes()

        # Set some attributes to save.
        kExpectedMinHypStr = 10.1
        kExpectedMaxHypStr = 20.2

        vrNodeDescriptor.setMinMaxHypStrength(kExpectedMinHypStr, kExpectedMaxHypStr)

        kExpectedMinNumHyp = 100
        kExpectedMaxNumHyp = 200

        vrNodeDescriptor.setMinMaxNumHypotheses(kExpectedMinNumHyp, kExpectedMaxNumHyp)

        kExpectedMinNSamples = 1000
        kExpectedMaxNSamples = 2000

        vrNodeDescriptor.setMinMaxNSamples(kExpectedMinNSamples, kExpectedMaxNSamples)

        # Read the attributes back
        minHypStr, maxHypStr = vrNodeDescriptor.getMinMaxHypStrength()
        self.assertAlmostEqual(kExpectedMinHypStr, minHypStr, places=5)
        self.assertAlmostEqual(kExpectedMaxHypStr, maxHypStr, places=5)

        minNumHyp, maxNumHyp = vrNodeDescriptor.getMinMaxNumHypotheses()
        self.assertEqual(kExpectedMinNumHyp, minNumHyp)
        self.assertEqual(kExpectedMaxNumHyp, maxNumHyp)

        minNSamples, maxNSamples = vrNodeDescriptor.getMinMaxNSamples()
        self.assertEqual(kExpectedMinNSamples, minNSamples)
        self.assertEqual(kExpectedMaxNSamples, maxNSamples)

        # Write attributes to file.
        vrNode.writeAttributes()

        # Force a close.
        del dataset

        # test open
        dataset = Dataset.openDataset(tmpBagFile.getName(), BAG_OPEN_READONLY)

        # Check that the optional variable resolution node layer exists.
        vrNode = dataset.getVRNode()
        self.assertIsNotNone(vrNode)

        vrNodeDescriptor = vrNode.getDescriptor()

        # Read the attributes back.
        minHypStr, maxHypStr = vrNodeDescriptor.getMinMaxHypStrength()
        self.assertAlmostEqual(kExpectedMinHypStr, minHypStr, places=5)
        self.assertAlmostEqual(kExpectedMaxHypStr, maxHypStr, places=5)

        minNumHyp, maxNumHyp = vrNodeDescriptor.getMinMaxNumHypotheses()
        self.assertEqual(kExpectedMinNumHyp, minNumHyp)
        self.assertEqual(kExpectedMaxNumHyp, maxNumHyp)

        minNSamples, maxNSamples = vrNodeDescriptor.getMinMaxNSamples()
        self.assertEqual(kExpectedMinNSamples, minNSamples)
        self.assertEqual(kExpectedMaxNSamples, maxNSamples)

        # Force a close.
        del dataset

    def testWriteRead(self):
        tmpBagFile = testUtils.RandomFileGuard("name")

        # Check dataset was created successfully.
        metadata = Metadata()

        metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)

        dataset = Dataset.create(tmpBagFile.getName(), metadata,
                                 chunkSize, compressionLevel)
        self.assertIsNotNone(dataset)

        dataset.createVR(chunkSize, compressionLevel, True)

        # Check the optional variable resolution node layer exists.
        vrNode = dataset.getVRNode()
        self.assertIsNotNone(vrNode)

        # Write one record.
        kExpectedItem0 = BagVRNodeItem(123.456, 42, 1701)

        kRowStart = 0  # not used
        kColumnStart = 0
        kRowEnd = 0  # not used
        kColumnEnd = 0

        buffer = VRNodeLayerItems((kExpectedItem0,))  # LayerItem((kExpectedItem0,))
        vrNode.write(kRowStart, kColumnStart, kRowEnd, kColumnEnd, buffer)

        # Read the record back.
        buffer = vrNode.read(kRowStart, kColumnStart, kRowEnd, kColumnEnd)
        self.assertIsNotNone(buffer)

        result = buffer.asVRNodeItems()
        self.assertEqual(len(result), 1)

        # Check the expected value of BagVRNodeItem::hyp_strength.
        self.assertEqual(result[0].hyp_strength, kExpectedItem0.hyp_strength)

        # Check the expected value of BagVRNodeItem::num_hypotheses.
        self.assertEqual(result[0].num_hypotheses, kExpectedItem0.num_hypotheses)

        # Check the expected value of BagVRNodeItem::n_samples.
        self.assertEqual(result[0].n_samples, kExpectedItem0.n_samples)

        # Force a close.
        del dataset


if __name__ == '__main__':
    unittest.main(
        testRunner=xmlrunner.XMLTestRunner(output='test-reports'),
        failfast=False, buffer=False, catchbreak=False
    )
