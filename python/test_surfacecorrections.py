from bagPy import *
import shutil, pathlib, math
import bagMetadataSamples, testUtils
import sys


# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "/../examples/sample-data"
chunkSize = 100
compressionLevel = 6


# define the unit test methods:

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

    assert(correctionsResult[0].x == kExpectedItem0.x)
    assert(correctionsResult[0].y == kExpectedItem0.y)
    assert(correctionsResult[0].z[0] == kExpectedItem0.z[0])
    assert(correctionsResult[0].z[1] == kExpectedItem0.z[1])

    del dataset #ensure dataset is deleted before tmpFile


# run the unit test methods
testReadIrregular()
testCreateIrregular()
testCreateGridded()
testCreateWriteIrregular()

#TODO add more tests

