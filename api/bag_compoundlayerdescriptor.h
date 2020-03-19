#ifndef BAG_COMPOUNDLAYERDESCRIPTOR_H
#define BAG_COMPOUNDLAYERDESCRIPTOR_H

#include "bag_compoundlayer.h"
#include "bag_config.h"
#include "bag_fordec.h"
#include "bag_layerdescriptor.h"
#include "bag_types.h"

#include <memory>


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)  // std classes do not have DL-interface when exporting
#endif

namespace BAG {

class BAG_API CompoundLayerDescriptor final : public LayerDescriptor
{
public:
    static std::shared_ptr<CompoundLayerDescriptor> create(
        const std::string& name, DataType indexType,
        RecordDefinition definition, uint64_t chunkSize,
        unsigned int compressionLevel, Dataset& dataset);

    static std::shared_ptr<CompoundLayerDescriptor> open(
        const std::string& name, Dataset& dataset);

    //TODO Temp, make sure only move operations are used until development is done.
    CompoundLayerDescriptor(const CompoundLayerDescriptor&) = delete;
    CompoundLayerDescriptor(CompoundLayerDescriptor&&) = delete;
    CompoundLayerDescriptor& operator=(const CompoundLayerDescriptor&) = delete;
    CompoundLayerDescriptor& operator=(CompoundLayerDescriptor&&) = delete;

    std::weak_ptr<Dataset> getDataset() const &;
    const RecordDefinition& getDefinition() const & noexcept;

protected:
    CompoundLayerDescriptor(const std::string& name, DataType indexType,
        RecordDefinition definition, uint64_t chunkSize,
        unsigned int compressionLevel, Dataset& dataset);

private:
    DataType getDataTypeProxy() const noexcept override;
    uint8_t getElementSizeProxy() const noexcept override;

    //! The dataset this layer is from.
    std::weak_ptr<Dataset> m_pBagDataset;
    //! The data type (depends on layer type).
    DataType m_dataType = DT_UNKNOWN_DATA_TYPE;
    //! The size of a single index in the HDF5 file.
    uint8_t m_elementSize = 0;
    //! The record definition.
    RecordDefinition m_definition;
};

}  // namespace BAG

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif  // BAG_COMPOUNDLAYERDESCRIPTOR_H

