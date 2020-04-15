from bagPy import *
from math import isclose
import shutil, pathlib
import bagMetadataSamples, testUtils
import sys

# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "/../examples/sample-data"
chunkSize = 100
compressionLevel = 6

# NOTE .. Testing the methods that forward to std::vector<VRTrackingList::value_type> is not covered.


def testCreateOpen():
    tmpBagFile = testUtils.RandomFileGuard("name")

    metadata = Metadata()

    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)

    dataset = Dataset.create(tmpBagFile.getName(), metadata, chunkSize,
        compressionLevel)
    assert(dataset)

    # Check no VR tracking list exists.
    vrTrackingList = dataset.getVRTrackingList()
    assert(not vrTrackingList)

    # Check VR tracking list was created successfully.
    dataset.createVR(chunkSize, compressionLevel, False)
    vrTrackingList = dataset.getVRTrackingList()
    assert(vrTrackingList)

    # Check no items are in the tracking list after creation.
    assert(vrTrackingList.empty())

    # Add 1 record.
    kExpectedItem0 = BagVRTrackingItem(1, 2, 11, 22, 3.4, 5.6, 7, 8)

    vrTrackingList.push_back(kExpectedItem0)

    # Check one item was stored.
    assert(not vrTrackingList.empty());
    assert(vrTrackingList.size() == 1);

    vrTrackingList.write()


    # Read the expected record
    vrTrackingList1 = dataset.getVRTrackingList()
    assert(vrTrackingList1)

    # Check one item was read.
    assert(vrTrackingList1.size() == 1)

    # Check the expected values are in the item.
    actual = vrTrackingList1.front()
    assert(kExpectedItem0.row == actual.row)
    assert(kExpectedItem0.col== actual.col)
    assert(kExpectedItem0.sub_row == actual.sub_row)
    assert(kExpectedItem0.sub_col== actual.sub_col)
    assert(isclose(kExpectedItem0.depth, actual.depth, abs_tol = 1e-5))
    assert(isclose(kExpectedItem0.uncertainty, actual.uncertainty, abs_tol = 1e-5))
    assert(kExpectedItem0.track_code == actual.track_code)
    assert(kExpectedItem0.list_series == actual.list_series)

    # Add a second record to an existing vrTrackingList.
    vrTrackingList1 = dataset.getVRTrackingList()
    assert(vrTrackingList1)

    # Check one item was read.
    assert(vrTrackingList1.size() == 1)

    kExpectedItem1 = BagVRTrackingItem(9, 8, 101, 202, 7.6, 5.4, 3, 2)

    vrTrackingList1.push_back(kExpectedItem1)
    assert(vrTrackingList1.size() == 2)

    vrTrackingList1.write()


    vrTrackingList2 = dataset.getVRTrackingList()

    # Check the expected values are in the item.
    assert(vrTrackingList2.size() == 2)

    # Check the first expected values are exactly the same.
    #assert(kExpectedItem0 == vrTrackingList2[0])
    actual = vrTrackingList2.at(0)
    assert(kExpectedItem0.row == actual.row)
    assert(kExpectedItem0.col== actual.col)
    assert(kExpectedItem0.sub_row == actual.sub_row)
    assert(kExpectedItem0.sub_col== actual.sub_col)
    assert(isclose(kExpectedItem0.depth, actual.depth, abs_tol = 1e-5))
    assert(isclose(kExpectedItem0.uncertainty, actual.uncertainty, abs_tol = 1e-5))
    assert(kExpectedItem0.track_code == actual.track_code)
    assert(kExpectedItem0.list_series == actual.list_series)

    # Check the second expected values are exactly the same.
    #assert(kExpectedItem1 == vrTrackingList2[1])
    actual = vrTrackingList2.at(1)
    assert(kExpectedItem1.row == actual.row)
    assert(kExpectedItem1.col== actual.col)
    assert(kExpectedItem1.sub_row == actual.sub_row)
    assert(kExpectedItem1.sub_col== actual.sub_col)
    assert(isclose(kExpectedItem1.depth, actual.depth, abs_tol = 1e-5))
    assert(isclose(kExpectedItem1.uncertainty, actual.uncertainty, abs_tol = 1e-5))
    assert(kExpectedItem1.track_code == actual.track_code)
    assert(kExpectedItem1.list_series == actual.list_series)

    # Force a close
    del dataset


    # Read the BAG.
    dataset = Dataset.openDataset(tmpBagFile.getName(), BAG_OPEN_READONLY)

    # Check dataset was read successfully.
    assert(dataset)

    vrTrackingList3 = dataset.getVRTrackingList()

    # Check the expected number of items in the tracking list are there.
    kExpectedNumItems = 2
    assert(vrTrackingList3.size() == kExpectedNumItems)

    # Check the first expected values are exactly the same.
    #assert(kExpectedItem0 == vrTrackingList3[0])
    actual = vrTrackingList3.at(0)
    assert(kExpectedItem0.row == actual.row)
    assert(kExpectedItem0.col== actual.col)
    assert(kExpectedItem0.sub_row == actual.sub_row)
    assert(kExpectedItem0.sub_col== actual.sub_col)
    assert(isclose(kExpectedItem0.depth, actual.depth, abs_tol = 1e-5))
    assert(isclose(kExpectedItem0.uncertainty, actual.uncertainty, abs_tol = 1e-5))
    assert(kExpectedItem0.track_code == actual.track_code)
    assert(kExpectedItem0.list_series == actual.list_series)

    # Check the second expected values are exactly the same.
    #assert(kExpectedItem1 == vrTrackingList3[1])
    actual = vrTrackingList3.at(1)
    assert(kExpectedItem1.row == actual.row)
    assert(kExpectedItem1.col== actual.col)
    assert(kExpectedItem1.sub_row == actual.sub_row)
    assert(kExpectedItem1.sub_col== actual.sub_col)
    assert(isclose(kExpectedItem1.depth, actual.depth, abs_tol = 1e-5))
    assert(isclose(kExpectedItem1.uncertainty, actual.uncertainty, abs_tol = 1e-5))
    assert(kExpectedItem1.track_code == actual.track_code)
    assert(kExpectedItem1.list_series == actual.list_series)

    # Force a close
    del dataset


testCreateOpen()

