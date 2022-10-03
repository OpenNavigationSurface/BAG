%begin %{
#ifdef _MSC_VER
#ifdef SWIGPYTHON
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
#endif
%}

%module bag_compoundlayer

%{
#include "bag_compoundlayer.h"
%}

#define final

%import "bag_layer.i"
%import "bag_layeritems.i"
%import "bag_uint8array.i"
%import "bag_valuetable.i"

%include <std_shared_ptr.i>
%shared_ptr(BAG::CompoundLayer)

namespace BAG {
class CompoundLayer final : public Layer
{
public:
    CompoundLayer(const CompoundLayer&) = delete;
    CompoundLayer(CompoundLayer&&) = delete;
    CompoundLayer& operator=(const CompoundLayer&) = delete;
    CompoundLayer& operator=(CompoundLayer&&) = delete;

    bool operator==(const CompoundLayer &rhs) const noexcept;
    bool operator!=(const CompoundLayer &rhs) const noexcept;

    ValueTable& getValueTable() & noexcept;
    %ignore getValueTable() const& noexcept;

    //UInt8Array readVR(uint32_t indexStart, uint32_t indexEnd) const;
    //void writeVR(uint32_t indexStart, uint32_t indexEnd, const uint8_t* buffer);
};

%extend CompoundLayer
{
    LayerItems readVR(
        uint32_t indexStart,
        uint32_t indexEnd) const
    {
        return BAG::LayerItems{$self->readVR(indexStart, indexEnd)};
    }

    void writeVR(
        uint32_t indexStart,
        uint32_t indexEnd,
        const LayerItems& items)
    {
        $self->writeVR(indexStart, indexEnd, items.data());
    }
}

}
