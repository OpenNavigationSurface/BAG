%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%module bag_metadata

%{
#include "bag_metadata.h"
#include "bag_exceptions.h"
%}

%import "bag_dataset.i"
%import "bag_types.i"
%include <std_string.i>
%include <stdint.i>

namespace BAG
{

#ifdef SWIGPYTHON
%exceptionclass BAG::ErrorLoadingMetadata;

%exception Metadata::loadFromFile {
    try {
        $action
    } catch(BAG::ErrorLoadingMetadata &e) {
        BAG::ErrorLoadingMetadata *ecopy = new BAG::ErrorLoadingMetadata(e);
        PyObject *err = SWIG_NewPointerObj(ecopy, SWIGTYPE_p_BAG__ErrorLoadingMetadata, 1);
        PyErr_SetObject(SWIG_Python_ExceptionType(SWIGTYPE_p_BAG__ErrorLoadingMetadata), err);
        SWIG_fail;
    }
}
#endif

class Metadata final
{
public:
    Metadata() noexcept;
    Metadata(const Metadata&) = delete;
    Metadata(Metadata&& other) = default;
    explicit Metadata(std::shared_ptr<Dataset> pDataset);
    ~Metadata() noexcept;

    Metadata& operator=(const Metadata&) = delete;
    Metadata& operator=(Metadata&&) = delete;

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

