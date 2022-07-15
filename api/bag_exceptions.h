#ifndef BAG_EXCEPTIONS_H
#define BAG_EXCEPTIONS_H

#include "bag_config.h"
#include "bag_errors.h"
#include "bag_types.h"

#include <exception>
#include <sstream>


namespace BAG {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4275)
#pragma warning(disable: 4251) // class 'std::basic_string<char,std::char_traits<char>,std::allocator<char>>' needs to have dll-interface to be used by clients of struct 'BAG::UknownMetadataProfile'
#endif

// General
//! Compression was requested, but no chunking was specified.
struct BAG_API CompressionNeedsChunkingSet final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "If compression is desired, a chunk positive chunk size must be set.";
    }
};

// Attribute related.
//! Attribute type not supported (yet)
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

//! Invalid key type specified.
struct BAG_API InvalidKeyType final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "The type specified for the key type is not valid.";
    }
};

//! A name is required to find a unique compound layer.
struct BAG_API NameRequired final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "A name is required to find a unique compound layer.";
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
struct BAG_API UnsupportedSimpleLayerType final : virtual std::exception
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
//! The layer descriptor is not valid.
struct BAG_API InvalidLayerDescriptor final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "The specified layer descriptor is not valid.";
    }
};


// LayerItems related.
//! The type cannot be cast to the specified type.
struct BAG_API InvalidCast final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "The LayerItems cannot be cast to the specified type.";
    }
};


//! Legacy CRS related.
//! Ran into a problem (internal).
struct CoordSysError : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "Conversion Error";
    };
};

//! Can not convert the datum.
struct InvalidDatumError final : virtual CoordSysError {};

//! Can not convert the ellipsoid.
struct InvalidEllipsoidError final : virtual CoordSysError {};


// Metadata related.
//! Attempt to make an unsupported interleaved layer.
struct BAG_API MetadataNotFound final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "The mandatory Metadata dataset was not found.";
    }
};

//! An unknown metadata profile was specified when a known profile was expected.
struct BAG_API UknownMetadataProfile final : virtual std::exception
{
    UknownMetadataProfile(std::string profile)
    {
        m_message = "Metadata profile '" + profile +
                "' is unknown, but a known profile was expected.";
    }

    const char* what() const noexcept override
    {
        return m_message.c_str();
    }

    std::string m_message;
};

//! An unrecognized metadata profile was specified.
struct BAG_API UnrecognizedMetadataProfile final : virtual std::exception
{
    UnrecognizedMetadataProfile(std::string profile)
    {
        m_message = "Metadata profile '" + profile +
                "' is not recognized.";
    }

    const char* what() const noexcept override
    {
        return m_message.c_str();
    }

    std::string m_message;
};

//! An error occurred loading metadata.
struct BAG_API ErrorLoadingMetadata final : virtual std::exception
{
    ErrorLoadingMetadata(BagError bagError) : m_error(bagError)
    {
        m_message = "While importing metadata as XML, an error value " +
                std::to_string(m_error) + " was returned.";
    }

    const char* what() const noexcept override
    {
        return m_message.c_str();
    }

    BagError m_error = BAG_SUCCESS;
    std::string m_message;
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

//! Invalid corrector specified (1-10).
struct BAG_API InvalidCorrector final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "Invalid corrector specified.  The range is 1-10, but a Surface "
            "Corrections layer may contain less.";
    }
};

//! The surface type is not supported for this operation.
struct BAG_API UnsupportedSurfaceType final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "The type of Surface Correction layer (gridded or sparse) is not"
            " support for this operation.";
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

//! Attempt to use an invalid value.
struct BAG_API InvalidValue final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "Invalid value encountered.  Either an unknown type is present "
            "or it does not match the definition";
    }
};

//! The dimensions of the values in the CompoundLayer are invalid.
struct BAG_API InvalidValueSize final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "Invalid dimensions of the values in the spatial data has been "
            "encountered.";
    }
};

//! Layer requires chunking set because it is dynamically sized.
struct BAG_API LayerRequiresChunkingSet final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "This layer requires a chunk size because it is dynamically "
            "sized.";
    }
};

//! The dataset must use variable resolution to read or write variable resolution metadata using the CompoundLayer.
struct BAG_API DatasetRequiresVariableResolution final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "The dataset must use variable resolution to read or write "
            "variable resolution metadata using the CompoundLayer.";
    }
};

//! Attempt to write a value to an invalid key.
struct BAG_API InvalidValueKey final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "Invalid key specified while writing a spatial metadata value.";
    }
};

//! Attempt to use an invalid key.
struct BAG_API ValueNotFound final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "Invalid key specified.  The key must be greater "
            "than 0, and less than the number of existing values.";
    }
};


// VRRefinement related.
//! VR Refinements are the wrong dimensions.
struct BAG_API InvalidVRRefinementDimensions final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "The variable resolution refinement layer is not 1 dimensional.";
    }
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

}  // namespace BAG

#endif  // BAG_EXCEPTIONS_H

