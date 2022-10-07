#ifndef BAG_COMPOUNDLAYER
#define BAG_COMPOUNDLAYER

#include "bag_compounddatatype.h"
#include "bag_compoundlayerdescriptor.h"
#include "bag_config.h"
#include "bag_deleteh5dataset.h"
#include "bag_fordec.h"
#include "bag_layer.h"
#include "bag_types.h"
#include "bag_valuetable.h"

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

//! The interface for a compound layer (spatial metadata).
class BAG_API CompoundLayer final : public Layer
{
public:
    CompoundLayer(Dataset& dataset, CompoundLayerDescriptor& descriptor,
                  std::unique_ptr<::H5::DataSet, DeleteH5dataSet> h5keyDataSet,
                  std::unique_ptr<::H5::DataSet, DeleteH5dataSet> h5vrKeyDataSet,
                  std::unique_ptr<::H5::DataSet, DeleteH5dataSet> h5recordDataSet);

    CompoundLayer(const CompoundLayer&) = delete;
    CompoundLayer(CompoundLayer&&) = delete;

    CompoundLayer& operator=(const CompoundLayer&) = delete;
    CompoundLayer& operator=(CompoundLayer&&) = delete;

    bool operator==(const CompoundLayer &rhs) const noexcept {
        return m_pH5keyDataSet == rhs.m_pH5keyDataSet &&
               m_pH5vrKeyDataSet == rhs.m_pH5vrKeyDataSet &&
               m_pH5valueDataSet == rhs.m_pH5valueDataSet &&
               m_pValueTable == rhs.m_pValueTable;
    }

    bool operator!=(const CompoundLayer &rhs) const noexcept {
        return !(rhs == *this);
    }

    std::shared_ptr<CompoundLayerDescriptor> getDescriptor() & noexcept;
    std::shared_ptr<const CompoundLayerDescriptor> getDescriptor() const & noexcept;

    ValueTable& getValueTable() & noexcept;
    const ValueTable& getValueTable() const & noexcept;

    UInt8Array readVR(uint32_t indexStart, uint32_t indexEnd) const;
    void writeVR(uint32_t indexStart, uint32_t indexEnd, const uint8_t* buffer);

protected:
    static std::shared_ptr<CompoundLayer> create(DataType keyType,
                                                 const std::string& name, GeorefMetadataProfile profile, Dataset& dataset,
                                                 const RecordDefinition& definition, uint64_t chunkSize,
                                                 int compressionLevel);
    static std::shared_ptr<CompoundLayer> open(Dataset& dataset,
                                               CompoundLayerDescriptor& descriptor);

private:
    static std::unique_ptr<::H5::DataSet, DeleteH5dataSet>
        createH5keyDataSet(const Dataset& inDataSet,
            const CompoundLayerDescriptor& descriptor);

    static std::unique_ptr<::H5::DataSet, DeleteH5dataSet>
        createH5vrKeyDataSet(const Dataset& inDataSet,
            const CompoundLayerDescriptor& descriptor);

    static std::unique_ptr<::H5::DataSet, DeleteH5dataSet>
        createH5valueDataSet(const Dataset& inDataSet,
            const CompoundLayerDescriptor& descriptor);

    const ::H5::DataSet& getValueDataSet() const &;

    void setValueTable(std::unique_ptr<ValueTable> table) noexcept;

    UInt8Array readProxy(uint32_t rowStart, uint32_t columnStart,
        uint32_t rowEnd, uint32_t columnEnd) const override;

    void writeProxy(uint32_t rowStart, uint32_t columnStart, uint32_t rowEnd,
        uint32_t columnEnd, const uint8_t* buffer) override;

    void writeAttributesProxy() const override;

    //! The HDF5 DataSet containing the single resolution keys.
    std::unique_ptr<::H5::DataSet, DeleteH5dataSet> m_pH5keyDataSet;
    //! The HDF5 DataSet containing the variable resolution keys.
    std::unique_ptr<::H5::DataSet, DeleteH5dataSet> m_pH5vrKeyDataSet;
    //! The HDF5 DataSet containing the values of the spatial metadata.
    std::unique_ptr<::H5::DataSet, DeleteH5dataSet> m_pH5valueDataSet;
    //! The records in memory.
    std::unique_ptr<ValueTable> m_pValueTable;

    friend Dataset;
    friend ValueTable;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

}  // namespace BAG

#endif  // BAG_COMPOUNDLAYER

