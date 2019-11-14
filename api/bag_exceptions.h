#ifndef BAG_EXCEPTIONS_H
#define BAG_EXCEPTIONS_H

#include "bag_config.h"

#include <exception>


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4275)
#endif


namespace BAG {

// Attribute related.
//! Attribute type not supported (yet)  //TODO temp exception
struct BAG_API UnsupportedAttributeType final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "The H5 type of this attribute is not float.  Not supported (yet).";
    }
};

// CompoundDataType related.
//! Layer not found.
struct BAG_API InvalidType final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "The type specified does not match what is stored.";
    }
};

// Dataset related.
//! Layer not found.
struct BAG_API DatasetNotFound final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "There is no dataset to work with.";
    }
};


// Group related.
//! Attempt to use an unknown layer type.
struct BAG_API UnsupportedGroupType final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "An unsupported group type was specified.";
    }
};


// Layer related.
//! Invalid buffer provided for the write.
struct BAG_API InvalidBuffer final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "The specified buffer is NULL.";
    }
};

//! Invalid dimensions specified for the read.
struct BAG_API InvalidReadSize final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "The dimensions specified for reading are not valid.";
    }
};

//! Invalid dimensions specified for the write.
struct BAG_API InvalidWriteSize final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "The dimensions specified for writing are not valid.";
    }
};

//! Layer already exists.
struct BAG_API LayerExists final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "The specified layer already exists.";
    }
};

//! Layer not found.
struct BAG_API LayerNotFound final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "The specified layer was not found.";
    }
};

//! Attempted to modify a read only Dataset.
struct BAG_API ReadOnlyError final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "Attempted to modify a read only Dataset.";
    }
};

//! Attempt to use get the element size of an unsupported layer type.
struct BAG_API UnsupportedElementSize final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "Element size not supported for this type of layer.";
    }
};

//! Attempt to use an unknown layer type.
struct BAG_API UnsupportedLayerType final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "An unsupported layer type was specified.";
    }
};

//! Attempt to use an incorrect layer type.
struct BAG_API UnknownSimpleLayerType final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "Attempted to specify an unsupported combination of group and "
            "layer type when making an interleaved layer.";
    }
};

//! Attempt to make an unsupported interleaved layer.
struct BAG_API UnsupportedInterleavedLayer final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "Attempted to specify an unsupported combination of group and "
            "layer type when making an interleaved layer.";
    }
};


// LayerDescriptor related.
//! Attempt to make an unsupported interleaved layer.
struct BAG_API UnexpectedLayerDescriptorType final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "An unexpected layer descriptor type was encountered.";
    }
};


// Metadata related.
//! Attempt to make an unsupported interleaved layer.
struct BAG_API MetadataNotFound final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "The mandatory Metadata dataset was not found.";
    }
};


// SimpleLayer related.
//! Cannot convert DataType to an HDF5 DataType.
struct BAG_API UnsupportedDataType final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "The specified DataType cannot be converted to an HDF5 DataType.";
    }
};


// SurfaceCorrections related.
//! Too many corrections specified.
struct BAG_API TooManyCorrections final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "Too many corrections specified.  The limit is 10.";
    }
};

//! Attempt to use an unknown surface type.
struct BAG_API UnknownSurfaceType final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "Unknown surface type specified.";
    }
};

//! Unknown reason why number of corrections could not be read from the BAG.
struct BAG_API CannotReadNumCorrections final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "Cannot read the number of corrections from the surface corrections dataset.";
    }
};

}  // namespace BAG

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif  // BAG_EXCEPTIONS_H

