#ifndef BAG_INTERLEAVEDLAYER
#define BAG_INTERLEAVEDLAYER

#include "bag_config.h"
#include "bag_dataset.h"
#include "bag_fordec.h"
#include "bag_layer.h"
#include "bag_types.h"

#include <memory>


namespace BAG {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

class BAG_API InterleavedLayer final : public Layer
{

protected:

    InterleavedLayer(Dataset& dataset,
        const InterleavedLayerDescriptor& descriptor,
        std::unique_ptr<::H5::DataSet, Dataset::DeleteH5DataSet> h5dataSet);

    InterleavedLayer(const InterleavedLayer&) = delete;
    InterleavedLayer(InterleavedLayer&&) = delete;
    InterleavedLayer& operator=(const InterleavedLayer&) = delete;
    InterleavedLayer& operator=(InterleavedLayer&&) = delete;

    static std::unique_ptr<InterleavedLayer> create(Dataset& dataset,
        const InterleavedLayerDescriptor& descriptor);

    static std::unique_ptr<InterleavedLayer> open(Dataset& dataset,
        const InterleavedLayerDescriptor& descriptor);

private:

    std::unique_ptr<uint8_t[]> readProxy(uint32_t rowStart,
        uint32_t columnStart, uint32_t rowEnd, uint32_t columnEnd) const override;

    void writeProxy(uint32_t rowStart, uint32_t columnStart, uint32_t rowEnd,
        uint32_t columnEnd, const uint8_t *buffer) const override;

    //! The HDF5 DataSet.
    std::unique_ptr<H5::DataSet, Dataset::DeleteH5DataSet> m_pH5dataSet;

    friend Dataset;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

}   //namespace BAG

#endif  //BAG_INTERLEAVEDLAYER