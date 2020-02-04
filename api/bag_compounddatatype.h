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
        : type(FLOAT32)
        , f(value)
    {
    }
    explicit CompoundDataType(uint32_t value) noexcept
        : type(UINT32)
        , ui32(value)
    {
    }
    explicit CompoundDataType(bool value) noexcept
        : type(BOOL)
        , b(value)
    {
    }
    explicit CompoundDataType(std::string value) noexcept
        : type(STRING)
        , s(std::move(value))
    {
    }

    CompoundDataType(const CompoundDataType& other)
        : type(other.type)
    {
        switch (type)
        {
        case FLOAT32:
            f = other.f;
            break;
        case UINT32:
            ui32 = other.ui32;
            break;
        case BOOL:
            b = other.b;
            break;
        case STRING:
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
        case FLOAT32:
            f = other.f;
            break;
        case UINT32:
            ui32 = other.ui32;
            break;
        case BOOL:
            b = other.b;
            break;
        case STRING:
            new(&s) std::string{std::move(other.s)};
            break;
        default:
            throw InvalidType{};
        }
    }

    ~CompoundDataType() noexcept
    {
        if (type == STRING)
            s.~basic_string<char>();
    }

    CompoundDataType& operator=(const CompoundDataType& rhs)
    {
        if (this == &rhs)
            return *this;

        if (type == STRING && rhs.type == STRING)
            s = rhs.s;
        else if (type == STRING)
            s.~basic_string<char>();

        switch (rhs.type)
        {
        case FLOAT32:
            f = rhs.f;
            break;
        case UINT32:
            ui32 = rhs.ui32;
            break;
        case BOOL:
            b = rhs.b;
            break;
        case STRING:
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
        if (type == STRING && rhs.type == STRING)
            s = std::move(rhs.s);
        else if (type == STRING)
            s.~basic_string<char>();

        switch (rhs.type)
        {
        case FLOAT32:
            f = rhs.f;
            break;
        case UINT32:
            ui32 = rhs.ui32;
            break;
        case BOOL:
            b = rhs.b;
            break;
        case STRING:
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
        if (type == STRING)
            s.~basic_string<char>();

        type = FLOAT32;
        f = rhs;

        return *this;
    }
    CompoundDataType& operator=(uint32_t rhs) noexcept
    {
        if (type == STRING)
            s.~basic_string<char>();

        type = UINT32;
        ui32 = rhs;

        return *this;
    }
    CompoundDataType& operator=(bool rhs) noexcept
    {
        if (type == STRING)
            s.~basic_string<char>();

        type = BOOL;
        b = rhs;

        return *this;
    }
    CompoundDataType& operator=(std::string rhs) noexcept
    {
        if (type == STRING)
            s = rhs;
        else
        {
            type = STRING;
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
        case FLOAT32:
            return f == rhs.f;
        case UINT32:
            return ui32 == rhs.ui32;
        case BOOL:
            return b == rhs.b;
        case STRING:
            return s == rhs.s;
        default:
            return false;
        }
    }

    float asFloat() const
    {
        if (type != FLOAT32) throw InvalidType{};

        return f;
    }
    uint32_t asUInt32() const
    {
        if (type != UINT32) throw InvalidType{};

        return ui32;
    }
    bool asBool() const
    {
        if (type != BOOL) throw InvalidType{};

        return b;
    }
    const std::string& asString() const &
    {
        if (type != STRING) throw InvalidType{};

        return s;
    }
    std::string& asString() &
    {
        if (type != STRING) throw InvalidType{};

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

    DataType type = UNKNOWN_DATA_TYPE;
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
