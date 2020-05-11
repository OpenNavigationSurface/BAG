
#include "bag_exceptions.h"
#include "bag_hdfhelper.h"

#include <array>
#include <H5Cpp.h>
#include <numeric>


namespace BAG {

//! Create an HDF5 CompType based on the layer and group types.
/*!
\param layerType
    The type of layer.
    The supported types are NODE and ELEVATION.
\param groupType
    The group type of the layer.
    For NODE, the supported types are: Hypothesis_Strength, Num_Hypotheses.
    For ELEVATION, the supported types are: Shoal_Elevation, Std_Dev, Num_Soundings.

\return
    The compound type of the specified layer and group type.
*/
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

//! Create an HDF5 CompType used for file I/O based upon the Record Definition.
/*!
\param definition
    The list of fields making up the record.

\return
    The HDF5 CompType to be used when reading and writing to an HDF file.
*/
::H5::CompType createH5fileCompType(
    const RecordDefinition& definition)
{
    ::H5::CompType h5type{getRecordSize(definition)};
    size_t fieldOffset = 0;

    for (const auto& field : definition)
    {
        h5type.insertMember(field.name, fieldOffset,
            getH5fileType(static_cast<DataType>(field.type)));

        fieldOffset += Layer::getElementSize(static_cast<DataType>(field.type));
    }

    return h5type;
}

//! Create an HDF5 CompType to be used in memory based upon the Record Definition.
/*!
\param definition
    The list of fields making up the record.

\return
    The HDF5 CompType to be used when reading and writing from memory.
*/
::H5::CompType createH5memoryCompType(
    const RecordDefinition& definition)
{
    ::H5::CompType h5type{getRecordSize(definition)};
    size_t fieldOffset = 0;

    for (const auto& field : definition)
    {
        h5type.insertMember(field.name, fieldOffset,
            getH5memoryType(static_cast<DataType>(field.type)));

        fieldOffset += Layer::getElementSize(static_cast<DataType>(field.type));
    }

    return h5type;
}

//! Get the chunk size from an HDF5 file.
/*!
\param h5file
    The HDF5 file.
\param path
    The path to the HDF5 DataSet.

\return
    The chunk size of the specified HDF5 DataSet in the HDF5 file.
    0 if the HDF5 DataSet does not use chunking.
*/
uint64_t getChunkSize(
    const ::H5::H5File& h5file,
    const std::string& path)
{
    //Get the elevation HD5 dataset.
    const auto h5dataset = h5file.openDataSet(path);
    const auto h5pList = h5dataset.getCreatePlist();

    if (h5pList.getLayout() == H5D_CHUNKED)
    {
        std::array<hsize_t, kRank> maxDims{};

        const int rankChunk = h5pList.getChunk(kRank, maxDims.data());
        if (rankChunk == kRank)
            return {maxDims[0]};  // Using {} to prevent narrowing.
    }

    return 0;
}

//! Get the compression level from an HDF5 file.
/*!
\param h5file
    The HDF5 file.
\param path
    The path to the HDF5 DataSet.

\return
    The compression level of the specified HDF5 DataSet in the HDF5 file.
    0 if the HDF5 DataSet is not compressed.
*/
int getCompressionLevel(
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
            return static_cast<int>(cdValues.front());
    }

    return 0;
}

//! Get the size of a record in memory.
/*!
\param definition
    The list of fields in the record.

\return
    The record size in memory, ignoring alignment.
*/
size_t getRecordSize(
    const RecordDefinition& definition)
{
    return std::accumulate(cbegin(definition), cend(definition), 0ULL,
        [](size_t sum, const auto& field) {
            return sum + Layer::getElementSize(static_cast<DataType>(field.type));
        });
}

//! Determine the HDF5 file DataType from the specified data type.
/*!
\param type
    The data type to be matched.

\return
    The matching HDF5 type used in file reading and writing.
*/
const ::H5::AtomType& getH5fileType(
    DataType type)
{
    static ::H5::StrType strType{::H5::PredType::C_S1, H5T_VARIABLE};

    switch(type)
    {
    case DT_UINT32:
        return H5::PredType::NATIVE_UINT32;
    case DT_FLOAT32:
        return ::H5::PredType::NATIVE_FLOAT;
    case DT_UINT8:
        return ::H5::PredType::NATIVE_UINT8;
    case DT_UINT16:
        return ::H5::PredType::NATIVE_UINT16;
    case DT_UINT64:
        return ::H5::PredType::NATIVE_UINT64;
    case DT_BOOLEAN:
        return ::H5::PredType::NATIVE_HBOOL;
    case DT_STRING:
        return strType;
    case DT_COMPOUND:  //[fallthrough]
    case DT_UNKNOWN_DATA_TYPE:  //[fallthrough]
    default:
        throw UnsupportedDataType{};
    }
}

