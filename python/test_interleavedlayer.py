from bagPy import *
from math import isclose
import shutil, pathlib
import bagMetadataSamples, testUtils


# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "/../examples/sample-data"
chunkSize = 100
compressionLevel = 6


# define the unit test methods:

def testGetLayerAndRead():
    bagFileName = datapath + "/micro151.bag"

    dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READONLY)
    assert(dataset)

    kLayerType = Shoal_Elevation
    layer = dataset.getLayer(kLayerType)
    assert(layer)

    result = layer.read(1, 2, 2, 4) # 2x3
    assert(len(result) > 0)

    kExpectedNumNodes = 6;

    buffer = result.asFloatItems()
    assert(len(buffer) == kExpectedNumNodes)

    kExpectedBuffer = (-32.40, -32.38, -32.38, -32.33, -31.89, -31.98)

    assert(all(isclose(actual, expected, abs_tol = 1e-5)
        for actual, expected in zip(buffer, kExpectedBuffer)))

    del dataset


# run the unit test methods
#testGetLayerAndRead() #TODO: disabled because micro151 is not in git.

