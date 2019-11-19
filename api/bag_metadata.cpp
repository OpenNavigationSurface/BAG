#include "bag_dataset.h"
#include "bag_exceptions.h"
#include "bag_layer.h"
#include "bag_metadata.h"
#include "bag_metadata_export.h"
#include "bag_metadata_import.h"
#include "bag_private.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

#include <fstream>
#include <h5cpp.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif


namespace BAG
{

constexpr hsize_t kMetadataChunkSize = 1024;

Metadata::Metadata() noexcept
    : m_pMetaStruct(std::make_unique<BagMetadata>())
{
    // Can throw, but only if a new fails.  std::terminate() is fine then.
    bagInitMetadata(*m_pMetaStruct);
}

Metadata::Metadata(Dataset& dataset)
    : m_pBagDataset(dataset.shared_from_this())
    , m_pMetaStruct(std::make_unique<BagMetadata>())
{
    bagInitMetadata(*m_pMetaStruct);

    const auto& h5file = dataset.getH5file();

    try
    {
        m_pH5dataSet = std::unique_ptr<::H5::DataSet, DeleteH5dataSet>(
            new ::H5::DataSet{h5file.openDataSet(METADATA_PATH)}, DeleteH5dataSet{});
    }
    catch(...)
    {
        throw MetadataNotFound{};
    }

    H5std_string buffer;
    const ::H5::StrType stringType{*m_pH5dataSet};
    m_pH5dataSet->read(buffer, stringType);

    this->loadFromBuffer(buffer);
}

Metadata::~Metadata() noexcept
{
    if (m_pMetaStruct)
    {
        // Prevent any exceptions from escaping.
        try {
            bagFreeMetadata(*m_pMetaStruct);
        }
        catch(...)
        {}
    }
}


double Metadata::columnResolution() const noexcept
{
    return m_pMetaStruct->spatialRepresentationInfo->columnResolution;
}

uint32_t Metadata::columns() const noexcept
{
    return m_pMetaStruct->spatialRepresentationInfo->numberOfColumns;
}

void Metadata::createH5dataSet(
    const Dataset& inDataset)
{
    m_pBagDataset = inDataset.shared_from_this();

    const auto& h5file = inDataset.getH5file();

    const auto buffer = exportMetadataToXML(this->getStruct());
    const hsize_t xmlLength{buffer.size()};
    const hsize_t kUnlimitedSize = H5F_UNLIMITED;
    const ::H5::DataSpace h5dataSpace{1, &xmlLength, &kUnlimitedSize};

    const ::H5::DSetCreatPropList h5createPropList{};
    h5createPropList.setChunk(1, &kMetadataChunkSize);

    m_pH5dataSet = std::unique_ptr<::H5::DataSet, DeleteH5dataSet>(
        new ::H5::DataSet{h5file.createDataSet(METADATA_PATH,
            ::H5::PredType::C_S1, h5dataSpace, h5createPropList)},
            DeleteH5dataSet{});

    m_pH5dataSet->extend(&xmlLength);
}

const BagMetadata& Metadata::getStruct() const noexcept
{
    return *m_pMetaStruct;
}

size_t Metadata::getXMLlength() const noexcept
{
    return m_xmlLength;
}

std::string Metadata::horizontalReferenceSystemAsWKT() const
{
    const auto* type = m_pMetaStruct->horizontalReferenceSystem->type;
    if (!type || type != std::string{"WKT"})
        return {};

    return m_pMetaStruct->horizontalReferenceSystem->definition;
}

double Metadata::llCornerX() const noexcept
{
    return m_pMetaStruct->spatialRepresentationInfo->llCornerX;
}

double Metadata::llCornerY() const noexcept
{
    return m_pMetaStruct->spatialRepresentationInfo->llCornerY;
}

void Metadata::loadFromFile(const std::string& fileName)
{
    const BagError err = bagImportMetadataFromXmlFile(fileName.c_str(),
        *m_pMetaStruct, false);
    if (err)
        throw err;

    std::ifstream ifs{fileName, std::ios_base::in|std::ios_base::binary};
    ifs.seekg(0, std::ios_base::end);
    m_xmlLength = ifs.tellg();
}

void Metadata::loadFromBuffer(const std::string& xmlBuffer)
{
    const BagError err = bagImportMetadataFromXmlBuffer(xmlBuffer.c_str(),
        static_cast<int>(xmlBuffer.size()), *m_pMetaStruct, false);
    if (err)
        throw err;

    m_xmlLength = xmlBuffer.size();
}

double Metadata::rowResolution() const noexcept
{
    return m_pMetaStruct->spatialRepresentationInfo->rowResolution;
}

uint32_t Metadata::rows() const noexcept
{
    return m_pMetaStruct->spatialRepresentationInfo->numberOfRows;
}

double Metadata::urCornerX() const noexcept
{
    return m_pMetaStruct->spatialRepresentationInfo->urCornerX;
}

double Metadata::urCornerY() const noexcept
{
    return m_pMetaStruct->spatialRepresentationInfo->urCornerY;
}

std::string Metadata::verticalReferenceSystemAsWKT() const
{
    const auto* type = m_pMetaStruct->verticalReferenceSystem->type;
    if (!type || type != std::string{"WKT"})
        return {};

    return m_pMetaStruct->horizontalReferenceSystem->definition;
}

void Metadata::write() const
{
    const auto buffer = exportMetadataToXML(this->getStruct());

    const hsize_t bufferLen = buffer.size();
    const hsize_t kMaxSize = H5F_UNLIMITED;
    const ::H5::DataSpace h5dataSpace{1, &bufferLen, &kMaxSize};

    m_pH5dataSet->write(buffer, ::H5::PredType::C_S1, h5dataSpace);
}

void Metadata::DeleteH5dataSet::operator()(::H5::DataSet* ptr) noexcept
{
    delete ptr;
}

}   //namespace BAG


