from bagPy import *
import shutil, pathlib, math
import bagMetadataSamples, testUtils
import sys


# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "/../examples/sample-data"
kExpectedChunkSize = 100
kExpectedCompressionLevel = 6


# define the unit test methods:
def testCreation():
    tmpFile = testUtils.RandomFileGuard("name")
    #print(tmpFile.getName())
    metadata = Metadata()
    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)
    assert(metadata)

    # Create the dataset.
    dataset = Dataset.create(tmpFile.getName(), metadata, 
        kExpectedChunkSize, kExpectedCompressionLevel)
    assert(dataset)

    descriptor = SimpleLayerDescriptor.create(dataset, Elevation,
        kExpectedChunkSize, kExpectedCompressionLevel)
    assert(descriptor)

    assert(descriptor.getLayerType() == Elevation)
    assert(descriptor.getChunkSize() == kExpectedChunkSize)
    assert(descriptor.getCompressionLevel() == kExpectedCompressionLevel)

    del dataset #ensure dataset is deleted before tmpFile


def testGetSetName():
    tmpFile = testUtils.RandomFileGuard("name")
    #print(tmpFile.getName())
    metadata = Metadata()
    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)
    assert(metadata)

    # Create the dataset.
    dataset = Dataset.create(tmpFile.getName(), metadata, 
        kExpectedChunkSize, kExpectedCompressionLevel)
    assert(dataset)

    descriptor = SimpleLayerDescriptor.create(dataset, Elevation,
        kExpectedChunkSize, kExpectedCompressionLevel)
    assert(descriptor)

    kExpectedName = "Expected Name"
    descriptor.setName(kExpectedName)
    assert(descriptor.getName() == kExpectedName)

    del dataset #ensure dataset is deleted before tmpFile

def testGetDataType():
    tmpFile = testUtils.RandomFileGuard("name")
    #print(tmpFile.getName())
    metadata = Metadata()
    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)
    assert(metadata)

    # Create the dataset.
    dataset = Dataset.create(tmpFile.getName(), metadata, 
        kExpectedChunkSize, kExpectedCompressionLevel)
    assert(dataset)

    descriptor = SimpleLayerDescriptor.create(dataset, Elevation,
        kExpectedChunkSize, kExpectedCompressionLevel)
    assert(descriptor)

    assert(descriptor.getDataType() == Layer.getDataType(Elevation))

    del dataset #ensure dataset is deleted before tmpFile

def testGetLayerType():
    tmpFile = testUtils.RandomFileGuard("name")
    #print(tmpFile.getName())
    metadata = Metadata()
    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)
    assert(metadata)

    # Create the dataset.
    dataset = Dataset.create(tmpFile.getName(), metadata, 
        kExpectedChunkSize, kExpectedCompressionLevel)
    assert(dataset)

    descriptor = SimpleLayerDescriptor.create(dataset, Elevation,
        kExpectedChunkSize, kExpectedCompressionLevel)
    assert(descriptor)

    assert(descriptor.getLayerType() == Elevation)

    descriptor = SimpleLayerDescriptor.create(dataset, Std_Dev,
        kExpectedChunkSize, kExpectedCompressionLevel)
    assert(descriptor)

    assert(descriptor.getDataType() == Layer.getDataType(Std_Dev))
    
    del dataset #ensure dataset is deleted before tmpFile
    
def testGetSetMinMax():
    tmpFile = testUtils.RandomFileGuard("name")
    #print(tmpFile.getName())
    metadata = Metadata()
    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)
    assert(metadata)

    # Create the dataset.
    dataset = Dataset.create(tmpFile.getName(), metadata, 
        kExpectedChunkSize, kExpectedCompressionLevel)
    assert(dataset)

    descriptor = SimpleLayerDescriptor.create(dataset, Elevation,
        kExpectedChunkSize, kExpectedCompressionLevel)
    assert(descriptor)

    kExpectedMin = 1.2345
    kExpectedMax = 9876.54321
    descriptor.setMinMax(kExpectedMin, kExpectedMax)

    actualMinMax = descriptor.getMinMax()
    assert(math.isclose(actualMinMax[0], kExpectedMin, rel_tol=1e-7))
    assert(math.isclose(actualMinMax[1], kExpectedMax, rel_tol=1e-7))

    del dataset #ensure dataset is deleted before tmpFile
  
def testGetInternalPath():
    tmpFile = testUtils.RandomFileGuard("name")
    #print(tmpFile.getName())
    metadata = Metadata()
    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)
    assert(metadata)

    # Create the dataset.
    dataset = Dataset.create(tmpFile.getName(), metadata, 
        kExpectedChunkSize, kExpectedCompressionLevel)
    assert(dataset)

    descriptor = SimpleLayerDescriptor.create(dataset, Elevation,
        kExpectedChunkSize, kExpectedCompressionLevel)
    assert(descriptor)
    assert(descriptor.getInternalPath() == Layer.getInternalPath(Elevation))

    descriptor = SimpleLayerDescriptor.create(dataset, Uncertainty,
        kExpectedChunkSize, kExpectedCompressionLevel)
    assert(descriptor)
    assert(descriptor.getInternalPath() == Layer.getInternalPath(Uncertainty))
    
    del dataset #ensure dataset is deleted before tmpFile

def testGetElementSize():
    tmpFile = testUtils.RandomFileGuard("name")
    #print(tmpFile.getName())
    metadata = Metadata()
    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)
    assert(metadata)

    # Create the dataset.
    dataset = Dataset.create(tmpFile.getName(), metadata, 
        kExpectedChunkSize, kExpectedCompressionLevel)
    assert(dataset)

    descriptor = SimpleLayerDescriptor.create(dataset, Elevation,
        kExpectedChunkSize, kExpectedCompressionLevel)
    assert(descriptor)
    assert(descriptor.getElementSize() ==
        Layer.getElementSize(Layer.getDataType(Elevation)))
    
    del dataset #ensure dataset is deleted before tmpFile

def testGetChunkSize():
    tmpFile = testUtils.RandomFileGuard("name")
    #print(tmpFile.getName())
    metadata = Metadata()
    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)
    assert(metadata)

    # Create the dataset.
    dataset = Dataset.create(tmpFile.getName(), metadata, 
        kExpectedChunkSize, kExpectedCompressionLevel)
    assert(dataset)

    descriptor = SimpleLayerDescriptor.create(dataset, Elevation,
        kExpectedChunkSize, kExpectedCompressionLevel)
    assert(descriptor)
    assert(descriptor.getChunkSize() == kExpectedChunkSize)
    
    del dataset #ensure dataset is deleted before tmpFile

def testGetCompressionLevel():
    tmpFile = testUtils.RandomFileGuard("name")
    #print(tmpFile.getName())
    metadata = Metadata()
    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)
    assert(metadata)

    # Create the dataset.
    dataset = Dataset.create(tmpFile.getName(), metadata, 
        kExpectedChunkSize, kExpectedCompressionLevel)
    assert(dataset)

    descriptor = SimpleLayerDescriptor.create(dataset, Elevation,
        kExpectedChunkSize, kExpectedCompressionLevel)
    assert(descriptor)
    assert(descriptor.getCompressionLevel() == kExpectedCompressionLevel)
    
    del dataset #ensure dataset is deleted before tmpFile


# run the unit test methods
testCreation()
testGetSetName()
testGetDataType()
testGetLayerType()
testGetSetMinMax()
testGetInternalPath()
testGetElementSize()
testGetChunkSize()
testGetCompressionLevel()