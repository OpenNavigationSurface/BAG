#ifndef OPENNAVSURF_BAG_BAG_UTIL_H
#define OPENNAVSURF_BAG_BAG_UTIL_H

#include "bag_compounddatatype.h"
#include "bag_c_types.h"

/* Internal utility functions - expose here so that we can unit-test them.
 * Note: Only expose "harmless" functions here. */
namespace BAG {
    BAG_COMPOUND_DATA_TYPE getValue(const CompoundDataType& field);
    CompoundDataType getValue(BAG_COMPOUND_DATA_TYPE field);
}

#endif //OPENNAVSURF_BAG_BAG_UTIL_H
