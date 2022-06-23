%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_exceptions

%{
#include "bag_exceptions.h"
%}


#define final
#define BAG_API

%include "std_except.i"
%include "exception.i" // is this needed?
%include "bag_exceptions.h"

