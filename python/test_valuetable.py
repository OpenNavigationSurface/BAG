from bagPy import *
import shutil, pathlib, math
import bagMetadataSamples, testUtils
import sys


# define constants used in multiple tests
datapath = str(pathlib.Path(__file__).parent.absolute()) + "/../examples/sample-data"
chunkSize = 100
compressionLevel = 6


# define the unit test methods:
print("Testing ValueTable")

def testReadEmpty():
    print("   Value Table testAddRecords()")
    tmpFile = testUtils.RandomFileGuard("name")

    metadata = Metadata()
    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)
    assert(metadata)

    dataset = Dataset.create(tmpFile.getName(), metadata, chunkSize, compressionLevel)
    assert(dataset)

    indexType = DT_UINT8;
    layerName = "elevation"
    kExpectedDefinitionSize = 4

    kFieldName0 = "float value"
    kFieldName1 = "uint32 value"
    kFieldName2 = "bool value"
    kFieldName3 = "string value"

    kExpFieldDef0 = FieldDefinition()
    kExpFieldDef0.name = kFieldName0
    kExpFieldDef0.type = DT_FLOAT32

    kExpFieldDef1 = FieldDefinition()
    kExpFieldDef1.name = kFieldName1
    kExpFieldDef1.type = DT_UINT32

    kExpFieldDef2 = FieldDefinition()
    kExpFieldDef2.name = kFieldName2
    kExpFieldDef2.type = DT_BOOLEAN

    kExpFieldDef3 = FieldDefinition()
    kExpFieldDef3.name = kFieldName3
    kExpFieldDef3.type = DT_STRING

    kExpectedDefinition = RecordDefinition(
        (kExpFieldDef0, kExpFieldDef1, kExpFieldDef2, kExpFieldDef3))

    compoundLayer = dataset.createCompoundLayer(indexType, layerName,
        kExpectedDefinition, chunkSize, compressionLevel)
    assert(compoundLayer)

    valueTable = compoundLayer.getValueTable()
    definition = valueTable.getDefinition()
    assert(len(definition) == kExpectedDefinitionSize)

    assert(definition[0] == kExpectedDefinition[0])
    assert(definition[1] == kExpectedDefinition[1])
    assert(definition[2] == kExpectedDefinition[2])
    assert(definition[3] == kExpectedDefinition[3])

    assert(len(valueTable.getRecords()) == 1)

    assert(valueTable.getFieldIndex(kFieldName0) == 0)
    assert(valueTable.getFieldIndex(kFieldName1) == 1)
    assert(valueTable.getFieldIndex(kFieldName2) == 2)
    assert(valueTable.getFieldIndex(kFieldName3) == 3)

    assert(valueTable.getFieldName(0) == kFieldName0)
    assert(valueTable.getFieldName(1) == kFieldName1)
    assert(valueTable.getFieldName(2) == kFieldName2)
    assert(valueTable.getFieldName(3) == kFieldName3)

    del dataset #ensure dataset is deleted before tmpFile


