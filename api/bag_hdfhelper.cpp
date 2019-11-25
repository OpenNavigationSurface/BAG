
#include "bag_exceptions.h"
#include "bag_hdfhelper.h"

#include <array>
#include <h5cpp.h>


namespace BAG {

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
        if (filter == H5Z_FILTER_DEFLATE)
            if (cdNelmts >= 1)
                return cdValues.front();
    }

    return 0;
}

::H5::CompType getH5compType(
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
                ::H5::PredType::NATIVE_UINT);
            break;
        default:
            throw 112233;  // Unknown group type.
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
                ::H5::PredType::NATIVE_INT);
            break;
        default:
            throw 112233;  // Unknown group type.
        }
    }

    return h5type;
}

const ::H5::PredType& getH5memoryType(DataType type)
{
    switch(type)
    {
    case UINT32:
        return ::H5::PredType::NATIVE_UINT32;
    case FLOAT32:
        return ::H5::PredType::NATIVE_FLOAT;
    case UINT8:
        return ::H5::PredType::NATIVE_UINT8;
    case UINT16:
        return ::H5::PredType::NATIVE_UINT16;
    case UINT64:
        return ::H5::PredType::NATIVE_UINT64;
    case BOOL:  //[fallthrough]
    case STRING:  //[fallthrough]
    case COMPOUND:  //[fallthrough]
    case UNKNOWN_DATA_TYPE:  //[fallthrough]
    default:
        throw UnsupportedDataType{};
    }
}

}  // namespace BAG

