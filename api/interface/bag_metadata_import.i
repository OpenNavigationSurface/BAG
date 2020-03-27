%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_metadata_import

%{
#include "bag_metadata_import.h"
%}

%include "bag_metadata_import.h"
