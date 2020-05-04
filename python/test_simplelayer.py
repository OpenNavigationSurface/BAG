from bagPy import *
from math import isclose
import shutil, pathlib, math
import bagMetadataSamples, testUtils
import sys


# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "/../examples/sample-data"
chunkSize = 100
compressionLevel = 6


# define the unit test methods:
print("Testing SimpleLayer")

def testGetName():
    bagFileName = datapath + "/sample.bag"
    dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READONLY)
    assert(dataset)

    layer = dataset.getLayer(Elevation)
    assert(layer)
    descriptor = layer.getDescriptor()
    assert(descriptor)

    assert(descriptor.getName() == getLayerTypeAsString(Elevation))

def testRead():
    bagFileName = datapath + "/NAVO_data/JD211_public_Release_1-4_UTM.bag"
    dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READONLY)
    assert(dataset)

    kLayerType = Elevation
    elevLayer = dataset.getLayer(kLayerType)

    rowStart = 288
    rowEnd = 289
    columnStart = 249
    columnEnd = 251

    result = elevLayer.read(rowStart, columnStart, rowEnd, columnEnd) # 2x3
    assert(result)

    kExpectedNumNodes = 6

    buffer = result.asFloatItems()
    assert(len(buffer) == kExpectedNumNodes)

    rows = (rowEnd - rowStart) + 1
    columns = (columnEnd - columnStart) + 1

    kExpectedBuffer = (1000000.0, -52.161003, -52.172005,
        1000000.0, -52.177002, -52.174004)

    assert(all(isclose(actual, expected, abs_tol = 1e-5)
        for actual, expected in zip(buffer, kExpectedBuffer)))

def testWrite():
    kLayerType = Elevation
    kExpectedNumNodes = 12
    kFloatValue = 123.456
    tmpFile = testUtils.RandomFileGuard("name")

    metadata = Metadata()
    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)
    assert(metadata)

    # Create the dataset.
    dataset = Dataset.create(tmpFile.getName(), metadata, chunkSize, compressionLevel)
    assert(dataset)

    layer = dataset.getLayer(kLayerType)
    assert(layer)

    fileDims = dataset.getDescriptor().getDims()

    kExpectedRows = dataset.getMetadata().getStruct().spatialRepresentationInfo.numberOfRows
    kExpectedColumns = dataset.getMetadata().getStruct().spatialRepresentationInfo.numberOfColumns
    assert(fileDims == (kExpectedRows, kExpectedColumns))


    # Open the dataset read/write and write to it.
    dataset = Dataset.openDataset(tmpFile.getName(), BAG_OPEN_READ_WRITE)
    assert(dataset)

    elevLayer = dataset.getLayer(kLayerType);
    assert(elevLayer)

    origBuffer = (kFloatValue,) * kExpectedNumNodes
    buffer = FloatLayerItems(origBuffer)

    # Write the floats to the elevation layer.
    elevLayer.write(1, 2, 3, 5, buffer)


    # Open the dataset and read what was written.
    dataset = Dataset.openDataset(tmpFile.getName(), BAG_OPEN_READONLY)
    assert(dataset)

    elevLayer = dataset.getLayer(kLayerType)
    assert(elevLayer)

    readBuffer = elevLayer.read(1, 2, 3, 5) # 3x4
    assert(readBuffer.size() > 0)

    floatBuffer = readBuffer.asFloatItems()

    assert(all(isclose(actual, expected, abs_tol = 1e-5)
        for actual, expected in zip(floatBuffer, origBuffer)))

    del dataset #ensure dataset is deleted before tmpFile

def testWriteAttributes():
    bagFileName = datapath + "/sample.bag"
    tmpFile = testUtils.RandomFileGuard("file", bagFileName)
    kLayerType = Elevation
    kExpectedMin = 0.0
    kExpectedMax = 0.0

    # Open the dataset read/write and write to it.
    dataset = Dataset.openDataset(tmpFile.getName(), BAG_OPEN_READ_WRITE)
    assert(dataset)
    elevLayer = dataset.getLayer(kLayerType)
    assert(elevLayer)
    descriptor = elevLayer.getDescriptor()
    assert(descriptor)

    # change min & max values
    originalMinMax = descriptor.getMinMax()
    kExpectedMin = originalMinMax[0] - 12.34
    kExpectedMax = originalMinMax[1] + 56.789
    descriptor.setMinMax(kExpectedMin, kExpectedMax)
    elevLayer.writeAttributes()

    # check that written values have changed
    dataset = Dataset.openDataset(tmpFile.getName(), BAG_OPEN_READONLY)
    assert(dataset)
    elevLayer = dataset.getLayer(kLayerType)
    descriptor = elevLayer.getDescriptor()
    actualMinMax = descriptor.getMinMax()
    assert(math.isclose(actualMinMax[0], kExpectedMin, rel_tol=1e-7))
    assert(math.isclose(actualMinMax[1], kExpectedMax, rel_tol=1e-7))

    del dataset #ensure dataset is deleted before tmpFile

    
def testGetSimpleLayer():
    bagFileName = datapath + "/sample.bag"
    dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READONLY)
    assert(dataset)

    layer = dataset.getSimpleLayer(Elevation)
    assert(layer)
    descriptor = layer.getDescriptor()
    assert(descriptor)

    assert(descriptor.getName() == getLayerTypeAsString(Elevation))

# run the unit test methods
testGetName()
testRead()
testWrite()
testWriteAttributes()
testGetSimpleLayer()