
#include "bag_dataset.h"
#include "bag_hdfhelper.h"
#include "bag_layer.h"
#include "bag_layerdescriptor.h"
#include "bag_private.h"

#include <H5Cpp.h>


namespace BAG {

//! Constructor.
/*!
\parm id
    The unique identifier of the layer.
\parm internalPath
    The HDF5 path of the layer.
\parm name
    The name of the layer.
\parm type
    The type of layer.
\param chunkSize
    The chunk size the HDF5 DataSet will use.
\param compressionLevel
    The compression level the HDF5 DataSet will use.
*/
LayerDescriptor::LayerDescriptor(
    uint32_t id,
    std::string internalPath,
    std::string name,
    LayerType type,
    uint64_t rows, uint64_t cols,
    uint64_t chunkSize,
    int compressionLevel)
    : m_id(id)
    , m_layerType(type)
    , m_internalPath(std::move(internalPath))
    , m_name(std::move(name))
    , m_compressionLevel(compressionLevel)
    , m_chunkSize(chunkSize)
    , m_minMax(std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest())
    , m_dims({rows, cols})
{
}

//! Constructor.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
\parm type
    The type of layer.
\parm internalPath
    The HDF5 path of the layer.
\parm name
    The name of the layer.
*/
LayerDescriptor::LayerDescriptor(
    const Dataset& dataset,
    LayerType type,
    uint64_t rows, uint64_t cols,
    std::string internalPath,
    std::string name)
    : m_id(dataset.getNextId())
    , m_layerType(type)
    , m_minMax(std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest())
    , m_dims({rows, cols})
{
    m_internalPath = internalPath.empty()
        ? Layer::getInternalPath(type)
        : std::move(internalPath);

    m_name = name.empty()
        ? kLayerTypeMapString.at(type)
        : std::move(name);

    const auto& h5file = dataset.getH5file();

    m_compressionLevel = BAG::getCompressionLevel(h5file, m_internalPath);
    m_chunkSize = BAG::getChunkSize(h5file, m_internalPath);
}


//! Retrieve the chunk size.
/*!
\return
    The chunk size.
*/
uint64_t LayerDescriptor::getChunkSize() const noexcept
{
    return m_chunkSize;
}

//! Retrieve the compression level.
/*!
\return
    The compression level of the layer.
*/
int LayerDescriptor::getCompressionLevel() const noexcept
{
    return m_compressionLevel;
}

//! Retrieve the data type.
/*!
\return
    The data type the layer contains.
*/
DataType LayerDescriptor::getDataType() const noexcept
{
    return this->getDataTypeProxy();
}

//! Retrieve the element size.
/*!
\return
    The element size of the layer.
*/
uint8_t LayerDescriptor::getElementSize() const noexcept
{
    return this->getElementSizeProxy();
}

//! Retrieve the unique id.
/*!
\return
    The id of the layer.
*/
uint32_t LayerDescriptor::getId() const noexcept
{
    return m_id;
}

//! Retrieve the HDF5 path.
/*!
\return
    The HDF5 path of the layer.  In the case of a GeorefMetadataLayer, the path to
    the unique group name is returned.  To read a DataSet using this path, add
    '/keys', '/varres_keys', or '/values'.
*/
const std::string& LayerDescriptor::getInternalPath() const & noexcept
{
    return m_internalPath;
}

//! Retrieve the type this layer contains.
/*!
\return
    The type this layer contains.
*/
LayerType LayerDescriptor::getLayerType() const noexcept
{
    return m_layerType;
}

//! Retrieve the minimum and maximum values.
/*!
\return
    The minimum and maximum values this layer contains.
*/
std::tuple<float, float> LayerDescriptor::getMinMax() const noexcept
{
    return m_minMax;
}

//! Retrieve the name.
/*!
\return
    The name of the layer.
*/
const std::string& LayerDescriptor::getName() const & noexcept
{
    return m_name;
}

//! Retrieve the dimensions (shape) of the layer.
//! Return dimensions as uint32_t rather than the underlying uint64_t to maintain compatibility with the rest of the
//! BAG API, which assumes rows and cols are uint32_t.
/*!
\return
    The row and column spacing/resolution of the grid
*/
std::tuple<uint32_t, uint32_t> LayerDescriptor::getDims() const & noexcept
{
    return std::tuple<uint32_t, uint32_t>{
      static_cast<uint32_t>(std::get<0>(m_dims)),
      static_cast<uint32_t>(std::get<1>(m_dims))
    };
}

//! Get the size of a buffer for reading a specified number rows and columns.
/*!
\param rows
    The number of rows that will be allocated for.
\param columns
    The number of columns that will be allocated for.

\return
    A buffer that can hold rows x columns of values of this layer.
*/
size_t LayerDescriptor::getReadBufferSize(
    uint64_t rows,
    uint64_t columns) const noexcept
{
    return rows * columns * this->getElementSize();
}

//! Set the minimum and maximum values that are found in the layer.
/*!
\param min
    The minimum value that is stored in this layer.
\param max
    The maximum value that is stored in this layer.

\return
    The descriptor.  Useful for chaining set calls.
*/
LayerDescriptor& LayerDescriptor::setMinMax(
    float min,
    float max) & noexcept
{
    m_minMax = {min, max};
    return *this;
}

LayerDescriptor& LayerDescriptor::setDims(uint64_t rows, uint64_t cols) & noexcept
{
    m_dims = {rows, cols};
    return *this;
}

//! Set the HDF5 path of the layer.
/*!
\param inPath
    The new HDF5 path of the layer.

\return
    The descriptor.  Useful for chaining set calls.
*/
LayerDescriptor& LayerDescriptor::setInternalPath(std::string inPath) & noexcept
{
    m_internalPath = std::move(inPath);
    return *this;
}

//! Set the name of the layer.
/*!
\param inName
    The new name of the layer.

\return
    The descriptor.  Useful for chaining set calls.
*/
LayerDescriptor& LayerDescriptor::setName(std::string inName) & noexcept
{
    m_name = std::move(inName);
    return *this;
}

}  // namespace BAG

