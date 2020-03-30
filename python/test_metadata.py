from bagPy import *
import pathlib
import bagMetadataSamples, testUtils


# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "\..\examples\sample-data"


# define the unit test methods:

def testDefaultConstructor():
    metadata = Metadata()
    assert(metadata)
    #print(metadata.llCornerX())
    assert(metadata.llCornerX() == INIT_VALUE)

def testConstructor():
    bagFileName = datapath + "\sample.bag"
    dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READONLY)
    assert(dataset)

    #metadata = Metadata()
    metadata = Metadata.fromSharedDataset(dataset)
    assert(metadata)
    print(metadata.llCornerX())
    assert(metadata.llCornerX() == 687910.0)
    wkt = metadata.horizontalReferenceSystemAsWKT()
    assert(wkt.startswith('''PROJCS["UTM-19N-Nad83'''))

def testGetStruct():
    bagFileName = datapath + "\sample.bag"
    dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READONLY)
    assert(dataset)

    metadata = dataset.getMetadata()
    assert(metadata)
    bagStruct = metadata.getStruct()
    assert(bagStruct)
    #print(bagStruct.fileIdentifier)
    assert(bagStruct.fileIdentifier == "Unique Identifier")
    assert(bagStruct.spatialRepresentationInfo)
    assert(bagStruct.spatialRepresentationInfo.llCornerX == 687910.0)

def testHorizontalReferenceSystemAsWKT():
    bagFileName = datapath + "\sample.bag"
    dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READONLY)
    assert(dataset)

    metadata = dataset.getMetadata()
    assert(metadata)
    horizontalRefSys = metadata.horizontalReferenceSystemAsWKT()
    assert(horizontalRefSys)
    assert(horizontalRefSys.startswith('''PROJCS["UTM-19N-Nad83'''))

def testLoadFromFile():
    tmpFile = testUtils.RandomFileGuard("string",
        bagMetadataSamples.kXMLv2MetadataBuffer)
    print(tmpFile.getName())

    metadata = Metadata()
    metadata.loadFromFile(tmpFile.getName())
    assert(metadata)

    assert(metadata.columnResolution() == 10.0)
    assert(metadata.horizontalReferenceSystemAsWKT())
    assert(metadata.llCornerX() == 687910.000000)
    assert(metadata.llCornerY() == 5554620.000000)
    assert(metadata.rowResolution() == 10.0)
    assert(metadata.urCornerX() == 691590.000000)
    assert(metadata.urCornerY() == 5562100.000000)

def testLoadFromBuffer():
    metadata = Metadata()
    metadata.loadFromBuffer(bagMetadataSamples.kXMLv2MetadataBuffer)
    assert(metadata)

    assert(metadata.columnResolution() == 10.0)
    assert(metadata.llCornerX() == 687910.000000)
    assert(metadata.llCornerY() == 5554620.000000)
    assert(metadata.rowResolution() == 10.0)
    assert(metadata.urCornerX() == 691590.000000)
    assert(metadata.urCornerY() == 5562100.000000)
    assert(metadata.rows() == 100)
    assert(metadata.columns() == 100)



# run the unit test methods
#testConstructor()  #TODO need to fix fromDataset method/constructor
testDefaultConstructor()
testGetStruct()
testHorizontalReferenceSystemAsWKT()
testLoadFromFile()
testLoadFromBuffer()