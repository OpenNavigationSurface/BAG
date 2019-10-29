
#include "bag_compoundlayer.h"
#include "bag_metadata.h"
#include "bag_trackinglist.h"

#include <h5cpp.h>


namespace BAG {

CompoundLayer::CompoundLayer(
    Dataset& dataset,
    LayerDescriptor& descriptor)
    : Layer(dataset, descriptor) //,Compound)
{
}


const std::vector<CompoundLayer::Definition>&
CompoundLayer::getDefinition() const noexcept
{
    return m_definitions;
}

std::unique_ptr<CompoundLayer> CompoundLayer::open(
    Dataset& dataset,
    LayerDescriptor& descriptor)
{
    auto h5DataSet = dataset.openLayerH5DataSet(descriptor);

    // Read the min/max attribute values.
    const auto possibleMinMax = dataset.getMinMax(descriptor.getLayerType());
    if (std::get<0>(possibleMinMax))
        descriptor.setMinMax({std::get<1>(possibleMinMax),
            std::get<1>(possibleMinMax)});

    return std::unique_ptr<CompoundLayer>(new CompoundLayer{dataset,
        descriptor});
}


std::unique_ptr<uint8_t[]> CompoundLayer::readProxy(
    uint32_t rowStart,
    uint32_t columnStart,
    uint32_t rowEnd,
    uint32_t columnEnd) const
{
    rowStart;  columnStart;  rowEnd;  columnEnd;
    //TODO Implement.
    return {};
}

void CompoundLayer::writeProxy(
    uint32_t rowStart,
    uint32_t columnStart,
    uint32_t rowEnd,
    uint32_t columnEnd,
    const uint8_t* buffer) const
{
    rowStart;  columnStart;  rowEnd;  columnEnd;  buffer;
    //TODO Implement.

    //TODO update min/max.  Here?  Layer::write()? elsewhere?
}

}

