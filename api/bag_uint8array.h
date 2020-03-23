#ifndef BAG_UINT8ARRAY_H
#define BAG_UINT8ARRAY_H

#include <cstdint>


namespace BAG
{

//! Class to get past SWIG not being able to handle std::unique_ptr<uint8_t[]>.
class UInt8Array final
{
public:
    UInt8Array() = default;
    explicit UInt8Array(size_t len)
        : m_array(new uint8_t[len]), m_len(len), m_doCleanup(false)
    {}

    UInt8Array(const UInt8Array&) = delete;
    UInt8Array(UInt8Array&&) = delete;
    UInt8Array& operator=(const UInt8Array&) = delete;
    UInt8Array& operator=(UInt8Array&&) = delete;

    ~UInt8Array()
    {
        if (m_doCleanup)
            delete[] m_array;
    }

    uint8_t* get() &
    {
        return m_array;
    }

    const uint8_t* get() const &
    {
        return m_array;
    }

    uint8_t* release()
    {
        m_doCleanup = false;
        return m_array;
    }

    size_t size()
    {
        return m_len;
    }

private:
    uint8_t* m_array = nullptr;
    size_t m_len = 0;
    bool m_doCleanup = true;
};

}  // namespace BAG

#endif  // BAG_UINT8ARRAY_H

