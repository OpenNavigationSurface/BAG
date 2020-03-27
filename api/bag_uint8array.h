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
    UInt8Array(UInt8Array&&) = delete;
    UInt8Array& operator=(const UInt8Array&) = delete;
    UInt8Array& operator=(UInt8Array&&) = delete;

    uint8_t* get() &
    {
        return m_array.get();
    }

    const uint8_t* get() const &
    {
        return m_array.get();
    }

    uint8_t* release()
    {
        return m_array.release();
    }

    uint8_t& operator[](size_t index) &
    {
        if (index >= m_len)
            throw std::out_of_range{"Invalid index."};

        return *(m_array.get() + index);
    }

    size_t size()
    {
        return m_len;
    }

private:
    std::unique_ptr<uint8_t[]> m_array;
    size_t m_len = 0;
};

}  // namespace BAG

#endif  // BAG_UINT8ARRAY_H

