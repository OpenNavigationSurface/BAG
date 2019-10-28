
#include "bag_interleavedlayer.h"
#include "bag_interleavedlayerdescriptor.h"
#include "bag_private.h"

#include <array>
#include <h5cpp.h>


namespace BAG {

namespace {

struct BagOptNodeGroup final
{
    float hyp_strength = 0.f;
    uint32_t num_hypotheses = 0;
};

struct BagOptElevationSolutionGroup final
{
    float shoal_elevation = 0.f;
    float stddev = 0.f;
    uint32_t num_soundings = 0;

};

::H5::CompType getH5dataType(
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

}   //namespace

InterleavedLayer::InterleavedLayer(
    Dataset& dataset,
    InterleavedLayerDescriptor& descriptor,
    std::unique_ptr<::H5::DataSet, Dataset::DeleteH5DataSet> h5dataSet)
    : Layer(dataset, descriptor)
    , m_pH5dataSet(std::move(h5dataSet))
{
}

std::unique_ptr<InterleavedLayer> InterleavedLayer::open(
    Dataset& dataset,
    InterleavedLayerDescriptor& descriptor)
{
    auto h5DataSet = dataset.openLayerH5DataSet(descriptor);

    // Read the min/max attribute values.
    const auto possibleMinMax = dataset.getMinMax(descriptor.getLayerType());
    if (std::get<0>(possibleMinMax))
        descriptor.setMinMax({std::get<1>(possibleMinMax),
            std::get<1>(possibleMinMax)});

    return std::unique_ptr<InterleavedLayer>(new InterleavedLayer{dataset,
        descriptor, std::move(h5DataSet)});
}


std::unique_ptr<uint8_t[]> InterleavedLayer::readProxy(
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
    const auto h5fileSpace = m_pH5dataSet->getSpace();
    h5fileSpace.selectHyperslab(H5S_SELECT_SET, count.data(), offset.data());

    if (!dynamic_cast<const InterleavedLayerDescriptor*>(&this->getDescriptor()))
        throw 123456;  // Invalid descriptor found.

    const auto& descriptor =
        static_cast<const InterleavedLayerDescriptor&>(this->getDescriptor());

    // Initialize the output buffer.
    const auto bufferSize = descriptor.getReadBufferSize(rows, columns);
    auto buffer = std::make_unique<uint8_t[]>(bufferSize + 10);
    memset(buffer.get(), 16, bufferSize + 10);

    // Prepare the memory space.
    const ::H5::DataSpace h5memSpace{RANK, count.data(), count.data()};

    // Set up the type.
    const auto h5dataType = getH5dataType(descriptor.getLayerType(),
        descriptor.getGroupType());

    m_pH5dataSet->read(buffer.get(), h5dataType, h5memSpace, h5fileSpace);

    return buffer;
}

void InterleavedLayer::writeProxy(
    uint32_t /*rowStart*/,
    uint32_t /*columnStart*/,
    uint32_t /*rowEnd*/,
    uint32_t /*columnEnd*/,
    const uint8_t* /*buffer*/) const
{
    return;  // Writing Interleaved layers not supported.
}

}   //namespace BAG

