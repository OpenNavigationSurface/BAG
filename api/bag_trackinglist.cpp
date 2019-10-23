
#include "bag_dataset.h"
#include "bag_errors.h"
#include "bag_private.h"
#include "bag_trackinglist.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

#include <array>
#include <h5cpp.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif


namespace BAG {

TrackingList::TrackingList(Dataset& dataset)
    : m_pBagDataset(dataset.shared_from_this())
{
    // Read the tracking list from the BAG file.
    // Get the attribute TRACKING_LIST_LENGTH_NAME (uint32)
    const auto h5dataSet = dataset.getH5file().openDataSet(TRACKING_LIST_PATH);
    const auto attribute = h5dataSet.openAttribute(TRACKING_LIST_LENGTH_NAME);

    unsigned int length = 0;
    attribute.read(attribute.getDataType(), &length);

    if (length == 0)
        return;

    // Read the rank to size m_corrections properly.
    const auto h5dataSpace = h5dataSet.getSpace();
    std::array<hsize_t, H5S_MAX_RANK> dims{};
    const int ndims = h5dataSpace.getSimpleExtentDims(dims.data(), nullptr);

    size_t numItems = 1;
    for (int i=0; i<ndims; ++i)
        numItems *= dims[i];

    m_items.resize(numItems);

    // Set up the structure for reading.
    ::H5::CompType h5type(sizeof(value_type));
    h5type.insertMember("row", HOFFSET(value_type, row), ::H5::PredType::NATIVE_UINT32);
    h5type.insertMember("col", HOFFSET(value_type, col), ::H5::PredType::NATIVE_UINT32);
    h5type.insertMember("depth", HOFFSET(value_type, depth), ::H5::PredType::NATIVE_FLOAT);
    h5type.insertMember("uncertainty", HOFFSET(value_type, uncertainty), ::H5::PredType::NATIVE_FLOAT);
    h5type.insertMember("track_code", HOFFSET(value_type, track_code), ::H5::PredType::NATIVE_UINT8);
    h5type.insertMember("list_series", HOFFSET(value_type, list_series), ::H5::PredType::NATIVE_INT16);

    h5dataSet.read(m_items.data(), h5type);
}

TrackingList::TrackingList(size_t numItems)
    : m_items(numItems)
{
}

TrackingList::TrackingList(
    std::initializer_list<TrackingList::value_type> items)
    : m_items(items)
{
}

TrackingList::iterator TrackingList::begin() &
{
    return std::begin(m_items);
}

TrackingList::const_iterator TrackingList::cbegin() const & noexcept
{
    return std::cbegin(m_items);
}

TrackingList::const_iterator TrackingList::cend() const & noexcept
{
    return std::cend(m_items);
}

bool TrackingList::empty() const noexcept
{
    return m_items.empty();
}

TrackingList::iterator TrackingList::end() &
{
    return std::end(m_items);
}

TrackingList::reference TrackingList::operator[](size_t index) & noexcept
{
    return m_items[index];
}

TrackingList::const_reference TrackingList::operator[](
    size_t index) const & noexcept
{
    return m_items[index];
}

size_t TrackingList::size() const noexcept
{
    return m_items.size();
}

void TrackingList::clear() noexcept
{
    m_items.clear();
}

void TrackingList::push_back(const value_type& value)
{
    m_items.push_back(value);
}

void TrackingList::push_back(value_type&& value)
{
    m_items.push_back(std::move(value));
}

template <typename... Args>
TrackingList::reference TrackingList::emplace_back(Args&&... args) &
{
    return m_items.emplace_back(std::forward<Args>(args)...);
}

TrackingList::reference TrackingList::front() &
{
    return m_items.front();
}

TrackingList::const_reference TrackingList::front() const &
{
    return m_items.front();
}

TrackingList::reference TrackingList::back() &
{
    return m_items.back();
}

TrackingList::const_reference TrackingList::back() const &
{
    return m_items.back();
}

void TrackingList::reserve(size_t newCapacity)
{
    m_items.reserve(newCapacity);
}

void TrackingList::resize(size_t count)
{
    m_items.resize(count);
}

TrackingList::value_type* TrackingList::data() & noexcept
{
    return m_items.data();
}

const TrackingList::value_type* TrackingList::data() const & noexcept
{
    return m_items.data();
}

}   //namespace BAG

