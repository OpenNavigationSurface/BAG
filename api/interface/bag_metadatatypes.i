%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_metadatatypes

%{
#include "bag_metadatatypes.h"
%}

%include "bag_metadatatypes.h"
