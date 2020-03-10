%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_descriptor
%include <std_shared_ptr.i>
%shared_ptr(BAG::Descriptor)

%{
#include "bag_descriptor.h"
%}

#define final
namespace BAG
{
class Metadata;
}

%import "bag_layerdescriptor.i"
%include "bag_descriptor.h"
