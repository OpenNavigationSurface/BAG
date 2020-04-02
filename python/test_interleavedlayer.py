from bagPy import *
import shutil, pathlib
import bagMetadataSamples, testUtils


# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "\..\examples\sample-data"
chunkSize = 100
compressionLevel = 6


# define the unit test methods:

def testGetLayerAndRead():
    bagFileName = datapath + "\micro151.bag"
    #print(bagFileName)
    dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READONLY)
    assert(dataset)

    kLayerType = Shoal_Elevation
    layer = dataset.getLayer(kLayerType)

    buffer = layer.read(1, 2, 2, 4) # 2x3
    assert(len(buffer) > 0)

    kExpectedNumNodes = 6;
    kExpectedBuffer = (-32.40, -32.38, -32.38, -32.33, -31.89, -31.98)

    #TODO is a conversion need for buffer to float, as in c++?
    for i in range(0,kExpectedNumNodes):
        assert(kExpectedBuffer[i] == buffer[i])

# run the unit test methods
#testGetLayerAndRead() #TODO: disabled because micro151 is not in git.