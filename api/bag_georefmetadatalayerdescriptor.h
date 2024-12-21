#ifndef BAG_GEOREFMETDATALAYERDESCRIPTOR_H
#define BAG_GEOREFMETDATALAYERDESCRIPTOR_H

#include "bag_georefmetadatalayer.h"
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

//! Describe a georeferenced metadata layer.
class BAG_API GeorefMetadataLayerDescriptor final : public LayerDescriptor
{
public:
    static std::shared_ptr<GeorefMetadataLayerDescriptor>
        create(Dataset& dataset,
            const std::string& name, GeorefMetadataProfile profile, DataType keyType,
            RecordDefinition definition, uint32_t rows, uint32_t cols,
            uint64_t chunkSize, int compressionLevel);
    static std::shared_ptr<GeorefMetadataLayerDescriptor>
        open(Dataset& dataset, const std::string& name);

    GeorefMetadataLayerDescriptor(const GeorefMetadataLayerDescriptor&) = delete;
    GeorefMetadataLayerDescriptor(GeorefMetadataLayerDescriptor&&) = delete;

    GeorefMetadataLayerDescriptor& operator=(const GeorefMetadataLayerDescriptor&) = delete;
    GeorefMetadataLayerDescriptor& operator=(GeorefMetadataLayerDescriptor&&) = delete;

    bool operator==(const GeorefMetadataLayerDescriptor &rhs) const noexcept {
        return m_profile == rhs.m_profile &&
               m_keyType == rhs.m_keyType &&
               m_elementSize == rhs.m_elementSize &&
               m_definition == rhs.m_definition &&
               weak_ptr_equals(m_pBagDataset, rhs.m_pBagDataset);
    }

    bool operator!=(const GeorefMetadataLayerDescriptor &rhs) const noexcept {
        return !(rhs == *this);
    }

    std::weak_ptr<Dataset> getDataset() const &;
    const RecordDefinition& getDefinition() const & noexcept;
    GeorefMetadataProfile getProfile();

protected:
    GeorefMetadataLayerDescriptor(Dataset& dataset, const std::string& name, GeorefMetadataProfile profile,
                                  DataType keyType, RecordDefinition definition,
                                  uint32_t rows, uint32_t cols, uint64_t chunkSize,
                                  int compressionLevel);

private:
    DataType getDataTypeProxy() const noexcept override;
    uint8_t getElementSizeProxy() const noexcept override;

    //! The dataset this layer is from.
    std::weak_ptr<Dataset> m_pBagDataset;
    //! The metadata profile that this layer conforms to.
    GeorefMetadataProfile m_profile = UNKNOWN_METADATA_PROFILE;
    //! The key type.
    DataType m_keyType = DT_UNKNOWN_DATA_TYPE;
    //! The size of a single key in the single/variable resolution HDF5 file.
    uint8_t m_elementSize = 0;
    //! The list of fields making up the record/value.
    RecordDefinition m_definition;

    friend GeorefMetadataLayer;
};

}  // namespace BAG

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif  // BAG_GEOREFMETDATALAYERDESCRIPTOR_H

