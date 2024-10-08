%begin %{
#ifdef _MSC_VER
#ifdef SWIGPYTHON
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
#endif
%}

%module bag_vrtrackinglist

%{
#include "bag_vrtrackinglist.h"
%}

%import "bag_types.i"

%include <std_vector.i>
%include <stdint.i>
%include <std_shared_ptr.i>
%shared_ptr(BAG::VRTrackingList)

%template(VRTrackingItems) std::vector<BAG::VRTrackingList::value_type>;


namespace BAG {

class VRTrackingList final
{
public:
    using value_type = VRTrackingItem;
    using iterator = std::vector<value_type>::iterator;
    using const_iterator = std::vector<value_type>::const_iterator;
    using reference = value_type&;
    using const_reference = const value_type&;

    VRTrackingList(const VRTrackingList&) = delete;
    VRTrackingList(VRTrackingList&&) = delete;

    VRTrackingList& operator=(const VRTrackingList&) = delete;
    VRTrackingList& operator=(VRTrackingList&&) = delete;

    bool operator==(const VRTrackingList &rhs) const noexcept;
    bool operator!=(const VRTrackingList &rhs) const noexcept;

    iterator begin() & noexcept;
    //const_iterator begin() const & noexcept;
    const_iterator cbegin() const & noexcept;
    const_iterator cend() const & noexcept;
    iterator end() & noexcept;
    //const_iterator end() const & noexcept;

    void clear() noexcept;

    %ignore push_back(value_type&& value);
    // Enable the copying version of push_back as newer versions of SWIG (i.e., 4.1+) on some copilers (e.g., clang)
    // appear to change invalidate references to values pushed onto the tracking list, which results in a segfault.
    // The copying version doesn't do this, however, users need to be aware that if the values pushed onto the list
    // change, the original reference will not reflect those changes. This is also consistent with how
    // ``bag_trackinglist.i`` works.
    void push_back(const value_type& value);

    //template <typename... Args>
    //void emplace_back(Args&&... args) &;

    reference front() &;
    //const_reference front() const &;
    reference back() &;
    //const_reference back() const &;

    void reserve(size_t newCapacity);
    void resize(size_t count);

    value_type* data() & noexcept;
    //const value_type* data() const & noexcept;

    bool empty() const noexcept;
    size_t size() const noexcept;

    %rename(at) operator[](size_t index) & noexcept;
    reference operator[](size_t index) & noexcept;
    //%rename(__getitem__) operator[](size_t index) const & noexcept;
    //const_reference operator[](size_t index) const & noexcept;

    void write() const;
};

#if 0
%extend VRTrackingList
{
    const_reference __getitem__(size_t index)const & noexcept
    {
        return (*$self)[index];
    }

    reference __setitem__(size_t index) & noexcept
    {
        return (*$self)[index];
    }
};
#endif

}  // namespace BAG

