%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_valuetable

%{
#include "bag_valuetable.h"
%}
#define final

namespace BAG
{
class CompoundLayer;
}


%import "bag_config.h"
%import "bag_compounddatatype.h"
%include "bag_valuetable.h"

