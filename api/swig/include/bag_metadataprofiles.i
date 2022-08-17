%begin %{
#ifdef _MSC_VER
#ifdef SWIGPYTHON
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
#endif
%}

%module bag_metadataprofiles

%{
#include "bag_metadataprofiles.h"
%}

%include "bag_metadataprofiles.h"

namespace BAG {

}
