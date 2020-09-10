
#include "bag_compoundlayerdescriptor.h"
#include "bag_dataset.h"
#include "bag_exceptions.h"
#include "bag_hdfhelper.h"
#include "bag_private.h"
#include "bag_valuetable.h"

#include <algorithm>
#include <array>
#include <H5Cpp.h>


namespace BAG {

namespace {

//! Convert a chunk of memory and a definition into a Record.
/*!
\param buffer
    A chunk of memory representing a Record for a compound layer.
\param definition
    The definition of the Record for a compound layer.

\return
    A compound layer record using the buffer and definition.
*/
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
        case DT_BOOLEAN:
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

            // Clean up the char* allocated by HDF reading.
            free(const_cast<char*>(str));
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

//! Convert a Record into a chunk of memory.
/*!
\param record
    The record to convert into a chunk of memory.
\param buffer
    The memory to store the record in.
*/
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
        case DT_BOOLEAN:
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
            *reinterpret_cast<char**>(buffer + fieldOffset) =
                const_cast<char*>(field.asString().data());
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

//! Constructor.
/*!
\param layer
    The layer the value table holds records/values for.
*/
ValueTable::ValueTable(
    const CompoundLayer& layer)
    : m_layer(layer)
{
    // Read the Records DataSet.
    const auto& h5valueDataSet = m_layer.getValueDataSet();

    const auto fileDataSpace = h5valueDataSet.getSpace();
    hsize_t numRecords = 0;
    const auto numDims = fileDataSpace.getSimpleExtentDims(&numRecords);
    if (numDims != 1)
        throw InvalidValueSize{};

    m_records.resize(numRecords);

    if (numRecords == 1)  // No user defined records.
        return;

    constexpr hsize_t startIndex = 0;
    fileDataSpace.selectHyperslab(H5S_SELECT_SET, &numRecords, &startIndex);

    auto pDescriptor = std::dynamic_pointer_cast<const CompoundLayerDescriptor>(
        m_layer.getDescriptor());
    const auto& definition = pDescriptor->getDefinition();

    const size_t recordSize = getRecordSize(definition);
    std::vector<uint8_t> buffer(recordSize * numRecords, 0);

    const auto memDataType = createH5memoryCompType(definition);
    const ::H5::DataSpace memDataSpace;
    memDataSpace.setExtentSimple(1, &numRecords);

    h5valueDataSet.read(buffer.data(), memDataType, memDataSpace, fileDataSpace);

    // Convert the raw memory into Records.
    size_t rawIndex = 0;

    for (auto& record : m_records)
    {
        record = convertMemoryToRecord(buffer.data() + (rawIndex * recordSize),
            definition);
        ++rawIndex;
    }
}

//! Add a record/value to the end of the list.
/*!
\param record
    The record/value.

\return
    The index of the added record.
*/
size_t ValueTable::addRecord(
    const Record& record)
{
    if (!this->validateRecord(record))
        throw InvalidValue{};

    const auto newKey = m_records.size();
    this->writeRecord(newKey, record);

    m_records.emplace_back(record);

    return newKey;
}

//! Add multiple records/values to the end of the list.
/*!
\param records
    The records/values.
*/
void ValueTable::addRecords(
    const Records& records)
{
    if (records.empty())
        return;

    const bool allValid = std::all_of(cbegin(records), cend(records),
        [this](const auto& record) {
            return this->validateRecord(record);
        });
    if (!allValid)
        throw InvalidValue{};

    this->writeRecords(records);

    m_records.insert(end(m_records), cbegin(records), cend(records));
}

//! Convert a record/value to a chunk of memory.
/*!
\param record
    The record/value to convert.

\return
    A copy of the record/value as a chunk of memory.
*/
std::vector<uint8_t> ValueTable::convertRecordToRaw(
    const Record& record) const
{
    auto pDescriptor =
        std::dynamic_pointer_cast<const CompoundLayerDescriptor>(
            m_layer.getDescriptor());

    std::vector<uint8_t> buffer(getRecordSize(pDescriptor->getDefinition()), 0);

    convertRecordToMemory(record, buffer.data());

    return buffer;
}

//! Convert multiple records/values to a chunk of memory.
/*!
\param records
    The records/values to convert.

\return
    A copy of the records/values as a chunk of memory.
*/
std::vector<uint8_t> ValueTable::convertRecordsToRaw(
    const std::vector<Record>& records) const
{
    if (records.empty())
        return {};

    auto pDescriptor =
        std::dynamic_pointer_cast<const CompoundLayerDescriptor>(
            m_layer.getDescriptor());

    const auto recordSize = getRecordSize(pDescriptor->getDefinition());
    std::vector<uint8_t> buffer(recordSize * records.size(), 0);

    // Write values into memory.
    size_t offset = 0;

    for (const auto& record : records)
    {
        convertRecordToMemory(record, buffer.data() + offset);
        offset += recordSize;
    }

    return buffer;
}

//! Retrieve the record/value definition.
/*!
\return
    The list of fields that define the record/value.
*/
const RecordDefinition& ValueTable::getDefinition() const & noexcept
{
    return std::dynamic_pointer_cast<const CompoundLayerDescriptor>(
        m_layer.getDescriptor())->getDefinition();
}

//! Retrieve the value of a specific field in a specific record.
/*!
\param key
    The key of the record/value.
    Must be greater than 0.
\param name
    The name of the field.

\return
    The value specified.
    An exception is thrown if the key or field name is invalid.
*/
const CompoundDataType& ValueTable::getValue(
    size_t key,
    const std::string& name) const &
{
    if (key == 0 || key >= m_records.size())
        throw ValueNotFound{};

    const size_t fieldIndex = this->getFieldIndex(name);

    return this->getValue(key, fieldIndex);
}

//! Retrieve the value of a specific field in a specific record/value.
/*!
\param key
    The key of the record/value.
    Must be greater than 0.
\param fieldIndex
    The index of the field.

\return
    The value specified.
    An exception is thrown if the record index or field index are invalid.
*/
const CompoundDataType& ValueTable::getValue(
    size_t key,
    size_t fieldIndex) const &
{
    if (key == 0 || key >= m_records.size())
        throw ValueNotFound{};

    const auto& definition = this->getDefinition();
    if (fieldIndex >= definition.size())
        throw FieldNotFound{};

    const auto& record = m_records[key];

    return record[fieldIndex];
}

//! Retrieve the field index of the named field.
/*!
\param name
    The name of the field.

\return
    The field index of the named field.
*/
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

//! Retrieve the field name of the indexed field.
/*!
\param index
    The index of the field.

\return
    The field name of the indexed field.
*/
const char* ValueTable::getFieldName(
    size_t index) const &
{
    const auto& definition = this->getDefinition();
    if (index >= definition.size())
        throw FieldNotFound{};

    return definition[index].name;
}

//! Retrieve all the records/values.
/*!
\return
    All the records/values.
    NOTE!  This includes the no data value record at index 0.
*/
const Records& ValueTable::getRecords() const & noexcept
{
    return m_records;
}

//! Set a value in a specific field in a specific record.
/*!
\param key
    The record/value key.
    Must be greater than 0.
\param name
    The name of the field.
\param value
    The value to put into the field.
*/
void ValueTable::setValue(
    size_t key,
    const std::string& name,
    const CompoundDataType& value)
{
    if (key == 0 || key >= m_records.size())
        throw ValueNotFound{};

    const size_t fieldIndex = this->getFieldIndex(name);

    this->setValue(key, fieldIndex, value);
}

//! Set a value in a specific field in a specific record.
/*!
\param key
    The record key.
    Must be greater than 0.
\param fieldIndex
    The index of the field.
\param value
    The value to put into the field.
*/
void ValueTable::setValue(
    size_t key,
    size_t fieldIndex,
    const CompoundDataType& value)
{
    if (key == 0 || key >= m_records.size())
        throw ValueNotFound{};

    auto& record = m_records[key];
    record[fieldIndex] = value;

    this->writeRecord(key, record);
}

//! Determine if the specified record/value matches the definition used by the value table.
/*!
\param record
    The record/value.

\return
    \e true if the record/value matches the definition.
    \e false otherwise
*/
bool ValueTable::validateRecord(
    const Record& record) const
{
    auto pDescriptor =
        std::dynamic_pointer_cast<const CompoundLayerDescriptor>(
            m_layer.getDescriptor());
    if (!pDescriptor)
        throw InvalidDescriptor{};

    const auto& definition = pDescriptor->getDefinition();

    if (record.size() != definition.size())
        return false;

    size_t defIndex = 0;

    for (const auto& field : record)
    {
        const auto defType = static_cast<DataType>(definition[defIndex++].type);

        if (defType == DT_UNKNOWN_DATA_TYPE
            || field.getType() == DT_UNKNOWN_DATA_TYPE)
            return false;

        const auto fieldType = field.getType();
        if (fieldType != defType)
            return false;
    }

    return true;
}

//! Write a record/value to the HDF5 DataSet at the specified key.
/*!
\param key
    The record key.
    Must be greater than 0.
\param record
    The record/value to write.
*/
void ValueTable::writeRecord(
    size_t key,
    const Record& record)
{
    if (key == 0 || key > m_records.size())
        throw InvalidValueKey{};

    const hsize_t fileRecordIndex = key;

    // Prepare the memory details.
    const auto rawMemory = this->convertRecordToRaw(record);

    auto pDescriptor =
        std::dynamic_pointer_cast<const CompoundLayerDescriptor>(
            m_layer.getDescriptor());

    const auto memDataType = createH5memoryCompType(pDescriptor->getDefinition());

    constexpr hsize_t one = 1;
    ::H5::DataSpace memDataSpace(1, &one, &one);

    // Prepare the file details.
    const auto& h5valueDataSet = m_layer.getValueDataSet();

    if (key == m_records.size())
    {
        // Make room for a new record.
        const hsize_t newNumRecords = fileRecordIndex + 1;
        h5valueDataSet.extend(&newNumRecords);
    }

    const auto fileDataSpace = h5valueDataSet.getSpace();

    // select the record to write
    fileDataSpace.selectElements(H5S_SELECT_SET, 1, &fileRecordIndex);

    h5valueDataSet.write(rawMemory.data(), memDataType, memDataSpace,
        fileDataSpace);
}

//! Write multiple records/values at the end of the list to the HDF5 DataSet.
/*!
\param records
    The records/values.
*/
void ValueTable::writeRecords(
    const std::vector<Record>& records)
{
    if (records.empty())
        return;

    // Prepare the memory details.
    const auto rawMemory = this->convertRecordsToRaw(records);

    auto pDescriptor =
        std::dynamic_pointer_cast<const CompoundLayerDescriptor>(
            m_layer.getDescriptor());

    const auto memDataType = createH5memoryCompType(pDescriptor->getDefinition());

    const hsize_t numRecords = records.size();
    ::H5::DataSpace memDataSpace(1, &numRecords, &numRecords);

    // Prepare the file details.
    const auto& h5valueDataSet = m_layer.getValueDataSet();

    // Make room for the new records.
    const hsize_t newNumRecords = m_records.size() + numRecords;
    h5valueDataSet.extend(&newNumRecords);

    const auto fileDataSpace = h5valueDataSet.getSpace();

    // Specify the key to begin writing to.
    const hsize_t keyToModify = m_records.size();
    fileDataSpace.selectHyperslab(H5S_SELECT_SET, &numRecords, &keyToModify);

    h5valueDataSet.write(rawMemory.data(), memDataType, memDataSpace,
        fileDataSpace);
}

}  // namespace BAG

