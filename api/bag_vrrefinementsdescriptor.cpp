
#include "bag_exceptions.h"
#include "bag_private.h"
#include "bag_vrrefinementsdescriptor.h"


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
VRRefinementsDescriptor::VRRefinementsDescriptor(
    uint32_t id,
    uint64_t chunkSize,
    int compressionLevel)
    : LayerDescriptor(id, VR_REFINEMENT_PATH,
        kLayerTypeMapString.at(VarRes_Refinement), VarRes_Refinement, chunkSize,
        compressionLevel)
{
}

//! Constructor.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
*/
VRRefinementsDescriptor::VRRefinementsDescriptor(
    const Dataset& dataset)
    : LayerDescriptor(dataset, VarRes_Refinement, VR_REFINEMENT_PATH)
{
}

//! Create a new variable resolution refinements descriptor.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
\param chunkSize
    The chunk size the HDF5 DataSet will use.
\param compressionLevel
    The compression level the HDF5 DataSet will use.

\return
    The new variable resolution refinements descriptor.
*/
std::shared_ptr<VRRefinementsDescriptor> VRRefinementsDescriptor::create(
    const Dataset& dataset,
    uint64_t chunkSize,
    int compressionLevel)
{
    return std::shared_ptr<VRRefinementsDescriptor>(
        new VRRefinementsDescriptor{dataset.getNextId(), chunkSize,
            compressionLevel});
}

//! Open an existing variable resolution refinements descriptor.
/*!
\param dataset
    The BAG Dataset this layer belongs to.

\return
    The existing variable resolution refinements descriptor.
*/
std::shared_ptr<VRRefinementsDescriptor> VRRefinementsDescriptor::open(
    const Dataset& dataset)
{
    return std::shared_ptr<VRRefinementsDescriptor>(
        new VRRefinementsDescriptor{dataset});
}


//! \copydoc LayerDescriptor::getDataType
DataType VRRefinementsDescriptor::getDataTypeProxy() const noexcept
{
    return DT_COMPOUND;
}

//! \copydoc LayerDescriptor::getElementSize
uint8_t VRRefinementsDescriptor::getElementSizeProxy() const noexcept
{
    return sizeof(BagVRRefinementsItem);
}

//! Retrieve the minimum and maximum depth.
/*!
\return
    The minimum and maximum depth.
*/
std::tuple<float, float>
VRRefinementsDescriptor::getMinMaxDepth() const noexcept
{
    return {m_minDepth, m_maxDepth};
}

//! Retrieve the minimum and maximum uncertainty.
/*!
\return
    The minimum and maximum uncertainty.
*/
std::tuple<float, float>
VRRefinementsDescriptor::getMinMaxUncertainty() const noexcept
{
    return {m_minUncertainty, m_maxUncertainty};
}

//! Set the minimum and maximum depth.
/*!
\param minDepth
    The new minimum depth.
\param maxDepth
    The new maximum depth.

\return
    The variable resolution refinements layer descriptor.
*/
VRRefinementsDescriptor& VRRefinementsDescriptor::setMinMaxDepth(
    float minDepth,
    float maxDepth) & noexcept
{
    m_minDepth = minDepth;
    m_maxDepth = maxDepth;
    return *this;
}

//! Set the minimum and maximum uncertainty.
/*!
\param minUncertainty
    The new minimum uncertainty.
\param maxUncertainty
    The new maximum uncertainty.

\return
    The variable resolution refinements layer descriptor.
*/
VRRefinementsDescriptor& VRRefinementsDescriptor::setMinMaxUncertainty(
    float minUncertainty,
    float maxUncertainty) & noexcept
{
    m_minUncertainty = minUncertainty;
    m_maxUncertainty = maxUncertainty;
    return *this;
}

}  // namespace BAG

