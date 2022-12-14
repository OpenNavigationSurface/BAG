import unittest
import pathlib

import xmlrunner

from bagPy import *


# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "/../examples/sample-data"
chunkSize = 100
compressionLevel = 6


class TestInterleavedLegacyLayer(unittest.TestCase):
    def testGetLayerAndRead(self):
        bagFileName = datapath + "/example_w_qc_layers.bag"

        dataset = Dataset.openDataset(bagFileName, BAG_OPEN_READONLY)
        self.assertIsNotNone(dataset)

        kLayerType = Hypothesis_Strength
        layer = dataset.getLayer(kLayerType)
        self.assertIsNotNone(layer)

        # 2x3
        result = layer.read(247, 338, 248, 340)
        self.assertIsNotNone(result)

        kExpectedNumNodes = 6

        buffer = result.asFloatItems()
        self.assertEqual(len(buffer), kExpectedNumNodes)

        kExpectedBuffer = (1.0e6, 1.0e6, 0.0, 1.0e6, 0.0, 0.0)
        for actual, expected in zip(buffer, kExpectedBuffer):
            self.assertAlmostEqual(actual, expected, places=5)

        del dataset


if __name__ == '__main__':
    unittest.main(
        testRunner=xmlrunner.XMLTestRunner(output='test-reports'),
        failfast=False, buffer=False, catchbreak=False
    )
