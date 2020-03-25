%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_compoundlayer

%{
#include "../bag_compoundlayer.h"
%}

#define final

%import "../bag_config.h"
%import "bag_layer.i"
%import "bag_valuetable.i"

namespace BAG {
class BAG_API CompoundLayer final : public Layer
{
public:
    //TODO Temp, make sure only move operations are used until development is done.
    CompoundLayer(const CompoundLayer&) = delete;
    CompoundLayer(CompoundLayer&&) = delete;
    CompoundLayer& operator=(const CompoundLayer&) = delete;
    CompoundLayer& operator=(CompoundLayer&&) = delete;

    ValueTable& getValueTable() & noexcept;
    %ignore getValueTable() const& noexcept;
};
}