//! Determine the HDF5 memory DataType from the specified DataType.
/*!
\param type
    The data type to be matched.

\return
    The matching HDF5 type used in memory.
*/
const ::H5::AtomType& getH5memoryType(
    DataType type)
{
    static ::H5::StrType strType{::H5::PredType::C_S1, H5T_VARIABLE};

    switch(type)
    {
    case DT_UINT32:
        return H5::PredType::NATIVE_UINT32;
    case DT_FLOAT32:
        return ::H5::PredType::NATIVE_FLOAT;
    case DT_UINT8:
        return ::H5::PredType::NATIVE_UINT8;
    case DT_UINT16:
        return ::H5::PredType::NATIVE_UINT16;
    case DT_UINT64:
        return ::H5::PredType::NATIVE_UINT64;
    case DT_BOOLEAN:
        return ::H5::PredType::NATIVE_HBOOL;
    case DT_STRING:
        return strType;
    case DT_COMPOUND:  //[fallthrough]
    case DT_UNKNOWN_DATA_TYPE:  //[fallthrough]
    default:
        throw UnsupportedDataType{};
    }
}

//! Create an attribute on an HDF5 DataSet.
/*!
\param h5dataSet
    The HDF5 DataSet to create the attribute on.
\param attributeType
    The HDF5 type of the attribute.
\param path
    The HDF5 path of the attribute.
    The path cannot be nullptr.

\return
    The new HDF5 attribute.
*/
::H5::Attribute createAttribute(
    const ::H5::DataSet& h5dataSet,
    const ::H5::PredType& attributeType,
    const char* path)
{
    return h5dataSet.createAttribute(path, attributeType, {});
}

//! Create attributes on an HDF5 DataSet.
/*!
\param h5dataSet
    The HDF5 DataSet to create the attributes on.
\param attributeType
    The HDF5 type of the attributes.
\param paths
    The HDF5 paths of the attributes.
    If no paths are provided, no attributes are written.
    If a path is nullptr, that attribute is not written.
*/
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

//! Write an attribute to the specified HDF5 DataSet.
/*!
\param h5dataSet
    The HDF5 DataSet to create the attribute on.
\param attributeType
    The HDF5 type of the attribute.
\param valiue
    The value of the attribute.
\param path
    The HDF5 path of the attribute.
    The path cannot be nullptr.
*/
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

//! Write attributes to an HDF5 DataSet.
/*!
\param h5dataSet
    The HDF5 DataSet to create the attributes on.
\param attributeType
    The HDF5 type of the attributes.
\param paths
    The HDF5 paths of the attributes.
    If no paths are provided, no attributes are written.
    If a path is nullptr, that attribute is not written.
*/
template <typename T>
void writeAttributes(
    const ::H5::DataSet& h5dataSet,
    const ::H5::PredType& attributeType,
    T value,
    const std::vector<const char*>& paths)
{
    if (paths.empty())
        return;

    for (const auto& path : paths)
        if (path)
            writeAttribute(h5dataSet, attributeType, value, path);
}


// Explicit template instantiations.

//! Write a float point value to an attribute.
/*
\param h5dataSet
    The HDF5 DataSet to create the attributes on.
\param attributeType
    The HDF5 type of the attribute.
\param value
    The value to write to the attribute.
\param path
    The HDF5 path to the attribute in the HDF5 DataSet.
    Cannot be nullptr.
*/
template void writeAttribute<float>(const ::H5::DataSet& h5dataSet,
    const ::H5::PredType& attributeType, float value, const char* path);

//! Write an unsigned 32 bit integer value to an attribute.
/*
\param h5dataSet
    The HDF5 DataSet to create the attributes on.
\param attributeType
    The HDF5 type of the attribute.
\param value
    The value to write to the attribute.
\param path
    The HDF5 path to the attribute in the HDF5 DataSet.
    Cannot be nullptr.
*/
template void writeAttribute<uint32_t>(const ::H5::DataSet& h5dataSet,
    const ::H5::PredType& attributeType, uint32_t value, const char* path);

//! Write a floating point value to multiple attributes.
/*
\param h5dataSet
    The HDF5 DataSet to create the attributes on.
\param attributeType
    The HDF5 type of the attribute.
\param value
    The value to write to the attribute.
\param paths
    The HDF5 path to the attributes in the HDF5 DataSet.
    If no paths provided, no attributes are written.
    If any path is nullptr, that attribute is not written.
*/
template void writeAttributes<float>(const ::H5::DataSet& h5dataSet,
    const ::H5::PredType& attributeType, float value,
    const std::vector<const char*>& paths);

//! Write an unsigned 32 bit integer value to multiple attributes.
/*
\param h5dataSet
    The HDF5 DataSet to create the attributes on.
\param attributeType
    The HDF5 type of the attribute.
\param value
    The value to write to the attribute.
\param paths
    The HDF5 path to the attributes in the HDF5 DataSet.
    If no paths provided, no attributes are written.
    If any path is nullptr, that attribute is not written.
*/
template void writeAttributes<uint32_t>(const ::H5::DataSet& h5dataSet,
    const ::H5::PredType& attributeType, uint32_t value,
    const std::vector<const char*>& paths);

}  // namespace BAG

