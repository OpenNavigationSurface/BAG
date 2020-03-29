#ifndef BAG_COMPOUNDLAYER
#define BAG_COMPOUNDLAYER

#include "bag_compounddatatype.h"
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

class BAG_API CompoundLayer final : public Layer
{
public:
    CompoundLayer(const CompoundLayer&) = delete;
    CompoundLayer(CompoundLayer&&) = delete;

    CompoundLayer& operator=(const CompoundLayer&) = delete;
    CompoundLayer& operator=(CompoundLayer&&) = delete;

    ValueTable& getValueTable() & noexcept;
    const ValueTable& getValueTable() const & noexcept;

protected:
    CompoundLayer(Dataset& dataset, CompoundLayerDescriptor& descriptor,
        std::unique_ptr<::H5::DataSet, DeleteH5dataSet> h5indexDataSet,
        std::unique_ptr<::H5::DataSet, DeleteH5dataSet> h5recordDataSet);

    static std::unique_ptr<CompoundLayer> create(DataType indexType,
        const std::string& name, Dataset& dataset,
        const RecordDefinition& definition, uint64_t chunkSize,
        int compressionLevel);
    static std::unique_ptr<CompoundLayer> open(Dataset& dataset,
        CompoundLayerDescriptor& descriptor);

private:
    static std::unique_ptr<::H5::DataSet, DeleteH5dataSet>
        createH5indexDataSet(const Dataset& inDataSet,
            const CompoundLayerDescriptor& descriptor);

    static std::unique_ptr<::H5::DataSet, DeleteH5dataSet>
        createH5recordDataSet(const Dataset& inDataSet,
            const CompoundLayerDescriptor& descriptor);

    const ::H5::DataSet& getRecordDataSet() const &;

    void setValueTable(std::unique_ptr<ValueTable> table) noexcept;

    std::unique_ptr<UInt8Array> readProxy(uint32_t rowStart, uint32_t columnStart,
        uint32_t rowEnd, uint32_t columnEnd) const override;

    void writeProxy(uint32_t rowStart, uint32_t columnStart, uint32_t rowEnd,
        uint32_t columnEnd, const uint8_t* buffer) override;

    void writeAttributesProxy() const override;

    //! The index DataSet in the HDF5 file.
    std::unique_ptr<::H5::DataSet, DeleteH5dataSet> m_pH5indexDataSet;
    //! The record DataSet in the HDF5 file.
    std::unique_ptr<::H5::DataSet, DeleteH5dataSet> m_pH5recordDataSet;  //TODO move to ValueTable?
    //! The value table.
    std::unique_ptr<ValueTable> m_pValueTable;

    friend Dataset;
    friend ValueTable;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

}  // namespace BAG

#endif  // BAG_COMPOUNDLAYER

