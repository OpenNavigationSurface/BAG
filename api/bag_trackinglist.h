#ifndef BAG_TRACKINGLIST_H
#define BAG_TRACKINGLIST_H

#include "bag_config.h"
#include "bag_fordec.h"
#include "bag_types.h"

#include <memory>
#include <vector>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif


namespace BAG {

class BAG_API TrackingList final
{
public:
    using value_type = BagTrackingItem;
    using iterator = std::vector<value_type>::iterator;
    using const_iterator = std::vector<value_type>::const_iterator;
    using reference = value_type&;
    using const_reference = const value_type&;

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
    //! Custom deleter to avoid needing a definition for ::H5::DataSet::~DataSet().
    struct BAG_API DeleteH5DataSet final
    {
        void operator()(::H5::DataSet* ptr) noexcept;
    };

    std::unique_ptr<::H5::DataSet, DeleteH5DataSet> createH5dataSet(
        int compressionLevel);
    std::unique_ptr<::H5::DataSet, DeleteH5DataSet> openH5dataSet();

    //! The associated dataset.
    std::weak_ptr<const Dataset> m_pBagDataset;
    //! The items making up the tracking list.
    std::vector<value_type> m_items;
    //! The length attribute in the tracking list DataSet.
    uint32_t m_length = 0;

    std::unique_ptr<::H5::DataSet, DeleteH5DataSet> m_pH5dataSet;

    friend Dataset;
};

template <typename... Args>
void TrackingList::emplace_back(Args&&... args) &
{
    m_items.emplace_back(std::forward<Args>(args)...);
}

}   //namespace BAG

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif  //BAG_TRACKINGLIST_H

