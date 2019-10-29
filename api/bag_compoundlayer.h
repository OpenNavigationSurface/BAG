#ifndef BAG_COMPOUNDLAYER
#define BAG_COMPOUNDLAYER

#include "bag_config.h"
#include "bag_dataset.h"
#include "bag_layer.h"
#include "bag_types.h"

#include <string>
#include <vector>


namespace BAG {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

class BAG_API CompoundLayer final : public Layer
{
public:

    struct Definition final
    {
        std::string m_name;
        DataType m_type;  //TODO Other types?  strings, double, int, short, char?
    };

    const std::vector<Definition>& getDefinition() const noexcept;

protected:

    CompoundLayer(Dataset& dataset, LayerDescriptor& descriptor);

    static std::unique_ptr<CompoundLayer> open(Dataset& dataset,
        LayerDescriptor& descriptor);

    //TODO implement create()

private:

    std::unique_ptr<uint8_t[]> readProxy(uint32_t rowStart, uint32_t columnStart,
        uint32_t rowEnd, uint32_t columnEnd) const override;

    void writeProxy(uint32_t rowStart, uint32_t columnStart, uint32_t rowEnd,
        uint32_t columnEnd, const uint8_t* buffer) const override;

    //! The HDF5 DataSet.
    std::unique_ptr<H5::DataSet, Dataset::DeleteH5DataSet> m_pH5dataSet;

    std::vector<Definition> m_definitions;
};

}   //namespace BAG

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif  //BAG_COMPOUNDLAYER

