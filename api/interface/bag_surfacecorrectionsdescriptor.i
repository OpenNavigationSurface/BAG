%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_surfacecorrectionsdescriptor

%include <std_shared_ptr.i>
%shared_ptr(BAG::SurfaceCorrectionsDescriptor)

%{
#include "bag_surfacecorrectionsdescriptor.h"
%}

#define final

%import "bag_layerdescriptor.i"
%include "bag_surfacecorrectionsdescriptor.h"
