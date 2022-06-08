%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_metadataprofiles

%{
#include "../bag_metadataprofiles.h"
%}

namespace BAG
{

} // namespace BAG
