from bagPy import *
from math import isclose
import shutil, pathlib
import bagMetadataSamples, testUtils
import sys


# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "/../examples/sample-data"
chunkSize = 100
compressionLevel = 6


# define the unit test methods:
print("Testing SurfaceCorrections")

def testReadIrregular():
    bagFileName = datapath + "/sample.bag"

    dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READONLY)
    assert(dataset)

    corrections = dataset.getSurfaceCorrections()
    assert(corrections)

    descriptor = corrections.getDescriptor()
    assert(descriptor.getSurfaceType() == BAG_SURFACE_IRREGULARLY_SPACED)

    kExpectedNumCorrectors = 2
    assert(descriptor.getNumCorrectors() == kExpectedNumCorrectors)

def testCreateIrregular():
    tmpFile = testUtils.RandomFileGuard("name")

    dataset = Dataset.create(tmpFile.getName(), Metadata(), chunkSize, compressionLevel)
    assert(dataset)

    kExpectedSurfaceType = BAG_SURFACE_IRREGULARLY_SPACED
    kExpectedNumCorrectors = 4
    corrections = dataset.createSurfaceCorrections(
        kExpectedSurfaceType, kExpectedNumCorrectors, chunkSize, compressionLevel)
    assert(corrections)

    descriptor = corrections.getDescriptor()

    assert(descriptor.getNumCorrectors() == kExpectedNumCorrectors)
    assert(descriptor.getSurfaceType() == kExpectedSurfaceType)
    assert(descriptor.getVerticalDatums() == "")
    assert(descriptor.getOrigin() == (0.0, 0.0))
    assert(descriptor.getSpacing() == (0.0, 0.0))

    del dataset #ensure dataset is deleted before tmpFile

def testCreateGridded():
    tmpFile = testUtils.RandomFileGuard("name")

    dataset = Dataset.create(tmpFile.getName(), Metadata(), chunkSize, compressionLevel)
    assert(dataset)

    kExpectedSurfaceType = BAG_SURFACE_GRID_EXTENTS
    kExpectedNumCorrectors = 2
    corrections = dataset.createSurfaceCorrections(
        kExpectedSurfaceType, kExpectedNumCorrectors, chunkSize, compressionLevel)
    assert(corrections)

    descriptor = corrections.getDescriptor()

    assert(descriptor.getNumCorrectors() == kExpectedNumCorrectors)
    assert(descriptor.getSurfaceType() == kExpectedSurfaceType)
    assert(descriptor.getVerticalDatums() == "")
    assert(descriptor.getOrigin() == (0.0, 0.0))
    assert(descriptor.getSpacing() == (0.0, 0.0))

    del dataset #ensure dataset is deleted before tmpFile

def testCreateWriteIrregular():
    tmpFile = testUtils.RandomFileGuard("name")

    dataset = Dataset.create(tmpFile.getName(), Metadata(), chunkSize, compressionLevel)
    assert(dataset)

    kExpectedSurfaceType = BAG_SURFACE_IRREGULARLY_SPACED
    kExpectedNumCorrectors = 2

    corrections = dataset.createSurfaceCorrections(kExpectedSurfaceType,
        kExpectedNumCorrectors, chunkSize, compressionLevel)
    assert(corrections)

    descriptor = corrections.getDescriptor()
    assert(descriptor)

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
    assert(result)

    correctionsResult = result.asVerticalDatumCorrections()
    assert(len(correctionsResult) == 1)

    assert(isclose(correctionsResult[0].x, kExpectedItem0.x, abs_tol = 1e-5))
    assert(isclose(correctionsResult[0].y, kExpectedItem0.y, abs_tol = 1e-5))

    actualZValues = correctionsResult[0].zValues()
    expectedZValues = kExpectedItem0.zValues()

    assert(all(isclose(actual, expected, abs_tol = 1e-5)
        for actual, expected in zip(actualZValues, expectedZValues)))

    del dataset #ensure dataset is deleted before tmpFile


