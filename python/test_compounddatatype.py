import unittest
import pathlib

import xmlrunner

from bagPy import *

# import bagMetadataSamples

# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "/../examples/sample-data"


class TestCompoundDataType(unittest.TestCase):
    def testDefaultConstructor(self):
        cdt = CompoundDataType()
        self.assertIsNotNone(cdt)
        self.assertEqual(cdt.getType(), DT_UNKNOWN_DATA_TYPE)

    def testConstructorFloat(self):
        kExpectedValue = 42.3
        kExpectedType = DT_FLOAT32
        cdt = CompoundDataType(kExpectedValue)
        self.assertIsNotNone(cdt)
        self.assertEqual(cdt.getType(), kExpectedType)

        floatVal = cdt.asFloat()
        self.assertAlmostEqual(floatVal, kExpectedValue, places=5)

        floatVal = getFloat(cdt)
        self.assertAlmostEqual(floatVal, kExpectedValue, places=5)

    def testConstructorUInt32(self):
        kExpectedValue = 42
        kExpectedType = DT_UINT32
        cdt = CompoundDataType(kExpectedValue)
        self.assertIsNotNone(cdt)
        self.assertEqual(cdt.getType(), kExpectedType)

        uintVal = cdt.asUInt32()
        self.assertEqual(uintVal, kExpectedValue)

        uintVal = getUInt32(cdt)
        self.assertEqual(uintVal, kExpectedValue)

    def testConstructorBool(self):
        kExpectedValue = True
        kExpectedType = DT_BOOLEAN
        cdt = CompoundDataType(kExpectedValue)
        self.assertIsNotNone(cdt)
        self.assertEqual(cdt.getType(), kExpectedType)

        boolVal = cdt.asBool()
        self.assertEqual(boolVal, kExpectedValue)

        boolVal = getBool(cdt)
        self.assertEqual(boolVal, kExpectedValue)


    def testConstructorString(self):
        kExpectedValue = "Test Constructor"
        kExpectedType = DT_STRING
        cdt = CompoundDataType(kExpectedValue)
        self.assertIsNotNone(cdt)
        self.assertEqual(cdt.getType(), kExpectedType)

        stringVal = str(cdt.asString())
        self.assertEqual(stringVal, kExpectedValue)

        stringVal = getString(cdt)
        self.assertEqual(stringVal, kExpectedValue)

    def testAssignFloat(self):
        cdt = CompoundDataType()
        self.assertEqual(cdt.getType(), DT_UNKNOWN_DATA_TYPE)
        kExpectedValue = 123.456
        cdt.assignFloat(kExpectedValue)
        self.assertEqual(cdt.getType(), DT_FLOAT32)
        floatVal = getFloat(cdt)
        self.assertAlmostEqual(floatVal, kExpectedValue, places=5)

    def testAssignUInt32(self):
        cdt = CompoundDataType()
        self.assertEqual(cdt.getType(), DT_UNKNOWN_DATA_TYPE)
        kExpectedValue = 101
        cdt.assignUInt32(kExpectedValue)
        self.assertEqual(cdt.getType(), DT_UINT32)
        uintVal = getUInt32(cdt)
        self.assertEqual(uintVal, kExpectedValue)

    def testAssignBool(self):
        cdt = CompoundDataType()
        self.assertEqual(cdt.getType(), DT_UNKNOWN_DATA_TYPE)
        kExpectedValue = True
        cdt.assignBool(kExpectedValue)
        self.assertEqual(cdt.getType(), DT_BOOLEAN)
        boolVal = getBool(cdt)
        self.assertEqual(boolVal, kExpectedValue)

    def testAssignString(self):
        cdt = CompoundDataType()
        self.assertEqual(cdt.getType(), DT_UNKNOWN_DATA_TYPE)
        kExpectedValue = "Test Constructor"
        cdt.assignString(kExpectedValue)
        self.assertEqual(cdt.getType(), DT_STRING)
        stringVal = getString(cdt)
        self.assertEqual(stringVal, kExpectedValue)

    def testCopyConstuct(self):
        cdt1 = CompoundDataType()
        cdt2 = CompoundDataType("Test")
        stringVal = str(cdt2.asString())
        self.assertEqual(stringVal, "Test")

        cdt1 = cdt2
        stringVal = str(cdt1.asString())
        self.assertEqual(stringVal, "Test")
        self.assertEqual(cdt1, cdt2)

    def testCopyAssign(self):
        cdt1 = CompoundDataType(123.45)
        floatVal = cdt1.asFloat()
        self.assertAlmostEqual(floatVal, 123.45, places=5)

        cdt2 = CompoundDataType(cdt1)
        floatVal = cdt2.asFloat()
        self.assertAlmostEqual(floatVal, 123.45, places=5)
        self.assertEqual(cdt1, cdt2)


if __name__ == '__main__':
    unittest.main(
        testRunner=xmlrunner.XMLTestRunner(output='test-reports'),
        failfast=False, buffer=False, catchbreak=False
    )
