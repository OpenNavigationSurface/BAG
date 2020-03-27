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
%import "bag_layerdescriptor.i"

namespace BAG {

    class Metadata;

    class BAG_API Descriptor final
    {
    public:
        Descriptor() = default;
        Descriptor(const Metadata& metadata);

        //TODO Temp, make sure only move operations are used until development is done.
        Descriptor(const Descriptor&) = delete;
        Descriptor(Descriptor&&) = delete;
        Descriptor& operator=(const Descriptor&) = delete;
        %ignore operator=(Descriptor&&);

        std::vector<LayerType> getLayerTypes() const;

        bool isReadOnly() const noexcept;
        Descriptor& setReadOnly(bool readOnly) & noexcept;

        const std::string& getVersion() const& noexcept;
        Descriptor& setVersion(std::string inVersion) & noexcept;

        const std::vector<std::shared_ptr<const LayerDescriptor>>&
            getLayerDescriptors() const& noexcept;

        const LayerDescriptor& getLayerDescriptor(LayerType id) const&;
        Descriptor& addLayerDescriptor(const LayerDescriptor& inDescriptor)&;

        const std::string& getHorizontalReferenceSystem() const& noexcept;
        void setHorizontalReferenceSystem(
            const std::string& horizontalReferenceSystem) & noexcept;

        const std::string& getVerticalReferenceSystem() const& noexcept;
        void setVerticalReferenceSystem(
            const std::string& verticalReferenceSystem) & noexcept;

        const std::tuple<uint32_t, uint32_t>& getDims() const& noexcept;
        void setDims(uint32_t rows, uint32_t columns) & noexcept;

        const std::tuple<double, double, double, double>&
            getProjectedCover() const& noexcept;
        void setProjectedCover(double llX, double llY, double urX,
            double urY) & noexcept;

        const std::tuple<double, double>& getOrigin() const& noexcept;
        void setOrigin(double llX, double llY) & noexcept;

        const std::tuple<double, double>& getGridSpacing() const& noexcept;
        void setGridSpacing(double xSpacing, double ySpacing) & noexcept;

        std::vector<uint32_t> getLayerIds() const noexcept;
    };
}
