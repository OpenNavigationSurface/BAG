%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_record

%{
#include "bag_record.h"
%}

%include "bag_compounddatatype.i"

%include "bag_record.h"
