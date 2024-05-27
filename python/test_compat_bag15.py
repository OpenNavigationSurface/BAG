import unittest
from pathlib import Path
import tempfile
import shutil
import logging

import bagPy as BAG

logger = logging.getLogger(__file__)


class TestCompatBAG15(unittest.TestCase):
    def setUp(self) -> None:
        self.datapath: Path = Path(Path(__file__).parent.parent, 'examples', 'sample-data')

        self.tmp_dir = tempfile.mkdtemp()
        logger.info(f"tmp_dir: {self.tmp_dir}")

    def tearDown(self) -> None:
        shutil.rmtree(self.tmp_dir)

    def test_open_read_only(self) -> None:
        bag_filename = str(Path(self.datapath, 'sample.bag'))

        dataset = BAG.Dataset.openDataset(bag_filename, BAG.BAG_OPEN_READONLY)
        self.assertIsNotNone(dataset)

        kNumExpectedLayers = 4
        numLayerTypes = len(dataset.getLayerTypes())

        self.assertEqual(numLayerTypes, kNumExpectedLayers)

        bag_elev = dataset.getLayer(BAG.Elevation)
        self.assertIsNotNone(bag_elev)

        dataset.close()

    def test_open_read_write(self):
        bag_filename = str(Path(self.datapath, 'sample.bag'))

        dataset = BAG.Dataset.openDataset(bag_filename, BAG.BAG_OPEN_READ_WRITE)
        self.assertIsNotNone(dataset)

        kNumExpectedLayers = 4
        numLayerTypes = len(dataset.getLayerTypes())

        self.assertEqual(numLayerTypes, kNumExpectedLayers)

        bag_elev = dataset.getLayer(BAG.Elevation)
        self.assertIsNotNone(bag_elev)

        dataset.close()
