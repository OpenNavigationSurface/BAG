#include "bag_dataset.h"
#include "bag_layer.h"
#include "bag_metadata.h"
#include "bag_metadata_import.h"
#include "bag_private.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

#include <h5cpp.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif


namespace BAG
{

Metadata::Metadata() noexcept
{
    // Can throw, but only if a new fails.  std::terminate() is fine then.
    bagInitMetadata(m_metaStruct);
}

Metadata::~Metadata() noexcept
{
    // Prevent any exceptions from escaping.
    try {
        bagFreeMetadata(m_metaStruct);
    }
    catch(...)
    {};
}

Metadata::Metadata(Dataset& dataset)
    : m_pBagDataset(dataset.shared_from_this())
{
    bagInitMetadata(m_metaStruct);

    const auto& h5file = dataset.getH5file();

    m_pH5DataSet = std::unique_ptr<::H5::DataSet, DeleteH5DataSet>(
        new ::H5::DataSet{h5file.openDataSet(METADATA_PATH)}, DeleteH5DataSet{});

    H5std_string buffer;
    const ::H5::StrType stringType{*m_pH5DataSet};
    m_pH5DataSet->read(buffer, stringType);

    this->loadFromBuffer(buffer);
}


double Metadata::columnResolution() const noexcept
{
    return m_metaStruct.spatialRepresentationInfo->columnResolution;
}

const BagMetadata& Metadata::getStruct() const noexcept
{
    return m_metaStruct;
}

const char* Metadata::horizontalCRSasWKT() const
{
    const auto* type = m_metaStruct.horizontalReferenceSystem->type;
    if (!type || std::string{type} != std::string{"WKT"})
        return nullptr;

    return m_metaStruct.horizontalReferenceSystem->definition;
}

double Metadata::llCornerX() const noexcept
{
    return m_metaStruct.spatialRepresentationInfo->llCornerX;
}

double Metadata::llCornerY() const noexcept
{
    return m_metaStruct.spatialRepresentationInfo->llCornerY;
}

void Metadata::loadFromFile(const std::string& fileName)
{
    const BagError err = bagImportMetadataFromXmlFile(fileName.c_str(),
        m_metaStruct, false);
    if (err)
        throw err;
}

void Metadata::loadFromBuffer(const std::string& xmlBuffer)
{
    const BagError err = bagImportMetadataFromXmlBuffer(xmlBuffer.c_str(),
        static_cast<int>(xmlBuffer.size()), m_metaStruct, false);
    if (err)
        throw err;
}

double Metadata::rowResolution() const noexcept
{
    return m_metaStruct.spatialRepresentationInfo->rowResolution;
}

double Metadata::urCornerX() const noexcept
{
    return m_metaStruct.spatialRepresentationInfo->urCornerX;
}

double Metadata::urCornerY() const noexcept
{
    return m_metaStruct.spatialRepresentationInfo->urCornerY;
}

void Metadata::DeleteH5DataSet::operator()(::H5::DataSet* ptr) noexcept
{
    delete ptr;
}

}   //namespace BAG


