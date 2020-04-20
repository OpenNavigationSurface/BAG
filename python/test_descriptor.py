from bagPy import *
import shutil, pathlib
import bagMetadataSamples, testUtils

# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "\..\examples\sample-data"
chunkSize = 100
compressionLevel = 6


# define the unit test methods:

def testConstructors():
    descriptor = Descriptor()
    assert(descriptor)
    kExpectedVersion = "1.5.0"
    descriptor.setVersion(kExpectedVersion)
    #print(descriptor.getVersion())
    assert(descriptor.getVersion() == kExpectedVersion)

    descriptor2 = Descriptor()
    assert(descriptor2)
    #print(descriptor2.getVersion())
    assert(descriptor2.getVersion() == "")

    descriptor2 = descriptor
    assert(descriptor2.getVersion() == kExpectedVersion);


def testGetLayerTypes():
    descriptor = Descriptor()
    assert(descriptor)
    layerTypes = descriptor.getLayerTypes()
    assert(len(layerTypes) == 0)

    metadata = Metadata()
    metadata.loadFromBuffer(bagMetadataSamples.kXMLv2MetadataBuffer)
    assert(metadata)

    tmpFile = testUtils.RandomFileGuard("name")
    #print(tmpFile.getName())

    dataset = Dataset.create(tmpFile.getName(), metadata, chunkSize, compressionLevel)
    assert(dataset)

    descriptor = dataset.getDescriptor()
    assert(descriptor)

    layerTypes = descriptor.getLayerTypes()
    assert(len(layerTypes) == 2)
        
    assert(Elevation in layerTypes)
    assert(Uncertainty in layerTypes)

    assert(getLayerTypeAsString(layerTypes[0]) == "Elevation")
    assert(getLayerTypeAsString(layerTypes[1]) == "Uncertainty")

    del dataset #ensure dataset is deleted before tmpFile

def testReadOnly():
    descriptor = Descriptor()
    assert(descriptor)
    assert(descriptor.isReadOnly())
    assert(descriptor.setReadOnly(True))
    assert(descriptor.setReadOnly(False))
    assert(not descriptor.isReadOnly())

def testGetLayerDescriptors():
    metadata = Metadata()
    metadata.loadFromBuffer(bagMetadataSamples.kXMLv2MetadataBuffer)
    assert(metadata)

    tmpFile = testUtils.RandomFileGuard("name")
    #print(tmpFile.getName())

    dataset = Dataset.create(tmpFile.getName(), metadata, chunkSize, compressionLevel)
    assert(dataset)

    descriptor = dataset.getDescriptor()
    assert(descriptor)

    layerDescriptors = descriptor.getLayerDescriptors()
    assert(len(layerDescriptors) == 2)
    #layerDescriptors = None
    
    del dataset #ensure dataset is deleted before tmpFile


def testGetLayerDescriptor():
    descriptor = Descriptor()
    assert(descriptor)
    #descriptor.getLayerDescriptor(Elevation) #expected to throw - enable after adding test platform

    metadata = Metadata()
    metadata.loadFromBuffer(bagMetadataSamples.kXMLv2MetadataBuffer)
    assert(metadata)

    tmpFile = testUtils.RandomFileGuard("name")
    #print(tmpFile.getName())

    dataset = Dataset.create(tmpFile.getName(), metadata, chunkSize, compressionLevel)
    assert(dataset)

    descriptor = dataset.getDescriptor()
    assert(descriptor)
    
    assert(descriptor.getLayerDescriptor(Elevation))
    assert(descriptor.getLayerDescriptor(Uncertainty))

    del dataset #ensure dataset is deleted before tmpFile

