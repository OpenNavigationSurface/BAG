#ifndef BAG_VERTICALDATUMCORRECTIONS_H
#define BAG_VERTICALDATUMCORRECTIONS_H

#include "bag_config.h"
#include "bag_fordec.h"
#include "bag_types.h"

#include <memory>
#include <vector>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif


namespace BAG {

class BAG_API VerticalDatumCorrections final
{
public:
    using value_type = BagVerticalDatumCorrections;
    using iterator = std::vector<value_type>::iterator;
    using const_iterator = std::vector<value_type>::const_iterator;
    using reference = value_type&;
    using const_reference = const value_type&;

    //TODO Think about how to create one of these to write to a new BAG.
    VerticalDatumCorrections() = default;
    explicit VerticalDatumCorrections(Dataset& dataset);
    explicit VerticalDatumCorrections(size_t numItems);
    VerticalDatumCorrections(std::initializer_list<value_type> items);

    iterator begin() &;
    iterator end() &;
    const_iterator cbegin() const & noexcept;
    const_iterator cend() const & noexcept;

    void clear() noexcept;
    void push_back(const value_type& value);
    void push_back(value_type&& value);
    template <typename... Args>
    reference emplace_back(Args&&... args) &;
    reference & front() &;
    const_reference front() const &;
    reference & back() &;
    const_reference back() const &;
    void reserve(size_t newCapacity);
    void resize(size_t count);
    value_type* data() & noexcept;
    const value_type* data() const & noexcept;

    bool empty() const noexcept;
    size_t size() const noexcept;
    reference operator[](size_t index) & noexcept;
    const_reference operator[](size_t index) const & noexcept;

    //TODO
    //void write(??);

    const std::string& getVerticalDatum() const & noexcept;
    uint8_t getSurfaceType() const noexcept;

private:
    //! The associated dataset.
    std::weak_ptr<Dataset> m_pBagDataset;
    //! The corrections.
    std::vector<value_type> m_corrections;
    //! The type of surface this correction applies to.
    uint8_t m_surfaceType = 0;
    //! The name of the vertical datum.
    std::string m_verticalDatum;
    //! The number of stored corrections.
    int m_numStoredCorrections = 0;
};

}   //namespace BAG

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif  //BAG_VERTICALDATUMCORRECTIONS_H

