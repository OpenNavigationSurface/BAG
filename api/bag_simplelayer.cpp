
#include "bag_private.h"
#include "bag_simplelayer.h"
#include "bag_simplelayerdescriptor.h"

#include <array>
#include <h5cpp.h>


namespace BAG {

namespace {

#if 0 // TODO Needed?
const ::H5:PredType& getH5dataType(
    LayerType layerType)
{
    switch (layerType)
    {
    case Elevation: // Uncertainty, Nominal_Elevation
        h5type.insertMember("hyp_strength",
            HOFFSET(BagOptNodeGroup, hyp_strength),
            ::H5::PredType::NATIVE_FLOAT);
        break;
    case Num_Hypotheses:
        h5type.insertMember("num_hypotheses",
            HOFFSET(BagOptNodeGroup, num_hypotheses),
            ::H5::PredType::NATIVE_UINT);
        break;
    default:
        throw 112233;  // Unknown group type.
        break;
    }
}
#endif

const ::H5::PredType& getH5PredType(DataType type)
{
    switch(type)
    {
    case UINT32:
        return ::H5::PredType::NATIVE_UINT32;
    case FLOAT32:
        return ::H5::PredType::NATIVE_FLOAT;
    default:
        throw 1122;  // Unsupported DataType.
    }
}

}  // namespace

SimpleLayer::SimpleLayer(
    Dataset& dataset,
    const LayerDescriptor& descriptor,
    std::unique_ptr<::H5::DataSet, Dataset::DeleteH5DataSet> h5dataSet)
    : Layer(dataset, descriptor)
    , m_pH5dataSet(std::move(h5dataSet))
{
}

std::unique_ptr<SimpleLayer> SimpleLayer::create(
    Dataset& dataset,
    const LayerType type)
{
    auto descriptor = SimpleLayerDescriptor::create(type);
    auto h5DataSet = SimpleLayer::createH5dataSet(dataset, *descriptor);

    return std::unique_ptr<SimpleLayer>(new SimpleLayer{dataset, *descriptor,
        std::move(h5DataSet)});
}

std::unique_ptr<SimpleLayer> SimpleLayer::open(
    Dataset& dataset,
    const LayerDescriptor& descriptor)
{
    auto h5DataSet = dataset.openLayerH5DataSet(descriptor);

    return std::unique_ptr<SimpleLayer>(new SimpleLayer{dataset, descriptor,
        std::move(h5DataSet)});
}


std::unique_ptr<::H5::DataSet, Dataset::DeleteH5DataSet>
SimpleLayer::createH5dataSet(
    const Dataset& inDataset,
    const LayerDescriptor& descriptor)
{
#if 0
    if ((dataspace_id = H5Screate_simple(RANK, dims, NULL)) < 0)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_DATASPACE_FAILURE);
    }
#endif

    const auto dims = descriptor.getDims();
    std::array<hsize_t, 2> size{std::get<0>(dims), std::get<1>(dims)};
    ::H5::DataSpace h5dataSpace{RANK, size.data()};  //TODO Check that max dim size == starting size

#if 0
    if ((datatype_id = H5Tcopy(H5T_NATIVE_FLOAT)) < 0)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_TYPE_COPY_FAILURE);
    }

    if ((status = H5Tset_order(datatype_id, H5T_ORDER_LE)) < 0)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_SET_PROPERTY_FAILURE);
    }
#endif

    ::H5::AtomType h5dataType{::H5::PredType::NATIVE_FLOAT};
    h5dataType.setOrder(H5T_ORDER_LE);

#if 0
    if ((plist_id = H5Pcreate(H5P_DATASET_CREATE)) < 0)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_PROPERTY_CLASS_FAILURE);
    }

    status = H5Pset_fill_time  (plist_id, H5D_FILL_TIME_ALLOC);
    status = H5Pset_fill_value (plist_id, datatype_id, &null_elv);
    check_hdf_status();
#endif

    const ::H5::DSetCreatPropList h5createPropList{};
    h5createPropList.setFillTime(H5D_FILL_TIME_ALLOC);

    constexpr float kFillValue = BAG_NULL_ELEVATION;
    h5createPropList.setFillValue(h5dataType, &kFillValue);


#if 0
    if (data->compressionLevel > 0 && data->compressionLevel <= 9)
    {
        status = H5Pset_layout (plist_id, H5D_CHUNKED);
        status = H5Pset_chunk(plist_id, RANK, chunk_size);

        if ((status = H5Pset_deflate (plist_id, data->compressionLevel)) < 0)
        {
            status = H5Fclose (file_id);
            return (BAG_HDF_SET_PROPERTY_FAILURE);
        }
    }
    else if (data->compressionLevel) /* if anything other than zero */
    {
        return (BAG_HDF_INVALID_COMPRESSION_LEVEL);
    }
#endif

    const int compressionLevel = descriptor.getCompressionLevel();
    if (compressionLevel > 0 && compressionLevel <= 9)
    {
        h5createPropList.setLayout(H5D_CHUNKED);

        std::array<uint64_t, 2> chunkSize{descriptor.getChunkSize(),
            descriptor.getChunkSize()};
        h5createPropList.setChunk(RANK, chunkSize.data());
        h5createPropList.setDeflate(compressionLevel);
    }


