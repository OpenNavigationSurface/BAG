#ifndef BAG_COMPOUNDDATATYPES_H
#define BAG_COMPOUNDDATATYPES_H

#include "bag_exceptions.h"
#include "bag_types.h"

#include <cstdint>
#include <string>
#include <vector>


namespace BAG {

//! This class is a tagged union (aka variant).
/*!
    The supported types of this tagged union are:
        DT_FLOAT32, DT_UINT32, DT_BOOLEAN, DT_STRING
*/
class CompoundDataType final {
public:
    //! The default constructor.
    CompoundDataType()
    {}
    //! A constructor that takes a float and populates members appropriately.
    /*!
    \param value
        A 32 bit floating point.
    */
    explicit CompoundDataType(float value) noexcept
        : type(DT_FLOAT32)
        , m_data(value)
    {
    }
    //! A constructor that takes an unsigned 32 bit integer and populates members appropriately.
    /*!
    \param value
        A 32 bit unsigned integer.
    */
    explicit CompoundDataType(uint32_t value) noexcept
        : type(DT_UINT32)
        , m_data(value)
    {
    }
    //! A constructor that takes a boolean and populates members appropriately.
    /*!
    \param value
        A boolean.
    */
    explicit CompoundDataType(bool value) noexcept
        : type(DT_BOOLEAN)
        , m_data(value)
    {
    }
    //! A constructor that takes a string and populates members appropriately.
    /*!
    \param value
        A string.
    */
    explicit CompoundDataType(std::string value) noexcept
        : type(DT_STRING)
        , m_data(std::move(value))
    {
    }
    //! Copy constructor.
    /*!
    \param other
        The object to be copied.
    */
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
    //! Move constructor.
    /*!
    \param other
        The object to be moved from.
    */
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
    //! Destructor.
    ~CompoundDataType() noexcept
    {
        if (type == DT_STRING)
            m_data.m_s.~basic_string<char>();
    }

    //! Copy assignment operator.
    /*!
        \param rhs
            The object to be copied.

        \return
            The newly assigned to object.
    */
    CompoundDataType& operator=(const CompoundDataType& rhs)
    {
        if (this == &rhs)
            return *this;

        if (type == DT_STRING)
        {
            if (rhs.type == DT_STRING)
                m_data.m_s = rhs.m_data.m_s;
            else
                m_data.m_s.~basic_string<char>();
        }

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
    //! Move assignment operator.
    /*!
        \param rhs
            The object to be moved from.

        \return
            The newly assigned to object.
    */
    CompoundDataType& operator=(CompoundDataType&& rhs)
    {
        if (type == DT_STRING)
        {
            if (rhs.type == DT_STRING)
                m_data.m_s = std::move(rhs.m_data.m_s);
            else
                m_data.m_s.~basic_string<char>();
        }

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
    //! Floating point assignment operator.
    /*!
    \param rhs
        The object to copy.

    \return
        The newly assigned to object.
    */
    CompoundDataType& operator=(float rhs) noexcept
    {
        if (type == DT_STRING)
            m_data.m_s.~basic_string<char>();

        type = DT_FLOAT32;
        m_data.m_f = rhs;

        return *this;
    }
    //! 32 bit unsigned integer assignment operator.
    /*!
    \param rhs
        The object to copy.

    \return
        The newly assigned to object.
    */
    CompoundDataType& operator=(uint32_t rhs) noexcept
    {
        if (type == DT_STRING)
            m_data.m_s.~basic_string<char>();

        type = DT_UINT32;
        m_data.m_ui32 = rhs;

        return *this;
    }
    //! Boolean assignment operator.
    /*!
    \param rhs
        The object to copy.

    \return
        The newly assigned to object.
    */
    CompoundDataType& operator=(bool rhs) noexcept
    {
        if (type == DT_STRING)
            m_data.m_s.~basic_string<char>();

        type = DT_BOOLEAN;
        m_data.m_b = rhs;

        return *this;
    }
    //! String assignment operator.
    /*!
    \param rhs
        The object to copy.

    \return
        The newly assigned to object.
    */
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

    //! Is equal operator.
    /*!
    \param rhs
        The object to compare with.
    \return
        \e true if \e rhs is equal; \e false otherwise
    */
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

    bool operator!=(const CompoundDataType &rhs) const noexcept {
        return !(rhs == *this);
    }

    //! Retrieve the value as a float point.
    /*!
        Retrieve the value as a floating point.  Throw an exception if it is not
        a floating point.
    \return
        The value as a floating point.
    */
    float asFloat() const
    {
        if (type != DT_FLOAT32)
            throw InvalidType{};

        return m_data.m_f;
    }
    //! Retrieve the value as a 32 bit unsigned integer.
    /*!
        Retrieve the value as a 32 bit unsigned integer.  Throw an exception if
        it is not a 32 bit unsigned integer.
    \return
        The value as a 32 bit unsigned integer.
    */
    uint32_t asUInt32() const
    {
        if (type != DT_UINT32)
            throw InvalidType{};

        return m_data.m_ui32;
    }
    //! Retrieve the value as a boolean.
    /*!
        Retrieve the value as a boolean.  Throw an exception if it is not a
        boolean.
    \return
        The value as a boolean.
    */
    bool asBool() const
    {
        if (type != DT_BOOLEAN)
            throw InvalidType{};

        return m_data.m_b;
    }
    //! Retrieve the value as a constant string.
    /*!
        Retrieve the value as a string.  Throw an exception if it is not a
        string.
    \return
        The value as a string.
    */
    const std::string& asString() const &
    {
        if (type != DT_STRING)
            throw InvalidType{};

        return m_data.m_s;
    }
    //! Retrieve the value as a mutable string.
    /*!
        Retrieve the value as a string.  Throw an exception if it is not a
        string.
    \return
        The value as a string.
    */
    std::string& asString() &
    {
        if (type != DT_STRING)
            throw InvalidType{};

        return m_data.m_s;
    }

    //! Retrieve the type of value being stored.
    /*!
    \return
        The type of the value being stored.
    */
    DataType getType() const noexcept
    {
        return type;
    }

private:
    //! The type of the tagged union.
    DataType type = DT_UNKNOWN_DATA_TYPE;
    //! The union that holds the different values.
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
/*!
\param v
    The CompoundDataType to return the value of.

\return
    The template parameter type specified.
*/
template <typename T>
T get(const CompoundDataType& v)
{
    return detail::GetFromCompoundDataType<T>::get(v);
}

}  // namespace BAG

#endif  // BAG_COMPOUNDDATATYPES_H

