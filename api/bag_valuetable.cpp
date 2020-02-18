
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
        case BOOL:
            value = *reinterpret_cast<const bool*>(buffer + fieldOffset);
            break;
        case FLOAT32:
            value = *reinterpret_cast<const float*>(buffer + fieldOffset);
            break;
        case UINT32:
            value = *reinterpret_cast<const uint32_t*>(buffer + fieldOffset);
            break;
        case STRING:
        {
            const auto address =
                *reinterpret_cast<const std::uintptr_t*>(buffer + fieldOffset);
            str = reinterpret_cast<const char*>(address);
            value = std::string{str ? str : ""};

            // Clean up the char* allocated by HDF reading.
            free(const_cast<char*>(str));
            break;
        }
        case UINT8:  //[[fallthrough]]
        case UINT16:  //[[fallthrough]]
        case UINT64:  //[[fallthrough]]
        case COMPOUND:  //[[fallthrough]]
        case UNKNOWN_DATA_TYPE:  //[[fallthrough]]
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
        case BOOL:
            *reinterpret_cast<bool*>(buffer + fieldOffset) = field.asBool();
            break;
        case FLOAT32:
            *reinterpret_cast<float*>(buffer + fieldOffset) = field.asFloat();
            break;
        case UINT32:
            *reinterpret_cast<uint32_t*>(buffer + fieldOffset) = field.asUInt32();
            break;
        case STRING:
        {
            *reinterpret_cast<char**>(buffer + fieldOffset) =
                const_cast<char*>(field.asString().data());
            break;
        }
        case UINT8:  //[[fallthrough]]
        case UINT16:  //[[fallthrough]]
        case UINT64:  //[[fallthrough]]
        case COMPOUND:  //[[fallthrough]]
        case UNKNOWN_DATA_TYPE:  //[[fallthrough]]
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

    if (numRecords == 1)  // No user defined records.
        return;

    // Do not include the empty (NDV) record.
    --numRecords;
    m_records.resize(numRecords);

    constexpr hsize_t startIndex = 1;
    fileDataSpace.selectHyperslab(H5S_SELECT_SET, &numRecords, &startIndex);

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
    if (recordIndex >= m_records.size())
        throw RecordNotFound{};

    const size_t fieldIndex = this->getFieldIndex(name);

    return this->getValue(recordIndex, fieldIndex);
}

const CompoundDataType& ValueTable::getValue(
    size_t recordIndex,
    size_t fieldIndex) const &
{
    if (recordIndex >= m_records.size())
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
    if (recordIndex >= m_records.size())
        throw RecordNotFound{};

    const size_t fieldIndex = this->getFieldIndex(name);

    this->setValue(recordIndex, fieldIndex, value);
}

void ValueTable::setValue(
    size_t recordIndex,
    size_t fieldIndex,
    const CompoundDataType& value)
{
    if (recordIndex >= m_records.size())
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

        if (defType == UNKNOWN_DATA_TYPE || field.getType() == UNKNOWN_DATA_TYPE)
            return false;

        if (fieldType != defType)
            return false;
    }

    return true;
}

//! Write a record to the DataSet.
/*!
\param recordIndex
    The index to write the record to.  This does not include the empty (NDV)
    record at index 0, so this value will need to be increased by 1 to allow for
    that.
\param record
    The record to write.
*/
void ValueTable::writeRecord(
    size_t recordIndex,
    const Record& record)
{
    if (recordIndex > m_records.size())
        throw InvalidRecordsIndex{};

    // The record index must include the empty (NDV) record.
    const size_t fileRecordIndex = recordIndex + 1;

    // Prepare the memory details.
    const auto rawMemory = this->convertRecordToRaw(record);

    const auto& descriptor =
        dynamic_cast<const CompoundLayerDescriptor&>(m_layer.getDescriptor());

    const auto memDataType = createH5memoryCompType(descriptor.getDefinition());

    constexpr hsize_t one = 1;
    ::H5::DataSpace memDataSpace(1, &one, &one);

    // Prepare the file details.
    const auto& h5recordDataSet = m_layer.getRecordDataSet();

    if (recordIndex == m_records.size())
    {
        // Make room for a new record.
        const hsize_t newNumRecords = fileRecordIndex + 1;
        h5recordDataSet.extend(&newNumRecords);
    }

    const auto fileDataSpace = h5recordDataSet.getSpace();

    // select the record to write
    fileDataSpace.selectElements(H5S_SELECT_SET, 1, &fileRecordIndex);

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

    // Make room for the new records.  Account for the empty (NDV) record.
    const hsize_t newNumRecords = m_records.size() + numRecords + 1;
    h5recordDataSet.extend(&newNumRecords);

    const auto fileDataSpace = h5recordDataSet.getSpace();

    // specify the index to begin writing to.  account for the empty (NDV) record.
    const hsize_t indexToModify = m_records.size() + 1;
    fileDataSpace.selectHyperslab(H5S_SELECT_SET, &numRecords, &indexToModify);

    h5recordDataSet.write(rawMemory.data(), memDataType, memDataSpace,
        fileDataSpace);
}

}  // namespace BAG

#ifdef _MSC_VER
#pragma warning(pop)
#endif

