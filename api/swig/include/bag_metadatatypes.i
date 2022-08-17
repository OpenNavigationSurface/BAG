%begin %{
#ifdef _MSC_VER
#ifdef SWIGPYTHON
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
#endif
%}

%module bag_metadatatypes

%{
#include "bag_metadatatypes.h"
%}

#define BAG_EXTERNAL

%include "bag_metadatatypes.h"

