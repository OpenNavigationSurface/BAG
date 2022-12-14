
#include "bag_dataset.h"
#include "bag_simplelayerdescriptor.h"


namespace BAG {

//! Constructor.
/*!
\param id
    The unique layer id.
\param type
    The layer type.
\param chunkSize
    The chunk size the HDF5 DataSet will use.
\param compressionLevel
    The compression level the HDF5 DataSet will use.
*/
SimpleLayerDescriptor::SimpleLayerDescriptor(
    uint32_t id,
    LayerType type,
    uint64_t chunkSize,
    int compressionLevel)
    : LayerDescriptor(id, Layer::getInternalPath(type),
        kLayerTypeMapString.at(type), type, chunkSize, compressionLevel)
    , m_elementSize(Layer::getElementSize(Layer::getDataType(type)))
{
}

//! Constructor.
/*!
\param dataset
    The BAG Dataset this layer will belong to.
\param type
    The layer type.
*/
SimpleLayerDescriptor::SimpleLayerDescriptor(
    const Dataset& dataset,
    LayerType type)
    : LayerDescriptor(dataset, type)
    , m_elementSize(Layer::getElementSize(Layer::getDataType(type)))
{
}

//! Create a new simple layer descriptor.
/*!
\param dataset
    The BAG Dataset this layer will belong to.
\param type
    The layer type.
\param chunkSize
    The chunk size the HDF5 DataSet will use.
\param compressionLevel
    The compression level the HDF5 DataSet will use.

\return
    The new simple layer descriptor.
*/
std::shared_ptr<SimpleLayerDescriptor> SimpleLayerDescriptor::create(
    const Dataset& dataset,
    LayerType type,
    uint64_t chunkSize,
    int compressionLevel)
{
    return std::shared_ptr<SimpleLayerDescriptor>(
        new SimpleLayerDescriptor{dataset.getNextId(), type, chunkSize,
        compressionLevel});
}

//! Open an existing simple layer descriptor.
/*!
\param dataset
    The BAG Dataset this layer will belong to.
\param type
    The layer type.

\return
    The simple layer descriptor.
*/
std::shared_ptr<SimpleLayerDescriptor> SimpleLayerDescriptor::open(
    const Dataset& dataset,
    LayerType type)
{
    return std::shared_ptr<SimpleLayerDescriptor>(
        new SimpleLayerDescriptor{dataset, type});
}


//! \copydoc LayerDescriptor::getDataType
DataType SimpleLayerDescriptor::getDataTypeProxy() const noexcept
{
    return Layer::getDataType(this->getLayerType());
}

//! \copydoc LayerDescriptor::getElementSize
uint8_t SimpleLayerDescriptor::getElementSizeProxy() const noexcept
{
    return m_elementSize;
}

}  // namespace BAG

