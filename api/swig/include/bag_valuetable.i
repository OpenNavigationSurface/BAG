%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_valuetable

%{
#include "bag_valuetable.h"
#include "bag_compounddatatype.h"
%}

%include "bag_compounddatatype.i"

%include <std_string.i>
%include <stdint.i>


namespace BAG {

class ValueTable final
{
public:
    ValueTable(const ValueTable&) = delete;
    ValueTable(ValueTable&&) = delete;

    ValueTable& operator=(const ValueTable&) = delete;
    ValueTable& operator=(ValueTable&&) = delete;

    const Records& getRecords() const & noexcept;
    const RecordDefinition& getDefinition() const & noexcept;
    const CompoundDataType& getValue(size_t recordIndex,
        const std::string& name) const &;
    const CompoundDataType& getValue(size_t recordIndex,
        size_t fieldIndex) const &;

    size_t getFieldIndex(const std::string& name) const;
    const char* getFieldName(size_t index) const &;

    size_t addRecord(const Record& record);
    void addRecords(const Records& records);

    void setValue(size_t recordIndex, const std::string& name,
        const CompoundDataType& value);
    void setValue(size_t recordIndex, size_t fieldIndex,
        const CompoundDataType& value);
};

%rename(_addRecord) addRecord;
%extend ValueTable {
    %pythoncode %{
        def addRecord(self, record):
            """
              Override addRecord to get around SWIG mis-handling (which may be due to
              mis-configuration) of std::vector boxing and unboxing.
            """
            new_rec = Record()
            for r in record:
                new_rec.append(r)
            return _bagPyd.ValueTable_addRecord(self, new_rec)
    %}
};

}  // namespace BAG

