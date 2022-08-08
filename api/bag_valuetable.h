#ifndef BAG_VALUE_TABLE_H
#define BAG_VALUE_TABLE_H

#include "bag_config.h"
#include "bag_fordec.h"

#include <memory>


namespace BAG {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)  // std classes do not have DLL-interface when exporting
#endif

//! The interface for the values of the spatial metadata in the compound layer.
class BAG_API ValueTable final
{
public:
    ValueTable(const ValueTable&) = delete;
    ValueTable(ValueTable&&) = delete;

    ValueTable& operator=(const ValueTable&) = delete;
    ValueTable& operator=(ValueTable&&) = delete;

    // Don't overload == because there is a circular reference between CompoundLayer
    //   ValueTable and implementing == on ValueTable causes compilers problems.

    const Records& getRecords() const & noexcept;
    const RecordDefinition& getDefinition() const & noexcept;
    const CompoundDataType& getValue(size_t key,
        const std::string& name) const &;
    const CompoundDataType& getValue(size_t key,
        size_t fieldIndex) const &;

    size_t getFieldIndex(const std::string& name) const;
    const char* getFieldName(size_t index) const &;

    size_t addRecord(const Record& record);
    void addRecords(const Records& records);
    void setValue(size_t key, const std::string& name,
        const CompoundDataType& value);
    void setValue(size_t key, size_t fieldIndex,
        const CompoundDataType& value);

protected:
    explicit ValueTable(const CompoundLayer& layer);

private:
    std::vector<uint8_t> convertRecordToRaw(const Record& record) const;
    std::vector<uint8_t> convertRecordsToRaw(
        const std::vector<Record>& records) const;

    bool validateRecord(const Record& record) const;

    void writeRecord(size_t key, const Record& record);
    void writeRecords(const std::vector<Record>& records);

    //! The layer these records pertain to.
    const CompoundLayer& m_layer;
    //! The stored values.
    Records m_records;

    friend CompoundLayer;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

}  // namespace BAG

#endif  // BAG_VALUE_TABLE_H

