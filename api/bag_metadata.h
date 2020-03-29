#ifndef BAG_METADATA_H
#define BAG_METADATA_H

#include "bag_config.h"
#include "bag_deleteh5dataset.h"
#include "bag_fordec.h"
#include "bag_metadatatypes.h"

#include <memory>
#include <string>


namespace H5 {

class DataSet;

}  // namespace H5

namespace BAG {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)  // std classes do not have DLL-interface when exporting
#endif

class BAG_API Metadata final
{
public:
    Metadata() noexcept;
    explicit Metadata(Dataset& dataset);

    ~Metadata() noexcept;

    Metadata(const Metadata& other) = delete;
    Metadata(Metadata&& other) = default;

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

private:

    void createH5dataSet(const Dataset& inDataSet);

    void write() const;

    //! The dataset the metadata is part of.
    std::weak_ptr<const Dataset> m_pBagDataset;
    //! The C struct behind this class.
    std::unique_ptr<BagMetadata> m_pMetaStruct;
    //! The HDF5 DataSet this class wraps.
    std::unique_ptr<::H5::DataSet, DeleteH5dataSet> m_pH5dataSet;
    //! Length of the XML (from file or buffer).
    size_t m_xmlLength = 0;

    friend Dataset;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

}  // namespace BAG

#endif  // BAG_METADATA_H


