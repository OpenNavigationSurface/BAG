#ifndef BAG_METADATA_H
#define BAG_METADATA_H

#include "bag_config.h"
#include "bag_fordec.h"
#include "bag_metadatatypes.h"

#include <memory>
#include <string>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

namespace H5
{

class DataSet;

}

namespace BAG
{

class BAG_API Metadata final
{
public:
    Metadata() noexcept;
    explicit Metadata(Dataset& dataset);
    Metadata(const Metadata& other) = delete;
    Metadata(Metadata&& other) = default;
    Metadata& operator=(const Metadata&) = delete;
    Metadata& operator=(Metadata&&) = delete;
    ~Metadata() noexcept;

    const BagMetadata& getStruct() const noexcept;

    double columnResolution() const noexcept;
    const char* horizontalCRSasWKT() const;
    double llCornerX() const noexcept;
    double llCornerY() const noexcept;
    double rowResolution() const noexcept;
    double urCornerX() const noexcept;
    double urCornerY() const noexcept;

    void loadFromFile(const std::string& fileName);
    void loadFromBuffer(const std::string& xmlBuffer);

    size_t getXMLlength() const noexcept;

    void write() const;

private:
    //! Custom deleter to avoid needing a definition for ::H5::DataSet::~DataSet().
    struct BAG_API DeleteH5dataSet final
    {
        void operator()(::H5::DataSet* ptr) noexcept;
    };

    void createH5dataSet(const Dataset& inDataSet);

    //! The dataset the metadata is part of.
    std::weak_ptr<const Dataset> m_pBagDataset;
    //! The C structs behind this class.
    std::unique_ptr<BagMetadata> m_pMetaStruct;
    //! The HDF5 DataSet this class wraps.
    std::unique_ptr<::H5::DataSet, DeleteH5dataSet> m_pH5dataSet;
    //! Length of the XML (from file or buffer).
    size_t m_xmlLength = 0;

    friend Dataset;
};

}   //namespace BAG

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif  //BAG_METADATA_H