def testAddRecord():
    print("   Value Table testAddRecords()")
    tmpFile = testUtils.RandomFileGuard("name")

    kExpectedLayerName = getLayerTypeAsString(Uncertainty)

    # Write a record.
    metadata = Metadata()
    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)
    assert(metadata)

    dataset = Dataset.create(tmpFile.getName(), metadata, chunkSize, compressionLevel)
    assert(dataset)

    indexType = DT_UINT16

    # THe record definition.
    definition = RecordDefinition(7)
    definition[0].name = "first name"
    definition[0].type = DT_STRING
    definition[1].name = "last name"
    definition[1].type = DT_STRING
    definition[2].name = "float value0"
    definition[2].type = DT_FLOAT32
    definition[3].name = "bool value"
    definition[3].type = DT_BOOLEAN
    definition[4].name = "uint32 value"
    definition[4].type = DT_UINT32
    definition[5].name = "float value1"
    definition[5].type = DT_FLOAT32
    definition[6].name = "address"
    definition[6].type = DT_STRING

    layer = dataset.createCompoundLayer(indexType,
        kExpectedLayerName, definition, chunkSize, compressionLevel)

    # There is one No Data Value record at index 0.
    valueTable = layer.getValueTable()
    assert(len(valueTable.getRecords()) == 1)

    # A record matching the definition.
    kExpectedNewRecord0 = Record(7)
    kExpectedNewRecord0[0] = CompoundDataType("Bob")
    kExpectedNewRecord0[1] = CompoundDataType("Jones")
    kExpectedNewRecord0[2] = CompoundDataType(42.2)
    kExpectedNewRecord0[3] = CompoundDataType(True)
    kExpectedNewRecord0[4] = CompoundDataType(102)
    kExpectedNewRecord0[5] = CompoundDataType(1234.567)
    kExpectedNewRecord0[6] = CompoundDataType("101 Tape Drive")

    kExpectedNumRecords = 2

    index = valueTable.addRecord(kExpectedNewRecord0)
    assert(index == 1)
    assert(len(valueTable.getRecords()) == kExpectedNumRecords)


    # Read the new record.
    dataset = Dataset.openDataset(tmpFile.getName(), BAG_OPEN_READONLY)
    assert(dataset)

    layer = dataset.getCompoundLayer(kExpectedLayerName)
    assert(layer)

    valueTable = layer.getValueTable()
    records = valueTable.getRecords()
    assert(len(records) == kExpectedNumRecords)

    kRecordIndex = 1
    fieldIndex = 0

    field0value = valueTable.getValue(kRecordIndex, fieldIndex)
    assert(field0value == kExpectedNewRecord0[fieldIndex])
    fieldIndex += 1

    field1value = valueTable.getValue(kRecordIndex, fieldIndex)
    assert(field1value == kExpectedNewRecord0[fieldIndex])
    fieldIndex += 1

    field2value = valueTable.getValue(kRecordIndex, fieldIndex)
    assert(field2value == kExpectedNewRecord0[fieldIndex])
    fieldIndex += 1

    field3value = valueTable.getValue(kRecordIndex, fieldIndex)
    assert(field3value == kExpectedNewRecord0[fieldIndex])
    fieldIndex += 1

    field4value = valueTable.getValue(kRecordIndex, fieldIndex)
    assert(field4value == kExpectedNewRecord0[fieldIndex])
    fieldIndex += 1

    field5value = valueTable.getValue(kRecordIndex, fieldIndex)
    assert(field5value == kExpectedNewRecord0[fieldIndex])
    fieldIndex += 1

    field6value = valueTable.getValue(kRecordIndex, fieldIndex)
    assert(field6value == kExpectedNewRecord0[fieldIndex])
    fieldIndex += 1


    # Set some new values an existing record.
    dataset = Dataset.openDataset(tmpFile.getName(), BAG_OPEN_READ_WRITE)
    assert(dataset)

    layer = dataset.getCompoundLayer(kExpectedLayerName)
    assert(layer)

    valueTable = layer.getValueTable()

    records = valueTable.getRecords()
    assert(len(records) == kExpectedNumRecords)


    # Set the values.
    kRecordIndex = 1
    fieldIndex = 0

    kExpectedNewRecord1 = Record(7)
    kExpectedNewRecord1[0] = CompoundDataType("Ernie")
    kExpectedNewRecord1[1] = CompoundDataType("Jones")
    kExpectedNewRecord1[2] = CompoundDataType(987.6543)
    kExpectedNewRecord1[3] = CompoundDataType(False)
    kExpectedNewRecord1[4] = CompoundDataType(1001)
    kExpectedNewRecord1[5] = CompoundDataType(0.08642)
    kExpectedNewRecord1[6] = CompoundDataType("404 Disk Drive")

    valueTable.setValue(kRecordIndex, fieldIndex, kExpectedNewRecord1[fieldIndex])

    # Read values back from memory.
    field0value = valueTable.getValue(kRecordIndex, fieldIndex)
    assert(field0value == kExpectedNewRecord1[fieldIndex])

    fieldIndex += 1
    print(fieldIndex)
    valueTable.setValue(kRecordIndex, fieldIndex, kExpectedNewRecord1[fieldIndex])

    field1value = valueTable.getValue(kRecordIndex, fieldIndex)
    assert(field1value == kExpectedNewRecord1[fieldIndex])

    fieldIndex += 1
    valueTable.setValue(kRecordIndex, fieldIndex, kExpectedNewRecord1[fieldIndex])

    field2value = valueTable.getValue(kRecordIndex, fieldIndex)
    assert(field2value == kExpectedNewRecord1[fieldIndex])

    fieldIndex += 1
    valueTable.setValue(kRecordIndex, fieldIndex, kExpectedNewRecord1[fieldIndex])

    field3value = valueTable.getValue(kRecordIndex, fieldIndex)
    assert(field3value == kExpectedNewRecord1[fieldIndex])

    fieldIndex += 1
    valueTable.setValue(kRecordIndex, fieldIndex, kExpectedNewRecord1[fieldIndex])

    field4value = valueTable.getValue(kRecordIndex, fieldIndex)
    assert(field4value == kExpectedNewRecord1[fieldIndex])

    fieldIndex += 1
    valueTable.setValue(kRecordIndex, fieldIndex, kExpectedNewRecord1[fieldIndex])

    field5value = valueTable.getValue(kRecordIndex, fieldIndex)
    assert(field5value == kExpectedNewRecord1[fieldIndex])

    fieldIndex += 1
    valueTable.setValue(kRecordIndex, fieldIndex, kExpectedNewRecord1[fieldIndex])

    field6value = valueTable.getValue(kRecordIndex, fieldIndex)
    assert(field6value == kExpectedNewRecord1[fieldIndex])


    # Read new values back from the HDF5 file.
    dataset = Dataset.openDataset(tmpFile.getName(), BAG_OPEN_READONLY)
    assert(dataset)

    layer = dataset.getCompoundLayer(kExpectedLayerName)
    assert(layer)

    valueTable = layer.getValueTable()
    records = valueTable.getRecords()
    assert(len(records) == kExpectedNumRecords)

    kRecordIndex = 1
    fieldIndex = 0

    field0value = valueTable.getValue(kRecordIndex, fieldIndex)
    assert(field0value == kExpectedNewRecord1[fieldIndex])
    fieldIndex += 1

    field1value = valueTable.getValue(kRecordIndex, fieldIndex)
    assert(field1value == kExpectedNewRecord1[fieldIndex])
    fieldIndex += 1

    field2value = valueTable.getValue(kRecordIndex, fieldIndex)
    assert(field2value == kExpectedNewRecord1[fieldIndex])
    fieldIndex += 1

    field3value = valueTable.getValue(kRecordIndex, fieldIndex)
    assert(field3value == kExpectedNewRecord1[fieldIndex])
    fieldIndex += 1

    field4value = valueTable.getValue(kRecordIndex, fieldIndex)
    assert(field4value == kExpectedNewRecord1[fieldIndex])
    fieldIndex += 1

    field5value = valueTable.getValue(kRecordIndex, fieldIndex)
    assert(field5value == kExpectedNewRecord1[fieldIndex])
    fieldIndex += 1

    field6value = valueTable.getValue(kRecordIndex, fieldIndex)
    assert(field6value == kExpectedNewRecord1[fieldIndex])
    fieldIndex += 1

    del dataset #ensure dataset is deleted before tmpFile