def testFromMetadata():
    metadata = Metadata()
    metadata.loadFromBuffer(bagMetadataSamples.kXMLv2MetadataBuffer)

    descriptor = Descriptor(metadata)
    assert(descriptor)

    kExpectedHorizontalReferenceSystem = '''PROJCS["UTM-19N-Nad83",
    GEOGCS["unnamed",
        DATUM["North_American_Datum_1983",
            SPHEROID["North_American_Datum_1983",6378137,298.2572201434276],
            TOWGS84[0,0,0,0,0,0,0]],
        PRIMEM["Greenwich",0],
        UNIT["degree",0.0174532925199433],
        EXTENSION["Scaler","0,0,0,0.02,0.02,0.001"],
        EXTENSION["Source","CARIS"]],
    PROJECTION["Transverse_Mercator"],
    PARAMETER["latitude_of_origin",0],
    PARAMETER["central_meridian",-69],
    PARAMETER["scale_factor",0.9996],
    PARAMETER["false_easting",500000],
    PARAMETER["false_northing",0],
    UNIT["metre",1]]'''
    horiz = descriptor.getHorizontalReferenceSystem()
    assert(str(horiz) == kExpectedHorizontalReferenceSystem)

    kExpectedVerticalReferenceSystem = '''VERT_CS["Alicante height",
    VERT_DATUM["Alicante",2000]]'''
    assert(descriptor.getVerticalReferenceSystem() == kExpectedVerticalReferenceSystem)

    kExpectedDimXY = 100
    dims = descriptor.getDims()
    #print(dims)
    assert(dims[0] == kExpectedDimXY)
    assert(dims[1] == kExpectedDimXY)

    minX,minY,maxX,maxY = 687910.0, 5554620.0, 691590.0, 5562100.0
    cover = descriptor.getProjectedCover()
    #print(cover)
    assert(cover[0][0] == minX)
    assert(cover[0][1] == minY)
    assert(cover[1][0] == maxX)
    assert(cover[1][1] == maxY)

    kExpectedOriginX, kExpectedOriginY = 687910.0, 5554620.0
    origin = descriptor.getOrigin()
    #print(origin)
    assert(origin[0] == kExpectedOriginX)
    assert(origin[1] == kExpectedOriginY)

    kExpectedGridSpacingXY = 10.0
    spacing = descriptor.getGridSpacing()
    #print(spacing)
    assert(spacing[0] == kExpectedGridSpacingXY);
    assert(spacing[1] == kExpectedGridSpacingXY);

def testSetValues():
    descriptor = Descriptor()

    kExpected = "abc"
    descriptor.setHorizontalReferenceSystem(kExpected)
    assert(descriptor.getHorizontalReferenceSystem() == kExpected)

    kExpected = "def"
    descriptor.setVerticalReferenceSystem(kExpected)
    assert(descriptor.getVerticalReferenceSystem() == kExpected)

    kExpectedDim0 = 24
    kExpectedDim1 = 68
    descriptor.setDims(kExpectedDim0, kExpectedDim1)
    assert(descriptor.getDims() == (kExpectedDim0,kExpectedDim1))

    kExpectedLLx = 0
    kExpectedLLy = 1
    kExpectedURx = 1001
    kExpectedURy = 2020
    descriptor.setProjectedCover(kExpectedLLx, kExpectedLLy, kExpectedURx, kExpectedURy)
    assert(descriptor.getProjectedCover() ==
        ((kExpectedLLx, kExpectedLLy), (kExpectedURx, kExpectedURy)))

    kExpectedLLx = 12.3456
    kExpectedLLy = 98.7654321
    descriptor.setOrigin(kExpectedLLx, kExpectedLLy)
    assert(descriptor.getOrigin() == (kExpectedLLx, kExpectedLLy))

    kExpectedXspacing = 12.3456
    kExpectedYspacing = 98.7654321
    descriptor.setGridSpacing(kExpectedXspacing, kExpectedYspacing)
    assert(descriptor.getOrigin() == (kExpectedXspacing, kExpectedYspacing))


testConstructors()
testReadOnly()
testGetLayerTypes()
testGetLayerDescriptor()
testFromMetadata()
testSetValues()

#TODO: fix memory leak
#testGetLayerDescriptors()