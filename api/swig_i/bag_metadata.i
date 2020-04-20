%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_metadata


%{
#include "../bag_dataset.h"
#include "../bag_metadata.h"
%}

%include <std_shared_ptr.i>
%shared_ptr(BAG::Dataset)

//%import "bag_dataset.i"  //TODO Is the fordec below and #include above the solution?
%import "bag_types.i"

%include <std_string.i>
%include <stdint.i>

namespace H5 {

class DataSet;

}  // namespace H5


// define typemap so that returned Metadata objects are converted correctly
%typemap(out, optimal="1") BAG::Metadata %{
    $result = SWIG_NewPointerObj(($1_ltype*)&$1, $&1_descriptor, 0);
%}


namespace BAG
{

class Dataset;

class Metadata final
{
public:
    Metadata() noexcept;
    Metadata(const Metadata&) = delete;
    Metadata(Metadata&& other) = default;
    ~Metadata() noexcept;

    Metadata& operator=(const Metadata&) = delete;
    Metadata& operator=(Metadata&&) = delete;

    Metadata fromDataset(Dataset& dataset);

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

}  // namespace BAG

