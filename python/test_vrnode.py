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

    # Check dataset was created successfully.
    metadata = Metadata()

    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)

    dataset = Dataset.create(tmpBagFile.getName(), metadata, chunkSize,
        compressionLevel)
    assert(dataset)

    dataset.createVR(chunkSize, compressionLevel, True)

    # Check that the optional variable resolution node layer exists.
    vrNode = dataset.getVRNode()
    assert(vrNode)

    vrNodeDescriptor = vrNode.getDescriptor()

    vrNode.writeAttributes()

    # Set some attributes to save.
    kExpectedMinHypStr = 10.1
    kExpectedMaxHypStr = 20.2

    vrNodeDescriptor.setMinMaxHypStrength(kExpectedMinHypStr,
        kExpectedMaxHypStr)

    kExpectedMinNumHyp = 100
    kExpectedMaxNumHyp = 200

    vrNodeDescriptor.setMinMaxNumHypotheses(kExpectedMinNumHyp,
        kExpectedMaxNumHyp)

    kExpectedMinNSamples = 1000
    kExpectedMaxNSamples = 2000

    vrNodeDescriptor.setMinMaxNSamples(kExpectedMinNSamples,
        kExpectedMaxNSamples)

    # Read the attributes back
    minHypStr, maxHypStr = vrNodeDescriptor.getMinMaxHypStrength()
    assert(isclose(kExpectedMinHypStr, minHypStr, abs_tol = 1e-5))
    assert(isclose(kExpectedMaxHypStr, maxHypStr, abs_tol = 1e-5))

    minNumHyp, maxNumHyp = vrNodeDescriptor.getMinMaxNumHypotheses()
    assert(kExpectedMinNumHyp == minNumHyp)
    assert(kExpectedMaxNumHyp == maxNumHyp)

    minNSamples, maxNSamples = vrNodeDescriptor.getMinMaxNSamples()
    assert(kExpectedMinNSamples == minNSamples)
    assert(kExpectedMaxNSamples == maxNSamples)

    # Write attributes to file.
    vrNode.writeAttributes()

    # Force a close.
    del dataset

    # test open
    dataset = Dataset.openDataset(tmpBagFile.getName(), BAG_OPEN_READONLY)

    # Check that the optional variable resolution node layer exists.
    vrNode = dataset.getVRNode()
    assert(vrNode)

    vrNodeDescriptor = vrNode.getDescriptor()

    # Read the attributes back.
    minHypStr, maxHypStr = vrNodeDescriptor.getMinMaxHypStrength()
    assert(isclose(kExpectedMinHypStr, minHypStr, abs_tol = 1e-5))
    assert(isclose(kExpectedMaxHypStr, maxHypStr, abs_tol = 1e-5))

    minNumHyp, maxNumHyp = vrNodeDescriptor.getMinMaxNumHypotheses()
    assert(kExpectedMinNumHyp == minNumHyp)
    assert(kExpectedMaxNumHyp == maxNumHyp)

    minNSamples, maxNSamples = vrNodeDescriptor.getMinMaxNSamples()
    assert(kExpectedMinNSamples == minNSamples)
    assert(kExpectedMaxNSamples == maxNSamples)

    # Force a close.
    del dataset

def testWriteRead():
    tmpBagFile = testUtils.RandomFileGuard("name")

    # Check dataset was created successfully.
    metadata = Metadata()

    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)

    dataset = Dataset.create(tmpBagFile.getName(), metadata, chunkSize,
        compressionLevel)
    assert(dataset)

    dataset.createVR(chunkSize, compressionLevel, True)

    # Check the optional variable resolution node layer exists.
    vrNode = dataset.getVRNode()
    assert(vrNode)

    # Write one record.
    kExpectedItem0 = BagVRNodeItem(123.456, 42, 1701);

    #const auto* buffer = reinterpret_cast<const uint8_t*>(&kExpectedItem0);
    #TODO use UInt8Array ???
    # convert BagVRNodeItem -> array/tuple of uint8_t*

    kRowStart = 0  # not used
    kColumnStart = 0
    kRowEnd = 0  # not used
    kColumnEnd = 0

    #TODO read/write fix in the works
    #vrNode.write(kRowStart, kColumnStart, kRowEnd, kColumnEnd, buffer)

    # Read the record back.
    #TODO read/write fix in the works
    #buffer = vrNode.read(kRowStart, kColumnStart, kRowEnd, kColumnEnd)
    assert(buffer)

    #const auto* res = reinterpret_cast<const BagVRNodeItem*>(buffer.data());
    #TODO use UInt8Array ???
    # convert uint8_t* -> array/tuple of BagVRNodeItem

    # Check the expected value of BagVRNodeItem::hyp_strength.
    assert(res.hyp_strength == kExpectedItem0.hyp_strength)

    # Check the expected value of BagVRNodeItem::num_hypotheses.
    assert(res.num_hypotheses == kExpectedItem0.num_hypotheses)

    # Check the expected value of BagVRNodeItem::n_samples.
    assert(res.n_samples == kExpectedItem0.n_samples)

    # Force a close.
    del dataset


testCreateOpen()
testWriteRead()

