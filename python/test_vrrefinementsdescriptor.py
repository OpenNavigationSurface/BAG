from bagPy import *
from math import isclose
import shutil, pathlib
import bagMetadataSamples, testUtils
import sys

# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "/../examples/sample-data"
chunkSize = 100
compressionLevel = 6


print("Testing VRRefinementsDescriptor")

def testGetSetMinMaxDepth():
    tmpBagFile = testUtils.RandomFileGuard("name")

    metadata = Metadata()

    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)

    dataset = Dataset.create(tmpBagFile.getName(), metadata, chunkSize,
        compressionLevel)
    assert(dataset)

    dataset.createVR(chunkSize, compressionLevel, False)

    vrRefinements = dataset.getVRRefinements();
    assert(vrRefinements);

    vrRefinementDescriptor = vrRefinements.getDescriptor()

    # Set some expected minimum depth values and verify they are set.
    # Verify setting the min/max depth does not throw.
    kExpectedMinDepth = 1.1
    kExpectedMaxDepth = 2.2

    vrRefinementDescriptor.setMinMaxDepth(kExpectedMinDepth, kExpectedMaxDepth)

    # Verify the min/max depth is expected.
    minDepth, maxDepth = vrRefinementDescriptor.getMinMaxDepth()
    assert(isclose(kExpectedMinDepth, minDepth, abs_tol = 1e-5))
    assert(isclose(kExpectedMaxDepth, maxDepth, abs_tol = 1e-5))

    # Force a close.
    del dataset

def testGetSetMinMaxUncertainty():
    tmpBagFile = testUtils.RandomFileGuard("name")

    metadata = Metadata()

    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)

    dataset = Dataset.create(tmpBagFile.getName(), metadata, chunkSize,
        compressionLevel)
    assert(dataset)

    dataset.createVR(chunkSize, compressionLevel, False)

    vrRefinements = dataset.getVRRefinements()
    assert(vrRefinements)

    vrRefinementDescriptor = vrRefinements.getDescriptor()

    # Set some expected min/max uncertainty values and verify they are set.
    # Verify setting the min/max uncertainty does not throw.
    kExpectedMinUncert = 101.01
    kExpectedMaxUncert = 202.02

    vrRefinementDescriptor.setMinMaxUncertainty(kExpectedMinUncert,
        kExpectedMaxUncert)

    # Verify the min/max uncertainty is expected.
    minUncert, maxUncert = vrRefinementDescriptor.getMinMaxUncertainty()
    assert(isclose(kExpectedMinUncert, minUncert, abs_tol = 1e-5))
    assert(isclose(kExpectedMaxUncert, maxUncert, abs_tol = 1e-5))

    # Force a close.
    del dataset


testGetSetMinMaxDepth()
testGetSetMinMaxUncertainty()

