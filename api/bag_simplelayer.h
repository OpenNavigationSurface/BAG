#ifndef BAG_SIMPLELAYER_H
#define BAG_SIMPLELAYER_H

#include "bag_config.h"
#include "bag_fordec.h"
#include "bag_layer.h"
#include "bag_types.h"

#include <memory>


namespace BAG {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

class BAG_API SimpleLayer final : public Layer
{
public:

protected:

    SimpleLayer(Dataset& dataset, const LayerDescriptor& descriptor,
        std::unique_ptr<::H5::DataSet, Dataset::DeleteH5DataSet> h5dataSet);

    SimpleLayer(const SimpleLayer&) = delete;
    SimpleLayer(SimpleLayer&&) = delete;
    SimpleLayer& operator=(const SimpleLayer&) = delete;
    SimpleLayer& operator=(SimpleLayer&&) = delete;

    static std::unique_ptr<SimpleLayer> create(Dataset& dataset,
        const LayerDescriptor& descriptor);

    static std::unique_ptr<SimpleLayer> open(Dataset& dataset,
        const LayerDescriptor& descriptor);

private:

    std::unique_ptr<uint8_t[]> readProxy(uint32_t rowStart,
        uint32_t columnStart, uint32_t rowEnd, uint32_t columnEnd) const override;

    void writeProxy(uint32_t rowStart, uint32_t columnStart,
        uint32_t rowEnd, uint32_t columnEnd, const uint8_t* buffer) const override;

    //! The HDF5 DataSet.
    std::unique_ptr<H5::DataSet, Dataset::DeleteH5DataSet> m_pH5dataSet;

    friend Dataset;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

}   //namespace BAG

#endif  //BAG_SIMPLELAYER_H

