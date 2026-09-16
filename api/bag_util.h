#ifndef OPENNAVSURF_BAG_BAG_UTIL_H
#define OPENNAVSURF_BAG_BAG_UTIL_H

#include "bag_compounddatatype.h"
#include "bag_c_types.h"

/* Internal utility functions - expose here so that we can unit-test them.
 * Note: Only expose "harmless" functions here. */
namespace BAG {
    BAG_API BagCompoundDataType getValue(const BAG::CompoundDataType& field);
    BAG_API CompoundDataType getValue(const BagCompoundDataType& field);
}

#endif //OPENNAVSURF_BAG_BAG_UTIL_H
