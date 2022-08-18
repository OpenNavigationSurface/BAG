import unittest
import pathlib

import xmlrunner

from bagPy import *

import bagMetadataSamples, testUtils


# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "/../examples/sample-data"
chunkSize = 100
compressionLevel = 6


class TestSurfaceCorrections(unittest.TestCase):
    def testReadIrregular(self):
        bagFileName = datapath + "/sample.bag"

        dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READONLY)
        self.assertIsNotNone(dataset)

        corrections = dataset.getSurfaceCorrections()
        self.assertIsNotNone(corrections)

        descriptor = corrections.getDescriptor()
        self.assertEqual(descriptor.getSurfaceType(), BAG_SURFACE_IRREGULARLY_SPACED)

        kExpectedNumCorrectors = 2
        self.assertEqual(descriptor.getNumCorrectors(), kExpectedNumCorrectors)

    def testCreateIrregular(self):
        tmpFile = testUtils.RandomFileGuard("name")

        dataset = Dataset.create(tmpFile.getName(), Metadata(), chunkSize, compressionLevel)
        self.assertIsNotNone(dataset)

        kExpectedSurfaceType = BAG_SURFACE_IRREGULARLY_SPACED
        kExpectedNumCorrectors = 4
        corrections = dataset.createSurfaceCorrections(kExpectedSurfaceType, kExpectedNumCorrectors,
                                                       chunkSize, compressionLevel)
        self.assertIsNotNone(corrections)

        descriptor = corrections.getDescriptor()

        self.assertEqual(descriptor.getNumCorrectors(), kExpectedNumCorrectors)
        self.assertEqual(descriptor.getSurfaceType(), kExpectedSurfaceType)
        self.assertEqual(descriptor.getVerticalDatums(), "")
        self.assertEqual(descriptor.getOrigin(), (0.0, 0.0))
        self.assertEqual(descriptor.getSpacing(), (0.0, 0.0))

        del dataset #ensure dataset is deleted before tmpFile

    def testCreateGridded(self):
        tmpFile = testUtils.RandomFileGuard("name")

        dataset = Dataset.create(tmpFile.getName(), Metadata(), chunkSize, compressionLevel)
        self.assertIsNotNone(dataset)

        kExpectedSurfaceType = BAG_SURFACE_GRID_EXTENTS
        kExpectedNumCorrectors = 2
        corrections = dataset.createSurfaceCorrections(kExpectedSurfaceType, kExpectedNumCorrectors,
                                                       chunkSize, compressionLevel)
        self.assertIsNotNone(corrections)

        descriptor = corrections.getDescriptor()

        self.assertEqual(descriptor.getNumCorrectors(), kExpectedNumCorrectors)
        self.assertEqual(descriptor.getSurfaceType(), kExpectedSurfaceType)
        self.assertEqual(descriptor.getVerticalDatums(), "")
        self.assertEqual(descriptor.getOrigin(), (0.0, 0.0))
        self.assertEqual(descriptor.getSpacing(), (0.0, 0.0))

        del dataset #ensure dataset is deleted before tmpFile

    def testCreateWriteIrregular(self):
        tmpFile = testUtils.RandomFileGuard("name")

        dataset = Dataset.create(tmpFile.getName(), Metadata(), chunkSize, compressionLevel)
        self.assertIsNotNone(dataset)

        kExpectedSurfaceType = BAG_SURFACE_IRREGULARLY_SPACED
        kExpectedNumCorrectors = 2

        corrections = dataset.createSurfaceCorrections(kExpectedSurfaceType, kExpectedNumCorrectors,
                                                       chunkSize, compressionLevel)
        self.assertIsNotNone(corrections)

        descriptor = corrections.getDescriptor()
        self.assertIsNotNone(descriptor)

        # Write a record.
        kExpectedItem0 = BagVerticalDatumCorrections(1.2, 2.1, (3.4, 4.5))

        kRowStart = 0
        kColumnStart = 0
        kRowEnd = 0
        kColumnEnd = 0

        items = SurfaceCorrectionsLayerItems((kExpectedItem0,))
        corrections.write(kRowStart, kColumnStart, kRowEnd, kColumnEnd, items)

        # Read the record back.
        result = corrections.read(kRowStart, kColumnStart, kRowEnd, kColumnEnd)
        self.assertIsNotNone(result)

        correctionsResult = result.asVerticalDatumCorrections()
        self.assertEqual(len(correctionsResult), 1)

        self.assertAlmostEqual(correctionsResult[0].x, kExpectedItem0.x, places=5)
        self.assertAlmostEqual(correctionsResult[0].y, kExpectedItem0.y, places=5)

        actualZValues = correctionsResult[0].zValues()
        expectedZValues = kExpectedItem0.zValues()

        for actual, expected in zip(actualZValues, expectedZValues):
            self.assertAlmostEqual(actual, expected, places=5)

        del dataset #ensure dataset is deleted before tmpFile

    def testCreateWriteGridded(self):
        tmpFile = testUtils.RandomFileGuard("name")

        dataset = Dataset.create(tmpFile.getName(), Metadata(), chunkSize, compressionLevel)
        self.assertIsNotNone(dataset)

        kExpectedSurfaceType = BAG_SURFACE_GRID_EXTENTS
        kExpectedNumCorrectors = 3

        corrections = dataset.createSurfaceCorrections(kExpectedSurfaceType, kExpectedNumCorrectors,
                                                       chunkSize, compressionLevel)
        self.assertIsNotNone(corrections)

        descriptor = corrections.getDescriptor()
        self.assertIsNotNone(descriptor)

        # Write a record.
        kExpectedItem0 = BagVerticalDatumCorrectionsGridded((9.87, 6.543, 2.109876))

        kRowStart = 0
        kColumnStart = 0
        kRowEnd = 0
        kColumnEnd = 0

        items = SurfaceCorrectionsGriddedLayerItems((kExpectedItem0,))
        corrections.write(kRowStart, kColumnStart, kRowEnd, kColumnEnd, items)

        # Read the record back.
        result = corrections.read(kRowStart, kColumnStart, kRowEnd, kColumnEnd)
        self.assertIsNotNone(result)

        correctionsResult = result.asVerticalDatumCorrectionsGridded()
        self.assertEqual(len(correctionsResult), 1)

        actualZValues = correctionsResult[0].zValues()
        expectedZValues = kExpectedItem0.zValues()

        for actual, expected in zip(actualZValues, expectedZValues):
            self.assertAlmostEqual(actual, expected, places=5)

        del dataset #ensure dataset is deleted before tmpFile

    def testCreateWriteTwoGridded(self):
        tmpFile = testUtils.RandomFileGuard("name")

        kRowStart = 0
        kColumnStart = 0
        kRowEnd = 0
        kColumnEnd = 1

        kExpectedNumCorrectors = 3

        kExpectedItems = (BagVerticalDatumCorrectionsGridded((1.23, 4.56, 7.89)),
                          BagVerticalDatumCorrectionsGridded((9.87, 6.54, 3.21)))

        # Write the surface corrections.
        metadata = Metadata()
        metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)

        dataset = Dataset.create(tmpFile.getName(), metadata, chunkSize, compressionLevel)
        self.assertIsNotNone(dataset)

        kExpectedSurfaceType = BAG_SURFACE_GRID_EXTENTS

        corrections = dataset.createSurfaceCorrections(kExpectedSurfaceType,
            kExpectedNumCorrectors, chunkSize, compressionLevel)
        self.assertIsNotNone(corrections)

        descriptor = corrections.getDescriptor()
        self.assertIsNotNone(descriptor)
        self.assertEqual(descriptor.getNumCorrectors(), kExpectedNumCorrectors)
        self.assertEqual(descriptor.getSurfaceType(), kExpectedSurfaceType)

        items = SurfaceCorrectionsGriddedLayerItems(kExpectedItems)
        corrections.write(kRowStart, kColumnStart, kRowEnd, kColumnEnd, items)

        # Read the surface corrections back.
        dataset = Dataset.openDataset(tmpFile.getName(), BAG_OPEN_READONLY)
        self.assertIsNotNone(dataset)

        corrections = dataset.getSurfaceCorrections()
        self.assertIsNotNone(corrections)

        result = corrections.read(kRowStart, kColumnStart, kRowEnd, kColumnEnd)
        self.assertIsNotNone(result)

        correctionsResult = result.asVerticalDatumCorrectionsGridded()
        self.assertEqual(len(correctionsResult), 2)

        actualZValues0 = correctionsResult[0].zValues()
        expectedZValues0 = kExpectedItems[0].zValues()

        for actual, expected in zip(actualZValues0, expectedZValues0):
            self.assertAlmostEqual(actual, expected, places=5)

        actualZValues1 = correctionsResult[1].zValues()
        expectedZValues1 = kExpectedItems[1].zValues()

        for actual, expected in zip(actualZValues1, expectedZValues1):
            self.assertAlmostEqual(actual, expected, places=5)

        del dataset #ensure dataset is deleted before tmpFile

    def testReadCorrectedRow(self):
        """
        This test was thrown together from existing unit tests to
        test that readCorrectedRow can be called through python.
        The simple and corrections layers used do not match one
        another, so the returned results are of the correct type
        but have meaningless values.

        TODO: Adjust once suitable data has been found/created. This
            test just ensure that the methods can be called, and
            that results of the correct type are returned, but it
            currently returns meaningless values.

        :return:
        """
        tmpFile = testUtils.RandomFileGuard("name")

        metadata = Metadata()
        metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)

        dataset = Dataset.create(tmpFile.getName(), metadata, chunkSize, compressionLevel)
        self.assertIsNotNone(dataset)

        # Create and write float values to a simple layer.
        simpleLayer = dataset.createSimpleLayer(Average_Elevation, chunkSize, compressionLevel)
        self.assertIsNotNone(simpleLayer)
        kFloatValue = 123.456
        kExpectedNumNodes = 12
        origBuffer = (kFloatValue,) * kExpectedNumNodes
        buffer = FloatLayerItems(origBuffer)
        simpleLayer.write(1, 2, 3, 5, buffer)

        # Create and write the surface corrections layer
        kExpectedSurfaceType = BAG_SURFACE_GRID_EXTENTS
        kExpectedNumCorrectors = 3
        corrections = dataset.createSurfaceCorrections(kExpectedSurfaceType, kExpectedNumCorrectors,
                                                       chunkSize, compressionLevel)
        self.assertIsNotNone(corrections)

        kExpectedItem0 = BagVerticalDatumCorrectionsGridded((9.87, 6.543, 2.109876))
        kRowStart = 0
        kColumnStart = 0
        kRowEnd = 0
        kColumnEnd = 0
        items = SurfaceCorrectionsGriddedLayerItems((kExpectedItem0,))
        corrections.write(kRowStart, kColumnStart, kRowEnd, kColumnEnd, items)

        # get the Simple and SurfaceCorrections Layers
        correctionsB = dataset.getSurfaceCorrections()
        self.assertIsNotNone(correctionsB)
        simpleLayerB = dataset.getSimpleLayer(Average_Elevation)
        self.assertIsNotNone(simpleLayerB)

        row = 0
        corrector = 1
        correctedData = correctionsB.readCorrectedRow(row, kColumnStart,
                                                      kColumnEnd, corrector, simpleLayerB)

        # Since the corrections layer does not match the simple layer,
        # the returned values are meaningless, but are of the correct type.
        correctedFloats = correctedData.asFloatItems()
        self.assertEqual(len(correctedFloats), 1)

        del dataset #ensure dataset is deleted before tmpFile

    def testReadCorrected(self):
        """
        This test was thrown together from existing unit tests to
        test that readCorrected can be called through python.
        The simple and corrections layers used do not match one
        another, so the returned results are of the correct type
        but have meaningless values.

        TODO: Adjust once suitable data has been found/created. This
            test just ensure that the methods can be called, and
            that results of the correct type are returned, but it
            currently returns meaningless values.
        :return:
        """
        tmpFile = testUtils.RandomFileGuard("name")

        metadata = Metadata()
        metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)

        dataset = Dataset.create(tmpFile.getName(), metadata, chunkSize, compressionLevel)
        self.assertIsNotNone(dataset)

        # Create and write float values to a simple layer.
        simpleLayer = dataset.createSimpleLayer(Average_Elevation, chunkSize, compressionLevel)
        self.assertIsNotNone(simpleLayer)
        kFloatValue = 123.456
        kExpectedNumNodes = 12
        origBuffer = (kFloatValue,) * kExpectedNumNodes
        buffer = FloatLayerItems(origBuffer)
        simpleLayer.write(1, 2, 3, 5, buffer)

        # Create and write the surface corrections layer
        kExpectedSurfaceType = BAG_SURFACE_GRID_EXTENTS
        kExpectedNumCorrectors = 3
        corrections = dataset.createSurfaceCorrections(kExpectedSurfaceType, kExpectedNumCorrectors,
                                                       chunkSize, compressionLevel)
        self.assertIsNotNone(corrections)

        kExpectedItem0 = BagVerticalDatumCorrectionsGridded((9.87, 6.543, 2.109876))
        kRowStart = 0
        kColumnStart = 0
        kRowEnd = 0
        kColumnEnd = 0
        items = SurfaceCorrectionsGriddedLayerItems((kExpectedItem0,))
        corrections.write(kRowStart, kColumnStart, kRowEnd, kColumnEnd, items)

        # get the Simple and SurfaceCorrections Layers
        correctionsB = dataset.getSurfaceCorrections()
        self.assertIsNotNone(correctionsB)
        simpleLayerB = dataset.getSimpleLayer(Average_Elevation)
        self.assertIsNotNone(simpleLayerB)

        corrector = 1
        correctedData = correctionsB.readCorrected(kRowStart, kRowEnd, kColumnStart,
                                                   kColumnEnd, corrector, simpleLayerB)

        # Since the corrections layer does not match the simple layer,
        # the returned values are meaningless, but are of the correct type.
        correctedFloats = correctedData.asFloatItems()
        self.assertIsNotNone(len(correctedFloats) == 1)

        del dataset #ensure dataset is deleted before tmpFile

if __name__ == '__main__':
    unittest.main(
        testRunner=xmlrunner.XMLTestRunner(output='test-reports'),
        failfast=False, buffer=False, catchbreak=False
    )
