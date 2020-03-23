%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_simplelayerdescriptor

%{
#include "bag_simplelayerdescriptor.h"
%}

%include <std_shared_ptr.i>
%shared_ptr(BAG::SimpleLayerDescriptor)

#define final
%import "bag_layerdescriptor.i"
%include "bag_simplelayerdescriptor.h"
