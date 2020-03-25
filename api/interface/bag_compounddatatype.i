%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_compounddatatype

%{
#include "bag_compounddatatype.h"
%}

#define final

// note: had to change nested union in .h file as described here: 
// https://stackoverflow.com/questions/19191211/constructor-initialization-of-a-named-union-member

%import "bag_types.i"

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
        %rename(assignInt) operator=(uint32_t);
        CompoundDataType& operator=(uint32_t rhs);
        %rename(assignBool) operator=(bool);
        CompoundDataType& operator=(bool rhs);
        %rename(assignString) operator=(std::string);
        CompoundDataType& operator=(std::string rhs);

        bool operator==(const CompoundDataType& rhs) const noexcept;
        float asFloat() const;
        uint32_t asUInt32() const;
        bool asBool() const;
        %ignore asString() const&;
        std::string& asString()&;
        DataType getType() const noexcept;
};

struct FieldDefinition;
using RecordDefinition = std::vector<FieldDefinition>;
using Record = std::vector<CompoundDataType>;
using Records = std::vector<Record>;
}

