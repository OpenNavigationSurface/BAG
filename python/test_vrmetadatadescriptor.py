from bagPy import *
from math import isclose
import shutil, pathlib
import bagMetadataSamples, testUtils
import sys

# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "/../examples/sample-data"
chunkSize = 100
compressionLevel = 6

print("Testing VRMetadataDescriptor")

def testGetMinMaxDimensions():
    tmpFile = testUtils.RandomFileGuard("name")

    metadata = Metadata()
    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML);

    dataset = Dataset.create(tmpFile.getName(), metadata, chunkSize,
        compressionLevel)
    assert(dataset)

    dataset.createVR(chunkSize, compressionLevel, False)

    vrMetadata = dataset.getVRMetadata()
    assert(vrMetadata)

    vrMetadataDescriptor = vrMetadata.getDescriptor()

    # Set some expected minimum dimension values and verify they are set.
    kExpectedMinDimX = 1
    kExpectedMinDimY = 2

    vrMetadataDescriptor.setMinDimensions(kExpectedMinDimX, kExpectedMinDimY)

    # Verify the min X and Y dimensions are expected.
    minDimX, minDimY = vrMetadataDescriptor.getMinDimensions()
    assert(minDimX == kExpectedMinDimX)
    assert(minDimY == kExpectedMinDimY)

    # Set some expected maximum dimension values and verify they are set.
    kExpectedMaxDimX = 101
    kExpectedMaxDimY = 202

    vrMetadataDescriptor.setMaxDimensions(kExpectedMaxDimX, kExpectedMaxDimY)

    # Verify the max X and Y dimensions are expected.
    maxDimX, maxDimY = vrMetadataDescriptor.getMaxDimensions()
    assert(maxDimX == kExpectedMaxDimX)
    assert(maxDimY == kExpectedMaxDimY)

    # Force a close.
    del dataset

def testGetSetMinMaxResolution():
    tmpFile = testUtils.RandomFileGuard("name")

    metadata = Metadata()
    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML);

    dataset = Dataset.create(tmpFile.getName(), metadata, chunkSize,
        compressionLevel)
    assert(dataset)

    dataset.createVR(chunkSize, compressionLevel, False)

    vrMetadata = dataset.getVRMetadata()
    assert(vrMetadata)

    vrMetadataDescriptor = vrMetadata.getDescriptor()

    # Set some expected minimum resolution values and verify they are set.
    kExpectedMinResX = 1.1
    kExpectedMinResY = 2.2

    vrMetadataDescriptor.setMinResolution(kExpectedMinResX, kExpectedMinResY)

    # Verify the min X and Y resolution is expected.
    minResX, minResY = vrMetadataDescriptor.getMinResolution();
    assert(isclose(kExpectedMinResX, minResX, abs_tol = 1e-5))
    assert(isclose(kExpectedMinResY, minResY, abs_tol = 1e-5))

    # Set some expected maximum resolution values and verify they are set.
    kExpectedMaxResX = 444.4
    kExpectedMaxResY = 555.5

    vrMetadataDescriptor.setMaxResolution(kExpectedMaxResX, kExpectedMaxResY)

    # Verify the max X and Y resolution is expected.
    maxResX, maxResY = vrMetadataDescriptor.getMaxResolution()
    assert(isclose(kExpectedMaxResX, maxResX, abs_tol = 1e-5))
    assert(isclose(kExpectedMaxResY, maxResY, abs_tol = 1e-5))

    # Force a close.
    del dataset


# Run the tests.
testGetMinMaxDimensions()
testGetSetMinMaxResolution()

