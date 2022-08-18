import unittest
import pathlib

import xmlrunner

from bagPy import *

import bagMetadataSamples, testUtils

# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "/../examples/sample-data"
chunkSize = 100
compressionLevel = 6


class TestDescriptor(unittest.TestCase):
    def testConstructors(self):
        descriptor = Descriptor()
        self.assertIsNotNone(descriptor)
        kExpectedVersion = "1.5.0"
        descriptor.setVersion(kExpectedVersion)
        self.assertEqual(descriptor.getVersion(), kExpectedVersion)

        descriptor2 = Descriptor()
        self.assertIsNotNone(descriptor2)
        self.assertEqual(descriptor2.getVersion(), "")

        descriptor2 = descriptor
        self.assertEqual(descriptor2.getVersion(), kExpectedVersion)

    def testGetLayerTypes(self):
        descriptor = Descriptor()
        self.assertIsNotNone(descriptor)
        layerTypes = descriptor.getLayerTypes()
        self.assertEqual(len(layerTypes), 0)

        metadata = Metadata()
        metadata.loadFromBuffer(bagMetadataSamples.kXMLv2MetadataBuffer)
        self.assertIsNotNone(metadata)

        tmpFile = testUtils.RandomFileGuard("name")
        dataset = Dataset.create(tmpFile.getName(), metadata, chunkSize, compressionLevel)
        self.assertIsNotNone(dataset)

        descriptor = dataset.getDescriptor()
        self.assertIsNotNone(descriptor)

        layerTypes = descriptor.getLayerTypes()
        self.assertEqual(len(layerTypes), 2)

        self.assertTrue(Elevation in layerTypes)
        self.assertTrue(Uncertainty in layerTypes)

        self.assertEqual(getLayerTypeAsString(layerTypes[0]), "Elevation")
        self.assertEqual(getLayerTypeAsString(layerTypes[1]), "Uncertainty")

        del dataset #ensure dataset is deleted before tmpFile

    def testGetLayerIds(self):
        descriptor = Descriptor()
        self.assertIsNotNone(descriptor)
        layerIds = descriptor.getLayerIds()
        self.assertEqual(len(layerIds), 0)

        metadata = Metadata()
        metadata.loadFromBuffer(bagMetadataSamples.kXMLv2MetadataBuffer)
        self.assertIsNotNone(metadata)

        tmpFile = testUtils.RandomFileGuard("name")
        dataset = Dataset.create(tmpFile.getName(), metadata, chunkSize, compressionLevel)
        self.assertIsNotNone(dataset)

        descriptor = dataset.getDescriptor()
        self.assertIsNotNone(descriptor)

        layerIds = descriptor.getLayerIds()
        self.assertEqual(len(layerIds), 2)

        self.assertEqual(layerIds[0], 0)
        self.assertEqual(layerIds[1], 1)

        del dataset #ensure dataset is deleted before tmpFile

    def testReadOnly(self):
        descriptor = Descriptor()
        self.assertIsNotNone(descriptor)
        self.assertTrue(descriptor.isReadOnly())
        self.assertTrue(descriptor.setReadOnly(True))
        self.assertTrue(descriptor.setReadOnly(False))
        self.assertTrue(not descriptor.isReadOnly())

    def testGetLayerDescriptors(self):
        metadata = Metadata()
        metadata.loadFromBuffer(bagMetadataSamples.kXMLv2MetadataBuffer)
        self.assertIsNotNone(metadata)

        tmpFile = testUtils.RandomFileGuard("name")

        dataset = Dataset.create(tmpFile.getName(), metadata, chunkSize, compressionLevel)
        self.assertIsNotNone(dataset)

        descriptor = dataset.getDescriptor()
        self.assertIsNotNone(descriptor)

        layerDescriptors = descriptor.getLayerDescriptors()
        self.assertEqual(len(layerDescriptors), 2)

        del dataset #ensure dataset is deleted before tmpFile

    def testGetLayerDescriptor(self):
        descriptor = Descriptor()
        self.assertIsNotNone(descriptor)

        metadata = Metadata()
        metadata.loadFromBuffer(bagMetadataSamples.kXMLv2MetadataBuffer)
        self.assertIsNotNone(metadata)

        tmpFile = testUtils.RandomFileGuard("name")
        dataset = Dataset.create(tmpFile.getName(), metadata, chunkSize, compressionLevel)
        self.assertIsNotNone(dataset)

        descriptor = dataset.getDescriptor()
        self.assertIsNotNone(descriptor)

        self.assertTrue(descriptor.getLayerDescriptor(Elevation))
        self.assertTrue(descriptor.getLayerDescriptor(Uncertainty))

        del dataset #ensure dataset is deleted before tmpFile

    def testFromMetadata(self):
        metadata = Metadata()
        metadata.loadFromBuffer(bagMetadataSamples.kXMLv2MetadataBuffer)

        descriptor = Descriptor(metadata)
        self.assertIsNotNone(descriptor)

        kExpectedHorizontalReferenceSystem = '''PROJCS["UTM-19N-Nad83",
        GEOGCS["unnamed",
            DATUM["North_American_Datum_1983",
                SPHEROID["North_American_Datum_1983",6378137,298.2572201434276],
                TOWGS84[0,0,0,0,0,0,0]],
            PRIMEM["Greenwich",0],
            UNIT["degree",0.0174532925199433],
            EXTENSION["Scaler","0,0,0,0.02,0.02,0.001"],
            EXTENSION["Source","CARIS"]],
        PROJECTION["Transverse_Mercator"],
        PARAMETER["latitude_of_origin",0],
        PARAMETER["central_meridian",-69],
        PARAMETER["scale_factor",0.9996],
        PARAMETER["false_easting",500000],
        PARAMETER["false_northing",0],
        UNIT["metre",1]]'''
        horiz = str(descriptor.getHorizontalReferenceSystem())
        self.assertEqual(testUtils.replace_whitespace(horiz),
                         testUtils.replace_whitespace(kExpectedHorizontalReferenceSystem))

        kExpectedVerticalReferenceSystem = '''VERT_CS["Alicante height",
        VERT_DATUM["Alicante",2000]]'''
        self.assertEqual(testUtils.replace_whitespace(descriptor.getVerticalReferenceSystem()),
                         testUtils.replace_whitespace(kExpectedVerticalReferenceSystem))

        kExpectedDimXY = 100
        dims = descriptor.getDims()
        self.assertEqual(dims[0], kExpectedDimXY)
        self.assertEqual(dims[1], kExpectedDimXY)

        minX, minY, maxX, maxY = 687910.0, 5554620.0, 691590.0, 5562100.0
        cover = descriptor.getProjectedCover()
        self.assertEqual(cover[0][0], minX)
        self.assertEqual(cover[0][1], minY)
        self.assertEqual(cover[1][0], maxX)
        self.assertEqual(cover[1][1], maxY)

        kExpectedOriginX, kExpectedOriginY = 687910.0, 5554620.0
        origin = descriptor.getOrigin()
        self.assertEqual(origin[0], kExpectedOriginX)
        self.assertEqual(origin[1], kExpectedOriginY)

        kExpectedGridSpacingXY = 10.0
        spacing = descriptor.getGridSpacing()
        self.assertEqual(spacing[0], kExpectedGridSpacingXY)
        self.assertEqual(spacing[1], kExpectedGridSpacingXY)

    def testSetValues(self):
        descriptor = Descriptor()

        kExpected = "abc"
        descriptor.setHorizontalReferenceSystem(kExpected)
        self.assertEqual(descriptor.getHorizontalReferenceSystem(), kExpected)

        kExpected = "def"
        descriptor.setVerticalReferenceSystem(kExpected)
        self.assertEqual(descriptor.getVerticalReferenceSystem(), kExpected)

        kExpectedDim0 = 24
        kExpectedDim1 = 68
        descriptor.setDims(kExpectedDim0, kExpectedDim1)
        self.assertEqual(descriptor.getDims(), (kExpectedDim0, kExpectedDim1))

        kExpectedLLx = 0
        kExpectedLLy = 1
        kExpectedURx = 1001
        kExpectedURy = 2020
        descriptor.setProjectedCover(kExpectedLLx, kExpectedLLy, kExpectedURx, kExpectedURy)
        self.assertEqual(descriptor.getProjectedCover(),
                         ((kExpectedLLx, kExpectedLLy), (kExpectedURx, kExpectedURy)))

        kExpectedLLx = 12.3456
        kExpectedLLy = 98.7654321
        descriptor.setOrigin(kExpectedLLx, kExpectedLLy)
        self.assertEqual(descriptor.getOrigin(), (kExpectedLLx, kExpectedLLy))

        kExpectedXspacing = 12.3456
        kExpectedYspacing = 98.7654321
        descriptor.setGridSpacing(kExpectedXspacing, kExpectedYspacing)
        self.assertEqual(descriptor.getOrigin(), (kExpectedXspacing, kExpectedYspacing))

        # test chaining of set methods
        descriptor.setOrigin(0.0, 0.0).setGridSpacing(0.0, 0.0)
        self.assertEqual(descriptor.getOrigin(), (0.0, 0.0))
        self.assertEqual(descriptor.getOrigin(), (0.0, 0.0))


if __name__ == '__main__':
    unittest.main(
        testRunner=xmlrunner.XMLTestRunner(output='test-reports'),
        failfast=False, buffer=False, catchbreak=False
    )
