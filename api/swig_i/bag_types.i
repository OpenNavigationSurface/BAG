%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_types

%{
#include "../bag_types.h"
%}

%include <stl.i>
%include <std_string.i>
%include <std_unordered_map.i>

namespace std
{
    %template(LayerTypeVector) vector<BAG::LayerType>;
    %template(LayerTypeMap) unordered_map<BAG::LayerType, std::string>;
}

%include "../bag_types.h"

%inline
%{
    const std::string getLayerTypeAsString(BAG::LayerType lt)
    {
        return BAG::kLayerTypeMapString.at(lt);
    }
%}
