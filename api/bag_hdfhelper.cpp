
#include "bag_exceptions.h"
#include "bag_hdfhelper.h"

#include <array>
#include <H5Cpp.h>
#include <numeric>


namespace BAG {

//! Create an HDF5 compound type based on the layer and group types.
::H5::CompType createH5compType(
    LayerType layerType,
    GroupType groupType)
{
    ::H5::CompType h5type;

    if (groupType == NODE)
    {
        switch (layerType)
        {
        case Hypothesis_Strength:
            h5type = ::H5::CompType{sizeof(float)};
            h5type.insertMember("hyp_strength",
                0,
                ::H5::PredType::NATIVE_FLOAT);
            break;
        case Num_Hypotheses:
            h5type = ::H5::CompType{sizeof(unsigned int)};
            h5type.insertMember("num_hypotheses",
                0,
                ::H5::PredType::NATIVE_UINT32);
            break;
        default:
            throw UnsupportedLayerType{};
        }
    }
    else if (groupType == ELEVATION)
    {
        switch(layerType)
        {
        case Shoal_Elevation:
            h5type = ::H5::CompType{sizeof(float)};
            h5type.insertMember("shoal_elevation",
                0,
                ::H5::PredType::NATIVE_FLOAT);
            break;
        case Std_Dev:
            h5type = ::H5::CompType{sizeof(float)};
            h5type.insertMember("stddev",
                0,
                ::H5::PredType::NATIVE_FLOAT);
            break;
        case Num_Soundings:
            h5type = ::H5::CompType{sizeof(int)};
            h5type.insertMember("num_soundings",
                0,
                ::H5::PredType::NATIVE_INT32);
            break;
        default:
            throw UnsupportedLayerType{};
        }
    }
    else
        throw UnsupportedGroupType{};

    return h5type;
}

//! Create an HDF5 file Compound Type based upon the Record Definition.
::H5::CompType createH5fileCompType(
    const RecordDefinition& definition)
{
    ::H5::CompType h5type{getH5compSize(definition)};
    size_t fieldOffset = 0;

    for (const auto& field : definition)
    {
        h5type.insertMember(field.name, fieldOffset,
            getH5fileType(static_cast<DataType>(field.type)));

        fieldOffset += Layer::getElementSize(static_cast<DataType>(field.type));
    }

    return h5type;
}

//! Create an HDF5 memory Compound Type based upon the Record Definition.
::H5::CompType createH5memoryCompType(
    const RecordDefinition& definition)
{
    ::H5::CompType h5type{getH5compSize(definition)};
    size_t fieldOffset = 0;

    for (const auto& field : definition)
    {
        h5type.insertMember(field.name, fieldOffset,
            getH5memoryType(static_cast<DataType>(field.type)));

        fieldOffset += Layer::getElementSize(static_cast<DataType>(field.type));
    }

    return h5type;
}

//! Get the chunk size from a HDF file.
uint64_t getChunkSize(
    const ::H5::H5File& h5file,
    const std::string& path)
{
    //Get the elevation HD5 dataset.
    const auto h5dataset = h5file.openDataSet(path);
    const auto h5pList = h5dataset.getCreatePlist();

    if (h5pList.getLayout() == H5D_CHUNKED)
    {
        std::array<hsize_t, RANK> maxDims{};

        const int rankChunk = h5pList.getChunk(RANK, maxDims.data());
        if (rankChunk == RANK)
            return {maxDims[0]};  // Using {} to prevent narrowing.
    }

    return 0;
}

//! Get the compression level from a HDF file.
unsigned int getCompressionLevel(
    const ::H5::H5File& h5file,
    const std::string& path)
{
    //Get the elevation HD5 dataset.
    const auto h5dataset = h5file.openDataSet(path);
    const auto h5pList = h5dataset.getCreatePlist();

    for (int i=0; i<h5pList.getNfilters(); ++i)
    {
        unsigned int flags = 0;
        size_t cdNelmts = 10;
        constexpr size_t nameLen = 64;
        std::array<unsigned int, 10> cdValues{};
        std::array<char, 64> name{};
        unsigned int filterConfig = 0;

        const auto filter = h5pList.getFilter(i, flags, cdNelmts,
            cdValues.data(), nameLen, name.data(), filterConfig);
        if (filter == H5Z_FILTER_DEFLATE && cdNelmts >= 1)
            return cdValues.front();
    }

    return 0;
}

size_t getH5compSize(
    const RecordDefinition& definition)
{
    return std::accumulate(cbegin(definition), cend(definition), 0ULL,
        [](size_t sum, const auto& field) {
            return sum + Layer::getElementSize(static_cast<DataType>(field.type));
        });
}

//! Determine the HDF5 file DataType from the specified DataType.
const ::H5::AtomType& getH5fileType(
    DataType type)
{
    static ::H5::StrType strType{::H5::PredType::C_S1, H5T_VARIABLE};

    switch(type)
    {
    case UINT32:
        return H5::PredType::NATIVE_UINT32;
    case FLOAT32:
        return ::H5::PredType::NATIVE_FLOAT;
    case UINT8:
        return ::H5::PredType::NATIVE_UINT8;
    case UINT16:
        return ::H5::PredType::NATIVE_UINT16;
    case UINT64:
        return ::H5::PredType::NATIVE_UINT64;
    case BOOL:
        return ::H5::PredType::NATIVE_HBOOL;
    case STRING:
        return strType;
    case COMPOUND:  //[fallthrough]
    case UNKNOWN_DATA_TYPE:  //[fallthrough]
    default:
        throw UnsupportedDataType{};
    }
}

//! Determine the HDF5 memory DataType from the specified DataType.
const ::H5::AtomType& getH5memoryType(
    DataType type)
{
    static ::H5::StrType strType{::H5::PredType::C_S1, H5T_VARIABLE};

    switch(type)
    {
    case UINT32:
        return H5::PredType::NATIVE_UINT32;
    case FLOAT32:
        return ::H5::PredType::NATIVE_FLOAT;
    case UINT8:
        return ::H5::PredType::NATIVE_UINT8;
    case UINT16:
        return ::H5::PredType::NATIVE_UINT16;
    case UINT64:
        return ::H5::PredType::NATIVE_UINT64;
    case BOOL:
        return ::H5::PredType::NATIVE_HBOOL;
    case STRING:
        return strType;
    case COMPOUND:  //[fallthrough]
    case UNKNOWN_DATA_TYPE:  //[fallthrough]
    default:
        throw UnsupportedDataType{};
    }
}

::H5::Attribute createAttribute(
    const ::H5::DataSet& h5dataSet,
    const ::H5::PredType& attributeType,
    const char* path)
{
    return h5dataSet.createAttribute(path, attributeType, {});
}

void createAttributes(
    const ::H5::DataSet& h5dataSet,
    const ::H5::PredType& attributeType,
    const std::vector<const char*>& paths)
{
    if (paths.empty())
        return;

    for (const auto& path : paths)
        if (path)
            createAttribute(h5dataSet, attributeType, path);
}

template <typename T>
void writeAttribute(
    const ::H5::DataSet& h5dataSet,
    const ::H5::PredType& attributeType,
    T value,
    const char* path)
{
    const auto att = h5dataSet.openAttribute(path);
    att.write(attributeType, &value);
}

template <typename T>
void writeAttributes(
    const ::H5::DataSet& h5dataSet,
    const ::H5::PredType& attributeType,
    T value,
    const std::vector<const char*>& paths)
{
    for (const auto& path : paths)
        if (path)
            writeAttribute(h5dataSet, attributeType, value, path);
}


// Explicit template instantiations.
template void writeAttribute<float>(const ::H5::DataSet& h5dataSet,
    const ::H5::PredType& attributeType, float value, const char* path);

template void writeAttribute<uint32_t>(const ::H5::DataSet& h5dataSet,
    const ::H5::PredType& attributeType, uint32_t value, const char* path);

template void writeAttributes<float>(const ::H5::DataSet& h5dataSet,
    const ::H5::PredType& attributeType, float value,
    const std::vector<const char*>& paths);

template void writeAttributes<uint32_t>(const ::H5::DataSet& h5dataSet,
    const ::H5::PredType& attributeType, uint32_t value,
    const std::vector<const char*>& paths);

}  // namespace BAG

