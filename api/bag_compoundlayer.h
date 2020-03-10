#ifndef BAG_COMPOUNDLAYER
#define BAG_COMPOUNDLAYER

#include "bag_compounddatatype.h"
#include "bag_config.h"
#include "bag_fordec.h"
#include "bag_layer.h"
#include "bag_types.h"
#include "bag_valuetable.h"

#include <memory>
#include <string>
#include <vector>

namespace H5 {

class DataSet;

}  // namespace H5

namespace BAG {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif


class BAG_API CompoundLayer final : public Layer
{
public:
    //TODO Temp, make sure only move operations are used until development is done.
    CompoundLayer(const CompoundLayer&) = delete;
    CompoundLayer(CompoundLayer&&) = delete;
    CompoundLayer& operator=(const CompoundLayer&) = delete;
    CompoundLayer& operator=(CompoundLayer&&) = delete;

    ValueTable& getValueTable() & noexcept;
    const ValueTable& getValueTable() const & noexcept;

protected:
    //! Custom deleter to avoid needing a definition for ::H5::DataSet::~DataSet().
    struct BAG_API DeleteH5dataSet final
    {
        void operator()(::H5::DataSet* ptr) noexcept;
    };

    CompoundLayer(Dataset& dataset, CompoundLayerDescriptor& descriptor,
        std::unique_ptr<::H5::DataSet, DeleteH5dataSet> h5indexDataSet,
        std::unique_ptr<::H5::DataSet, DeleteH5dataSet> h5recordDataSet);

    static std::unique_ptr<CompoundLayer> create(DataType indexType,
        const std::string& name, Dataset& dataset,
        const RecordDefinition& definition, uint64_t chunkSize,
        unsigned int compressionLevel);
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

    std::unique_ptr<uint8_t[]> readProxy(uint32_t rowStart, uint32_t columnStart,
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

}   //namespace BAG

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif  //BAG_COMPOUNDLAYER