#if 0
    if ((dataset_id = H5Dcreate(file_id, ELEVATION_PATH, datatype_id, dataspace_id, plist_id)) < 0)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_GROUP_FAILURE);
    }
#endif

    const auto& h5file = inDataset.getH5file();

    auto pH5dataSet = std::unique_ptr<::H5::DataSet, Dataset::DeleteH5DataSet>(
            new ::H5::DataSet{h5file.createDataSet(descriptor.getInternalPath(),
                h5dataType, h5dataSpace, h5createPropList)},
            Dataset::DeleteH5DataSet{});


#if 0
    /*! Add the attributes to the \a elevation dataset */
    if ((status = bagCreateAttribute (*bag_handle, dataset_id, (u8 *)MIN_ELEVATION_NAME, sizeof(f32), BAG_ATTR_F32)) != BAG_SUCCESS)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
    }
    if ((status = bagWriteAttribute (*bag_handle, dataset_id, (u8 *)MIN_ELEVATION_NAME, (void *) &(data->min_elevation) )) != BAG_SUCCESS)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
    }
    if ((status = bagCreateAttribute (*bag_handle, dataset_id, (u8 *)MAX_ELEVATION_NAME, sizeof(f32), BAG_ATTR_F32)) != BAG_SUCCESS)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
    }
    if ((status = bagWriteAttribute (*bag_handle, dataset_id, (u8 *)MAX_ELEVATION_NAME, (void *) &(data->max_elevation) )) != BAG_SUCCESS)
    {
        status = H5Fclose (file_id);
        return (BAG_HDF_CREATE_ATTRIBUTE_FAILURE);
    }
#endif

    const char* minAttName = MIN_ELEVATION_NAME;
    const char* maxAttName = MAX_ELEVATION_NAME;

    if (descriptor.getLayerType() == Uncertainty)
    {
        minAttName = MIN_UNCERTAINTY_NAME;
        maxAttName = MAX_UNCERTAINTY_NAME;
    }
    else
        throw UnknownSimpleLayerType{};

    const ::H5::DataSpace minElevDataSpace{};
    const auto minElevAtt = pH5dataSet->createAttribute(minAttName,
        ::H5::PredType::NATIVE_FLOAT, minElevDataSpace);

    constexpr float minElev = 0.f;
    minElevAtt.write(::H5::PredType::NATIVE_FLOAT, &minElev);

    const ::H5::DataSpace maxElevDataSpace{};
    const auto maxElevAtt = pH5dataSet->createAttribute(maxAttName,
        ::H5::PredType::NATIVE_FLOAT, maxElevDataSpace);

    constexpr float maxElev = 0.f;
    maxElevAtt.write(::H5::PredType::NATIVE_FLOAT, &maxElev);

    return pH5dataSet;
}

std::unique_ptr<uint8_t[]> SimpleLayer::readProxy(
    uint32_t rowStart,
    uint32_t columnStart,
    uint32_t rowEnd,
    uint32_t columnEnd) const
{
    //TODO Consider if this is reading 1 row and 1 column .. (RANK == 1)
    const auto rows = (rowEnd - rowStart) + 1;
    const auto columns = (columnEnd - columnStart) + 1;
    const std::array<hsize_t, RANK> count{rows, columns};
    const std::array<hsize_t, RANK> offset{rowStart, columnStart};

    // Query the file for the specified rows and columns.
    const auto h5dataSpace = m_pH5dataSet->getSpace();
    h5dataSpace.selectHyperslab(H5S_SELECT_SET, count.data(), offset.data());

    // Initialize the output buffer.
    const auto bufferSize = this->getDescriptor().getReadBufferSize(rows,
        columns);
    auto buffer = std::make_unique<uint8_t[]>(bufferSize);

    // Prepare the memory space.
    const ::H5::DataSpace h5memSpace{RANK, count.data(), count.data()};

    m_pH5dataSet->read(buffer.get(), H5Dget_type(m_pH5dataSet->getId()),
        h5memSpace, h5dataSpace);

    return buffer;
}

void SimpleLayer::writeProxy(
    uint32_t rowStart,
    uint32_t columnStart,
    uint32_t rowEnd,
    uint32_t columnEnd,
    const uint8_t* buffer) const
{
    //TODO Consider if this is writing 1 row and 1 column .. (RANK == 1)
    const auto rows = (rowEnd - rowStart) + 1;
    const auto columns = (columnEnd - columnStart) + 1;
    const std::array<hsize_t, RANK> count{rows, columns};
    const std::array<hsize_t, RANK> offset{rowStart, columnStart};

    // Query the file for the specified rows and columns.
    const auto h5dataSpace = m_pH5dataSet->getSpace();
    h5dataSpace.selectHyperslab(H5S_SELECT_SET, count.data(), offset.data());

    // Prepare the memory space.
    const ::H5::DataSpace h5memSpace{2, count.data(), count.data()};

    m_pH5dataSet->write(buffer, H5Dget_type(m_pH5dataSet->getId()),
        h5memSpace, h5dataSpace);
}

}   //namespace BAG

