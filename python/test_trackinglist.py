from bagPy import *
import shutil, pathlib, math
import bagMetadataSamples, testUtils
import sys


# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "/../examples/sample-data"
chunkSize = 100
compressionLevel = 6


# define the unit test methods:
def testConstructOpen():
    tmpFile = testUtils.RandomFileGuard("name")
    #print(tmpFile.getName())
    metadata = Metadata()
    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)
    assert(metadata)

    # Create the dataset.
    dataset = Dataset.create(tmpFile.getName(), metadata, 
        chunkSize, compressionLevel)
    assert(dataset)

    trackingList = dataset.getTrackingList()

    # Add 1 record and save.  Is there one record with expected values?
    kExpectedItem0 = BagTrackingItem(1, 2, 3.4, 5.6, 7, 8) #createTrackingItem
    trackingList.push_back(kExpectedItem0)
    assert(trackingList.size() == 1)

    kExpectedItem1 = BagTrackingItem(11, 22, 33.44, 55.66, 77, 88) #createTrackingItem
    trackingList.push_back(kExpectedItem1)
    assert(trackingList.size() == 2);

    trackingList.write()

    dataset = Dataset.openDataset(tmpFile.getName(), BAG_OPEN_READONLY)
    assert(dataset)

    trackingList = dataset.getTrackingList()
    kExpectedNumItems = 2
    assert(trackingList.size() == kExpectedNumItems)
    
    del dataset #ensure dataset is deleted before tmpFile

def testConstructCreate():
    
    tmpFile = testUtils.RandomFileGuard("name")
    #print(tmpFile.getName())

    # Create the dataset.
    dataset = Dataset.create(tmpFile.getName(), Metadata(), 
        chunkSize, compressionLevel)
    assert(dataset)

    trackingList = dataset.getTrackingList()
    assert(trackingList.size() == 0)

    # Add 1 record and save.  Is there one record with expected values?
    kExpectedItem0 = BagTrackingItem(1, 2, 3.4, 5.6, 7, 8) #createTrackingItem
    trackingList.push_back(kExpectedItem0)
    assert(trackingList.size() == 1)

    trackingList.write()

    trackingList = dataset.getTrackingList()
    assert(trackingList.size() == 1)

    item0 = trackingList.front()
    assert(kExpectedItem0.row == item0.row)
    assert(kExpectedItem0.col == item0.col)
    assert(kExpectedItem0.depth == item0.depth)
    assert(kExpectedItem0.uncertainty == item0.uncertainty)
    assert(kExpectedItem0.track_code == item0.track_code)
    assert(kExpectedItem0.list_series == item0.list_series)

    kExpectedItem1 = BagTrackingItem(9, 8, 7.6, 5.4, 3, 2)
    trackingList.push_back(kExpectedItem1)
    assert(trackingList.size() == 2)

    trackingList.write()

    item0 = trackingList.at(0);
    assert(kExpectedItem0.row == item0.row)
    assert(kExpectedItem0.col == item0.col)
    assert(kExpectedItem0.depth == item0.depth)
    assert(kExpectedItem0.uncertainty == item0.uncertainty)
    assert(kExpectedItem0.track_code == item0.track_code)
    assert(kExpectedItem0.list_series == item0.list_series)

    item1 = trackingList.at(1);
    assert(kExpectedItem1.row == item1.row)
    assert(kExpectedItem1.col == item1.col)
    assert(kExpectedItem1.depth == item1.depth)
    assert(kExpectedItem1.uncertainty == item1.uncertainty)
    assert(kExpectedItem1.track_code == item1.track_code)
    assert(kExpectedItem1.list_series == item1.list_series)
    
    del dataset #ensure dataset is deleted before tmpFile


testConstructOpen()
testConstructCreate()