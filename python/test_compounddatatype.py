from bagPy import *
import pathlib, math
import bagMetadataSamples

# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "/../examples/sample-data"
print("+++")
print(datapath)

# define the unit test methods:

def testDefaultConstructor():
    cdt = CompoundDataType()
    assert(cdt)
    assert(cdt.getType() == DT_UNKNOWN_DATA_TYPE)

def testConstructorFloat():
    kExpectedValue = 42.3
    kExpectedType = DT_FLOAT32
    cdt = CompoundDataType(kExpectedValue)
    assert(cdt)
    assert(cdt.getType() == kExpectedType)

    floatVal = cdt.asFloat()
    #print(floatVal)
    assert(math.isclose(floatVal, kExpectedValue, rel_tol=1e-7))

    floatVal = getFloat(cdt)
    #print(floatVal)
    assert(math.isclose(floatVal, kExpectedValue, rel_tol=1e-7))

def testConstructorUInt32():
    kExpectedValue = 42
    kExpectedType = DT_UINT32
    cdt = CompoundDataType(kExpectedValue)
    assert(cdt)
    assert(cdt.getType() == kExpectedType)

    uintVal = cdt.asUInt32()
    #print(uintVal)
    assert(uintVal == kExpectedValue)

    uintVal = getUInt32(cdt)
    #print(uintVal)
    assert(uintVal == kExpectedValue)

def testConstructorBool():
    kExpectedValue = True
    kExpectedType = DT_BOOLEAN
    cdt = CompoundDataType(kExpectedValue)
    assert(cdt)
    assert(cdt.getType() == kExpectedType)

    boolVal = cdt.asBool()
    #print(boolVal)
    assert(boolVal == kExpectedValue)

    boolVal = getBool(cdt)
    #print(boolVal)
    assert(boolVal == kExpectedValue)

    
def testConstructorString():
    kExpectedValue = "Test Constructor"
    kExpectedType = DT_STRING
    cdt = CompoundDataType(kExpectedValue)
    assert(cdt)
    assert(cdt.getType() == kExpectedType)

    stringVal = str(cdt.asString())
    #print(stringVal)
    assert(stringVal == kExpectedValue)

    stringVal = getString(cdt)
    #print(stringVal)
    assert(stringVal == kExpectedValue)

def testAssignFloat():
    cdt = CompoundDataType()
    assert(cdt.getType() == DT_UNKNOWN_DATA_TYPE)
    kExpectedValue = 123.456
    cdt.assignFloat(kExpectedValue)
    assert(cdt.getType() == DT_FLOAT32)
    floatVal = getFloat(cdt)
    assert(math.isclose(floatVal, kExpectedValue, rel_tol=1e-7))
    
def testAssignUInt32():
    cdt = CompoundDataType()
    assert(cdt.getType() == DT_UNKNOWN_DATA_TYPE)
    kExpectedValue = 101
    cdt.assignUInt32(kExpectedValue)
    assert(cdt.getType() == DT_UINT32)
    uintVal = getUInt32(cdt)
    assert(uintVal == kExpectedValue)

def testAssignBool():
    cdt = CompoundDataType()
    assert(cdt.getType() == DT_UNKNOWN_DATA_TYPE)
    kExpectedValue = True
    cdt.assignBool(kExpectedValue)
    assert(cdt.getType() == DT_BOOLEAN)
    boolVal = getBool(cdt)
    assert(boolVal == kExpectedValue)

def testAssignString():
    cdt = CompoundDataType()
    assert(cdt.getType() == DT_UNKNOWN_DATA_TYPE)
    kExpectedValue = "Test Constructor"
    cdt.assignString(kExpectedValue)
    assert(cdt.getType() == DT_STRING)
    stringVal = getString(cdt)
    assert(stringVal == kExpectedValue)

def testCopyConstuct():
    cdt1 = CompoundDataType()
    cdt2 = CompoundDataType("Test")
    stringVal = str(cdt2.asString())
    #print(stringVal)
    assert(stringVal == "Test")

    cdt1 = cdt2
    stringVal = str(cdt1.asString())
    #print(stringVal)
    assert(stringVal == "Test")
    assert(cdt1 == cdt2)

def testCopyAssign():
    cdt1 = CompoundDataType(123.45)
    floatVal = cdt1.asFloat()
    #print(floatVal)  
    assert(math.isclose(floatVal, 123.45, rel_tol=1e-7))

    cdt2 = CompoundDataType(cdt1)
    floatVal = cdt2.asFloat()
    #print(floatVal)
    assert(math.isclose(floatVal, 123.45, rel_tol=1e-7))
    assert(cdt1 == cdt2)

# run the unit test methods
testDefaultConstructor()
testConstructorFloat()
testConstructorUInt32()
testConstructorBool()
testConstructorString()
testAssignFloat()
testAssignUInt32()
testAssignBool()
testAssignString()
testCopyConstuct()
testCopyAssign()

