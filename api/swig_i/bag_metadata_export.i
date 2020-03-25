%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_metadata_export

%{
#include "../bag_metadata_export.h"
%}

%import "../bag_config.h"
%include "../bag_metadata_export.h"

