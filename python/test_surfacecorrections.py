from bagPy import *
import shutil, pathlib, math
import bagMetadataSamples, testUtils
import sys


# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "\..\examples\sample-data"
chunkSize = 100
compressionLevel = 6


# define the unit test methods:

def testReadIrregular():
    bagFileName = datapath + "\sample.bag"
    #print(bagFileName)
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
    #print(tmpFile.getName())

    dataset = Dataset.create(tmpFile.getName(), Metadata(), chunkSize, compressionLevel)
    assert(dataset)

    kExpectedSurfaceType = BAG_SURFACE_IRREGULARLY_SPACED
    kExpectedNumCorrectors = 4
    corrections = dataset.createSurfaceCorrections(
        kExpectedSurfaceType, kExpectedNumCorrectors, chunkSize, compressionLevel)

    descriptor = corrections.getDescriptor()

    assert(descriptor.getNumCorrectors() == kExpectedNumCorrectors)    
    assert(descriptor.getSurfaceType() == kExpectedSurfaceType)
    assert(descriptor.getVerticalDatums() == "")
    assert(descriptor.getOrigin() == (0.0, 0.0))
    assert(descriptor.getSpacing() == (0.0, 0.0))

    del dataset #ensure dataset is deleted before tmpFile

def testCreateGridded():
    tmpFile = testUtils.RandomFileGuard("name")
    #print(tmpFile.getName())

    dataset = Dataset.create(tmpFile.getName(), Metadata(), chunkSize, compressionLevel)
    assert(dataset)

    kExpectedSurfaceType = BAG_SURFACE_GRID_EXTENTS
    kExpectedNumCorrectors = 2
    corrections = dataset.createSurfaceCorrections(
        kExpectedSurfaceType, kExpectedNumCorrectors, chunkSize, compressionLevel)
    
    descriptor = corrections.getDescriptor()

    assert(descriptor.getNumCorrectors() == kExpectedNumCorrectors)    
    assert(descriptor.getSurfaceType() == kExpectedSurfaceType)
    assert(descriptor.getVerticalDatums() == "")
    assert(descriptor.getOrigin() == (0.0, 0.0))
    assert(descriptor.getSpacing() == (0.0, 0.0))

    del dataset #ensure dataset is deleted before tmpFile

def testCreateWriteIrregular():
    tmpFile = testUtils.RandomFileGuard("name")
    #print(tmpFile.getName())

    dataset = Dataset.create(tmpFile.getName(), Metadata(), chunkSize, compressionLevel)
    assert(dataset)

    kExpectedSurfaceType = BAG_SURFACE_IRREGULARLY_SPACED
    kExpectedNumCorrectors = 2
    corrections = dataset.createSurfaceCorrections(
        kExpectedSurfaceType, kExpectedNumCorrectors, chunkSize, compressionLevel)

    descriptor = corrections.getDescriptor()

    kExpectedItem0 = BagVerticalDatumCorrections(1.2, 2.1, [3.4, 4.5])
    print(kExpectedItem0.x)
    print(kExpectedItem0.y)
    print(kExpectedItem0.z)
    zvals = kExpectedItem0.getZValues()
    
    print(zvals[0])
    print(zvals[1])
    #uint8Buffer = kExpectedItem0.toUInt8();

    kRowStart = 0
    kColumnStart = 0
    kRowEnd = 0
    kColumnEnd = 0
    corrections.write2(kRowStart, kColumnStart, kRowEnd, kColumnEnd, LayerItem(kExpectedItem0)) #uint8Buffer

    result = corrections.read2(kRowStart, kColumnStart, kRowEnd, kColumnEnd)
    #assert(result)

    correctionsResult = result.asBagVerticalDatumCorrections()

    print(correctionsResult.x)
    print(correctionsResult.y)

    #res = BagVerticalDatumCorrections(result) # 
    #assert(res)
    #print(res)
    #print(res.x)
    assert(correctionsResult.x == kExpectedItem0.x)
    assert(correctionsResult.y == kExpectedItem0.y)
    assert(correctionsResult.z[0] == kExpectedItem0.z[0])
    assert(correctionsResult.z[1] == kExpectedItem0.z[1])

    del dataset #ensure dataset is deleted before tmpFile


# run the unit test methods
testReadIrregular()
testCreateIrregular()
testCreateGridded()

#TODO fix read/write
#testCreateWriteIrregular()

#TODO add more tests