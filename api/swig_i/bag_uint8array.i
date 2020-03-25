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
%include "stdint.i"

namespace BAG
{
class UInt8Array final
{
public:
    UInt8Array() = default;
    explicit UInt8Array(size_t len);

    UInt8Array(const UInt8Array&) = delete;
    UInt8Array(UInt8Array&&) = delete;
    UInt8Array& operator=(const UInt8Array&) = delete;
    UInt8Array& operator=(UInt8Array&&) = delete;

    uint8_t* get() &;
    %ignore get() const &;
    uint8_t* release();
    %rename(__getitem__) operator[];
    size_t size();
};
}

%extend BAG::UInt8Array
{
    uint8_t& __getitem__(size_t index)
    {
        return (*($self))[index];
    }
}
