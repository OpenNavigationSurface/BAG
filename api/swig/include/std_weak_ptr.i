//https://stackoverflow.com/questions/24803149/how-to-use-weak-ptr-in-swig/

%{
#include <memory>
%}

%include <std_shared_ptr.i>

namespace std {

template<class Ty> class weak_ptr {
public:
    typedef Ty element_type;

    weak_ptr();
    weak_ptr(const weak_ptr&);
    template<class Other>
        weak_ptr(const weak_ptr<Other>&);
    template<class Other>
        weak_ptr(const shared_ptr<Other>&);

    weak_ptr(const shared_ptr<Ty>&);

    void swap(weak_ptr&);
    void reset();

    long use_count() const;
    bool expired() const;
    shared_ptr<Ty> lock() const;
};
}