def testAddRecords():
    print("   Value Table testAddRecords()")
    tmpFile = testUtils.RandomFileGuard("name")
    kExpectedLayerName = "elevation"

    kExpectedNewRecord0 = Record(3)
    kExpectedNewRecord0[0] = CompoundDataType(False)
    kExpectedNewRecord0[1] = CompoundDataType("string1")
    kExpectedNewRecord0[2] = CompoundDataType(True)

    # The expected Records.
    kExpectedRecords0 = Record(3)
    kExpectedRecords0[0] = CompoundDataType(True)
    kExpectedRecords0[1] = CompoundDataType("string 1")
    kExpectedRecords0[2] = CompoundDataType(True)
    kExpectedRecords1 = Record(3)
    kExpectedRecords1[0] = CompoundDataType(True)
    kExpectedRecords1[1] = CompoundDataType("string 2")
    kExpectedRecords1[2] = CompoundDataType(False)
    kExpectedRecords2 = Record(3)
    kExpectedRecords2[0] = CompoundDataType(False)
    kExpectedRecords2[1] = CompoundDataType("string 3")
    kExpectedRecords2[2] = CompoundDataType(True)
    kExpectedRecords3 = Record(3)
    kExpectedRecords3[0] = CompoundDataType(False)
    kExpectedRecords3[1] = CompoundDataType("string 4")
    kExpectedRecords3[2] = CompoundDataType(False)

    kExpectedRecords = Records(4)
    kExpectedRecords[0] = kExpectedRecords0
    kExpectedRecords[1] = kExpectedRecords1
    kExpectedRecords[2] = kExpectedRecords2
    kExpectedRecords[3] = kExpectedRecords3

    kExpectedNumRecords = len(kExpectedRecords) + 1;


    # Write a record.
    metadata = Metadata()
    metadata.loadFromBuffer(bagMetadataSamples.kMetadataXML)
    assert(metadata)

    dataset = Dataset.create(tmpFile.getName(), metadata, chunkSize, compressionLevel)
    assert(dataset)

    indexType = DT_UINT16

    definition = RecordDefinition(3)
    definition[0].name = "bool1"
    definition[0].type = DT_BOOLEAN
    definition[1].name = "string"
    definition[1].type = DT_STRING
    definition[2].name = "bool2"
    definition[2].type = DT_BOOLEAN

    layer = dataset.createCompoundLayer(indexType,
        kExpectedLayerName, definition, chunkSize, compressionLevel)

    valueTable = layer.getValueTable()
    assert(len(valueTable.getRecords()) == 1)

    valueTable.addRecords(kExpectedRecords)
    assert(len(valueTable.getRecords()) == kExpectedNumRecords)


    #Read the new records.
    dataset = Dataset.openDataset(tmpFile.getName(), BAG_OPEN_READONLY)
    assert(dataset)

    layer = dataset.getCompoundLayer(kExpectedLayerName)
    assert(layer)

    valueTable = layer.getValueTable()
    records = valueTable.getRecords()
    assert(len(records) == kExpectedNumRecords)

    kRecordIndex = 2
    fieldIndex = 1

    value = valueTable.getValue(kRecordIndex, fieldIndex)
    assert(value == kExpectedRecords[kRecordIndex - 1][fieldIndex])
    assert(value == records[kRecordIndex][fieldIndex])

    fieldName = "string"
    kExpectedFieldIndex = 1

    fieldIndex = valueTable.getFieldIndex(fieldName)
    assert(fieldIndex == kExpectedFieldIndex)

    value = valueTable.getValue(kRecordIndex, fieldName)
    assert(value == kExpectedRecords[kRecordIndex - 1][fieldIndex])
    assert(value == records[kRecordIndex][fieldIndex])

    del dataset #ensure dataset is deleted before tmpFile


# run the unit test methods

testReadEmpty()
testAddRecord()
testAddRecords()

print("test done")
