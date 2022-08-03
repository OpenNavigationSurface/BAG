#ifndef BAG_UINT8ARRAY_H
#define BAG_UINT8ARRAY_H

#include <cstdint>
#include <memory>
#include <stdexcept>


namespace BAG
{

//! Class to get past SWIG not being able to handle std::unique_ptr<uint8_t[]>.
class UInt8Array final
{
public:
    UInt8Array() = default;
    explicit UInt8Array(size_t len)
        : m_array(std::make_unique<uint8_t[]>(len)), m_len(len)
    {}

    UInt8Array(const UInt8Array&) = delete;
    UInt8Array(UInt8Array&&) = default;

    ~UInt8Array() = default;

    UInt8Array& operator=(const UInt8Array&) = delete;
    UInt8Array& operator=(UInt8Array&& rhs) noexcept
    {
        if (this == &rhs)
            return *this;

        this->m_array = std::move(rhs.m_array);
        this->m_len = rhs.m_len;

        return *this;
    }

    bool operator==(const UInt8Array &rhs) const noexcept {
        return m_array == rhs.m_array &&
               m_len == rhs.m_len;
    }

    bool operator!=(const UInt8Array &rhs) const noexcept {
        return !(rhs == *this);
    }

    uint8_t& operator[](size_t index) &
    {
        if (index >= m_len)
            throw std::out_of_range{"Invalid index."};

        return *(m_array.get() + index);
    }

    const uint8_t& operator[](size_t index) const &
    {
        if (index >= m_len)
            throw std::out_of_range{"Invalid index."};

        return *(m_array.get() + index);
    }

    explicit operator bool() const noexcept
    {
        return static_cast<bool>(m_array);
    }

    uint8_t* data() & noexcept
    {
        return m_array.get();
    }

    const uint8_t* data() const & noexcept
    {
        return m_array.get();
    }

    uint8_t* release() noexcept
    {
        return m_array.release();
    }

    size_t size() const noexcept
    {
        return m_len;
    }

private:
    std::unique_ptr<uint8_t[]> m_array;
    size_t m_len = 0;
};

}  // namespace BAG

#endif  // BAG_UINT8ARRAY_H

