#include "bag_dataset.h"
#include "bag_layer.h"

#include <iomanip>
#include <iostream>
#include <memory>

int main(
    int argc,
    char** argv)
{
    using BAG::Dataset;

    auto pDataset = Dataset::open(
        "D:/caris_work/BAG/BAG2/examples/sample-data/sample.bag",
        BAG_OPEN_READONLY);

    uint64_t numRows = 0, numCols = 0;
    std::tie(numRows, numCols) =
        pDataset->getDescriptor().getDims();

    double minX = 0., minY = 0., maxX = 0., maxY = 0.;
    std::tie(minX, minY) = pDataset->gridToGeo(0, 0);
    std::tie(maxX, maxY) = pDataset->gridToGeo(numRows - 1, numCols - 1);

    std::cout << std::setprecision(10) << minX << ' ' << minY << ' ' << maxX <<
        ' ' << maxY << '\n';

    for (auto type : pDataset->getLayerTypes())
    {
        const auto& descriptor = pDataset->getDescriptor();
        const auto& layer = pDataset->getLayer(type);
        const auto& layerDescriptor = layer.getDescriptor();

        const auto& name = layerDescriptor.getName();
        const auto dataType = layerDescriptor.getDataType();
        const auto layerType = layerDescriptor.getLayerType();

        auto buffer = layer.read(0, 0, numRows - 1, numCols - 1);
        //TODO Do stuff with buffer.
    }

    return 0;
}

