%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_interleavedlayer

%{
#include "bag_interleavedlayer.h"
%}

#define final
%import "bag_config.h"
%import "bag_layer.i"
%include "bag_interleavedlayer.h"

