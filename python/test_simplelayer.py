from bagPy import *
import shutil, pathlib
import bagMetadataSamples, testUtils
import sys


# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "\..\examples\sample-data"
chunkSize = 100
compressionLevel = 6


# define the unit test methods:

def testGetName():
    bagFileName = datapath + "\sample.bag"
    #print(bagFileName)
    dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READONLY)
    assert(dataset)

    layer = dataset.getLayer(Elevation)
    assert(layer)
    descriptor = layer.getDescriptor()
    assert(descriptor)

    print(descriptor.getName())
    print(kLayerTypeMapString)

    #TODO get map access to work
    #print(kLayerTypeMapString[Elevation])
    #assert(descriptor.getName() == str(kLayerTypeMapString[Elevation]))

def testRead():
    bagFileName = datapath + "\\NAVO_data\\JD211_public_Release_1-4_UTM.bag"
    dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READONLY)
    assert(dataset)

    kLayerType = Elevation
    elevLayer = dataset.getLayer(kLayerType)

    rowStart = 288
    rowEnd = 289
    columnStart = 249
    columnEnd = 251
    buffer = elevLayer.read(rowStart, columnStart, rowEnd, columnEnd) # 2x3

    kExpectedNumNodes = 6
    rows = (rowEnd - rowStart) + 1
    columns = (columnEnd - columnStart) + 1

    assert(buffer.size() > 0)

    datatype = Layer.getDataType(kLayerType)
    print(datatype)
    elemsize = Layer.getElementSize(datatype)
    print(elemsize)
    datasize = (elemsize * rows * columns)
    print(datasize)

    floatSize = sys.getsizeof(buffer[0])
    print(floatSize)

    #???
    #assert((kExpectedNumNodes * floatSize) == datasize)

    kExpectedBuffer = (1000000.0, -52.161003, -52.172005,
        1000000.0, -52.177002, -52.174004)

    # TODO: get UInt8Array [] working
    #for i in range (0, kExpectedNumNodes):
        #print(buffer[i])        
        #assert(kExpectedBuffer[i] == buffer[i])

# run the unit test methods
testGetName()
testRead()