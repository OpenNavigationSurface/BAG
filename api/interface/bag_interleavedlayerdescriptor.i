%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_interleavedlayerdescriptor

%include <std_shared_ptr.i>
%shared_ptr(BAG::InterleavedLayerDescriptor)

%{
#include "bag_interleavedlayerdescriptor.h"
%}

#define final

%import "bag_layerdescriptor.i"
%include "bag_interleavedlayerdescriptor.h"
