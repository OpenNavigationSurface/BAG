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
    Metadata(Metadata&& other) = delete;
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

private:
    std::weak_ptr<Dataset> m_pBagDataset;
    BagMetadata m_metaStruct;

    //! Custom deleter to avoid needing a definition for ::H5::DataSet::~DataSet().
    struct BAG_API DeleteH5DataSet final
    {
        void operator()(::H5::DataSet* ptr) noexcept;
    };
    std::unique_ptr<::H5::DataSet, DeleteH5DataSet> m_pH5DataSet;
};

}   //namespace BAG

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif  //BAG_METADATA_H


