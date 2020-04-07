from bagPy import *
import shutil, pathlib
import bagMetadataSamples, testUtils
import sys

#include "test_utils.h"
#include <bag_dataset.h>
#include <bag_metadata.h>
#include <bag_vrmetadata.h>
#include <bag_vrmetadatadescriptor.h>

#include <catch2/catch.hpp>




# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "\..\examples\sample-data"
chunkSize = 100
compressionLevel = 6

#  std::tuple<uint32_t, uint32_t> getMaxDimensions() const noexcept;
#  std::tuple<uint32_t, uint32_t> getMinDimensions() const noexcept;
#  VrMetadataDescriptor& setMaxDimensions(uint32_t maxDimX, uint32_t maxDimY) noexcept;
#  VrMetadataDescriptor& setMinDimensions(uint32_t minDimX, uint32_t minDimY) noexcept;
def testGetMinMaxDimensions():
    tmpFile = testUtils.RandomFileGuard("name")
    #print(tmpFile.getName())
    metadata = Metadata()
    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML);

    dataset = Dataset.create(tmpFile.getName(), metadata, chunkSize, compressionLevel)
    assert(dataset)

    dataset.createVR(chunkSize, compressionLevel, False)
    vrMetadata = dataset.getVRMetadata()
    assert(vrMetadata)

    vrMetadataDescriptor = vrMetadata.getDescriptor()

    # Set some expected minimum dimension values and verify they are set.
    kExpectedMinDimX = 1
    kExpectedMinDimY = 2
    vrMetadataDescriptor.setMinDimensions(kExpectedMinDimsX, kExpectedMinDimsY)

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


# Run the tests.
testGetMinMaxDimensions()

