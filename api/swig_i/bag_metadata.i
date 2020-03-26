%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_metadata

%{
#include "../bag_metadata.h"
%}

#define final

namespace H5
{
class DataSet;
}

%import "../bag_config.h"
%include <std_string.i>

namespace BAG
{
    class Dataset;

    class BAG_API Metadata final
    {
    public:
        Metadata() noexcept;
        //%ignore Metadata(Dataset&);
        explicit Metadata(Dataset& dataset);

        //TODO Temp, make sure only move operations are used until development is done.
        Metadata(const Metadata& other) = delete;
        Metadata(Metadata&& other) = default;
        Metadata& operator=(const Metadata&) = delete;
        Metadata& operator=(Metadata&&) = delete;
        ~Metadata() noexcept;

        const BagMetadata& getStruct() const & noexcept;

        uint32_t columns() const noexcept;
        double columnResolution() const noexcept;
        std::string horizontalReferenceSystemAsWKT() const;
        double llCornerX() const noexcept;
        double llCornerY() const noexcept;
        double rowResolution() const noexcept;
        uint32_t rows() const noexcept;
        double urCornerX() const noexcept;
        double urCornerY() const noexcept;
        std::string verticalReferenceSystemAsWKT() const;

        void loadFromFile(const std::string& fileName);
        void loadFromBuffer(const std::string& xmlBuffer);

        size_t getXMLlength() const noexcept;
    };
}


//%extend BAG::Metadata
//{
//    BAG::Metadata BAG::Metadata::fromDataset(BAG::Dataset& dataset)
//    {
//        return BAG::Metadata(dataset);
//    }
//}
