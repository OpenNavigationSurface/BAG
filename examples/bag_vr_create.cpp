/*! \file bag_vr_create.c
 * \brief Generate a BAG with variable resolution extensions with synthetic data
 *
 * In order to demonstrate how to deal with variable resolution extensions in
 * a BAG-VR, this code generates a file ab initio (assuming that the XML
 * metadata information is available), and makes synthetic data for it.
 *
 * Open Navigation Surface Working Group, 2015.  Visit the project website at
 * http://www.opennavsurf.org
 */

#include "getopt.h"

#include <bag_dataset.h>
#include <bag_descriptor.h>
#include <bag_simplelayer.h>
#include <bag_simplelayerdescriptor.h>
#include <bag_vrmetadata.h>
#include <bag_vrnode.h>
#include <bag_vrrefinement.h>

#include <cmath>
#include <cstdio>
#include <iostream>
#include <string>


namespace {

enum Cmd {
    XML_FILENAME = 1,
    OUTPUT_BAG,
    ARGC_EXPECTED
};

using std::cos;

void generateData(
    uint32_t x,
    float y_min,
    float y_max,
    std::vector<float>& data)
{
    const auto length = data.size();
    uint32_t p = 0;

    for (auto& d : data)
        d = (y_min + 0.5f * (y_max - y_min)
            * (1.0f + cos(2.0f * M_PI * p++ / static_cast<float>(length)
            + x / static_cast<float>(length))));
}

std::shared_ptr<BAG::Dataset> constructBasicBAG(
    const std::string& xmlFileName,
    const std::string& outFileName)
{
    // Create the metadata.
    BAG::Metadata metadata;
    metadata.loadFromFile(xmlFileName);

    // Create the Dataset.  Mandatory layers are created during this call.
    constexpr uint64_t chunkSize = 100;
    constexpr unsigned int compressionLevel = 1;

    auto dataset = BAG::Dataset::create(outFileName, std::move(metadata),
        chunkSize, compressionLevel);
    if (!dataset)
        return {};

    // Get the elevation layer.
    auto* elevationLayer = dataset->getSimpleLayer(Elevation);
    if(!elevationLayer)
        return {};

    //NOTE:  When writing data to a layer, the min/max attributes update
    // automatically.

    // Set the min/max values.
    constexpr float minElevation = -10.f;
    constexpr float maxElevation = 10.f;

    elevationLayer->getDescriptor().setMinMax(minElevation, maxElevation);

    // Get the uncertainty layer.
    auto* uncertaintyLayer = dataset->getSimpleLayer(Uncertainty);
    if(!uncertaintyLayer)
        return {};

    // Set the min/max values.
    constexpr float minUncert = 0.f;
    constexpr float maxUncert = 10.f;

    uncertaintyLayer->getDescriptor().setMinMax(minUncert, maxUncert);

    const auto& descriptor = dataset->getDescriptor();

    // Populate the elevation and uncertainty layers.
    uint32_t lowResRows = 0;
    uint32_t lowResColumns = 0;
    std::tie(lowResRows, lowResColumns) = descriptor.getDims();

    std::vector<float> depth(lowResColumns);
    std::vector<float> uncertainty(lowResColumns);

    constexpr uint32_t columnStart = 0;
    const uint32_t columnEnd = lowResColumns - 1;

    for (uint32_t row=0; row<lowResRows; ++row)
    {
        // Write a row to the elevation layer.
        generateData(row, minElevation, maxElevation, depth);

        auto* buffer = reinterpret_cast<uint8_t*>(depth.data());
        elevationLayer->write(row, columnStart, row, columnEnd, buffer);

        // Write a row to the uncertainty layer.
        generateData(row, minUncert, maxUncert, uncertainty);

        buffer = reinterpret_cast<uint8_t*>(uncertainty.data());
        uncertaintyLayer->write(row, columnStart, row, columnEnd, buffer);
    }

    // Create the optional NumHypotheses and HypStrength layers.
    dataset->createSimpleLayer(Num_Hypotheses, chunkSize, compressionLevel);
    dataset->createSimpleLayer(Hypothesis_Strength, chunkSize, compressionLevel);

    return dataset;
}

void generateVRData(
    uint32_t rows,
    float minZ,
    float maxZ,
    float minU,
    float maxU,
    std::vector<BAG::VRRefinementItem>& data)
{
    if (data.size() < (rows*rows))
        return;

    uint32_t index = 0;

    for (uint32_t row=0; row<rows; ++row)
        for (uint32_t column=0; column<rows; ++column, ++index)
        {
            const auto rowPercent = column / static_cast<float>(rows);
            const auto columnPercent = row / static_cast<float>(rows);

            data[index].depth = minZ + 0.5f * (maxZ - minZ)
                * (1.0f + cos(2 * M_PI * columnPercent - rowPercent));
            data[index].depth_uncrt = minU + 0.5f * (maxU - minU)
                * (1.0f + cos(2 * M_PI * columnPercent - rowPercent));
        }
}

bool augmentVRBAG(
    BAG::Dataset& dataset)
{
    // Create the optional VR layers.
    constexpr uint64_t chunkSize = 100;
    constexpr unsigned int compressionLevel = 1;

    constexpr bool kCreateNode = true;
    dataset.createVR(chunkSize, compressionLevel, kCreateNode);

    // Get the number of rows and columns in the Simple layers.
    const auto& descriptor = dataset.getDescriptor();

    uint32_t lowResRows = 0;
    uint32_t lowResColumns = 0;
    std::tie(lowResRows, lowResColumns) = descriptor.getDims();

    std::vector<uint32_t> refColumns(lowResColumns);

    for (uint32_t i=0; i<lowResColumns; ++i)
        refColumns[i] = 2 + (i % 20);

    std::vector<float> lowResNumHypotheses(lowResColumns);
    std::vector<float> lowResHypStrength(lowResColumns);

    // The Node_Group layer is used to preserve the auxiliary information for
    // the low resolution data
    for (uint32_t column=0; column<lowResColumns; ++column)
    {
        lowResNumHypotheses[column] = 1.0f + column % 10;
        lowResHypStrength[column] = 4.0f * (column % 10) / 10.0f;
    }

    // Write the NumHypotheses and HypStrength data.

    auto* numHypothesesLayer = dataset.getSimpleLayer(Num_Hypotheses);
    if(!numHypothesesLayer)
        return false;

    auto* hypStrengthLayer = dataset.getSimpleLayer(Hypothesis_Strength);
    if (!hypStrengthLayer)
        return false;

    constexpr uint32_t columnStart = 0;
    const uint32_t columnEnd = lowResColumns - 1;

    for (uint32_t row=0; row<lowResRows; ++row)
    {
        auto* buffer = reinterpret_cast<uint8_t*>(lowResNumHypotheses.data());
        numHypothesesLayer->write(row, columnStart, row, columnEnd, buffer);

        buffer = reinterpret_cast<uint8_t*>(lowResHypStrength.data());
        hypStrengthLayer->write(row, columnStart, row, columnEnd, buffer);
    }

    // The VarRes_Metadata_Group is used to keep information about the size of
    // the refined grids, and where to find them in the list of refinements and
    // auxiliary data.
    // The VarRes_Refinement_Group has the actual refinements,
    // The VarRes_Node_Group has the auxiliary information.

    std::vector<BAG::VRMetadataItem> vrMetadataItems(lowResColumns);
    uint32_t numCells = 0;

    double nodeSpacingX = 0.;
    double nodeSpacingY = 0.;
    std::tie(nodeSpacingX, nodeSpacingY) = descriptor.getGridSpacing();

    uint32_t totalRefs = 0;

    constexpr float kMinElevation = -10.0f;
    constexpr float kMaxElevation = 10.0f;
    constexpr float kMinUncert = 0.0f;
    constexpr float kMaxUncert = 10.0f;

    auto* vrRefinementLayer = dataset.getVRRefinement();
    if (!vrRefinementLayer)
        return false;

    std::vector<BAG::VRRefinementItem> vrRefinements(21*21);
    std::vector<BAG::VRNodeItem> vrNodes(21*21);

    auto* vrNodeLayer = dataset.getVRNode();
    if (!vrNodeLayer)
        return false;

    auto* vrMetadataLayer = dataset.getVRMetadata();
    if (!vrMetadataLayer)
        return false;

    for (uint32_t row=0; row<lowResRows; ++row)
    {
        for(uint32_t column=0; column<lowResColumns; ++column)
        {
            auto& metadataItem = vrMetadataItems[column];
            const auto refColumn = refColumns[column];

            metadataItem.index = numCells;
            metadataItem.dimensions_x = refColumn;
            metadataItem.dimensions_y = refColumn;
            metadataItem.resolution_x = (nodeSpacingX - 0.1) / (refColumn - 1);
            metadataItem.resolution_y = (nodeSpacingY - 0.1) / (refColumn - 1);
            metadataItem.sw_corner_x = (nodeSpacingX
                - (metadataItem.dimensions_x - 1)
                * metadataItem.resolution_x) / 2.0f;
            metadataItem.sw_corner_y = (nodeSpacingY
                - (metadataItem.dimensions_y - 1)
                * metadataItem.resolution_y) / 2.0f;

            // Refinement information.
            totalRefs = refColumn * refColumn;

            generateVRData(refColumn, kMinElevation, kMaxElevation,
                kMinUncert, kMaxUncert, vrRefinements);

            auto* buffer = reinterpret_cast<uint8_t*>(vrRefinements.data());
            vrRefinementLayer->write(0, numCells, 0,
                numCells + totalRefs - 1, buffer);

            // Auxiliary information.
            for (uint32_t ref=0; ref<totalRefs; ++ref)
            {
                vrNodes[ref].hyp_strength = 4.0f * (ref % 10) / 10.0f;
                vrNodes[ref].num_hypotheses = 1 + ref % 10;
                vrNodes[ref].n_samples = 1 + ref % 15;
            }

            buffer = reinterpret_cast<uint8_t*>(vrNodes.data());
            vrNodeLayer->write(0, numCells, 0, numCells + totalRefs - 1,
                buffer);

            // Move on in the refinement/node group arrays
            numCells += totalRefs;
        }

        auto* buffer = reinterpret_cast<uint8_t*>(vrMetadataItems.data());
        vrMetadataLayer->write(row, 0, row, lowResColumns - 1, buffer);
    }

    return true;
}

}  // namespace


