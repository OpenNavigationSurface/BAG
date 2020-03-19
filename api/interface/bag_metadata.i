%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_metadata

%{
#include "bag_metadata.h"
%}

#define final

namespace H5
{
class DataSet;
}
namespace BAG
{
class Dataset;
}

%import "bag_config.h"
%include "bag_metadata.h"

// TODO might need to modify this to prevent unneeded wrappers
