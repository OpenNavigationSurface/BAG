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

//! Invalid descriptor found.
struct BAG_API InvalidDescriptor final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "The descriptor is not a CompoundDataDescriptor.";
    }
};

//! Invalid index type specified.
struct BAG_API InvalidIndexType final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "The type specified for the index type is not valid.";
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

//! Invalid layer id.
struct BAG_API InvalidLayerId final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "Invalid layer id specified.";
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
        return "Cannot read the number of corrections from the surface "
            "corrections dataset.";
    }
};

// Value Table related.
//! The specified field does not exist.
struct BAG_API FieldNotFound final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "The specified field does not exist.";
    }
};

//! Attempt to use an invalid record.
struct BAG_API InvalidRecord final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "Invalid record encountered.  Either an unknown type is present "
            "or it does not match the definition";
    }
};

//! The dimensions of the Records in the CompoundLayer are invalid.
struct BAG_API InvalidCompoundRecordsSize final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "Invalid record encountered.  Either an unknown type is present "
            "or it does not match the definition";
    }
};

//! Attempt to write a record to an invalid index.
struct BAG_API InvalidRecordsIndex final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "Invalid record index specified while writing a record.";
    }
};

//! Attempt to use an invalid record index.
struct BAG_API RecordNotFound final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "Invalid record index specified.  Record index must be greater "
            "than 0, and less than the number of records present.";
    }
};

// VRRefinement related.
struct BAG_API InvalidVRRefinementDimensions final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "The variable resolution refinement layer is not 1 dimensional.";
    }
};

}  // namespace BAG

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif  // BAG_EXCEPTIONS_H

