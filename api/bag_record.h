//
// Created by Brian C Miles on 8/8/22.
//

#ifndef OPENNS_BAG_RECORD_H
#define OPENNS_BAG_RECORD_H

#include <vector>

#include "bag_compounddatatype.h"

namespace BAG {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)  // std classes do not have DLL-interface when exporting
#endif


class BAG_API Record {
public:
    using iterator=typename CompoundDataTypeVector::iterator;
    using const_iterator=typename CompoundDataTypeVector::const_iterator;

    // Constructors
    Record() {}
    Record(size_t numFields) {
        m_fields = CompoundDataTypeVector(numFields);
    }
    // Allow initialization from a CompoundDataTypeVector literal
    Record(const CompoundDataTypeVector& fields) : m_fields(fields) {}

    // Destructor
    ~Record() {
        m_fields.clear();
        m_fields.shrink_to_fit();
    }

    // Copy constructor
    Record(const Record& other) : m_fields(other.m_fields) {}

    // Copy assignment
    Record& operator=(const Record& other) {
        if (this != &other) {
            m_fields.clear();
            m_fields = other.m_fields;
        }
        return *this;
    }

    // Move constructor
    Record(Record&& other) {
        m_fields.clear();
        m_fields = other.m_fields;
        // Release resources from other
        other.m_fields.clear();
        other.m_fields.shrink_to_fit();
    }

    // Move assignment
    Record& operator=(Record&& other) {
        if (this != &other) {
            m_fields.clear();
            m_fields.shrink_to_fit();

            m_fields = other.m_fields;

            // Release resources from other
            other.m_fields.clear();
            other.m_fields.shrink_to_fit();
        }
        return *this;
    }

    bool operator==(const Record &rhs) const {
        return m_fields == rhs.m_fields;
    }

    bool operator!=(const Record &rhs) const {
        return !(rhs == *this);
    }

    /**
     * getitem implemented to make overloading __getitem__ in Python easier.
     * @param pos
     * @return
     */
    CompoundDataType& getitem(size_t pos) {
        return m_fields[pos];
    }

    CompoundDataType& operator[](size_t pos) {
        return m_fields[pos];
    }

    const CompoundDataType& operator[](size_t pos) const {
        return m_fields[pos];
    }

    // Iterators (pass through to underlying vector)
    iterator begin() { return m_fields.begin(); }
    iterator end() { return m_fields.end(); }
    const_iterator begin() const { return m_fields.begin(); }
    const_iterator end() const { return m_fields.end(); }

    CompoundDataTypeVector& getFields() {
        return m_fields;
    }

    CompoundDataTypeVector& getFields() const {
        return const_cast<CompoundDataTypeVector &>(m_fields);
    }

    size_t size() const {
        return m_fields.size();
    }

    void resize(size_t count) {
        m_fields.resize(count);
    }

    void reserve(size_t new_cap) {
        m_fields.reserve(new_cap);
    }

    void emplace_back(CompoundDataType &new_field) {
        m_fields.emplace_back(new_field);
    }

private:
    CompoundDataTypeVector m_fields;
};

using Records = std::vector<BAG::Record>;

} // namespace BAG

#endif //OPENNS_BAG_RECORD_H
