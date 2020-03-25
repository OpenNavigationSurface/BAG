%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_c_types

%{
#include "bag_c_types.h"
%}

%include "bag_c_types.h"