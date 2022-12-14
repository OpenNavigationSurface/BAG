%begin %{
#ifdef _MSC_VER
#ifdef SWIGPYTHON
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
#endif
%}

%module bag_metadata

%{
#include "bag_metadata.h"
%}

%import "bag_dataset.i"
%import "bag_types.i"
%include <std_string.i>
%include <stdint.i>

namespace BAG
{

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

    bool operator==(const Metadata &rhs) const noexcept;
    bool operator!=(const Metadata &rhs) const noexcept;

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

