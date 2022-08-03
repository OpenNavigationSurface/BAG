#ifndef BAG_TRACKINGLIST_H
#define BAG_TRACKINGLIST_H

#include "bag_config.h"
#include "bag_deleteh5dataset.h"
#include "bag_fordec.h"
#include "bag_types.h"

#include <memory>
#include <vector>


namespace H5 {

class DataSet;

}  // namespace H5

namespace BAG {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)  // std classes do not have DLL-interface when exporting
#endif

//! The interface for a tracking list.
class BAG_API TrackingList final
{
public:
    using value_type = TrackingItem;
    using iterator = std::vector<value_type>::iterator;
    using const_iterator = std::vector<value_type>::const_iterator;
    using reference = value_type&;
    using const_reference = const value_type&;

    TrackingList(const TrackingList&) = delete;
    TrackingList(TrackingList&&) = delete;

    TrackingList& operator=(const TrackingList&) = delete;
    TrackingList& operator=(TrackingList&&) = delete;

    bool operator==(const TrackingList &rhs) const noexcept {
        return m_pH5dataSet == rhs.m_pH5dataSet &&
               weak_ptr_equals(m_pBagDataset, rhs.m_pBagDataset) &&
               itemsEqual(rhs.m_items);
    }

    bool operator!=(const TrackingList &rhs) const noexcept {
        return !(rhs == *this);
    }

    iterator begin() & noexcept;
    const_iterator begin() const & noexcept;
    iterator end() & noexcept;
    const_iterator end() const & noexcept;
    const_iterator cbegin() const & noexcept;
    const_iterator cend() const & noexcept;

    void clear() noexcept;
    void push_back(const value_type& value);
    void push_back(value_type&& value);
    template <typename... Args>
    void emplace_back(Args&&... args) &;
    reference front() &;
    const_reference front() const &;
    reference back() &;
    const_reference back() const &;
    void reserve(size_t newCapacity);
    void resize(size_t count);
    value_type* data() & noexcept;
    const value_type* data() const & noexcept;

    bool empty() const noexcept;
    size_t size() const noexcept;
    reference operator[](size_t index) & noexcept;
    const_reference operator[](size_t index) const & noexcept;

    void write() const;

protected:
    explicit TrackingList(const Dataset& dataset);
    TrackingList(const Dataset& dataset, int compressionLevel);

private:

    std::unique_ptr<::H5::DataSet, DeleteH5dataSet> createH5dataSet(
        int compressionLevel);
    std::unique_ptr<::H5::DataSet, DeleteH5dataSet> openH5dataSet();

    //! The associated BAG Dataset.
    std::weak_ptr<const Dataset> m_pBagDataset;
    //! The items in the tracking list.
    std::vector<value_type> m_items;
    //! The HDF5 DataSet this class wraps.
    std::unique_ptr<::H5::DataSet, DeleteH5dataSet> m_pH5dataSet;

    bool itemsEqual(std::vector<value_type> other) const {
        auto size = m_items.size();
        bool areEqual = size == other.size();
        if (!areEqual) return areEqual;

        for (size_t i = 0; i < size; i++) {
            auto ours = m_items[i];
            auto theirs = other[i];
            if (ours.row != theirs.row ||
                ours.col != theirs.col ||
                ours.depth != theirs.depth ||
                ours.uncertainty != theirs.uncertainty ||
                ours.track_code != theirs.track_code ||
                ours.list_series != theirs.list_series) {
                return false;
            }
        }

        return areEqual;
    }

    friend Dataset;
};

//! Add an item to the end of the tracking list.
/*!
\param args
    One or more parameters to hand to the constructor of TrackingItem.
*/
template <typename... Args>
void TrackingList::emplace_back(Args&&... args) &
{
    m_items.emplace_back(std::forward<Args>(args)...);
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

}  // namespace BAG

#endif  // BAG_TRACKINGLIST_H

