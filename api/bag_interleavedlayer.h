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
public:
    //TODO Temp, make sure only move operations are used until development is done.
    InterleavedLayer(const InterleavedLayer&) = delete;
    InterleavedLayer(InterleavedLayer&&) = delete;
    InterleavedLayer& operator=(const InterleavedLayer&) = delete;
    InterleavedLayer& operator=(InterleavedLayer&&) = delete;

protected:
    //! Custom deleter to avoid needing a definition for ::H5::DataSet::~DataSet().
    struct BAG_API DeleteH5dataSet final
    {
        void operator()(::H5::DataSet* ptr) noexcept;
    };

    InterleavedLayer(Dataset& dataset,
        InterleavedLayerDescriptor& descriptor,
        std::unique_ptr<::H5::DataSet, DeleteH5dataSet> h5dataSet);

    static std::unique_ptr<InterleavedLayer> open(Dataset& dataset,
        InterleavedLayerDescriptor& descriptor);

private:
    std::unique_ptr<uint8_t[]> readProxy(uint32_t rowStart,
        uint32_t columnStart, uint32_t rowEnd, uint32_t columnEnd) const override;

    void writeProxy(uint32_t rowStart, uint32_t columnStart, uint32_t rowEnd,
        uint32_t columnEnd, const uint8_t *buffer) override;

    void writeAttributesProxy() const override;

    //! The HDF5 DataSet.
    std::unique_ptr<H5::DataSet, DeleteH5dataSet> m_pH5dataSet;

    friend Dataset;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

}   //namespace BAG

#endif  //BAG_INTERLEAVEDLAYER