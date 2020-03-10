%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_compounddatatype

%{
#include "bag_compounddatatype.h"
%}
#define final

// note: had to change nested union in .h file as described here: 
// https://stackoverflow.com/questions/19191211/constructor-initialization-of-a-named-union-member

// todo: fix warnings about operator= ignored. 
// see help page about overloaded operators: http://www.swig.org/Doc4.0/SWIGPlus.html#SWIGPlus_nn28

%import "bag_types.i"
%include "bag_compounddatatype.h"

