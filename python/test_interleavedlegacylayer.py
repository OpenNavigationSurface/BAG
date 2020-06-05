from bagPy import *
from math import isclose
import shutil, pathlib
import bagMetadataSamples, testUtils


# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "/../examples/sample-data"
chunkSize = 100
compressionLevel = 6


# define the unit test methods:
print("Testing InterleavedLegacyLayer")

def testGetLayerAndRead():
    bagFileName = datapath + "/example_w_qc_layers.bag"

    dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READONLY)
    assert(dataset)

    kLayerType = Hypothesis_Strength
    layer = dataset.getLayer(kLayerType)
    assert(layer)

    result = layer.read(247, 338, 248, 340) # 2x3
    assert(result)

    kExpectedNumNodes = 6;

    buffer = result.asFloatItems()
    assert(len(buffer) == kExpectedNumNodes)

    kExpectedBuffer = (1.0e6, 1.0e6, 0.0, 1.0e6, 0.0, 0.0)

    assert(all(isclose(actual, expected, abs_tol = 1e-5)
        for actual, expected in zip(buffer, kExpectedBuffer)))

    del dataset


# run the unit test methods
testGetLayerAndRead()

