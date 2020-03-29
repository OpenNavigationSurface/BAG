#ifndef BAG_COMPOUNDDATATYPES_H
#define BAG_COMPOUNDDATATYPES_H

#include "bag_exceptions.h"
#include "bag_types.h"

#include <cstdint>
#include <string>
#include <vector>


namespace BAG {

class CompoundDataType final {
public:

    CompoundDataType()
    {}

    explicit CompoundDataType(float value) noexcept
        : type(DT_FLOAT32)
        , m_data(value)
    {
    }
    explicit CompoundDataType(uint32_t value) noexcept
        : type(DT_UINT32)
        , m_data(value)
    {
    }
    explicit CompoundDataType(bool value) noexcept
        : type(DT_BOOLEAN)
        , m_data(value)
    {
    }
    explicit CompoundDataType(std::string value) noexcept
        : type(DT_STRING)
        , m_data(std::move(value))
    {
    }

    CompoundDataType(const CompoundDataType& other)
        : type(other.type)
    {
        switch (type)
        {
        case DT_FLOAT32:
            m_data.m_f = other.m_data.m_f;
            break;
        case DT_UINT32:
            m_data.m_ui32 = other.m_data.m_ui32;
            break;
        case DT_BOOLEAN:
            m_data.m_b = other.m_data.m_b;
            break;
        case DT_STRING:
            new(&m_data.m_s) std::string{other.m_data.m_s};
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
            m_data.m_f = other.m_data.m_f;
            break;
        case DT_UINT32:
            m_data.m_ui32 = other.m_data.m_ui32;
            break;
        case DT_BOOLEAN:
            m_data.m_b = other.m_data.m_b;
            break;
        case DT_STRING:
            new(&m_data.m_s) std::string{std::move(other.m_data.m_s)};
            break;
        default:
            throw InvalidType{};
        }
    }

    ~CompoundDataType() noexcept
    {
        if (type == DT_STRING)
            m_data.m_s.~basic_string<char>();
    }

    CompoundDataType& operator=(const CompoundDataType& rhs)
    {
        if (this == &rhs)
            return *this;

        if (type == DT_STRING)
            if (rhs.type == DT_STRING)
                m_data.m_s = rhs.m_data.m_s;
            else
                m_data.m_s.~basic_string<char>();

        switch (rhs.type)
        {
        case DT_FLOAT32:
            m_data.m_f = rhs.m_data.m_f;
            break;
        case DT_UINT32:
            m_data.m_ui32 = rhs.m_data.m_ui32;
            break;
        case DT_BOOLEAN:
            m_data.m_b = rhs.m_data.m_b;
            break;
        case DT_STRING:
            new(&m_data.m_s) std::string{rhs.m_data.m_s};
            break;
        default:
            throw InvalidType{};
        }

        type = rhs.type;

        return *this;
    }
    CompoundDataType& operator=(CompoundDataType&& rhs)
    {
        if (type == DT_STRING)
            if (rhs.type == DT_STRING)
                m_data.m_s = std::move(rhs.m_data.m_s);
            else
                m_data.m_s.~basic_string<char>();

        switch (rhs.type)
        {
        case DT_FLOAT32:
            m_data.m_f = rhs.m_data.m_f;
            break;
        case DT_UINT32:
            m_data.m_ui32 = rhs.m_data.m_ui32;
            break;
        case DT_BOOLEAN:
            m_data.m_b = rhs.m_data.m_b;
            break;
        case DT_STRING:
            new(&m_data.m_s) std::string{rhs.m_data.m_s};
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
            m_data.m_s.~basic_string<char>();

        type = DT_FLOAT32;
        m_data.m_f = rhs;

        return *this;
    }
    CompoundDataType& operator=(uint32_t rhs) noexcept
    {
        if (type == DT_STRING)
            m_data.m_s.~basic_string<char>();

        type = DT_UINT32;
        m_data.m_ui32 = rhs;

        return *this;
    }
    CompoundDataType& operator=(bool rhs) noexcept
    {
        if (type == DT_STRING)
            m_data.m_s.~basic_string<char>();

        type = DT_BOOLEAN;
        m_data.m_b = rhs;

        return *this;
    }
    CompoundDataType& operator=(std::string rhs) noexcept
    {
        if (type == DT_STRING)
            m_data.m_s = rhs;
        else
        {
            type = DT_STRING;
            new(&m_data.m_s) std::string{std::move(rhs)};
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
            return m_data.m_f == rhs.m_data.m_f;
        case DT_UINT32:
            return m_data.m_ui32 == rhs.m_data.m_ui32;
        case DT_BOOLEAN:
            return m_data.m_b == rhs.m_data.m_b;
        case DT_STRING:
            return m_data.m_s == rhs.m_data.m_s;
        default:
            return false;
        }
    }

    float asFloat() const
    {
        if (type != DT_FLOAT32)
            throw InvalidType{};

        return m_data.m_f;
    }
    uint32_t asUInt32() const
    {
        if (type != DT_UINT32)
            throw InvalidType{};

        return m_data.m_ui32;
    }
    bool asBool() const
    {
        if (type != DT_BOOLEAN)
            throw InvalidType{};

        return m_data.m_b;
    }
    const std::string& asString() const &
    {
        if (type != DT_STRING)
            throw InvalidType{};

        return m_data.m_s;
    }
    std::string& asString() &
    {
        if (type != DT_STRING)
            throw InvalidType{};

        return m_data.m_s;
    }

    DataType getType() const noexcept
    {
        return type;
    }

private:
    DataType type = DT_UNKNOWN_DATA_TYPE;

    union Data {
        Data(): m_b(false) {}
        Data(float f): m_f(f) {}
        Data(uint32_t ui32): m_ui32(ui32) {}
        Data(bool b): m_b(b) {}
        Data(std::string s): m_s(s) {}
        ~Data() {}

        float m_f;
        uint32_t m_ui32;
        bool m_b;
        std::string m_s;
    } m_data;
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

using RecordDefinition = std::vector<FieldDefinition>;
using Record = std::vector<CompoundDataType>;
using Records = std::vector<Record>;

}  // namespace BAG

#endif  // BAG_COMPOUNDDATATYPES_H
