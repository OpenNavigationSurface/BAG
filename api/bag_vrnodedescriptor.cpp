
#include "bag_exceptions.h"
#include "bag_private.h"
#include "bag_vrnodedescriptor.h"


namespace BAG {

//! Constructor.
/*!
\param id
    The unique layer id.
\param chunkSize
    The chunk size the HDF5 DataSet will use.
\param compressionLevel
    The compression level the HDF5 DataSet will use.
*/
VRNodeDescriptor::VRNodeDescriptor(
    uint32_t id,
    uint32_t rows, uint32_t cols,
    uint64_t chunkSize,
    int compressionLevel)
    : LayerDescriptor(id, VR_NODE_PATH,
        kLayerTypeMapString.at(VarRes_Node), VarRes_Node,
        rows, cols,
        chunkSize, compressionLevel)
{
}

//! Constructor.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
*/
VRNodeDescriptor::VRNodeDescriptor(
    const Dataset& dataset,
    uint32_t rows, uint32_t cols)
    : LayerDescriptor(dataset, VarRes_Node, rows, cols, VR_NODE_PATH)
{
}

//! Create a mew variable resolution node.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
\param chunkSize
    The chunk size the HDF5 DataSet will use.
\param compressionLevel
    The compression level the HDF5 DataSet will use.

\return
    A new variable resolution node layer.

*/
std::shared_ptr<VRNodeDescriptor> VRNodeDescriptor::create(
    const Dataset& dataset,
    uint64_t chunkSize,
    int compressionLevel)
{
    return std::shared_ptr<VRNodeDescriptor>(
        new VRNodeDescriptor{dataset.getNextId(), 1, 0, chunkSize,
            compressionLevel});
}

//! Open an existing variable resolution node.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
*/
std::shared_ptr<VRNodeDescriptor> VRNodeDescriptor::open(
    const Dataset& dataset, uint32_t rows, uint32_t cols)
{
    return std::shared_ptr<VRNodeDescriptor>(
        new VRNodeDescriptor{dataset, rows, cols});
}

//! \copydoc LayerDescriptor::getDataType
DataType VRNodeDescriptor::getDataTypeProxy() const noexcept
{
    return DT_COMPOUND;
}

//! \copydoc LayerDescriptor::getElementSize
uint8_t VRNodeDescriptor::getElementSizeProxy() const noexcept
{
    return sizeof(BagVRNodeItem);
}

//! Retrieve the minimum and maximum hypotheses strength.
/*!
\return
    The minimum and maximum hypotheses strength.
*/
std::tuple<float, float>
VRNodeDescriptor::getMinMaxHypStrength() const noexcept
{
    return {m_minHypStrength, m_maxHypStrength};
}

//! Retrieve the minimum and maximum n samples.
/*!
\return
    The minimum and maximum n samples.
*/
std::tuple<uint32_t, uint32_t>
VRNodeDescriptor::getMinMaxNSamples() const noexcept
{
    return {m_minNSamples, m_maxNSamples};
}

//! Retrieve the minimum and maximum number of hypotheses.
/*!
\return
    The minimum and maximum number of hypotheses.
*/
std::tuple<uint32_t, uint32_t>
VRNodeDescriptor::getMinMaxNumHypotheses() const noexcept
{
    return {m_minNumHypotheses, m_maxNumHypotheses};
}

//! Set the minimum and maximum hypotheses strength.
/*!
\param minHypStrength
    The minimum hypotheses strength.
\param maxHypStrength
    The maximum hypotheses strength.

\return
    The variable resolution node descriptor.
*/
VRNodeDescriptor& VRNodeDescriptor::setMinMaxHypStrength(
    float minHypStrength,
    float maxHypStrength) & noexcept
{
    m_minHypStrength = minHypStrength;
    m_maxHypStrength = maxHypStrength;
    return *this;
}

//! Set the minimum and maximum number of samples.
/*!
\param minNSamples
    The minimum number of samples.
\param maxNSamples
    The maximum number of samples.

\return
    The variable resolution node descriptor.
*/
VRNodeDescriptor& VRNodeDescriptor::setMinMaxNSamples(
    uint32_t minNSamples,
    uint32_t maxNSamples) & noexcept
{
    m_minNSamples = minNSamples;
    m_maxNSamples = maxNSamples;
    return *this;
}

//! Set the minimum and maximum number of hypotheses.
/*!
\param minNumHypotheses
    The minimum number of hypotheses.
\param maxNumHypotheses
    The maximum number of hypotheses.

\return
    The variable resolution node descriptor.
*/
VRNodeDescriptor& VRNodeDescriptor::setMinMaxNumHypotheses(
    uint32_t minNumHypotheses,
    uint32_t maxNumHypotheses) & noexcept
{
    m_minNumHypotheses = minNumHypotheses;
    m_maxNumHypotheses = maxNumHypotheses;
    return *this;
}

}  // namespace BAG

