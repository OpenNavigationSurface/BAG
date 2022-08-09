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

%import "bag_compounddatatype.i"

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
     * Overload [] (read) in Python
     */
    BAG::CompoundDataType& __getitem__(size_t pos) {
        return $self->getitem(pos);
    }

    /**
     *  Overload [] (write) in Python
     */
    void __setitem__(size_t pos, BAG::CompoundDataType& value) {
        $self->setitem(pos, value);
    }

    %pythoncode %{
        def with_fields(fields):
            """
                Factory method for creating a Record from fields, which should be of type Sequence[CompoundDataType]
            """
            r = Record(len(fields))
            for i, f in enumerate(fields):
                r[i] = f
            return r
    %}
};

using Records = std::vector<BAG::Record>;

}; // namespace BAG
