%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_record

%{
#include "bag_record.h"
#include "bag_compounddatatype.h"
%}

%include "bag_compounddatatype.i"

%include "typemaps.i"
%include "std_vector.i"


namespace BAG {

class Record {
public:
    using iterator=typename CompoundDataTypeVector::iterator;
    using const_iterator=typename CompoundDataTypeVector::const_iterator;

    // Constructors
    Record();
    Record(size_t numFields);
    // Allow initialization from a CompoundDataTypeVector literal
    Record(const CompoundDataTypeVector& fields);

    // Destructor
    ~Record();

    // Copy constructor
    Record(const Record& other);

    // Iterators (pass through to underlying vector)
    iterator begin();
    iterator end();

    CompoundDataTypeVector& getFields();
};

%extend Record {
    /**
     * Overload len() in Python
     */
    size_t __len__() {
        return $self->size();
    }

    /**
     * Overload [] in Python
     */
    BAG::CompoundDataType& __getitem__(size_t pos) {
        return $self->getitem(pos);
    }
};

using Records = std::vector<BAG::Record>;

}; // namespace BAG
