
#include "bag_exceptions.h"
#include "bag_private.h"
#include "bag_vrmetadatadescriptor.h"


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
VRMetadataDescriptor::VRMetadataDescriptor(
    uint32_t id,
    uint64_t chunkSize,
    int compressionLevel)
    : LayerDescriptor(id, VR_METADATA_PATH,
        kLayerTypeMapString.at(VarRes_Metadata), VarRes_Metadata, chunkSize,
        compressionLevel)
{
}

//! Constructor.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
*/
VRMetadataDescriptor::VRMetadataDescriptor(
    const Dataset& dataset)
    : LayerDescriptor(dataset, VarRes_Metadata, VR_METADATA_PATH)
{
}

//! Create a variable resolution metadata descriptor.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
\param chunkSize
    The chunk size the HDF5 DataSet will use.
\param compressionLevel
    The compression level the HDF5 DataSet will use.

\return
    The new variable resolution metadata descriptor.
*/
std::shared_ptr<VRMetadataDescriptor> VRMetadataDescriptor::create(
    const Dataset& dataset,
    uint64_t chunkSize,
    int compressionLevel)
{
    return std::shared_ptr<VRMetadataDescriptor>(
        new VRMetadataDescriptor{dataset.getNextId(), chunkSize,
            compressionLevel});
}

//! Open an existing variable resolution metadata descriptor.
/*!
\param dataset
    The BAG Dataset this layer belongs to.

\return
    The variable resolution metadata descriptor.
*/
std::shared_ptr<VRMetadataDescriptor> VRMetadataDescriptor::open(
    const Dataset& dataset)
{
    return std::shared_ptr<VRMetadataDescriptor>(
        new VRMetadataDescriptor{dataset});
}


//! \copydoc LayerDescriptor::getDataType
DataType VRMetadataDescriptor::getDataTypeProxy() const noexcept
{
    return DT_COMPOUND;
}

//! \copydoc LayerDescriptor::getElementSize
uint8_t VRMetadataDescriptor::getElementSizeProxy() const noexcept
{
    return sizeof(VRMetadataItem);
}

//! Retrieve the maximum dimensions.
/*!
\return
    The maximum X and Y dimensions.
*/
std::tuple<uint32_t, uint32_t>
VRMetadataDescriptor::getMaxDimensions() const noexcept
{
    return {m_maxDimX, m_maxDimY};
}

//! Retrieve the maximum resolution.
/*!
\return
    The maximum X and Y resolution.
*/
std::tuple<float, float>
VRMetadataDescriptor::getMaxResolution() const noexcept
{
    return {m_maxResX, m_maxResY};
}

//! Retrieve the minimum dimensions.
/*!
\return
    The minimum X and Y dimensions.
*/
std::tuple<uint32_t, uint32_t>
VRMetadataDescriptor::getMinDimensions() const noexcept
{
    return {m_minDimX, m_minDimY};
}

//! Retrieve the minimum resolution.
/*!
\return
    The minimum X and Y resolution.
*/
std::tuple<float, float>
VRMetadataDescriptor::getMinResolution() const noexcept
{
    return {m_minResX, m_minResY};
}

//! Set the maximum dimensions.
/*!
\param maxDimX
    The maximum X dimension.
\param maxDimY
    The maximum Y dimension.

\return
    The variable resolution metadata descriptor.
*/
VRMetadataDescriptor& VRMetadataDescriptor::setMaxDimensions(
    uint32_t maxDimX,
    uint32_t maxDimY) & noexcept
{
    m_maxDimX = maxDimX;
    m_maxDimY = maxDimY;
    return *this;
}

//! Set the maximum resolution.
/*!
\param maxResX
    The maximum X resolution.
\param maxResY
    The maximum Y resolution.

\return
    The variable resolution metadata descriptor.
*/
VRMetadataDescriptor& VRMetadataDescriptor::setMaxResolution(
    float maxResX,
    float maxResY) & noexcept
{
    m_maxResX = maxResX;
    m_maxResY = maxResY;
    return *this;
}

//! Set the minimum dimensions.
/*!
\param minDimX
    The minimum X dimension.
\param minDimY
    The minimum Y dimension.

\return
    The variable resolution metadata descriptor.
*/
VRMetadataDescriptor& VRMetadataDescriptor::setMinDimensions(
    uint32_t minDimX,
    uint32_t minDimY) & noexcept
{
    m_minDimX = minDimX;
    m_minDimY = minDimY;
    return *this;
}

//! Set the minimum resolution.
/*!
\param minResX
    The minimum X resolution.
\param minResY
    The minimum Y resolution.

\return
    The variable resolution metadata descriptor.
*/
VRMetadataDescriptor& VRMetadataDescriptor::setMinResolution(
    float minResX,
    float minResY) & noexcept
{
    m_minResX = minResX;
    m_minResY = minResY;
    return *this;
}

}  // namespace BAG

