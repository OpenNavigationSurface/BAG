from bagPy import *
from bagTestPy import *
import shutil, pathlib
import bagMetadataSamples

# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "\..\examples\sample-data"
chunkSize = 100
compressionLevel = 6


# define the unit test methods:

def testReadOnly():
    bagFileName = datapath + "\sample.bag"
    #print(bagFileName)
    dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READONLY)
    assert(dataset)
    kNumExpectedLayers = 4
    numLayerTypes = len(dataset.getLayerTypes())
    #print(numLayerTypes)
    assert(numLayerTypes == kNumExpectedLayers)

def testReadWrite():
    bagFileName = datapath + "\sample.bag"
    tmpFileName = RandomFileGuard()
    #print(tmpFileName.getName())
    shutil.copyfile(bagFileName, str(tmpFileName.getName()))

    dataset = Dataset.openDataset(tmpFileName.getName(), BAG_OPEN_READ_WRITE)
    assert(dataset)
    kNumExpectedLayers = 4
    numLayerTypes = len(dataset.getLayerTypes())
    #print(numLayerTypes)
    assert(numLayerTypes == kNumExpectedLayers)

    dataset.createSimpleLayer(Average_Elevation, chunkSize, compressionLevel)
    numLayers = len(dataset.getLayerTypes())
    #print(numLayers)
    assert(numLayers == (numLayerTypes + 1))

def testCreation():
    tmpFileName = RandomFileGuard()
    kNumExpectedLayers = 2
    metadata = Metadata()
    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML);
    dataset = Dataset.create(tmpFileName.getName(), metadata, chunkSize, compressionLevel)

    assert(dataset)
    numLayers = len(dataset.getLayerTypes())
    #print(numLayers)
    assert(numLayers == kNumExpectedLayers)

    Dataset.openDataset(tmpFileName.getName(), BAG_OPEN_READONLY)
    dataset = Dataset.openDataset(tmpFileName.getName(), BAG_OPEN_READONLY)
    assert(dataset)
    numLayers = len(dataset.getLayerTypes())
    #print(numLayers)
    assert(numLayers == kNumExpectedLayers)

def testGetLayerTypes():
    bagFileName = datapath + "/NAVO_data/JD211_Public_Release_1-5.bag"
    dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READONLY)
    assert(dataset)
    numLayers = len(dataset.getLayerTypes())
    #print(numLayers)
    assert(numLayers == 2)

def testAddLayer():
    bagFileName = datapath + "\sample.bag"
    tmpFileName = RandomFileGuard()
    #print(tmpFileName.getName())
    shutil.copyfile(bagFileName, str(tmpFileName.getName()))

    dataset = Dataset.openDataset(tmpFileName.getName(), BAG_OPEN_READ_WRITE)
    assert(dataset)
    kNumExpectedLayers = 4
    numLayers = len(dataset.getLayerTypes())
    #print(numLayers)
    assert(numLayers == kNumExpectedLayers)

    dataset.createSimpleLayer(Average_Elevation, chunkSize, compressionLevel)
    numLayers = len(dataset.getLayerTypes())
    #print(numLayers)
    assert(numLayers == (kNumExpectedLayers + 1))



# run the unit test methods
testReadOnly()
testReadWrite()
testCreation()
testGetLayerTypes()
testAddLayer()