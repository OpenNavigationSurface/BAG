#ifndef BAG_COMPOUNDDATATYPES_H
#define BAG_COMPOUNDDATATYPES_H

#include "bag_exceptions.h"
#include "bag_types.h"

#include <cstdint>
#include <string>


namespace BAG {

class CompoundDataType final {
public:

    CompoundDataType()
    {}

    explicit CompoundDataType(float value) noexcept
        : type(DT_FLOAT32)
        , f(value)
    {
    }
    explicit CompoundDataType(uint32_t value) noexcept
        : type(DT_UINT32)
        , ui32(value)
    {
    }
    explicit CompoundDataType(bool value) noexcept
        : type(DT_BOOL)
        , b(value)
    {
    }
    explicit CompoundDataType(std::string value) noexcept
        : type(DT_STRING)
        , s(std::move(value))
    {
    }

    CompoundDataType(const CompoundDataType& other)
        : type(other.type)
    {
        switch (type)
        {
        case DT_FLOAT32:
            f = other.f;
            break;
        case DT_UINT32:
            ui32 = other.ui32;
            break;
        case DT_BOOL:
            b = other.b;
            break;
        case DT_STRING:
            new(&s) std::string{other.s};
            break;
        default:
            throw InvalidType{};
        }
    }

    CompoundDataType(CompoundDataType&& other)
    {
        switch (type)
        {
        case DT_FLOAT32:
            f = other.f;
            break;
        case DT_UINT32:
            ui32 = other.ui32;
            break;
        case DT_BOOL:
            b = other.b;
            break;
        case DT_STRING:
            new(&s) std::string{std::move(other.s)};
            break;
        default:
            throw InvalidType{};
        }
    }

    ~CompoundDataType() noexcept
    {
        if (type == DT_STRING)
            s.~basic_string<char>();
    }

    CompoundDataType& operator=(const CompoundDataType& rhs)
    {
        if (this == &rhs)
            return *this;

        if (type == DT_STRING && rhs.type == DT_STRING)
            s = rhs.s;
        else if (type == DT_STRING)
            s.~basic_string<char>();

        switch (rhs.type)
        {
        case DT_FLOAT32:
            f = rhs.f;
            break;
        case DT_UINT32:
            ui32 = rhs.ui32;
            break;
        case DT_BOOL:
            b = rhs.b;
            break;
        case DT_STRING:
            new(&s) std::string{rhs.s};
            break;
        default:
            throw InvalidType{};
        }

        type = rhs.type;

        return *this;
    }
    CompoundDataType& operator=(CompoundDataType&& rhs)
    {
        if (type == DT_STRING && rhs.type == DT_STRING)
            s = std::move(rhs.s);
        else if (type == DT_STRING)
            s.~basic_string<char>();

        switch (rhs.type)
        {
        case DT_FLOAT32:
            f = rhs.f;
            break;
        case DT_UINT32:
            ui32 = rhs.ui32;
            break;
        case DT_BOOL:
            b = rhs.b;
            break;
        case DT_STRING:
            new(&s) std::string{rhs.s};
            break;
        default:
            throw InvalidType{};
        }

        type = rhs.type;

        return *this;
    }
    CompoundDataType& operator=(float rhs) noexcept
    {
        if (type == DT_STRING)
            s.~basic_string<char>();

        type = DT_FLOAT32;
        f = rhs;

        return *this;
    }
    CompoundDataType& operator=(uint32_t rhs) noexcept
    {
        if (type == DT_STRING)
            s.~basic_string<char>();

        type = DT_UINT32;
        ui32 = rhs;

        return *this;
    }
    CompoundDataType& operator=(bool rhs) noexcept
    {
        if (type == DT_STRING)
            s.~basic_string<char>();

        type = DT_BOOL;
        b = rhs;

        return *this;
    }
    CompoundDataType& operator=(std::string rhs) noexcept
    {
        if (type == DT_STRING)
            s = rhs;
        else
        {
            type = DT_STRING;
            new(&s) std::string{std::move(rhs)};
        }

        return *this;
    }

    bool operator==(const CompoundDataType& rhs) const noexcept
    {
        if (this->type != rhs.type)
            return false;

        switch (this->type)
        {
        case DT_FLOAT32:
            return f == rhs.f;
        case DT_UINT32:
            return ui32 == rhs.ui32;
        case DT_BOOL:
            return b == rhs.b;
        case DT_STRING:
            return s == rhs.s;
        default:
            return false;
        }
    }

    float asFloat() const
    {
        if (type != DT_FLOAT32) throw InvalidType{};

        return f;
    }
    uint32_t asUInt32() const
    {
        if (type != DT_UINT32) throw InvalidType{};

        return ui32;
    }
    bool asBool() const
    {
        if (type != DT_BOOL) throw InvalidType{};

        return b;
    }
    const std::string& asString() const &
    {
        if (type != DT_STRING) throw InvalidType{};

        return s;
    }
    std::string& asString() &
    {
        if (type != DT_STRING) throw InvalidType{};

        return s;
    }

    DataType getType() const noexcept
    {
        return type;
    }

private:
    union {
        float f;
        uint32_t ui32;
        bool b;
        std::string s;
    };

    DataType type = DT_UNKNOWN_DATA_TYPE;
};


namespace detail {

template <typename T>
struct GetFromCompoundDataType;

template<>
struct GetFromCompoundDataType<float> {
    static float get(const CompoundDataType& v) { return v.asFloat(); }
};

template<>
struct GetFromCompoundDataType<uint32_t> {
    static uint32_t get(const CompoundDataType& v) { return v.asUInt32(); }
};

template<>
struct GetFromCompoundDataType<bool> {
    static bool get(const CompoundDataType& v) { return v.asBool(); }
};

template<>
struct GetFromCompoundDataType<std::string> {
    static const std::string& get(const CompoundDataType& v) { return v.asString(); }
};

}  // namespace detail


//! A generic way to get the value in a CompoundDataType.
template <typename T>
T get(const CompoundDataType& v)
{
    return detail::GetFromCompoundDataType<T>::get(v);
}

// Terminology  -> record made of fields
struct FieldDefinition final
{
    //! The name of the definition.
    char* name;
    //! The type of the definition; represents a DataType.
    uint8_t type;
};

using RecordDefinition = std::vector<FieldDefinition>;
using Record = std::vector<CompoundDataType>;
using Records = std::vector<Record>;

}  // namespace BAG

#endif  // BAG_COMPOUNDDATATYPES_H
