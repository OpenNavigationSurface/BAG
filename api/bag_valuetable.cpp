
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
    The layer the value table holds records for.
*/
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

    const size_t recordSize = getRecordSize(definition);
    std::vector<uint8_t> buffer(recordSize * numRecords, 0);

    const auto memDataType = createH5memoryCompType(definition);
    const ::H5::DataSpace memDataSpace;
    memDataSpace.setExtentSimple(1, &numRecords);

    h5recordDataSet.read(buffer.data(), memDataType, memDataSpace, fileDataSpace);

    // Convert the raw memory into Records.
    size_t rawIndex = 0;

    for (auto& record : m_records)
    {
        record = convertMemoryToRecord(buffer.data() + (rawIndex * recordSize),
            definition);
        ++rawIndex;
    }
}

//! Add a record to the end of the list.
/*!
\param record
    The record.

\return
    The number of records.
*/
size_t ValueTable::addRecord(
    const Record& record)
{
    if (!this->validateRecord(record))
        throw InvalidRecord{};

    this->writeRecord(m_records.size(), record);

    m_records.emplace_back(record);

    return m_records.size() - 1;
}

//! Add multiple records to the end of the list.
/*!
\param records
    The records.
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
        throw InvalidRecord{};

    this->writeRecords(records);

    m_records.insert(end(m_records), cbegin(records), cend(records));
}

//! Convert a record to a chunk of memory.
/*!
\param record
    The record to convert.

\return
    A copy of the record as a chunk of memory.
*/
std::vector<uint8_t> ValueTable::convertRecordToRaw(
    const Record& record) const
{
    const auto& descriptor =
        dynamic_cast<const CompoundLayerDescriptor&>(m_layer.getDescriptor());

    std::vector<uint8_t> buffer(getRecordSize(descriptor.getDefinition()), 0);

    convertRecordToMemory(record, buffer.data());

    return buffer;
}

//! Convert multiple records to a chunk of memory.
/*!
\param records
    The record to convert.

\return
    A copy of the records as a chunk of memory.
*/
std::vector<uint8_t> ValueTable::convertRecordsToRaw(
    const std::vector<Record>& records) const
{
    if (records.empty())
        return {};

    const auto& descriptor =
        dynamic_cast<const CompoundLayerDescriptor&>(m_layer.getDescriptor());

    const auto recordSize = getRecordSize(descriptor.getDefinition());
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

//! Retrieve the record definition.
/*!
\return
    The list of fields that define the record.
*/
const RecordDefinition& ValueTable::getDefinition() const & noexcept
{
    return dynamic_cast<const CompoundLayerDescriptor&>(
        m_layer.getDescriptor()).getDefinition();
}

//! Retrieve the value of a specific field in a specific record.
/*!
\param recordIndex
    The index of the record.
\param name
    The name of the field.

\return
    The value specified.
    An exception is thrown if the record index or field name are invalid.
*/
const CompoundDataType& ValueTable::getValue(
    size_t recordIndex,
    const std::string& name) const &
{
    if (recordIndex >= m_records.size())
        throw RecordNotFound{};

    const size_t fieldIndex = this->getFieldIndex(name);

    return this->getValue(recordIndex, fieldIndex);
}

//! Retrieve the value of a specific field in a specific record.
/*!
\param recordIndex
    The index of the record.
\param fieldIndex
    The index of the field.

\return
    The value specified.
    An exception is thrown if the record index or field index are invalid.
*/
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

//! Retrieve all the records.
/*!
\return
    All the records.
*/
const Records& ValueTable::getRecords() const & noexcept
{
    return m_records;
}

//! Set a value in a specific field in a specific record.
/*!
\param recordIndex
    The record index.
\param name
    The name of the field.
\param value
    The value to put into the field.
*/
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

//! Set a value in a specific field in a specific record.
/*!
\param recordIndex
    The record index.
\param fieldIndex
    The index of the field.
\param value
    The value to put into the field.
*/
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

//! Determine if the specified record matches the definition used by the value table.
/*!
\param record
    The record.

\return
    \e true if the record matches the definition.
    \e false otherwise
*/
bool ValueTable::validateRecord(
    const Record& record) const
{
    const auto* descriptor =
        dynamic_cast<const CompoundLayerDescriptor*>(&m_layer.getDescriptor());
    if (!descriptor)
        throw InvalidDescriptor{};

    const auto& definition = descriptor->getDefinition();

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

//! Write a record to the HDF5 DataSet at the specified recordIndex.
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
    const hsize_t fileRecordIndex = recordIndex + 1;

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

//! Write multiple records at the end of the list to the HDF5 DataSet.
/*!
\param records
    The records.
*/
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