def testCreateWriteGridded():
    tmpFile = testUtils.RandomFileGuard("name")
    #print(tmpFile.getName())

    dataset = Dataset.create(tmpFile.getName(), Metadata(), chunkSize, compressionLevel)
    assert(dataset)

    kExpectedSurfaceType = BAG_SURFACE_GRID_EXTENTS
    kExpectedNumCorrectors = 3

    corrections = dataset.createSurfaceCorrections(kExpectedSurfaceType, 
        kExpectedNumCorrectors, chunkSize, compressionLevel)
    assert(corrections)

    descriptor = corrections.getDescriptor()
    assert(descriptor)

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
    assert(result)

    correctionsResult = result.asVerticalDatumCorrectionsGridded()
    assert(len(correctionsResult) == 1)

    actualZValues = correctionsResult[0].zValues()
    expectedZValues = kExpectedItem0.zValues()
    
    assert(all(isclose(actual, expected, abs_tol = 1e-5)
        for actual, expected in zip(actualZValues, expectedZValues)))

    del dataset #ensure dataset is deleted before tmpFile


def testCreateWriteTwoGridded():
    tmpFile = testUtils.RandomFileGuard("name")
    #print(tmpFile.getName())

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
    assert(dataset)

    kExpectedSurfaceType = BAG_SURFACE_GRID_EXTENTS

    corrections = dataset.createSurfaceCorrections(kExpectedSurfaceType, 
        kExpectedNumCorrectors, chunkSize, compressionLevel)
    assert(corrections)

    descriptor = corrections.getDescriptor()
    assert(descriptor)
    assert(descriptor.getNumCorrectors() == kExpectedNumCorrectors)
    assert(descriptor.getSurfaceType() == kExpectedSurfaceType)

    items = SurfaceCorrectionsGriddedLayerItems(kExpectedItems)
    corrections.write(kRowStart, kColumnStart, kRowEnd, kColumnEnd, items)


    # Read the surface corrections back.
    dataset = Dataset.openDataset(tmpFile.getName(), BAG_OPEN_READONLY)
    assert(dataset)

    corrections = dataset.getSurfaceCorrections()
    assert(corrections)

    result = corrections.read(kRowStart, kColumnStart, kRowEnd, kColumnEnd)
    assert(result)

    correctionsResult = result.asVerticalDatumCorrectionsGridded()
    assert(len(correctionsResult) == 2)

    actualZValues0 = correctionsResult[0].zValues()
    expectedZValues0 = kExpectedItems[0].zValues()
    
    assert(all(isclose(actual, expected, abs_tol = 1e-5)
        for actual, expected in zip(actualZValues0, expectedZValues0)))

    actualZValues1 = correctionsResult[1].zValues()
    expectedZValues1 = kExpectedItems[1].zValues()
    
    assert(all(isclose(actual, expected, abs_tol = 1e-5)
        for actual, expected in zip(actualZValues1, expectedZValues1)))

    del dataset #ensure dataset is deleted before tmpFile


