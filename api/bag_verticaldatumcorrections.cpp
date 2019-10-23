
#include "bag_dataset.h"
#include "bag_errors.h"
#include "bag_private.h"
#include "bag_verticaldatumcorrections.h"

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

VerticalDatumCorrections::VerticalDatumCorrections(Dataset& dataset)
    : m_pBagDataset(dataset.shared_from_this())
{
    // Read the tracking list from the BAG file.
    // Get the attribute TRACKING_LIST_LENGTH_NAME (uint32)
    const auto h5dataSet = dataset.getH5file().openDataSet(VERT_DATUM_CORR_PATH);

    {
        const auto att = h5dataSet.openAttribute(VERT_DATUM_CORR_SURFACE_TYPE);
        att.read(att.getDataType(), &m_surfaceType);
    }
    {
        const auto att = h5dataSet.openAttribute(VERT_DATUM_CORR_VERTICAL_DATUM);
        att.read(att.getDataType(), m_verticalDatum);
    }

    // Read the rank to size m_corrections properly.
    std::array<hsize_t, H5S_MAX_RANK> dims;
    const auto h5dataSpace = h5dataSet.getSpace();
    const int ndims = h5dataSpace.getSimpleExtentDims(dims.data(), nullptr);

    size_t numCorrections = 1;
    for (int i=0; i<ndims; ++i)
        numCorrections *= dims[i];

    m_corrections.resize(numCorrections);

    // Set up the complex structure.
    ::H5::CompType h5type(sizeof(value_type));

    // A different structure is used Depending on the surface type.
    if (static_cast<BAG_SURFACE_CORRECTION_TOPOGRAPHY>(m_surfaceType) == BAG_SURFACE_IRREGULARLY_SPACED)
    {
        h5type.insertMember("x", HOFFSET(value_type, x), ::H5::PredType::NATIVE_DOUBLE);
        h5type.insertMember("y", HOFFSET(value_type, y), ::H5::PredType::NATIVE_DOUBLE);
    }

    // Get the size from the DataSet.
    const ::H5::CompType h5dsType = h5dataSet.getCompType();
    const int index = h5dsType.getMemberIndex("z");
    const auto arrType = h5dsType.getMemberArrayType(index);
    std::array<hsize_t, H5S_MAX_RANK> arrDims{};
    m_numStoredCorrections = arrType.getArrayDims(arrDims.data());

    ::H5::ArrayType h5arrType{::H5::PredType::NATIVE_FLOAT,
        m_numStoredCorrections, arrDims.data()};

    h5type.insertMember("z", HOFFSET(value_type, z), h5arrType);

    h5dataSet.read(m_corrections.data(), h5type);
}

VerticalDatumCorrections::VerticalDatumCorrections(size_t numItems)
    : m_corrections(numItems)
{
}

VerticalDatumCorrections::VerticalDatumCorrections(
    std::initializer_list<VerticalDatumCorrections::value_type> items)
    : m_corrections(items)
{
}

VerticalDatumCorrections::iterator VerticalDatumCorrections::begin() &
{
    return std::begin(m_corrections);
}

VerticalDatumCorrections::const_iterator
VerticalDatumCorrections::cbegin() const & noexcept
{
    return std::cbegin(m_corrections);
}

VerticalDatumCorrections::const_iterator
VerticalDatumCorrections::cend() const & noexcept
{
    return std::cend(m_corrections);
}

bool VerticalDatumCorrections::empty() const noexcept
{
    return m_corrections.empty();
}

VerticalDatumCorrections::iterator VerticalDatumCorrections::end() &
{
    return std::end(m_corrections);
}

VerticalDatumCorrections::reference VerticalDatumCorrections::operator[](
    size_t index) & noexcept
{
    return m_corrections[index];
}

VerticalDatumCorrections::const_reference VerticalDatumCorrections::operator[](
    size_t index) const & noexcept
{
    return m_corrections[index];
}

size_t VerticalDatumCorrections::size() const noexcept
{
    return m_corrections.size();
}

void VerticalDatumCorrections::clear() noexcept
{
    m_corrections.clear();
}

void VerticalDatumCorrections::push_back(const value_type& value)
{
    m_corrections.push_back(value);
}

void VerticalDatumCorrections::push_back(value_type&& value)
{
    m_corrections.push_back(std::move(value));
}

template <typename... Args>
VerticalDatumCorrections::reference VerticalDatumCorrections::emplace_back(
    Args&&... args) &
{
    return m_corrections.emplace_back(std::forward<Args>(args)...);
}

VerticalDatumCorrections::reference VerticalDatumCorrections::front() &
{
    return m_corrections.front();
}

VerticalDatumCorrections::const_reference
VerticalDatumCorrections::front() const &
{
    return m_corrections.front();
}

VerticalDatumCorrections::reference VerticalDatumCorrections::back() &
{
    return m_corrections.back();
}

VerticalDatumCorrections::const_reference
VerticalDatumCorrections::back() const &
{
    return m_corrections.back();
}

void VerticalDatumCorrections::reserve(size_t newCapacity)
{
    m_corrections.reserve(newCapacity);
}

void VerticalDatumCorrections::resize(size_t count)
{
    m_corrections.resize(count);
}

VerticalDatumCorrections::value_type*
VerticalDatumCorrections::data() & noexcept
{
    return m_corrections.data();
}

const VerticalDatumCorrections::value_type*
VerticalDatumCorrections::data() const & noexcept
{
    return m_corrections.data();
}

const std::string& VerticalDatumCorrections::getVerticalDatum() const & noexcept
{
    return m_verticalDatum;
}

uint8_t VerticalDatumCorrections::getSurfaceType() const noexcept
{
    return m_surfaceType;
}

}   //namespace BAG

