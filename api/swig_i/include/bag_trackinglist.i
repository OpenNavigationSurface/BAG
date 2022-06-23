%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_trackinglist

%{
#include "bag_trackinglist.h"
%}

%import "bag_types.i"

%include <std_vector.i>

%template(TrackingItems) std::vector<BagTrackingItem>;


#define final

namespace BAG
{
    class Dataset;

    class TrackingList final
    {
    public:
        using value_type = TrackingItem;
        using iterator = std::vector<value_type>::iterator;
        using const_iterator = std::vector<value_type>::const_iterator;
        using reference = value_type&;
        using const_reference = const value_type&;

        TrackingList(const TrackingList&) = delete;
        TrackingList(TrackingList&&) = delete;
        TrackingList& operator=(const TrackingList&) = delete;
        TrackingList& operator=(TrackingList&&) = delete;

        iterator begin() & noexcept;
        %ignore begin() const &;
        iterator end() & noexcept;
        %ignore end() const &;
        const_iterator cbegin() const & noexcept;
        const_iterator cend() const & noexcept;

        void clear() noexcept;
        void push_back(const value_type& value);
        %ignore push_back(value_type&& value);
        %ignore emplace_back(Args&&... args) &;
        reference front() &;
        %ignore front() const &;
        reference back() &;
        %ignore back() const &;
        void reserve(size_t newCapacity);
        void resize(size_t count);
        value_type* data() & noexcept;
        %ignore data() const &;

        bool empty() const noexcept;
        size_t size() const noexcept;

        %rename(at) operator[](size_t index) & noexcept;
        reference operator[](size_t index) & noexcept;

        void write() const;
    };
}
