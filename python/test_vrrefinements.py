import unittest
import pathlib

import xmlrunner

from bagPy import *

import bagMetadataSamples, testUtils


# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "/../examples/sample-data"
chunkSize = 100
compressionLevel = 6


class TestVRRefinements(unittest.TestCase):
    def testCreateOpen(self):
        tmpBagFile = testUtils.RandomFileGuard("name")

        # test create
        metadata = Metadata()

        metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)

        dataset = Dataset.create(tmpBagFile.getName(), metadata,
                                 chunkSize, compressionLevel)
        self.assertIsNotNone(dataset)

        dataset.createVR(chunkSize, compressionLevel, False)

        # Check that the optional variable resolution refinement layer exists.
        vrRefinements = dataset.getVRRefinements()
        self.assertIsNotNone(vrRefinements)

        vrRefinements.writeAttributes()

        vrRefinementsDescriptor = vrRefinements.getDescriptor()

        # Set some attributes to save.
        kExpectedMinDepth = 10.1
        kExpectedMaxDepth = 20.2

        vrRefinementsDescriptor.setMinMaxDepth(kExpectedMinDepth, kExpectedMaxDepth)

        kExpectedMinUncertainty = 111.1
        kExpectedMaxUncertainty = 222.2

        vrRefinementsDescriptor.setMinMaxUncertainty(kExpectedMinUncertainty, kExpectedMaxUncertainty)

        # Read the attributes back.
        minDepth, maxDepth = vrRefinementsDescriptor.getMinMaxDepth()
        self.assertAlmostEqual(kExpectedMinDepth, minDepth, places=5)
        self.assertAlmostEqual(kExpectedMaxDepth, maxDepth, places=5)

        minUncertainty, maxUncertainty = vrRefinementsDescriptor.getMinMaxUncertainty()
        self.assertAlmostEqual(kExpectedMinUncertainty, minUncertainty, places=5)
        self.assertAlmostEqual(kExpectedMaxUncertainty, maxUncertainty, places=5)

        # Write attributes to file.
        vrRefinements.writeAttributes()

        # Force a close.
        del dataset

        # test open
        dataset = Dataset.openDataset(tmpBagFile.getName(), BAG_OPEN_READONLY)

        # Check that the optional variable resolution refinement layer exists.
        vrRefinements = dataset.getVRRefinements()
        self.assertIsNotNone(vrRefinements)

        vrRefinementsDescriptor = vrRefinements.getDescriptor()

        # Read the attributes back.
        minDepth, maxDepth = vrRefinementsDescriptor.getMinMaxDepth()
        self.assertAlmostEqual(kExpectedMinDepth, minDepth, places=5)
        self.assertAlmostEqual(kExpectedMaxDepth, maxDepth, places=5)

        minUncertainty, maxUncertainty = vrRefinementsDescriptor.getMinMaxUncertainty()
        self.assertAlmostEqual(kExpectedMinUncertainty, minUncertainty, places=5)
        self.assertAlmostEqual(kExpectedMaxUncertainty, maxUncertainty, places=5)

        # Force a close.
        del dataset

    def testWriteRead(self):
        tmpBagFile = testUtils.RandomFileGuard("name")

        metadata = Metadata()

        metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)

        dataset = Dataset.create(tmpBagFile.getName(), metadata,
                                 chunkSize, compressionLevel)
        self.assertIsNotNone(dataset)

        dataset.createVR(chunkSize, compressionLevel, False)

        # Check the variable resolution refinement layer exists.
        vrRefinements = dataset.getVRRefinements()
        self.assertIsNotNone(vrRefinements)

        # "Write one record.
        kExpectedItem0 = BagVRRefinementsItem(9.8, 0.654)

        kRowStart = 0  # unused
        kColumnStart = 0
        kRowEnd = 0  # unused
        kColumnEnd = 0

        buffer = VRRefinementsLayerItems((kExpectedItem0,))  # LayerItem((kExpectedItem0,))
        vrRefinements.write(kRowStart, kColumnStart, kRowEnd, kColumnEnd, buffer)

        # Read the record back.
        buffer = vrRefinements.read(kRowStart, kColumnStart, kRowEnd, kColumnEnd)
        self.assertIsNotNone(buffer)

        result = buffer.asVRRefinementsItems()
        self.assertEqual(len(result), 1)

        # Check the expected value of VRRefinementItem::depth.
        self.assertEqual(result[0].depth, kExpectedItem0.depth)

        # Check the expected value of VRRefinementItem::depth_uncrt.
        self.assertEqual(result[0].depth_uncrt, kExpectedItem0.depth_uncrt)

        # Force a close.
        del dataset


if __name__ == '__main__':
    unittest.main(
        testRunner=xmlrunner.XMLTestRunner(output='test-reports'),
        failfast=False, buffer=False, catchbreak=False
    )
