#ifndef BAG_EXCEPTIONS_H
#define BAG_EXCEPTIONS_H

#include "bag_config.h"

#include <exception>


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4275)
#endif


namespace BAG {

// Layer related.
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

//! Attempt to make an unsupported interleaved layer.
struct BAG_API UnsupportedInterleavedLayer final : virtual std::exception
{
    const char* what() const noexcept override
    {
        return "Attempted to specify an unsupported combination of group and "
            "layer type when making an interleaved layer.";
    }
};

}  // namespace BAG

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif  // BAG_EXCEPTIONS_H

