%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_layerdescriptor

%include <std_shared_ptr.i>
%shared_ptr(BAG::LayerDescriptor)

%{
#include "bag_layerdescriptor.h"
%}

namespace BAG
{
class Dataset;
}

%import "bag_config.h"
%import "bag_types.i"
%include "bag_layerdescriptor.h"

// TODO might need to modify this to prevent unneeded wrappers
