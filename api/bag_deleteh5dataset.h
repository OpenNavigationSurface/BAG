/*!
\file bag_deleteh5dataset.h
\brief Custom deleter to avoid needing a definition for ::H5::DataSet::~DataSet().
*/
#ifndef BAG_DELETEH5DATASET_H
#define BAG_DELETEH5DATASET_H

#include "bag_config.h"


namespace H5 {

class DataSet;

}  // namespace H5

namespace BAG {

//! Custom deleter for use with std::unique_ptr, to keep HDF5 dependencies out
//! of header files.
struct BAG_API DeleteH5dataSet final
{
    void operator()(::H5::DataSet* ptr) noexcept;
};

}  // namespace BAG

#endif  // BAG_DELETEH5DATASET_H

