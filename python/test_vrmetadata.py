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

    #const auto* buffer = reinterpret_cast<const uint8_t*>(&kExpectedItem0);
    #TODO use UInt8Array ???
    # convert uint8_t* -> array/tuple of BagVRMetadataItem
    kRowStart = 0
    kColumnStart = 0
    kRowEnd = 0
    kColumnEnd = 0

    #TODO read/write fix in the works
    #vrMetadata.write(kRowStart, kColumnStart, kRowEnd, kColumnEnd, buffer)

    # Read the record back.
    #TODO read/write fix in the works
    #buffer = vrMetadata.read(kRowStart, kColumnStart, kRowEnd, kColumnEnd)
    assert(buffer)

    #result = reinterpret_cast<const BAG::VRMetadataItem*>(buffer.data());
    #TODO convert uint8_t* -> VRMetadataItem

    # Check the expected value of VRMetadataItem::index.
    assert(result.index == kExpectedItem0.index)

    # Check the expected value of VRMetadataItem::dimensions_x.
    assert(result.dimensions_x == kExpectedItem0.dimensions_x)

    # Check the expected value of VRMetadataItem::dimensions_y.
    assert(result.dimensions_y == kExpectedItem0.dimensions_y)

    # Check the expected value of VRMetadataItem::resolution_x.
    assert(isclose(result.resolution_x, kExpectedItem0.resolution_x, abs_tol = 1e-5))

    # Check the expected value of VRMetadataItem::resolution_y.
    assert(isclose(result.resolution_y, kExpectedItem0.resolution_y, abs_tol = 1e-5))

    # Check the expected value of VRMetadataItem::sw_corner_x.".
    assert(isclose(result.sw_corner_x, kExpectedItem0.sw_corner_x, abs_tol = 1e-5))

    # Check the expected value of VRMetadataItem::sw_corner_y."
    assert(isclose(result.sw_corner_y, kExpectedItem0.sw_corner_y, abs_tol = 1e-5))

    # Force a close.
    del dataset


# run the unit test methods
testCreateOpen()
testWriteRead()

