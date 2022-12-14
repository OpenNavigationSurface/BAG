%begin %{
#ifdef _MSC_VER
#ifdef SWIGPYTHON
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
#endif
%}

%module bag_compounddatatype

%{
#include "bag_compounddatatype.h"
%}

%import "bag_types.i"

%include <std_string.i>
%include <std_vector.i>
%include <stdint.i>

namespace BAG {

class CompoundDataType final {
public:
    CompoundDataType();
    explicit CompoundDataType(float value) noexcept;
    explicit CompoundDataType(uint32_t value) noexcept;
    explicit CompoundDataType(bool value) noexcept;
    explicit CompoundDataType(std::string value) noexcept;

    CompoundDataType(const CompoundDataType& other);
    %ignore CompoundDataType(CompoundDataType&& other);
    ~CompoundDataType() noexcept;

    %rename(assignConst) operator=(const CompoundDataType&);
    CompoundDataType& operator=(const CompoundDataType& rhs);
    %rename(assign) operator=(CompoundDataType&&);
    CompoundDataType& operator=(CompoundDataType&& rhs);
    %rename(assignFloat) operator=(float);
    CompoundDataType& operator=(float rhs);
    %rename(assignUInt32) operator=(uint32_t);
    CompoundDataType& operator=(uint32_t rhs);
    %rename(assignBool) operator=(bool);
    CompoundDataType& operator=(bool rhs);
    %rename(assignString) operator=(std::string);
    CompoundDataType& operator=(std::string rhs);

    bool operator==(const CompoundDataType& rhs) const noexcept;
    bool operator!=(const CompoundDataType &rhs) const noexcept;
    float asFloat() const;
    uint32_t asUInt32() const;
    bool asBool() const;
    %ignore asString() const&;
    std::string asString();
    DataType getType() const noexcept;
};

using Record = std::vector<BAG::CompoundDataType>;
using Records = std::vector<BAG::Record>;

template <typename T> T get(const CompoundDataType& v);
%template(getFloat) get<float>;
%template(getUInt32) get<uint32_t>;
%template(getBool) get<bool>;
%template(getString) get<std::string>;

%extend CompoundDataType {
    %pythoncode %{
    def __eq__(self, other):
        self_type = self.getType()
        if self_type == other.getType():
            if self_type == DT_FLOAT32:
                return self.asFloat() == other.asFloat()
            if self_type == DT_BOOLEAN:
                return self.asBool() == other.asBool()
            if self_type == DT_UNKNOWN_DATA_TYPE:
                return True
            if self_type == DT_COMPOUND:
                return self == other
            if self_type == DT_STRING:
                return self.asString() == other.asString()
            else:
                # Lump DT_UINT8, DT_UINT16, DT_UINT32 together
                return self.asUInt32() == other.asUInt32()

        return False
    %}
};

}; // namespace BAG

%template(Record) std::vector<BAG::CompoundDataType>;
%template(Records) std::vector<BAG::Record>;
