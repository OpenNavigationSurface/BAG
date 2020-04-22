from bagPy import *
from math import isclose
import shutil, pathlib
import bagMetadataSamples, testUtils
import sys

# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "/../examples/sample-data"
chunkSize = 100
compressionLevel = 6


def testCreateOpen():
    tmpBagFile = testUtils.RandomFileGuard("name")

    # test create
    metadata = Metadata()
    assert(metadata)

    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)

    dataset = Dataset.create(tmpBagFile.getName(), metadata, chunkSize,
        compressionLevel)
    assert(dataset)

    dataset.createVR(chunkSize, compressionLevel, False)

    # Check that the optional variable resolution metadata layer exists.
    vrMetadata = dataset.getVRMetadata()
    assert(vrMetadata)

    vrMetadataDescriptor = vrMetadata.getDescriptor()
    assert(vrMetadataDescriptor)

    vrMetadata.writeAttributes()

    # Set some new attribute values.
    kExpectedMinDimX = 1
    kExpectedMinDimY = 2

    vrMetadataDescriptor.setMinDimensions(kExpectedMinDimX, kExpectedMinDimY)

    kExpectedMaxDimX = 100
    kExpectedMaxDimY = 200

    vrMetadataDescriptor.setMaxDimensions(kExpectedMaxDimX, kExpectedMaxDimY)

    kExpectedMinResX = 10.1
    kExpectedMinResY = 20.2

    vrMetadataDescriptor.setMinResolution(kExpectedMinResX, kExpectedMinResY)

    kExpectedMaxResX = 111.1
    kExpectedMaxResY = 222.2

    vrMetadataDescriptor.setMaxResolution(kExpectedMaxResX, kExpectedMaxResY)

    # Read the attributes back from memory.
    minDimX, minDimY = vrMetadataDescriptor.getMinDimensions()
    assert(kExpectedMinDimX == minDimX)
    assert(kExpectedMinDimY == minDimY)

    maxDimX, maxDimY = vrMetadataDescriptor.getMaxDimensions()
    assert(kExpectedMaxDimX == maxDimX)
    assert(kExpectedMaxDimY == maxDimY)

    minResX, minResY = vrMetadataDescriptor.getMinResolution()
    assert(isclose(kExpectedMinResX, minResX, abs_tol = 1e-5))
    assert(isclose(kExpectedMinResY, minResY, abs_tol = 1e-5))

    maxResX, maxResY = vrMetadataDescriptor.getMaxResolution()
    assert(isclose(kExpectedMaxResX, maxResX, abs_tol = 1e-5))
    assert(isclose(kExpectedMaxResY, maxResY, abs_tol = 1e-5))

    # Save the new attributes.
    vrMetadata.writeAttributes()

    # Force a close
    del dataset

    # Test opening
    dataset = Dataset.openDataset(tmpBagFile.getName(), BAG_OPEN_READONLY)

    # Check that the optional variable resolution metadata layer exists.
    vrMetadata = dataset.getVRMetadata()
    assert(vrMetadata)

    vrMetadataDescriptor = vrMetadata.getDescriptor()

    # Read the attributes from the file.
    minDimX, minDimY = vrMetadataDescriptor.getMinDimensions()
    assert(kExpectedMinDimX == minDimX)
    assert(kExpectedMinDimY == minDimY)

    maxDimX, maxDimY = vrMetadataDescriptor.getMaxDimensions()
    assert(kExpectedMaxDimX == maxDimX)
    assert(kExpectedMaxDimY == maxDimY)

    minResX, minResY = vrMetadataDescriptor.getMinResolution()
    assert(isclose(kExpectedMinResX, minResX, abs_tol = 1e-5))
    assert(isclose(kExpectedMinResY, minResY, abs_tol = 1e-5))

    maxResX, maxResY = vrMetadataDescriptor.getMaxResolution()
    assert(isclose(kExpectedMaxResX, maxResX, abs_tol = 1e-5))
    assert(isclose(kExpectedMaxResY, maxResY, abs_tol = 1e-5))

    # Force a close.
    del dataset

def testWriteRead():
    tmpBagFile = testUtils.RandomFileGuard("name")

    # test create
    metadata = Metadata()
    assert(metadata)

    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)

    dataset = Dataset.create(tmpBagFile.getName(), metadata, chunkSize, compressionLevel)
    assert(dataset)

    dataset.createVR(chunkSize, compressionLevel, False)

    # Check that the optional variable resolution metadata layer exists.
    vrMetadata = dataset.getVRMetadata()
    assert(vrMetadata)

    vrMetadataDescriptor = vrMetadata.getDescriptor()
    assert(vrMetadataDescriptor)

    # Write one record.
    kExpectedItem0 = BagVRMetadataItem(0, 1, 2, 3.45, 6.789, 1001.01, 4004.004)

    kRowStart = 0
    kColumnStart = 0
    kRowEnd = 0
    kColumnEnd = 0

    buffer = VRMetadataLayerItems((kExpectedItem0,))
    vrMetadata.write(kRowStart, kColumnStart, kRowEnd, kColumnEnd, buffer)


    # Read the record back.
    buffer = vrMetadata.read(kRowStart, kColumnStart, kRowEnd, kColumnEnd)
    assert(buffer)

    result = buffer.asVRMetadataItems()
    assert(len(result) == 1)

    # Check the expected value of VRMetadataItem::index.
    assert(result[0].index == kExpectedItem0.index)

    # Check the expected value of VRMetadataItem::dimensions_x.
    assert(result[0].dimensions_x == kExpectedItem0.dimensions_x)

    # Check the expected value of VRMetadataItem::dimensions_y.
    assert(result[0].dimensions_y == kExpectedItem0.dimensions_y)

    # Check the expected value of VRMetadataItem::resolution_x.
    assert(isclose(result[0].resolution_x, kExpectedItem0.resolution_x, abs_tol = 1e-5))

    # Check the expected value of VRMetadataItem::resolution_y.
    assert(isclose(result[0].resolution_y, kExpectedItem0.resolution_y, abs_tol = 1e-5))

    # Check the expected value of VRMetadataItem::sw_corner_x.".
    assert(isclose(result[0].sw_corner_x, kExpectedItem0.sw_corner_x, abs_tol = 1e-5))

    # Check the expected value of VRMetadataItem::sw_corner_y."
    assert(isclose(result[0].sw_corner_y, kExpectedItem0.sw_corner_y, abs_tol = 1e-5))

    # Force a close.
    del dataset


# run the unit test methods
testCreateOpen()
testWriteRead()

