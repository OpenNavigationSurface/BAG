%begin %{
#ifdef _MSC_VER
#ifdef SWIGPYTHON
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
#endif
%}

%module bag_georefmetadatalayer

%{
#include "bag_georefmetadatalayer.h"
%}

#define final

%import "bag_layer.i"
%import "bag_layeritems.i"
%import "bag_uint8array.i"
%import "bag_valuetable.i"

%include <std_shared_ptr.i>
%shared_ptr(BAG::GeorefMetadataLayer)

namespace BAG {

class GeorefMetadataLayerDescriptor;

class GeorefMetadataLayer final : public Layer
{
public:
    GeorefMetadataLayer(const GeorefMetadataLayer&) = delete;
    GeorefMetadataLayer(GeorefMetadataLayer&&) = delete;
    GeorefMetadataLayer& operator=(const GeorefMetadataLayer&) = delete;
    GeorefMetadataLayer& operator=(GeorefMetadataLayer&&) = delete;

    bool operator==(const GeorefMetadataLayer &rhs) const noexcept;
    bool operator!=(const GeorefMetadataLayer &rhs) const noexcept;

    std::shared_ptr<GeorefMetadataLayerDescriptor> getDescriptor() & noexcept;
    std::shared_ptr<const GeorefMetadataLayerDescriptor> getDescriptor() const & noexcept;

    ValueTable& getValueTable() & noexcept;
    %ignore getValueTable() const& noexcept;

    //UInt8Array readVR(uint32_t indexStart, uint32_t indexEnd) const;
    //void writeVR(uint32_t indexStart, uint32_t indexEnd, const uint8_t* buffer);
};

%extend GeorefMetadataLayer
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
