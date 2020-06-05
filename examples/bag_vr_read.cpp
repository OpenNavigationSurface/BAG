
#include "bag_dataset.h"
#include "bag_layer.h"
#include "bag_trackinglist.h"
#include "bag_vrmetadata.h"
#include "bag_vrmetadatadescriptor.h"
#include "bag_vrnode.h"
#include "bag_vrnodedescriptor.h"
#include "bag_vrrefinements.h"
#include "bag_vrrefinementsdescriptor.h"
#include "bag_vrtrackinglist.h"

#include <iomanip>
#include <iostream>
#include <memory>

namespace {

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

}

int main(
    int argc,
    char** argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage is: bag_read <inputBagFile>\n";
        return EXIT_FAILURE;
    }

    using BAG::Dataset;

    const std::string bagFileName = argv[1];

    auto pDataset = Dataset::open(bagFileName, BAG_OPEN_READONLY);

    const auto& descriptor = pDataset->getDescriptor();

    std::cout << "Dataset details:\n";

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
        std::cout << "\tVR tracking list item #" << itemNum++ << '\n';
        std::cout << "\t\trow == " << item.row << '\n';
        std::cout << "\t\tcol == " << item.col << '\n';
        std::cout << "\t\tdepth == " << item.depth << '\n';
        std::cout << "\t\tuncertainty == " << item.uncertainty << '\n';
        std::cout << "\t\ttrack_code == " << item.track_code << '\n';
        std::cout << "\t\tlist_series == " << item.list_series << '\n';
    }

    // VR stuff
    const auto* vrMetadata = pDataset->getVRMetadata();
    if (vrMetadata)
    {
        std::cout << "\nVR Metadata:\n";

        {
            auto pDescriptor =
                std::dynamic_pointer_cast<const BAG::VRMetadataDescriptor>(
                    vrMetadata->getDescriptor());

            std::cout << "\t" << pDescriptor->getName() << " Layer .. id(" <<
                pDescriptor->getId() << ")\n";

            printLayerDescriptor(*pDescriptor);

            // VRMetadataDescriptor specific info.
            const auto minDims = pDescriptor->getMinDimensions();
            std::cout << "\t\tmin dims == (" << std::get<0>(minDims) << ", " <<
                std::get<1>(minDims) << ")\n";

            const auto maxDims = pDescriptor->getMaxDimensions();
            std::cout << "\t\tmax dims == (" << std::get<0>(maxDims) << ", " <<
                std::get<1>(maxDims) << ")\n";

            const auto minRes = pDescriptor->getMinResolution();
            std::cout << "\t\tmin res == (" << std::get<0>(minRes) << ", " <<
                std::get<1>(minRes) << ")\n";

            const auto maxRes = pDescriptor->getMaxResolution();
            std::cout << "\t\tmax res == (" << std::get<0>(maxRes) << ", " <<
                std::get<1>(maxRes) << ")\n";
        }

        const auto* vrNode = pDataset->getVRNode();
        std::cout << "\nVR Node:\n";
        if (!vrNode)
            std::cout << "\tnot present\n";

        if (vrNode)
        {
            auto pDescriptor =
                std::dynamic_pointer_cast<const BAG::VRNodeDescriptor>(
                    vrNode->getDescriptor());

            std::cout << "\t" << pDescriptor->getName() << " Layer .. id(" <<
                pDescriptor->getId() << ")\n";

            printLayerDescriptor(*pDescriptor);

            // VRNodeDescriptor specific info.
            const auto minMaxHypStr = pDescriptor->getMinMaxHypStrength();
            std::cout << "\t\tmin/max hyp strength == (" <<
                std::get<0>(minMaxHypStr) << ", " << std::get<1>(minMaxHypStr)
                << ")\n";

            const auto minMaxNumHpy = pDescriptor->getMinMaxNumHypotheses();
            std::cout << "\t\tmin/max num hypotheses == (" <<
                std::get<0>(minMaxNumHpy) << ", " << std::get<1>(minMaxNumHpy)
                << ")\n";

            const auto minMaxNSam = pDescriptor->getMinMaxNSamples();
            std::cout << "\t\tmin/max n samples == (" <<
                std::get<0>(minMaxNSam) << ", " << std::get<1>(minMaxNSam) <<
                ")\n";
        }

        const auto* vrRefinements = pDataset->getVRRefinements();
        std::cout << "\nVR Refinements:\n";

        {
            auto pDescriptor =
                std::dynamic_pointer_cast<const BAG::VRRefinementsDescriptor>(
                    vrRefinements->getDescriptor());

            std::cout << "\t" << pDescriptor->getName() << " Layer .. id(" <<
                pDescriptor->getId() << ")\n";

            printLayerDescriptor(*pDescriptor);

            // VRRefinementDescriptor specific info.
            const auto minMaxDepth = pDescriptor->getMinMaxDepth();
            std::cout << "\t\tmin/max depth == (" <<
                std::get<0>(minMaxDepth) << ", " << std::get<1>(minMaxDepth) <<
                ")\n";

            const auto minMaxUncertainty = pDescriptor->getMinMaxUncertainty();
            std::cout << "\t\tmin/max uncertainty == (" <<
                std::get<0>(minMaxUncertainty) << ", " <<
                std::get<1>(minMaxUncertainty) << ")\n";
        }

        const auto* vrTrackingList = pDataset->getVRTrackingList();
        std::cout << "\nVR Tracking List:  (" << vrTrackingList->size() << " items)\n";

        itemNum = 0;
        for (const auto& item : *vrTrackingList)
        {
            std::cout << "\tVR tracking list item #" << itemNum++ << '\n';
            std::cout << "\t\tsub_row == " << item.sub_row << '\n';
            std::cout << "\t\tsub_col == " << item.sub_col << '\n';
            std::cout << "\t\trow == " << item.row << '\n';
            std::cout << "\t\tcol == " << item.col << '\n';
            std::cout << "\t\tdepth == " << item.depth << '\n';
            std::cout << "\t\tuncertainty == " << item.uncertainty << '\n';
            std::cout << "\t\ttrack_code == " << item.track_code << '\n';
            std::cout << "\t\tlist_series == " << item.list_series << '\n';
        }
    }

    return EXIT_SUCCESS;
}

