
#include "bag_dataset.h"
#include "bag_exceptions.h"
#include "bag_private.h"
#include "bag_vrtrackinglist.h"

#include <array>
#include <H5Cpp.h>


namespace BAG {

constexpr hsize_t kChunkSize = 1024;

VRTrackingList::VRTrackingList(
    const Dataset& dataset)
    : m_pBagDataset(dataset.shared_from_this())
{
    m_pH5dataSet = openH5dataSet();
}

VRTrackingList::VRTrackingList(
    const Dataset& dataset,
    int compressionLevel)
    : m_pBagDataset(dataset.shared_from_this())
{
    m_pH5dataSet = createH5dataSet(compressionLevel);
}


VRTrackingList::iterator VRTrackingList::begin() & noexcept
{
    return std::begin(m_items);
}

VRTrackingList::const_iterator VRTrackingList::begin() const & noexcept
{
    return std::begin(m_items);
}

VRTrackingList::const_iterator VRTrackingList::cbegin() const & noexcept
{
    return std::cbegin(m_items);
}

VRTrackingList::const_iterator VRTrackingList::cend() const & noexcept
{
    return std::cend(m_items);
}

bool VRTrackingList::empty() const noexcept
{
    return m_items.empty();
}

VRTrackingList::iterator VRTrackingList::end() & noexcept
{
    return std::end(m_items);
}

VRTrackingList::const_iterator VRTrackingList::end() const & noexcept
{
    return std::end(m_items);
}

VRTrackingList::reference VRTrackingList::operator[](size_t index) & noexcept
{
    return m_items[index];
}

VRTrackingList::const_reference VRTrackingList::operator[](
    size_t index) const & noexcept
{
    return m_items[index];
}

size_t VRTrackingList::size() const noexcept
{
    return m_items.size();
}

void VRTrackingList::clear() noexcept
{
    m_items.clear();
}

void VRTrackingList::push_back(const value_type& value)
{
    m_items.push_back(value);
}

void VRTrackingList::push_back(value_type&& value)
{
    m_items.push_back(value);
}

VRTrackingList::reference VRTrackingList::front() &
{
    return m_items.front();
}

VRTrackingList::const_reference VRTrackingList::front() const &
{
    return m_items.front();
}

VRTrackingList::reference VRTrackingList::back() &
{
    return m_items.back();
}

VRTrackingList::const_reference VRTrackingList::back() const &
{
    return m_items.back();
}

void VRTrackingList::reserve(size_t newCapacity)
{
    m_items.reserve(newCapacity);
}

void VRTrackingList::resize(size_t count)
{
    m_items.resize(count);
}

VRTrackingList::value_type* VRTrackingList::data() & noexcept
{
    return m_items.data();
}

const VRTrackingList::value_type* VRTrackingList::data() const & noexcept
{
    return m_items.data();
}

std::unique_ptr<::H5::DataSet, VRTrackingList::DeleteH5dataSet>
VRTrackingList::createH5dataSet(
    int compressionLevel)
{
    if (m_pBagDataset.expired())
        throw DatasetNotFound{};

    const auto pDataset = m_pBagDataset.lock();
    const auto& h5file = pDataset->getH5file();

    constexpr hsize_t numEntries = 0;
    constexpr hsize_t kUnlimitedSize = H5F_UNLIMITED;
    const ::H5::DataSpace h5dataSpace{1, &numEntries, &kUnlimitedSize};

    const ::H5::CompType h5dataType{sizeof(value_type)};

    h5dataType.insertMember("row", HOFFSET(value_type, row),
        ::H5::PredType::NATIVE_UINT32);
    h5dataType.insertMember("col", HOFFSET(value_type, col),
        ::H5::PredType::NATIVE_UINT32);
    h5dataType.insertMember("sub_row", HOFFSET(value_type, sub_row),
        ::H5::PredType::NATIVE_UINT32);
    h5dataType.insertMember("sub_col", HOFFSET(value_type, sub_col),
        ::H5::PredType::NATIVE_UINT32);
    h5dataType.insertMember("depth", HOFFSET(value_type, depth),
        ::H5::PredType::NATIVE_FLOAT);
    h5dataType.insertMember("uncertainty", HOFFSET(value_type, uncertainty),
        ::H5::PredType::NATIVE_FLOAT);
    h5dataType.insertMember("track_code", HOFFSET(value_type, track_code),
        ::H5::PredType::NATIVE_UINT8);
    h5dataType.insertMember("list_series", HOFFSET(value_type, list_series),
        ::H5::PredType::NATIVE_UINT16);

    const ::H5::DSetCreatPropList h5createPropList{};
    h5createPropList.setChunk(1, &kChunkSize);

    if (compressionLevel > 0 && compressionLevel <= kMaxCompressionLevel)
        h5createPropList.setDeflate(compressionLevel);

    const auto h5dataSet = h5file.createDataSet(VR_TRACKING_LIST_PATH,
        h5dataType, h5dataSpace, h5createPropList);

    const ::H5::DataSpace listLengthDataSpace{};
    const auto listLengthAtt = h5dataSet.createAttribute(
        VR_TRACKING_LIST_LENGTH_NAME, ::H5::PredType::NATIVE_UINT32,
        listLengthDataSpace);

    const uint32_t length = 0;
    listLengthAtt.write(::H5::PredType::NATIVE_UINT32, &length);

    return std::unique_ptr<::H5::DataSet, DeleteH5dataSet>(
        new ::H5::DataSet{h5dataSet}, DeleteH5dataSet{});
}

std::unique_ptr<::H5::DataSet, VRTrackingList::DeleteH5dataSet>
VRTrackingList::openH5dataSet()
{
    if (m_pBagDataset.expired())
        throw DatasetNotFound{};

    m_items.clear();

    const auto pDataset = m_pBagDataset.lock();

    // Read the tracking list from the BAG.
    // Get the attribute VR_TRACKING_LIST_LENGTH_NAME (uint32)
    const auto h5dataSet = pDataset->getH5file().openDataSet(
        VR_TRACKING_LIST_PATH);
    const auto attribute = h5dataSet.openAttribute(VR_TRACKING_LIST_LENGTH_NAME);

    uint32_t length = 0;
    attribute.read(attribute.getDataType(), &length);

    if (length == 0)
        return std::unique_ptr<::H5::DataSet, DeleteH5dataSet>(
            new ::H5::DataSet{h5dataSet}, DeleteH5dataSet{});

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

    h5type.insertMember("row", HOFFSET(value_type, row),
        ::H5::PredType::NATIVE_UINT32);
    h5type.insertMember("col", HOFFSET(value_type, col),
        ::H5::PredType::NATIVE_UINT32);
    h5type.insertMember("sub_row", HOFFSET(value_type, sub_row),
        ::H5::PredType::NATIVE_UINT32);
    h5type.insertMember("sub_col", HOFFSET(value_type, sub_col),
        ::H5::PredType::NATIVE_UINT32);
    h5type.insertMember("depth", HOFFSET(value_type, depth),
        ::H5::PredType::NATIVE_FLOAT);
    h5type.insertMember("uncertainty", HOFFSET(value_type, uncertainty),
        ::H5::PredType::NATIVE_FLOAT);
    h5type.insertMember("track_code", HOFFSET(value_type, track_code),
        ::H5::PredType::NATIVE_UINT8);
    h5type.insertMember("list_series", HOFFSET(value_type, list_series),
        ::H5::PredType::NATIVE_UINT16);

    h5dataSet.read(m_items.data(), h5type);

    return std::unique_ptr<::H5::DataSet, DeleteH5dataSet>(
        new ::H5::DataSet{h5dataSet}, DeleteH5dataSet{});
}

void VRTrackingList::write() const
{
    if (m_pBagDataset.expired() || !m_pH5dataSet)
        throw DatasetNotFound{};

    // Write the Attribute.
    const ::H5::DataSpace listLengthDataSpace{};
    const ::H5::Attribute listLengthAtt = m_pH5dataSet->openAttribute(
        VR_TRACKING_LIST_LENGTH_NAME);

    const uint32_t length = static_cast<uint32_t>(m_items.size());
    listLengthAtt.write(::H5::PredType::NATIVE_UINT32, &length);

    // Resize the DataSet to reflect the new data size.
    const hsize_t numItems = length;
    m_pH5dataSet->extend(&numItems);

    // Write the data.
    const ::H5::CompType h5type{sizeof(value_type)};

    h5type.insertMember("row", HOFFSET(value_type, row),
::H5::PredType::NATIVE_UINT32);
    h5type.insertMember("col", HOFFSET(value_type, col),
        ::H5::PredType::NATIVE_UINT32);
    h5type.insertMember("sub_row", HOFFSET(value_type, sub_row),
::H5::PredType::NATIVE_UINT32);
    h5type.insertMember("sub_col", HOFFSET(value_type, sub_col),
        ::H5::PredType::NATIVE_UINT32);
    h5type.insertMember("depth", HOFFSET(value_type, depth),
        ::H5::PredType::NATIVE_FLOAT);
    h5type.insertMember("uncertainty", HOFFSET(value_type, uncertainty),
        ::H5::PredType::NATIVE_FLOAT);
    h5type.insertMember("track_code", HOFFSET(value_type, track_code),
        ::H5::PredType::NATIVE_UINT8);
    h5type.insertMember("list_series", HOFFSET(value_type, list_series),
        ::H5::PredType::NATIVE_UINT16);

    constexpr hsize_t kMaxSize = H5F_UNLIMITED;
    ::H5::DataSpace h5memSpace{1, &numItems, &kMaxSize};
    ::H5::DataSpace h5fileSpace{1, &numItems, &kMaxSize};

    m_pH5dataSet->write(m_items.data(), h5type, h5memSpace, h5fileSpace);
}

void VRTrackingList::DeleteH5dataSet::operator()(::H5::DataSet* ptr) noexcept
{
    delete ptr;
}

}   //namespace BAG

