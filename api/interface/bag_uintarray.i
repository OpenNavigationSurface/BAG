%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_uintarray

%{
#include "bag_uintarray.h"
%}

%import "bag_config.h"

%include "stdint.i"
%include "bag_uintarray.h"
