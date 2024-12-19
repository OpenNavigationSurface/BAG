
#include "bag_dataset.h"
#include "bag_errors.h"
#include "bag_exceptions.h"
#include "bag_layer.h"
#include "bag_metadata.h"
#include "bag_metadata_export.h"
#include "bag_metadata_import.h"
#include "bag_private.h"

#include <fstream>
#include <H5Cpp.h>
#include <iostream>


namespace BAG
{

//! The HDF5 chunk size of the metadata.
constexpr hsize_t kMetadataChunkSize = 1024;

//! The default constructor.
Metadata::Metadata() noexcept
    : m_pMetaStruct(std::make_unique<BagMetadata>())
{
    // Can throw, but only if a new fails.  std::terminate() is fine then.
    bagInitMetadata(*m_pMetaStruct);
}

//! Constructor
/*!
\param pDataset
    A shared pointer to the BAG Dataset the metadata belongs to.
*/
Metadata::Metadata(std::shared_ptr<Dataset> pDataset)
    : Metadata(*pDataset)
{    
}

//! Constructor.
/*!
\param dataset
    The BAG Dataset the metadata belongs to.
*/
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

//! Destructor.
Metadata::~Metadata() noexcept
{
    if (m_pMetaStruct)
    {
        try
        {
            bagFreeMetadata(*m_pMetaStruct);
        }
        catch(...)  // Prevent any exceptions from escaping.
        {}
    }
}


//! Retrieve the column resolution.
/*!
\return
    The column resolution.
*/
double Metadata::columnResolution() const noexcept
{
    return m_pMetaStruct->spatialRepresentationInfo->columnResolution;
}

//! Retrieve the number of columns.
/*!
\return
    The number of columns.
*/
uint32_t Metadata::columns() const noexcept
{
    return m_pMetaStruct->spatialRepresentationInfo->numberOfColumns;
}

//! Create an HDF5 DataSet to store the metadata.
/*!
\param dataset
    The BAG Dataset this layer belongs to.
*/
void Metadata::createH5dataSet(
    const Dataset& dataset)
{
    m_pBagDataset = dataset.shared_from_this();

    const auto& h5file = dataset.getH5file();

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

//! Retrieve the C structure this class wraps.
/*!
\return
    The C structure this class wraps.
*/
const BagMetadata& Metadata::getStruct() const & noexcept
{
    return *m_pMetaStruct;
}

//! Retrieve the length of the XML in this metadata.
/*!
\return
    The length of the XML in this metadata.
*/
size_t Metadata::getXMLlength() const noexcept
{
    return m_xmlLength;
}

//! Retrieve a copy of the horizontal reference system as WKT.
/*!
\return
    A copy of the horizontal reference system as WKT.
    The string is empty if it is not WKT.
*/
std::string Metadata::horizontalReferenceSystemAsWKT() const
{
    const auto* type = m_pMetaStruct->horizontalReferenceSystem->type;
    if (!type || type != std::string{"WKT"})
        return {};

    return m_pMetaStruct->horizontalReferenceSystem->definition;
}

//! Retrieve the lower left corner's X value.
/*!
\return
    The lower left corner's X value.
*/
double Metadata::llCornerX() const noexcept
{
    return m_pMetaStruct->spatialRepresentationInfo->llCornerX;
}

//! Retrieve the lower left corner's Y value.
/*!
\return
    The lower left corner's Y value.
*/
double Metadata::llCornerY() const noexcept
{
    return m_pMetaStruct->spatialRepresentationInfo->llCornerY;
}

//! Populate the metadata from the XML in the specified file.
/*!
    Read the XML from the specified file.  An ErrorLoadingMetadata exception is
    thrown if an error occurs.

\param fileName
    The XML file to read into this class.
*/
void Metadata::loadFromFile(const std::string& fileName)
{
    const BagError err = bagImportMetadataFromXmlFile(fileName.c_str(),
        *m_pMetaStruct, false);
    if (err != BAG_SUCCESS)
        throw ErrorLoadingMetadata{err};

    std::ifstream ifs{fileName, std::ios_base::in|std::ios_base::binary};
    ifs.seekg(0, std::ios_base::end);
    m_xmlLength = ifs.tellg();
}

//! Populate the metadata from the XML in the specified buffer.
/*!
    Read the XML from the specified buffer.  An ErrorLoadingMetadata exception
    is thrown if an error occurs.

\param xmlBuffer
    The XML buffer.
*/
void Metadata::loadFromBuffer(const std::string& xmlBuffer)
{
    BagError err;
    // BAG stores metadata XML document in HDF5 as a null-terminated C-style string. In recent
    // versions of libxml2 (ca. 1.12+), the xmlParseMemory() function blows up if there are any
    // characters (including whitespace) between the closing '>' of the closing XML element in
    // and the null termination character. Since we don't know what version of libxml2 people
    // are linking against, lets strip all such characters before trying to load the metadata
    // XML document, which works fine on older versions of libxml2.
    auto pos = xmlBuffer.rfind('>');
    if (pos != std::string::npos) {
        auto xmlBuffer_stripped = xmlBuffer.substr(0, pos+1);
        err = bagImportMetadataFromXmlBuffer(xmlBuffer_stripped.c_str(),
        static_cast<int>(xmlBuffer_stripped.size()), *m_pMetaStruct, false);
    } else {
        // This branch is only needed if the metadata XML doesn't contain an XML element, i.e.
        // it isn't a well-formed XML document.
        err = bagImportMetadataFromXmlBuffer(xmlBuffer.c_str(),
        static_cast<int>(xmlBuffer.size()), *m_pMetaStruct, false);
    }

    if (err != BAG_SUCCESS)
        throw ErrorLoadingMetadata{err};

    m_xmlLength = xmlBuffer.size();
}

//! Retrieve the row resolution.
/*!
\return
    The row resolution.
*/
double Metadata::rowResolution() const noexcept
{
    return m_pMetaStruct->spatialRepresentationInfo->rowResolution;
}

//! Retrieve the number of rows.
/*!
\return
    The number of rows.
*/
uint32_t Metadata::rows() const noexcept
{
    return m_pMetaStruct->spatialRepresentationInfo->numberOfRows;
}

//! Retrieve the upper right corner's X value.
/*!
\return
    The upper right corner's X value.
*/
double Metadata::urCornerX() const noexcept
{
    return m_pMetaStruct->spatialRepresentationInfo->urCornerX;
}

//! Retrieve the upper right corner's Y value.
/*!
\return
    The upper right corner's Y value.
*/
double Metadata::urCornerY() const noexcept
{
    return m_pMetaStruct->spatialRepresentationInfo->urCornerY;
}

//! Retrieve a copy of the vertical reference system as WKT.
/*!
\return
    A copy of the vertical reference system as WKT.
    The string is empty if it is not WKT.
*/
std::string Metadata::verticalReferenceSystemAsWKT() const
{
    const auto* type = m_pMetaStruct->verticalReferenceSystem->type;
    if (!type || type != std::string{"WKT"})
        return {};

    return m_pMetaStruct->verticalReferenceSystem->definition;
}

//! Write the metadata to the HDF5 file.
void Metadata::write() const
{
    const auto buffer = exportMetadataToXML(this->getStruct());

    const hsize_t bufferLen = buffer.size();
    const hsize_t kMaxSize = H5F_UNLIMITED;
    const ::H5::DataSpace h5dataSpace{1, &bufferLen, &kMaxSize};

    m_pH5dataSet->write(buffer, ::H5::PredType::C_S1, h5dataSpace);
}

}   //namespace BAG


