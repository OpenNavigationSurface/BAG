%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_uint8array

%{
#include "../bag_uint8array.h"
%}

#define final
%include <stdint.i>

namespace BAG
{
    class UInt8Array final
    {
    public:
        UInt8Array() = default;
        explicit UInt8Array(size_t len);

        UInt8Array(const UInt8Array&) = delete;
        UInt8Array(UInt8Array&&) = default;

        UInt8Array& operator=(const UInt8Array&) = delete;
        %ignore operator=(UInt8Array&&);

        %rename(__getitem__) operator[](size_t index) &;
        uint8_t& operator[](size_t index) &;
        %ignore operator[](size_t index) const &;

        explicit operator bool() const noexcept;

        uint8_t* data() const & noexcept;
        %ignore data() const &;
        uint8_t* release() noexcept;
        size_t size() const noexcept;
    };
}

#if 0
%extend BAG::UInt8Array
{
    uint8_t& __getitem__(size_t index)
    {
        return (*($self))[index];
    }
}
#endif
