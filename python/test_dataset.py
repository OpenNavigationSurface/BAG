from bagPy import *
import shutil, pathlib
import bagMetadataSamples, testUtils


# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "/../examples/sample-data"
chunkSize = 100
compressionLevel = 6


# define the unit test methods:
print("Testing Dataset")

def testReadOnly():
    bagFileName = datapath + "/sample.bag"

    dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READONLY)
    assert(dataset)

    kNumExpectedLayers = 4
    numLayerTypes = len(dataset.getLayerTypes())

    assert(numLayerTypes == kNumExpectedLayers)

def testReadWrite():
    bagFileName = datapath + "/sample.bag"
    tmpFile = testUtils.RandomFileGuard("file", bagFileName)

    dataset = Dataset.openDataset(tmpFile.getName(), BAG_OPEN_READ_WRITE)
    assert(dataset)

    kNumExpectedLayers = 4
    numLayerTypes = len(dataset.getLayerTypes())
    assert(numLayerTypes == kNumExpectedLayers)

    simpleLayer = dataset.createSimpleLayer(Average_Elevation, chunkSize, compressionLevel)
    assert(simpleLayer)

    numLayerTypes2 = len(dataset.getLayerTypes())
    assert(numLayerTypes2 == (numLayerTypes + 1))

    del dataset #ensure dataset is deleted before tmpFile

def testCreation():
    tmpFile = testUtils.RandomFileGuard("name")
    metadata = Metadata()
    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML);

    dataset = Dataset.create(tmpFile.getName(), metadata, chunkSize, compressionLevel)
    assert(dataset)

    numLayerTypes = len(dataset.getLayerTypes())
    kNumExpectedLayers = 2
    assert(numLayerTypes == kNumExpectedLayers)

    dataset = Dataset.openDataset(tmpFile.getName(), BAG_OPEN_READONLY)
    assert(dataset)

    numLayerTypes = len(dataset.getLayerTypes())
    assert(numLayerTypes == kNumExpectedLayers)

    del dataset #ensure dataset is deleted before tmpFile

def testGetLayerTypes():
    bagFileName = datapath + "/NAVO_data/JD211_Public_Release_1-5.bag"
    dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READONLY)
    assert(dataset)

    numLayerTypes = len(dataset.getLayerTypes())
    assert(numLayerTypes == 2)

def testAddLayer():
    bagFileName = datapath + "/sample.bag"
    tmpFile = testUtils.RandomFileGuard("file", bagFileName)

    dataset = Dataset.openDataset(tmpFile.getName(), BAG_OPEN_READ_WRITE)
    assert(dataset)

    kNumExpectedLayers = 4
    numLayerTypes = len(dataset.getLayerTypes())
    assert(numLayerTypes == kNumExpectedLayers)

    simpleLayer = dataset.createSimpleLayer(Average_Elevation, chunkSize, compressionLevel)
    assert(simpleLayer)

    numLayerTypes = len(dataset.getLayerTypes())
    assert(numLayerTypes == (kNumExpectedLayers + 1))

    del dataset #ensure dataset is deleted before tmpFile

def testGetLayer():
    bagFileName = datapath + "/sample.bag"
    dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READ_WRITE)
    assert(dataset)

    kNumExpectedLayers = 4
    layers = dataset.getLayers();
    numLayers = len(layers)
    assert(numLayers == kNumExpectedLayers)

    for layer in layers:
        layerFromId = dataset.getLayer(layer.getDescriptor().getId())
        assert(layerFromId.getDescriptor().getInternalPath() 
               == layer.getDescriptor().getInternalPath())

def testGetTrackingList():
    bagFileName = datapath + "/sample.bag"
    dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READ_WRITE)
    assert(dataset)

    trackingList = dataset.getTrackingList()
    assert(trackingList)
    assert(trackingList.empty())
    assert(trackingList.size() == 0)

def testGetMetadata():
    bagFileName = datapath + "/sample.bag"
    dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READ_WRITE)
    assert(dataset)
    
    metadata = dataset.getMetadata()
    assert(metadata)

    kExpectedLLcornerX = 687910.0
    kExpectedLLcornerY = 5554620.0
    assert(metadata.llCornerX() == kExpectedLLcornerX)
    assert(metadata.llCornerY() == kExpectedLLcornerY)

def testGridToGeo():
    bagFileName = datapath + "/sample.bag"
    dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READ_WRITE)
    assert(dataset)

    x = 0.0
    y = 0.0
    xy = dataset.gridToGeo(0, 0)
    assert(xy[0] == 687910.0);
    assert(xy[1] == 5554620.0);

def testGeoToGrid():
    bagFileName = datapath + "/sample.bag"
    dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READ_WRITE)
    assert(dataset)

    row = 123
    column = 456
    rc = dataset.geoToGrid(687910.0, 5554620.0)
    assert(rc[0] == 0)
    assert(rc[1] == 0)

def testGetDescriptor():
    bagFileName = datapath + "/sample.bag"
    dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READ_WRITE)
    assert(dataset)

    descriptor = dataset.getDescriptor()
    assert(descriptor)
    assert(descriptor.isReadOnly() == False)



# run the unit test methods
testReadOnly()
testReadWrite()
testCreation()
testGetLayerTypes()
testAddLayer()
testGetLayer()
testGetTrackingList()
testGetMetadata()
testGridToGeo()
testGeoToGrid();
testGetDescriptor()