# This test was thrown together from existing unit tests to 
# test that readCorrectedRow can be called through python.
# The simple and corrections layers used do not match one 
# another, so the returned results are of the correct type 
# but have meaningless values.
def testReadCorrectedRow():
    tmpFile = testUtils.RandomFileGuard("name")

    metadata = Metadata()
    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)

    dataset = Dataset.create(tmpFile.getName(), metadata, chunkSize, compressionLevel)
    assert(dataset)

    # Create and write float values to a simple layer.
    simpleLayer = dataset.createSimpleLayer(Average_Elevation, chunkSize, compressionLevel)
    assert(simpleLayer)
    kFloatValue = 123.456
    kExpectedNumNodes = 12
    origBuffer = (kFloatValue,) * kExpectedNumNodes
    buffer = FloatLayerItems(origBuffer)
    simpleLayer.write(1, 2, 3, 5, buffer)

    # Create and write the surface corrections layer
    kExpectedSurfaceType = BAG_SURFACE_GRID_EXTENTS
    kExpectedNumCorrectors = 3
    corrections = dataset.createSurfaceCorrections(kExpectedSurfaceType,
        kExpectedNumCorrectors, chunkSize, compressionLevel)
    assert(corrections)

    kExpectedItem0 = BagVerticalDatumCorrectionsGridded((9.87, 6.543, 2.109876))
    kRowStart = 0
    kColumnStart = 0
    kRowEnd = 0
    kColumnEnd = 0
    items = SurfaceCorrectionsGriddedLayerItems((kExpectedItem0,))
    corrections.write(kRowStart, kColumnStart, kRowEnd, kColumnEnd, items)

    # get the Simple and SurfaceCorrections Layers
    correctionsB = dataset.getSurfaceCorrections()
    assert(correctionsB)
    simpleLayerB = dataset.getSimpleLayer(Average_Elevation)
    assert(simpleLayerB)

    row = 0
    corrector = 1
    correctedData = correctionsB.readCorrectedRow(row,
        kColumnStart, kColumnEnd, corrector, simpleLayerB)


    # Since the corrections layer does not match the simple layer,
    # the returned values are meaningless, but are of the correct type.
    correctedFloats = correctedData.asFloatItems()
    assert(len(correctedFloats) == 1)

    del dataset #ensure dataset is deleted before tmpFile


# This test was thrown together from existing unit tests to 
# test that readCorrected can be called through python.
# The simple and corrections layers used do not match one 
# another, so the returned results are of the correct type 
# but have meaningless values.
def testReadCorrected():
    tmpFile = testUtils.RandomFileGuard("name")

    metadata = Metadata()
    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)

    dataset = Dataset.create(tmpFile.getName(), metadata, chunkSize, compressionLevel)
    assert(dataset)

    # Create and write float values to a simple layer.
    simpleLayer = dataset.createSimpleLayer(Average_Elevation, chunkSize, compressionLevel)
    assert(simpleLayer)
    kFloatValue = 123.456
    kExpectedNumNodes = 12
    origBuffer = (kFloatValue,) * kExpectedNumNodes
    buffer = FloatLayerItems(origBuffer)
    simpleLayer.write(1, 2, 3, 5, buffer)

    # Create and write the surface corrections layer
    kExpectedSurfaceType = BAG_SURFACE_GRID_EXTENTS
    kExpectedNumCorrectors = 3
    corrections = dataset.createSurfaceCorrections(kExpectedSurfaceType,
        kExpectedNumCorrectors, chunkSize, compressionLevel)
    assert(corrections)

    kExpectedItem0 = BagVerticalDatumCorrectionsGridded((9.87, 6.543, 2.109876))
    kRowStart = 0
    kColumnStart = 0
    kRowEnd = 0
    kColumnEnd = 0
    items = SurfaceCorrectionsGriddedLayerItems((kExpectedItem0,))
    corrections.write(kRowStart, kColumnStart, kRowEnd, kColumnEnd, items)

    # get the Simple and SurfaceCorrections Layers
    correctionsB = dataset.getSurfaceCorrections()
    assert(correctionsB)
    simpleLayerB = dataset.getSimpleLayer(Average_Elevation)
    assert(simpleLayerB)

    corrector = 1
    correctedData = correctionsB.readCorrected(kRowStart, kRowEnd,
        kColumnStart, kColumnEnd, corrector, simpleLayerB)


    # Since the corrections layer does not match the simple layer,
    # the returned values are meaningless, but are of the correct type.
    correctedFloats = correctedData.asFloatItems()
    assert(len(correctedFloats) == 1)

    del dataset #ensure dataset is deleted before tmpFile


# run the unit test methods
testReadIrregular()
testCreateIrregular()
testCreateGridded()
testCreateWriteIrregular()
testCreateWriteGridded()
testCreateWriteTwoGridded()

#TODO: adjust these readCorrected() tests once suitable data 
# has been found/created. These tests just ensure that the 
# methods can be called, and that results of the correct type
# are returned, but they currently return meaningless values.
testReadCorrectedRow()
testReadCorrected()
