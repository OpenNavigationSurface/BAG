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

    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)

    dataset = Dataset.create(tmpBagFile.getName(), metadata, chunkSize,
        compressionLevel)
    assert(dataset)

    dataset.createVR(chunkSize, compressionLevel, False)

    # Check that the optional variable resolution refinement layer exists.
    vrRefinements = dataset.getVRRefinements()
    assert(vrRefinements)

    vrRefinements.writeAttributes()

    vrRefinementsDescriptor = vrRefinements.getDescriptor()

    # Set some attributes to save.
    kExpectedMinDepth = 10.1
    kExpectedMaxDepth = 20.2

    vrRefinementsDescriptor.setMinMaxDepth(kExpectedMinDepth, kExpectedMaxDepth)

    kExpectedMinUncertainty = 111.1
    kExpectedMaxUncertainty = 222.2

    vrRefinementsDescriptor.setMinMaxUncertainty(kExpectedMinUncertainty,
        kExpectedMaxUncertainty)

    # Read the attributes back.
    minDepth, maxDepth = vrRefinementsDescriptor.getMinMaxDepth()
    assert(isclose(kExpectedMinDepth, minDepth, abs_tol = 1e-5))
    assert(isclose(kExpectedMaxDepth, maxDepth, abs_tol = 1e-5))

    minUncertainty, maxUncertainty = vrRefinementsDescriptor.getMinMaxUncertainty()
    assert(isclose(kExpectedMinUncertainty, minUncertainty, abs_tol = 1e-5))
    assert(isclose(kExpectedMaxUncertainty, maxUncertainty, abs_tol = 1e-5))

    # Write attributes to file.
    vrRefinements.writeAttributes()

    # Force a close.
    del dataset

    # test open
    dataset = Dataset.openDataset(tmpBagFile.getName(), BAG_OPEN_READONLY)

    # Check that the optional variable resolution refinement layer exists.
    vrRefinements = dataset.getVRRefinements()
    assert(vrRefinements)

    vrRefinementsDescriptor = vrRefinements.getDescriptor();

    # Read the attributes back.
    minDepth, maxDepth = vrRefinementsDescriptor.getMinMaxDepth()
    assert(isclose(kExpectedMinDepth, minDepth, abs_tol = 1e-5))
    assert(isclose(kExpectedMaxDepth, maxDepth, abs_tol = 1e-5))

    minUncertainty, maxUncertainty = vrRefinementsDescriptor.getMinMaxUncertainty()
    assert(isclose(kExpectedMinUncertainty, minUncertainty, abs_tol = 1e-5))
    assert(isclose(kExpectedMaxUncertainty, maxUncertainty, abs_tol = 1e-5))

    # Force a close.
    del dataset

def testWriteRead():
    tmpBagFile = testUtils.RandomFileGuard("name")

    metadata = Metadata()

    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)

    dataset = Dataset.create(tmpBagFile.getName(), metadata, chunkSize,
        compressionLevel)
    assert(dataset)

    dataset.createVR(chunkSize, compressionLevel, False)

    # Check the variable resolution refinement layer exists.
    vrRefinements = dataset.getVRRefinements()
    assert(vrRefinements)

    # "Write one record.
    kExpectedItem0 = BagVRRefinementsItem(9.8, 0.654)

    kRowStart = 0  # unused
    kColumnStart = 0
    kRowEnd = 0  # unused
    kColumnEnd = 0

    #TODO rework this if possible.
    items = BagVRRefinementsItems((kExpectedItem0,))
    buffer = VRRefinementsLayerItems(items)  # LayerItem((kExpectedItem0,))
    vrRefinements.write2(kRowStart, kColumnStart, kRowEnd, kColumnEnd, buffer)

    # Read the record back.
    buffer = vrRefinements.read2(kRowStart, kColumnStart, kRowEnd, kColumnEnd)
    assert(buffer);

    result = buffer.asVRRefinementsItems()
    assert(len(result) == 1)

    # Check the expected value of VRRefinementItem::depth.
    assert(result[0].depth == kExpectedItem0.depth)

    # Check the expected value of VRRefinementItem::depth_uncrt.
    assert(result[0].depth_uncrt == kExpectedItem0.depth_uncrt)

    # Force a close.
    del dataset


testCreateOpen()
testWriteRead()

