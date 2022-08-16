import unittest
import logging

import bagPy as BAG


logger = logging.getLogger(__name__)


class TestRecord(unittest.TestCase):
    def test_record_creation(self):
        r: BAG.Record = BAG.Record()
        r.append(BAG.CompoundDataType("123.45"))
        r.append(BAG.CompoundDataType(42.23))
        r.append(BAG.CompoundDataType(23))

        self.assertEqual(3, len(r))
        self.assertEqual("123.45", BAG.getString(r[0]))
        self.assertAlmostEqual(42.23, BAG.getFloat(r[1]), places=5)
        self.assertEqual(23, BAG.getUInt32(r[2]))
