from bagPy import *
import shutil, pathlib
import bagMetadataSamples, testUtils


# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "\..\examples\sample-data"
chunkSize = 100
compressionLevel = 6


# define the unit test methods:

def testCreate():
    tmpFile = testUtils.RandomFileGuard("name")
    metadata = Metadata()
    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)
    assert(metadata)

    dataset = Dataset.create(tmpFile.getName(), metadata, chunkSize, compressionLevel)
    assert(dataset)

    descriptor = InterleavedLayerDescriptor.create(Hypothesis_Strength,
        NODE, chunkSize, compressionLevel, dataset)
    assert(descriptor)

    del dataset #ensure dataset is deleted before tmpFile

def testGetGroupType():
    tmpFile = testUtils.RandomFileGuard("name")
    metadata = Metadata()
    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)
    assert(metadata)

    dataset = Dataset.create(tmpFile.getName(), metadata, chunkSize, compressionLevel)
    assert(dataset)

    kExpectedGroup = NODE
    descriptor = InterleavedLayerDescriptor.create(Hypothesis_Strength,
        kExpectedGroup, chunkSize, compressionLevel, dataset)
    assert(descriptor)
    assert(descriptor.getGroupType() == kExpectedGroup)

    kExpectedGroup = ELEVATION
    descriptor = InterleavedLayerDescriptor.create(Num_Soundings,
        kExpectedGroup, chunkSize, compressionLevel, dataset)
    assert(descriptor)
    assert(descriptor.getGroupType() == kExpectedGroup)

    #TODO add assert that the interleaved layer creation throws with an invalid group and layer combination.
    #InterleavedLayerDescriptor.create(Hypothesis_Strength,
    #   ELEVATION, chunkSize, compressionLevel, dataset)

    del dataset #ensure dataset is deleted before tmpFile

def testElementSize():
    tmpFile = testUtils.RandomFileGuard("name")
    metadata = Metadata()
    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)
    assert(metadata)

    dataset = Dataset.create(tmpFile.getName(), metadata, chunkSize, compressionLevel)
    assert(dataset)

    kExpectedLayerType = Num_Hypotheses
    kExpectedGroupType = NODE
    descriptor = InterleavedLayerDescriptor.create(kExpectedLayerType,
        kExpectedGroupType, chunkSize, compressionLevel, dataset)
    assert(descriptor)
    assert(descriptor.getElementSize() ==
        Layer.getElementSize(Layer.getDataType(kExpectedLayerType)))

    kExpectedLayerType = Std_Dev
    kExpectedGroupType = ELEVATION
    descriptor = InterleavedLayerDescriptor.create(kExpectedLayerType,
        kExpectedGroupType, chunkSize, compressionLevel, dataset)
    assert(descriptor)
    assert(descriptor.getElementSize() ==
        Layer.getElementSize(Layer.getDataType(kExpectedLayerType)))

    del dataset #ensure dataset is deleted before tmpFile

# run the unit test methods
testCreate()
testGetGroupType()
testElementSize()