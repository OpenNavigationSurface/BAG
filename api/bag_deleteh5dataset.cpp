
#include "bag_deleteh5dataset.h"

#include <H5Cpp.h>


namespace BAG {

//! A specialized deleter to avoid needing definitions of HDF5 classes in header
//! files.
/*!
\param ptr
    A pointer to an HDF5 DataSet.
*/
void DeleteH5dataSet::operator()(::H5::DataSet* ptr) noexcept
{
    delete ptr;
}

}  // namespace BAG

