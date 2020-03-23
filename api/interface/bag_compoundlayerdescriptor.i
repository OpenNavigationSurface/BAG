%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_compoundlayerdescriptor

%{
#include "bag_compoundlayerdescriptor.h"
%}

#define final
%import "bag_config.h"
%import "bag_compoundlayer.i"
%import "bag_layerdescriptor.i"

%include <std_shared_ptr.i>
%shared_ptr(BAG::CompoundLayerDescriptor)

%include "bag_compoundlayerdescriptor.h"