int main(
    int argc,
    char* argv[])
{
    bool generateHelp = false;

    int c = getopt(argc, argv, "h");

    while (c != EOF)
    {
        switch (c)
        {
        case 'h':
            generateHelp = true;
            break;
        case '?':  //[[fallthrough]]
        default:
            std::cerr << "error: unknown option flag '" << +optopt << "'\n";
            break;
        }

        c = getopt(argc, argv, "h");
    }

    argc -= optind - 1;
    argv += optind - 1;

    if (argc != ARGC_EXPECTED || generateHelp)
    {
        std::cout << "bag_vr_create [" << __DATE__ << R"(] - Create a basic variable-resolution BAG file.
Syntax: bag_vr_create [opt] <xml_file> <output_file>
Options:
 -h Generate this help information.
)";

        return EXIT_FAILURE;
    }

    std::shared_ptr<BAG::Dataset> dataset;

    try
    {
        auto dataset = constructBasicBAG(argv[XML_FILENAME], argv[OUTPUT_BAG]);
        if (!dataset)
        {
            std::cerr << "error: failed to initialise basic BAG file.\n";
            return EXIT_FAILURE;
        }

        const auto& descriptor = dataset->getDescriptor();

        uint32_t rows = 0;
        uint32_t columns = 0;
        std::tie(rows, columns) = descriptor.getDims();

        double xSpacing = 0.;
        double ySpacing = 0.;
        std::tie(xSpacing, ySpacing) = descriptor.getGridSpacing();

        std::cout << "Basic BAG construction complete (" << rows << ", "
            << columns << ") cells, spacing (" << xSpacing << ", " << ySpacing
            << ").\n";

        if (!augmentVRBAG(*dataset))
        {
            std::cerr << "error: failed to add variable resolution layers to BAG\n";
            return EXIT_FAILURE;
        }

        std::cout << "Extended BAG-VR construction complete.\n";
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    dataset.reset();

    std::cout << "BAG-VR file closed.\n";

    return EXIT_SUCCESS;
}

