
#include "bag_dataset.h"
#include "bag_exceptions.h"
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

constexpr hsize_t kTrackingListChunkSize = TRACKING_LIST_BLOCK_SIZE;

TrackingList::TrackingList(const Dataset& dataset)
    : m_pBagDataset(dataset.shared_from_this())
{
    m_pH5dataSet = openH5dataSet();
}

TrackingList::TrackingList(
    const Dataset& dataset,
    int compressionLevel)
    : m_pBagDataset(dataset.shared_from_this())
{
    m_pH5dataSet = createH5dataSet(compressionLevel);
}


TrackingList::iterator TrackingList::begin() & noexcept
{
    return std::begin(m_items);
}

TrackingList::const_iterator TrackingList::begin() const & noexcept
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

TrackingList::iterator TrackingList::end() & noexcept
{
    return std::end(m_items);
}

TrackingList::const_iterator TrackingList::end() const & noexcept
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

void TrackingList::DeleteH5DataSet::operator()(::H5::DataSet* ptr) noexcept
{
    delete ptr;
}

std::unique_ptr<::H5::DataSet, TrackingList::DeleteH5DataSet>
TrackingList::createH5dataSet(
    int compressionLevel)
{
    if (m_pBagDataset.expired())
        throw DatasetNotFound{};

    auto pDataset = m_pBagDataset.lock();

    const auto& h5file = pDataset->getH5file();

    constexpr hsize_t numEntries = 0;
    constexpr hsize_t kUnlimitedSize = static_cast<hsize_t>(-1);
    const ::H5::DataSpace h5dataSpace{1, &numEntries, &kUnlimitedSize};

    const ::H5::CompType h5dataType{sizeof(BagTrackingItem)};

    h5dataType.insertMember("row", HOFFSET(BagTrackingItem, row), ::H5::PredType::NATIVE_UINT);
    h5dataType.insertMember("col", HOFFSET(BagTrackingItem, col), ::H5::PredType::NATIVE_UINT);
    h5dataType.insertMember("depth", HOFFSET(BagTrackingItem, depth), ::H5::PredType::NATIVE_FLOAT);
    h5dataType.insertMember("uncertainty", HOFFSET(BagTrackingItem, uncertainty), ::H5::PredType::NATIVE_FLOAT);
    h5dataType.insertMember("track_code", HOFFSET(BagTrackingItem, track_code), ::H5::PredType::NATIVE_UCHAR);
    h5dataType.insertMember("list_series", HOFFSET(BagTrackingItem, list_series), ::H5::PredType::NATIVE_SHORT);

    const ::H5::DSetCreatPropList h5createPropList{};
    h5createPropList.setChunk(1, &kTrackingListChunkSize);

    if (compressionLevel > 0 && compressionLevel <= 9)
        h5createPropList.setDeflate(compressionLevel);

    const auto h5dataSet = h5file.createDataSet(TRACKING_LIST_PATH,
        h5dataType, h5dataSpace, h5createPropList);

    const ::H5::DataSpace listLengthDataSpace{};
    const auto listLengthAtt = h5dataSet.createAttribute(
        TRACKING_LIST_LENGTH_NAME, ::H5::PredType::NATIVE_UINT32,
        listLengthDataSpace);

    const uint32_t length = 0;
    listLengthAtt.write(::H5::PredType::NATIVE_UINT32, &length);

    return std::unique_ptr<::H5::DataSet, DeleteH5DataSet>(
        new ::H5::DataSet{h5dataSet}, DeleteH5DataSet{});
}

std::unique_ptr<::H5::DataSet, TrackingList::DeleteH5DataSet>
TrackingList::openH5dataSet()
{
    if (m_pBagDataset.expired())
        throw DatasetNotFound{};

    m_items.clear();
    m_length = 0;

    auto pDataset = m_pBagDataset.lock();

    // Read the tracking list from the BAG file.
    // Get the attribute TRACKING_LIST_LENGTH_NAME (uint32)
    const auto h5dataSet = pDataset->getH5file().openDataSet(TRACKING_LIST_PATH);
    const auto attribute = h5dataSet.openAttribute(TRACKING_LIST_LENGTH_NAME);

    attribute.read(attribute.getDataType(), &m_length);

    if (m_length == 0)
        return std::unique_ptr<::H5::DataSet, DeleteH5DataSet>(
            new ::H5::DataSet{h5dataSet}, DeleteH5DataSet{});

    // Read the rank to size m_items properly.
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

    return std::unique_ptr<::H5::DataSet, DeleteH5DataSet>(
        new ::H5::DataSet{h5dataSet}, DeleteH5DataSet{});
}

void TrackingList::write() const
{
    if (m_pBagDataset.expired() || !m_pH5dataSet)
        throw DatasetNotFound{};

    // Write the Attribute.
    const ::H5::DataSpace listLengthDataSpace{};
    const ::H5::Attribute listLengthAtt = m_pH5dataSet->openAttribute(
        TRACKING_LIST_LENGTH_NAME);

    listLengthAtt.write(::H5::PredType::NATIVE_UINT32, &m_length);

    const ::H5::CompType h5type{sizeof(BagTrackingItem)};

    h5type.insertMember("row", HOFFSET(value_type, row), ::H5::PredType::NATIVE_UINT32);
    h5type.insertMember("col", HOFFSET(value_type, col), ::H5::PredType::NATIVE_UINT32);
    h5type.insertMember("depth", HOFFSET(value_type, depth), ::H5::PredType::NATIVE_FLOAT);
    h5type.insertMember("uncertainty", HOFFSET(value_type, uncertainty), ::H5::PredType::NATIVE_FLOAT);
    h5type.insertMember("track_code", HOFFSET(value_type, track_code), ::H5::PredType::NATIVE_UINT8);
    h5type.insertMember("list_series", HOFFSET(value_type, list_series), ::H5::PredType::NATIVE_INT16);

    const hsize_t numItems = m_length;
    ::H5::DataSpace h5memSpace{1, &numItems, &numItems};

    constexpr hsize_t kMaxSize = static_cast<hsize_t>(-1);
    ::H5::DataSpace h5fileSpace{1, &numItems, &kMaxSize};

    m_pH5dataSet->write(m_items.data(), h5type, h5memSpace, h5fileSpace);
}

}   //namespace BAG

