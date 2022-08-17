%begin %{
#ifdef _MSC_VER
#ifdef SWIGPYTHON
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
#endif
%}

%module bag_uint8array

%{
#include "bag_uint8array.h"
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
    UInt8Array(UInt8Array&& other) = default;
    ~UInt8Array() = default;

    UInt8Array& operator=(const UInt8Array&) = delete;
    //UInt8Array& operator=(UInt8Array&& rhs) noexcept;

    bool operator==(const UInt8Array &rhs) const noexcept;
    bool operator!=(const UInt8Array &rhs) const noexcept;

    %rename(at) operator[](size_t index) &;
    uint8_t& operator[](size_t index) &;
    //const uint8_t& operator[](size_t index) const &

    explicit operator bool() const noexcept;

    uint8_t* data() const & noexcept;
    //const uint8_t* data() const & noexcept;
    uint8_t* release() noexcept;

    size_t size() const noexcept;
};

}  // namespace BAG

