from bagPy import *
from math import isclose
import shutil, pathlib
import bagMetadataSamples, testUtils
import sys

# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "/../examples/sample-data"
chunkSize = 100
compressionLevel = 6


print("Testing VRNodeDescriptor")

def testGetSetminMaxHypStr():
    tmpBagFile = testUtils.RandomFileGuard("name")

    metadata = Metadata()

    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)

    dataset = Dataset.create(tmpBagFile.getName(), metadata, chunkSize,
        compressionLevel)
    assert(dataset)

    dataset.createVR(chunkSize, compressionLevel, True)

    vrNode = dataset.getVRNode()
    assert(vrNode)

    vrNodeDescriptor = vrNode.getDescriptor()

    # Set some expected minimum hyp strength values and verify they are set.
    # Verify setting the min/max hyp strength does not throw.
    kExpectedMinHypStr = 1.1
    kExpectedMaxHypStr = 2.2

    vrNodeDescriptor.setMinMaxHypStrength(kExpectedMinHypStr, kExpectedMaxHypStr)

    # Verify the min/max hyp strength is expected.
    minHypStr, maxHypStr = vrNodeDescriptor.getMinMaxHypStrength()
    assert(isclose(kExpectedMinHypStr, minHypStr, abs_tol = 1e-5))
    assert(isclose(kExpectedMaxHypStr, maxHypStr, abs_tol = 1e-5))

    # Force a close.
    del dataset

def testGetSetNumHypotheses():
    tmpBagFile = testUtils.RandomFileGuard("name")

    metadata = Metadata()

    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)

    dataset = Dataset.create(tmpBagFile.getName(), metadata, chunkSize,
        compressionLevel)
    assert(dataset)

    dataset.createVR(chunkSize, compressionLevel, True)

    vrNode = dataset.getVRNode()
    assert(vrNode)

    vrNodeDescriptor = vrNode.getDescriptor();

    # Set some expected min/max num hypotheses values and verify they are set.
    # Verify setting the min/max num hypotheses does not throw.
    kExpectedMinNumHyp = 10
    kExpectedMaxNumHyp = 20

    vrNodeDescriptor.setMinMaxNumHypotheses(kExpectedMinNumHyp, kExpectedMaxNumHyp)

    # Verify the min/max num hypotheses is expected.
    minNumHyp, maxNumHyp = vrNodeDescriptor.getMinMaxNumHypotheses()
    assert(kExpectedMinNumHyp == minNumHyp)
    assert(kExpectedMaxNumHyp == maxNumHyp)

    # Force a close.
    del dataset

def testGetSetMinMaxNSamples():
    tmpBagFile = testUtils.RandomFileGuard("name")

    metadata = Metadata()

    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)

    dataset = Dataset.create(tmpBagFile.getName(), metadata, chunkSize,
        compressionLevel)
    assert(dataset)

    dataset.createVR(chunkSize, compressionLevel, True)

    vrNode = dataset.getVRNode()
    assert(vrNode)

    vrNodeDescriptor = vrNode.getDescriptor()

    # Set some expected min/max n samples values and verify they are set.
    # Verify setting the min/max n samples does not throw.
    kExpectedMinNSamples = 100
    kExpectedMaxNSamples = 200

    vrNodeDescriptor.setMinMaxNSamples(kExpectedMinNSamples, kExpectedMaxNSamples)

    # Verify the min/max n samples is expected.
    minNSamples, maxNSamples = vrNodeDescriptor.getMinMaxNSamples()
    assert(kExpectedMinNSamples == minNSamples)
    assert(kExpectedMaxNSamples == maxNSamples)

    # Force a close.
    del dataset


testGetSetminMaxHypStr()
testGetSetNumHypotheses()
testGetSetMinMaxNSamples()

