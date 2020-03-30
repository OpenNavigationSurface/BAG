%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_descriptor

%{
#include "../bag_descriptor.h"
%}

#define final

%include <std_string.i>
%include <stdint.i>

%include <std_shared_ptr.i>
%shared_ptr(BAG::Descriptor)
%shared_ptr(BAG::LayerDescriptor)

%include <stl.i>
namespace std 
{
    %template(SharedLayerVector) vector<std::shared_ptr<BAG::LayerDescriptor>>;
    %template(LayerTypeVector) vector<BAG::LayerType>;
}

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
        std::vector<uint32_t> getLayerIds() const noexcept;
        const std::vector<std::shared_ptr<const LayerDescriptor>>&
            getLayerDescriptors() const & noexcept;
        const LayerDescriptor& getLayerDescriptor(LayerType id) const &;
        const std::string& getVersion() const & noexcept;
        const std::string& getHorizontalReferenceSystem() const & noexcept;
        void setHorizontalReferenceSystem(
            const std::string& horizontalReferenceSystem) & noexcept;
        const std::string& getVerticalReferenceSystem() const & noexcept;

#if 0
        //! Intentionally omit exposing of std::tuple method (unsupported by SWIG), 
        //! so it can be exposed with std::pair below.
        const std::tuple<uint32_t, uint32_t>& getDims() const & noexcept;
        const std::tuple<double, double, double, double>&
          getProjectedCover() const & noexcept;
        const std::tuple<double, double>& getOrigin() const & noexcept;
        const std::tuple<double, double>& getGridSpacing() const & noexcept;
#endif

        Descriptor& addLayerDescriptor(const LayerDescriptor& inDescriptor) &;
        void setVerticalReferenceSystem(
            const std::string& verticalReferenceSystem) & noexcept;
        void setDims(uint32_t rows, uint32_t columns) & noexcept;
        void setProjectedCover(double llX, double llY, double urX,
            double urY) & noexcept;
        void setOrigin(double llX, double llY) & noexcept;
        void setGridSpacing(double xSpacing, double ySpacing) & noexcept;
        Descriptor& setReadOnly(bool readOnly) & noexcept;
        Descriptor& setVersion(std::string inVersion) & noexcept;
    };
}


%extend BAG::Descriptor
{
    const std::pair<uint32_t, uint32_t> getDims() const & noexcept
    {
        uint32_t row=0.0, column=0.0;
        std::tie(row, column) = self->getDims();
        return std::pair<uint32_t, uint32_t>(row, column);
    }

    const std::pair<std::pair<double, double>, std::pair<double, double> > 
        getProjectedCover() const & noexcept
    {
        double llX = 0.0, llY = 0.0, urX = 0.0, urY = 0.0;
        std::tie(llX, llY, urX, urY) = self->getProjectedCover();
        auto ll = std::pair<double, double>(llX, llY);
        auto ur = std::pair<double, double>(urX, urY);
        return std::pair<std::pair<double, double>,std::pair<double, double>>(ll, ur);
    }

    const std::pair<double, double> getOrigin() const & noexcept
    {
        double x=0.0, y=0.0;
        std::tie(x, y) = self->getOrigin();
        return std::pair<double, double>(x, y);
    }

    const std::pair<double, double> getGridSpacing() const & noexcept
    {
        double x=0.0, y=0.0;
        std::tie(x, y) = self->getGridSpacing();
        return std::pair<double, double>(x, y);
    }
}
