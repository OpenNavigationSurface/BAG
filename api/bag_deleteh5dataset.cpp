#include "bag_deleteh5dataset.h"
#include <H5Cpp.h>

namespace BAG {

void DeleteH5dataSet::operator()(::H5::DataSet* ptr) noexcept
{
    delete ptr;
}

}