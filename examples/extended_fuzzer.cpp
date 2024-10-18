#include <iostream>
#include <iomanip>

#include <stddef.h>
#include <stdint.h>
#include <unistd.h>

#include "bag_dataset.h"

using BAG::Dataset;


void printLayerDescriptor(
        const BAG::LayerDescriptor& descriptor)
{
    std::cout << "\t\tchunkSize == " << descriptor.getChunkSize() << '\n';
    std::cout << "\t\tcompression level == " << descriptor.getCompressionLevel() << '\n';
    std::cout << "\t\tdata type == " << descriptor.getDataType() << '\n';
    std::cout << "\t\telement size == " << +descriptor.getElementSize() << '\n';
    std::cout << "\t\tinternalPath == " << descriptor.getInternalPath() << '\n';
    std::cout << "\t\tlayer type == " << descriptor.getLayerType() << '\n';

    const auto minMax = descriptor.getMinMax();
    std::cout << "\t\tmin max == (" << std::get<0>(minMax) << ", " <<
              std::get<1>(minMax) << ")\n";
}

extern "C" int LLVMFuzzerTestOneInputByFile(const char* filename) {


    auto pDataset = Dataset::open(filename, BAG_OPEN_READONLY);
    if (pDataset == NULL) {
        return EXIT_FAILURE;
    }

    const auto& descriptor = pDataset->getDescriptor();
    uint64_t numRows = 0, numCols = 0;
    std::tie(numRows, numCols) = descriptor.getDims();
    std::cout << "\trows, columns == " << numRows << ", " << numCols << '\n';

    double minX = 0., minY = 0., maxX = 0., maxY = 0.;
    std::tie(minX, minY) = pDataset->gridToGeo(0, 0);
    std::tie(maxX, maxY) = pDataset->gridToGeo(numRows - 1, numCols - 1);

    std::cout << "\tgrid cover (llx, lly), (urx, ury) == (" <<
              std::setprecision(10) << minX << ", " << minY << "), (" << maxX <<
              ", " << maxY << ")\n";

    const auto& dims = descriptor.getDims();
    std::cout << "\tdims == (" << std::get<0>(dims) << ", " <<
              std::get<1>(dims) << ")\n";

    const auto& gridSpacing = descriptor.getGridSpacing();
    std::cout << "\tgrid spacing == (" << std::get<0>(gridSpacing) << ", " <<
              std::get<1>(gridSpacing) << ")\n";

    const auto& origin = descriptor.getOrigin();
    std::cout << "\torigin == (" << std::get<0>(origin) << ", "
              << std::get<1>(origin) << ")\n";

    const auto& projCover = descriptor.getProjectedCover();
    std::cout << "\tprojected cover (llx, lly), (urx, ury) == (" << std::get<0>(projCover) << ", " <<
              std::get<1>(projCover) << "), (" << std::get<2>(projCover) << ", " <<
              std::get<3>(projCover) << ")\n";

    std::cout << "\tversion == " << descriptor.getVersion() << '\n';

    std::cout << "\thorizontal reference system ==\n" <<
              descriptor.getHorizontalReferenceSystem() << "\n\n";

    std::cout << "\tvertical reference system ==\n" << descriptor.getVerticalReferenceSystem() << '\n';

    std::cout << "\nLayers:\n";

    for (const auto& layer : pDataset->getLayers())
    {
        auto pDescriptor = layer->getDescriptor();

        std::cout << "\t" << pDescriptor->getName() << " Layer .. id(" <<
                  pDescriptor->getId() << ")\n";

        printLayerDescriptor(*pDescriptor);
    }

    const auto& trackingList = pDataset->getTrackingList();
    std::cout << "\nTracking List:  (" << trackingList.size() << " items)\n";

    size_t itemNum = 0;
    for (const auto& item : trackingList)
    {
        std::cout << "\tTracking list item #" << itemNum++ << '\n';
        std::cout << "\t\trow == " << item.row << '\n';
        std::cout << "\t\tcol == " << item.col << '\n';
        std::cout << "\t\tdepth == " << item.depth << '\n';
        std::cout << "\t\tuncertainty == " << item.uncertainty << '\n';
        std::cout << "\t\ttrack_code == " << item.track_code << '\n';
        std::cout << "\t\tlist_series == " << item.list_series << '\n';
    }

    pDataset->close();

    return EXIT_SUCCESS;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *buf, size_t len) {
    char filename[256];
    snprintf(filename, 255, "/tmp/libfuzzer.%d", getpid());

    // Save input file to temporary file so that we can read it.
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        return 0;
    }
    fwrite(buf, len, 1, fp);
    fclose(fp);

    return LLVMFuzzerTestOneInputByFile(filename);
}