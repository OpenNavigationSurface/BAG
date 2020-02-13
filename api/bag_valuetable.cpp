
#include "bag_compoundlayerdescriptor.h"
#include "bag_dataset.h"
#include "bag_exceptions.h"
#include "bag_hdfhelper.h"
#include "bag_private.h"
#include "bag_valuetable.h"

#include <algorithm>
#include <array>
#include <H5Cpp.h>


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

namespace BAG {

namespace {

//! Convert a raw chunk of memory and a definition into a Record.
Record convertMemoryToRecord(
    const uint8_t* const buffer,
    const RecordDefinition& definition)
{
    if (!buffer)
        return {};

    Record record;
    record.reserve(definition.size());

    size_t fieldOffset = 0;

    for (const auto& field : definition)
    {
        const auto fieldType = static_cast<DataType>(field.type);
        CompoundDataType value;
        const char* str = nullptr;

        switch(fieldType)
        {
        case DT_BOOL:
            value = *reinterpret_cast<const bool*>(buffer + fieldOffset);
            break;
        case DT_FLOAT32:
            value = *reinterpret_cast<const float*>(buffer + fieldOffset);
            break;
        case DT_UINT32:
            value = *reinterpret_cast<const uint32_t*>(buffer + fieldOffset);
            break;
        case DT_STRING:
        {
            const auto address =
                *reinterpret_cast<const std::uintptr_t*>(buffer + fieldOffset);
            str = reinterpret_cast<const char*>(address);
            value = std::string{str ? str : ""};
            //TODO Figure out where to clean up the allocation of the char* done by HDF5 (a copy is made above).
            //free(str);
            break;
        }
        case DT_UINT8:  //[[fallthrough]]
        case DT_UINT16:  //[[fallthrough]]
        case DT_UINT64:  //[[fallthrough]]
        case DT_COMPOUND:  //[[fallthrough]]
        case DT_UNKNOWN_DATA_TYPE:  //[[fallthrough]]
        default:
            throw UnsupportedDataType{};
        }

        record.emplace_back(value);
        fieldOffset += Layer::getElementSize(fieldType);
    }

    return record;
}

//! Convert a Record into a raw chunk of memory.
void convertRecordToMemory(
    const Record& record,
    uint8_t* buffer)
{
    size_t fieldOffset = 0;  // offset into buffer to the field

    for (const auto& field : record)
    {
        const auto fieldType = field.getType();
        switch(fieldType)
        {
        case DT_BOOL:
            *reinterpret_cast<bool*>(buffer + fieldOffset) = field.asBool();
            break;
        case DT_FLOAT32:
            *reinterpret_cast<float*>(buffer + fieldOffset) = field.asFloat();
            break;
        case DT_UINT32:
            *reinterpret_cast<uint32_t*>(buffer + fieldOffset) = field.asUInt32();
            break;
        case DT_STRING:
        {
            auto* ptr = reinterpret_cast<std::uintptr_t*>(buffer + fieldOffset);
            *ptr = reinterpret_cast<std::uintptr_t>(field.asString().data());
            break;
        }
        case DT_UINT8:  //[[fallthrough]]
        case DT_UINT16:  //[[fallthrough]]
        case DT_UINT64:  //[[fallthrough]]
        case DT_COMPOUND:  //[[fallthrough]]
        case DT_UNKNOWN_DATA_TYPE:  //[[fallthrough]]
        default:
            throw UnsupportedDataType{};
        }

        fieldOffset += Layer::getElementSize(fieldType);
    }
}

}  // namespace

ValueTable::ValueTable(
    const CompoundLayer& layer)
    : m_layer(layer)
{
    // Read the Records DataSet.
    const auto& h5recordDataSet = m_layer.getRecordDataSet();

    const auto fileDataSpace = h5recordDataSet.getSpace();
    const auto numDims = fileDataSpace.getSimpleExtentNdims();
    if (numDims != 1)
        throw InvalidCompoundRecordsSize{};

    hsize_t numRecords = 0;
    fileDataSpace.getSimpleExtentDims(&numRecords);

    m_records.resize(numRecords);

    const auto& descriptor =
        dynamic_cast<const CompoundLayerDescriptor&>(m_layer.getDescriptor());
    const auto& definition = descriptor.getDefinition();

    const size_t recordSize = getH5compSize(definition);
    std::vector<uint8_t> buffer(recordSize * numRecords, 0);

    const auto memDataType = createH5memoryCompType(definition);
    const ::H5::DataSpace memDataSpace;
    memDataSpace.setExtentSimple(1, &numRecords);

    h5recordDataSet.read(buffer.data(), memDataType, memDataSpace, fileDataSpace);

    //TODO copy this into bag.cpp; convert when receiving void* from C.
    // Convert the raw memory into Records.
    size_t rawIndex = 0;

    for (auto& record : m_records)
    {
        record = convertMemoryToRecord(buffer.data() + (rawIndex * recordSize),
            definition);
        ++rawIndex;
    }

    //TODO free all the variable length strings (char*) in buffer.
}

void ValueTable::addRecord(
    const Record& record)
{
    if (!this->validateRecord(record))
        throw InvalidRecord{};

    this->writeRecord(m_records.size(), record);

    m_records.emplace_back(record);
}

void ValueTable::addRecords(
    const Records& records)
{
    const bool allValid = std::all_of(cbegin(records), cend(records),
        [this](const auto& record) {
            return this->validateRecord(record);
        });
    if (!allValid)
        throw InvalidRecord{};

    this->writeRecords(records);

    m_records.insert(end(m_records), cbegin(records), cend(records));
}

std::vector<uint8_t> ValueTable::convertRecordToRaw(
    const Record& record) const
{
    const auto& descriptor =
        dynamic_cast<const CompoundLayerDescriptor&>(m_layer.getDescriptor());

    std::vector<uint8_t> buffer(getH5compSize(descriptor.getDefinition()), 0);

    convertRecordToMemory(record, buffer.data());

    return buffer;
}

std::vector<uint8_t> ValueTable::convertRecordsToRaw(
    const std::vector<Record>& records) const
{
    const auto& descriptor =
        dynamic_cast<const CompoundLayerDescriptor&>(m_layer.getDescriptor());

    const auto h5RecordSize = getH5compSize(descriptor.getDefinition());
    std::vector<uint8_t> buffer(h5RecordSize * records.size(), 0);

    // Write values into memory.
    size_t offset = 0;

    for (const auto& record : records)
    {
        convertRecordToMemory(record, buffer.data() + offset);
        offset += h5RecordSize;
    }

    return buffer;
}

const RecordDefinition& ValueTable::getDefinition() const & noexcept
{
    return dynamic_cast<const CompoundLayerDescriptor&>(
        m_layer.getDescriptor()).getDefinition();
}

const CompoundDataType& ValueTable::getValue(
    size_t recordIndex,
    const std::string& name) const &
{
    if (recordIndex == 0 || recordIndex >= m_records.size())
        throw RecordNotFound{};

    const size_t fieldIndex = this->getFieldIndex(name);

    return this->getValue(recordIndex, fieldIndex);
}

const CompoundDataType& ValueTable::getValue(
    size_t recordIndex,
    size_t fieldIndex) const &
{
    if (recordIndex == 0 || recordIndex >= m_records.size())
        throw RecordNotFound{};

    const auto& definition = this->getDefinition();
    if (fieldIndex >= definition.size())
        throw FieldNotFound{};

    const auto& record = m_records[recordIndex];

    return record[fieldIndex];
}

size_t ValueTable::getFieldIndex(
    const std::string& name) const
{
    const auto& definition = this->getDefinition();
    size_t fieldIndex = 0;

    for (const auto& field : definition)
    {
        if (name == field.name)
            return fieldIndex;

        ++fieldIndex;
    }

    throw FieldNotFound{};
}

const char* ValueTable::getFieldName(
    size_t index) const &
{
    const auto& definition = this->getDefinition();
    if (index >= definition.size())
        throw FieldNotFound{};

    return definition[index].name;
}

const Records& ValueTable::getRecords() const & noexcept
{
    return m_records;
}

void ValueTable::setValue(
    size_t recordIndex,
    const std::string& name,
    const CompoundDataType& value)
{
    if (recordIndex == 0 || recordIndex >= m_records.size())
        throw RecordNotFound{};

    const size_t fieldIndex = this->getFieldIndex(name);

    this->setValue(recordIndex, fieldIndex, value);
}

void ValueTable::setValue(
    size_t recordIndex,
    size_t fieldIndex,
    const CompoundDataType& value)
{
    if (recordIndex == 0 || recordIndex >= m_records.size())
        throw RecordNotFound{};

    auto& record = m_records[recordIndex];
    record[fieldIndex] = value;

    this->writeRecord(recordIndex, record);
}

//! Compare the record to the definition.  Not valid if any type is unknown.
bool ValueTable::validateRecord(
    const Record& record) const noexcept
{
    const auto& descriptor =
        dynamic_cast<const CompoundLayerDescriptor&>(m_layer.getDescriptor());

    const auto& definition = descriptor.getDefinition();

    if (record.size() != definition.size())
        return false;

    size_t defIndex = 0;

    for (const auto& field : record)
    {
        const auto defType = static_cast<DataType>(definition[defIndex++].type);
        const auto fieldType = field.getType();

        if (defType == DT_UNKNOWN_DATA_TYPE || field.getType() == DT_UNKNOWN_DATA_TYPE)
            return false;

        if (fieldType != defType)
            return false;
    }

    return true;
}

void ValueTable::writeRecord(
    size_t recordIndex,
    const Record& record)
{
    // Prepare the memory details.
    const auto rawMemory = this->convertRecordToRaw(record);

    const auto& descriptor =
        dynamic_cast<const CompoundLayerDescriptor&>(m_layer.getDescriptor());

    const auto memDataType = createH5memoryCompType(descriptor.getDefinition());

#if 0
{
const auto size = memDataType.getSize();
const auto numMembers = memDataType.getNmembers();
const auto m1Type = memDataType.getMemberClass(0);
const auto m1Offset = memDataType.getMemberOffset(0);
const auto member1 = memDataType.getMemberDataType(0);
const auto m1StrType = memDataType.getMemberStrType(0);
const auto m1Cset = m1StrType.getCset();
const auto m1Strpad = m1StrType.getStrpad();
const auto m1Size = member1.getSize();
const auto m2Type = memDataType.getMemberClass(1);
const auto m2Offset = memDataType.getMemberOffset(1);
const auto member2 = memDataType.getMemberDataType(1);
const auto m2Size = member2.getSize();
int j = 42; j;
}
#endif

    constexpr hsize_t one = 1;
    ::H5::DataSpace memDataSpace(1, &one, &one);

    // Prepare the file details.
    const auto& h5recordDataSet = m_layer.getRecordDataSet();

#if 0
{
const auto& fileDataType = h5recordDataSet.getCompType();
const bool dataTypesEqual = memDataType == fileDataType;
const auto size = fileDataType.getSize();
const auto numMembers = fileDataType.getNmembers();
const auto m1Type = fileDataType.getMemberClass(0);
const auto m1Offset = fileDataType.getMemberOffset(0);
const auto member1 = fileDataType.getMemberDataType(0);
const auto m1StrType = fileDataType.getMemberStrType(0);
const auto m1Cset = m1StrType.getCset();
const auto m1Strpad = m1StrType.getStrpad();
const auto m1Size = member1.getSize();
const auto m2Type = fileDataType.getMemberClass(1);
const auto m2Offset = fileDataType.getMemberOffset(1);
const auto member2 = fileDataType.getMemberDataType(1);
const auto m2Size = member2.getSize();
int j = 42; j;
}
#endif

    if (recordIndex == m_records.size())
    {
        // Make room for a new record.
        const hsize_t newNumRecords = recordIndex + 1;
        h5recordDataSet.extend(&newNumRecords);
    }
    else if (recordIndex > m_records.size())
        throw 123;  // trying to add a new record beyond the end

    const auto fileDataSpace = h5recordDataSet.getSpace();

    // select the record to write
    const hsize_t indexToModify = recordIndex;
    fileDataSpace.selectElements(H5S_SELECT_SET, 1, &indexToModify);

#if 0
{
const auto elemNpts = fileDataSpace.getSelectElemNpoints();
const auto numPts = fileDataSpace.getSelectNpoints();
int j = 42;  j;
}
#endif

    h5recordDataSet.write(rawMemory.data(), memDataType, memDataSpace,
        fileDataSpace);
}

void ValueTable::writeRecords(
    const std::vector<Record>& records)
{
    if (records.empty())
        return;

    // Prepare the memory details.
    const auto rawMemory = this->convertRecordsToRaw(records);

    const auto& descriptor =
        dynamic_cast<const CompoundLayerDescriptor&>(m_layer.getDescriptor());

    const auto memDataType = createH5memoryCompType(descriptor.getDefinition());
    const hsize_t numRecords = records.size();
    ::H5::DataSpace memDataSpace(1, &numRecords, &numRecords);

    // Prepare the file details.
    const auto& h5recordDataSet = m_layer.getRecordDataSet();

    // Make room for the new records.
    const hsize_t newNumRecords = m_records.size() + numRecords;
    h5recordDataSet.extend(&newNumRecords);

    const auto fileDataSpace = h5recordDataSet.getSpace();

    // select the record to write
    const hsize_t indexToModify = m_records.size();
    fileDataSpace.selectElements(H5S_SELECT_SET, numRecords, &indexToModify);

    h5recordDataSet.write(rawMemory.data(), memDataType, memDataSpace,
        fileDataSpace);
}

}  // namespace BAG

#ifdef _MSC_VER
#pragma warning(pop)
#endif

