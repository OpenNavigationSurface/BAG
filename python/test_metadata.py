import unittest
import pathlib

import xmlrunner

from bagPy import *

import bagMetadataSamples, testUtils


# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "/../examples/sample-data"


class TestMetadata(unittest.TestCase):
    def testDefaultConstructor(self):
        metadata = Metadata()
        self.assertIsNotNone(metadata)
        self.assertEqual(metadata.llCornerX(), INIT_VALUE)

    def testConstructor(self):
        bagFileName = datapath + "/sample.bag"
        dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READONLY)
        self.assertIsNotNone(dataset)

        metadata = Metadata(dataset)
        self.assertIsNotNone(metadata)
        self.assertEqual(metadata.llCornerX(), 687910.0)
        wkt = metadata.horizontalReferenceSystemAsWKT()
        self.assertTrue(wkt.startswith('''PROJCS["UTM-19N-Nad83'''))

    def testGetStruct(self):
        bagFileName = datapath + "/sample.bag"
        dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READONLY)
        self.assertIsNotNone(dataset)

        metadata = dataset.getMetadata()
        self.assertIsNotNone(metadata)
        bagStruct = metadata.getStruct()
        self.assertIsNotNone(bagStruct)
        self.assertEqual(bagStruct.fileIdentifier, "Unique Identifier")
        self.assertIsNotNone(bagStruct.spatialRepresentationInfo)
        self.assertEqual(bagStruct.spatialRepresentationInfo.llCornerX, 687910.0)

    def testHorizontalReferenceSystemAsWKT(self):
        bagFileName = datapath + "/sample.bag"
        dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READONLY)
        self.assertIsNotNone(dataset)

        metadata = dataset.getMetadata()
        self.assertIsNotNone(metadata)
        horizontalRefSys = metadata.horizontalReferenceSystemAsWKT()
        self.assertIsNotNone(horizontalRefSys)
        self.assertTrue(horizontalRefSys.startswith('''PROJCS["UTM-19N-Nad83'''))

    def testLoadFromFile(self):
        tmpFile = testUtils.RandomFileGuard("string",
            bagMetadataSamples.kXMLv2MetadataBuffer)

        metadata = Metadata()
        metadata.loadFromFile(tmpFile.getName())
        self.assertIsNotNone(metadata)

        self.assertEqual(metadata.columnResolution(), 10.0)
        self.assertIsNotNone(metadata.horizontalReferenceSystemAsWKT())
        self.assertEqual(metadata.llCornerX(), 687910.000000)
        self.assertEqual(metadata.llCornerY(), 5554620.000000)
        self.assertEqual(metadata.rowResolution(), 10.0)
        self.assertEqual(metadata.urCornerX(), 691590.000000)
        self.assertEqual(metadata.urCornerY(), 5562100.000000)

    def testLoadFromBuffer(self):
        metadata = Metadata()
        metadata.loadFromBuffer(bagMetadataSamples.kXMLv2MetadataBuffer)
        self.assertIsNotNone(metadata)

        self.assertEqual(metadata.columnResolution(), 10.0)
        self.assertEqual(metadata.llCornerX(), 687910.000000)
        self.assertEqual(metadata.llCornerY(), 5554620.000000)
        self.assertEqual(metadata.rowResolution(), 10.0)
        self.assertEqual(metadata.urCornerX(), 691590.000000)
        self.assertEqual(metadata.urCornerY(), 5562100.000000)
        self.assertEqual(metadata.rows(), 100)
        self.assertEqual(metadata.columns(), 100)


if __name__ == '__main__':
    unittest.main(
        testRunner=xmlrunner.XMLTestRunner(output='test-reports'),
        failfast=False, buffer=False, catchbreak=False
    )
