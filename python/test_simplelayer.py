from bagPy import *
import shutil, pathlib, math
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

    #print(descriptor.getName())
    #print(getLayerTypeAsString(Elevation))
    assert(descriptor.getName() == getLayerTypeAsString(Elevation))

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
    #print(datatype)
    elemsize = Layer.getElementSize(datatype)
    #print(elemsize)
    datasize = (elemsize * rows * columns)
    #print(datasize)

    #floatBuffer = float(buffer.get())

   # bufferSize = sys.getsizeof(buffer[0])
    #print(floatSize)
   # floatBufferSize = sys.getsizeof(floatBuffer[0])
   # print(floatBufferSize)
    #print(buffer[0])
    #print(buffer[0].value())
    ##print(buffer.get(0))
    #print(buffer.__getitem__(0))

    #???
    #assert((kExpectedNumNodes * floatSize) == datasize)

    kExpectedBuffer = (1000000.0, -52.161003, -52.172005,
        1000000.0, -52.177002, -52.174004)

    # TODO: get UInt8Array [] working
    #for i in range (0, kExpectedNumNodes):
        #print(buffer[i])        
        #assert(kExpectedBuffer[i] == buffer[i])


def testWrite():
    kLayerType = Elevation
    kExpectedNumNodes = 12
    kFloatValue = 123.456
    tmpFile = testUtils.RandomFileGuard("name")
    #print(tmpFile.getName())
    metadata = Metadata()
    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)
    assert(metadata)

    # Create the dataset.
    dataset = Dataset.create(tmpFile.getName(), metadata, chunkSize, compressionLevel)
    assert(dataset)
    layer = dataset.getLayer(kLayerType)
    assert(layer)
    fileDims = dataset.getDescriptor().getDims()
    kExpectedRows = dataset.getMetadata().getStruct().spatialRepresentationInfo.numberOfRows
    kExpectedColumns = dataset.getMetadata().getStruct().spatialRepresentationInfo.numberOfColumns
    assert(fileDims == (kExpectedRows, kExpectedColumns))

    # Open the dataset read/write and write to it.
    dataset = Dataset.openDataset(tmpFile.getName(), BAG_OPEN_READ_WRITE)
    assert(dataset)
    elevLayer = dataset.getLayer(kLayerType);
    assert(layer)    
    origBuffer = [kFloatValue] * kExpectedNumNodes
    print(origBuffer)

    uint8Buffer = [int(i) for i in origBuffer] 

    assert(elevLayer.write(1, 2, 3, 5, uint8Buffer))
            #reinterpret_cast<uint8_t*>(buffer.data()))); // 3x4
    
    # Open the dataset and read what was written.
    dataset = Dataset.openDataset(tmpFile.getName(), BAG_OPEN_READONLY)
    assert(dataset)
    elevLayer = dataset.getLayer(kLayerType)
    readBuffer = elevLayer.read(1, 2, 3, 5) # 3x4
    assert(readBuffer.size() > 0)

    floatBuffer = [float(i) for i in readBuffer]

    #const auto* floatBuffer = reinterpret_cast<const float*>(buffer.get());


    for i in (0, kExpectedNumNodes):
        assert(floatBuffer[i] == origBuffer[i])
    
    del dataset #ensure dataset is deleted before tmpFile



def testWriteAttributes():
    bagFileName = datapath + "\sample.bag"
    tmpFile = testUtils.RandomFileGuard("file", bagFileName)
    kLayerType = Elevation
    kExpectedMin = 0.0
    kExpectedMax = 0.0

    # Open the dataset read/write and write to it.
    dataset = Dataset.openDataset(tmpFile.getName(), BAG_OPEN_READ_WRITE)
    assert(dataset)
    elevLayer = dataset.getLayer(kLayerType)
    assert(elevLayer)    
    descriptor = elevLayer.getDescriptor()
    assert(descriptor)

    # change min & max values
    originalMinMax = descriptor.getMinMax()
    kExpectedMin = originalMinMax[0] - 12.34
    kExpectedMax = originalMinMax[1] + 56.789
    descriptor.setMinMax(kExpectedMin, kExpectedMax)
    elevLayer.writeAttributes()

    # check that written values have changed
    dataset = Dataset.openDataset(tmpFile.getName(), BAG_OPEN_READONLY)
    assert(dataset)
    elevLayer = dataset.getLayer(kLayerType)
    descriptor = elevLayer.getDescriptor()
    actualMinMax = descriptor.getMinMax()
    assert(math.isclose(actualMinMax[0], kExpectedMin, rel_tol=1e-7))
    assert(math.isclose(actualMinMax[1], kExpectedMax, rel_tol=1e-7))

    del dataset #ensure dataset is deleted before tmpFile


# run the unit test methods
testGetName()

#TODO fix read/write
#testRead()
#testWrite()

testWriteAttributes()