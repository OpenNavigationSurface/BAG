#ifndef BAG_LAYER_ITEM_H
#define BAG_LAYER_ITEM_H

#include "bag_c_types.h"
#include "bag_exceptions.h"
#include "bag_uint8array.h"

#include <cstring>
#include <vector>


namespace BAG
{

class LayerItems
{
public:
    explicit LayerItems(const UInt8Array& items)
        : m_data()
    {
        const auto bufferSize = items.size();
        m_data.resize(bufferSize);
        std::memcpy(m_data.data(), items.data(), bufferSize);
    }
    template <typename T>
    explicit LayerItems(const std::vector<T>& items)
    {
        const auto numBytes = sizeof(T) * items.size();
        m_data.resize(numBytes);

        std::memcpy(m_data.data(), items.data(), numBytes);
    }

    const uint8_t* data() const & noexcept
    {
        return m_data.data();
    }

    bool empty() const noexcept
    {
        return m_data.empty();
    }

    template <typename T>
    std::vector<T> getAs() const
    {
        const auto recordSize = sizeof(T);
        if (m_data.size() % recordSize > 0)
            throw InvalidCast{};

        const auto numRecords = m_data.size() / recordSize;

        std::vector<T> result;
        result.resize(numRecords);

        std::memcpy(result.data(), m_data.data(), m_data.size());

        return result;
    }

    size_t size() const noexcept
    {
        return m_data.size();
    }

private:
    std::vector<uint8_t> m_data;
};

}

#endif  //BAG_LAYER_ITEM_H

