/*!
\file bag_attributeinfo.h
\brief internal attribute info struct and method.
*/
#ifndef BAG_ATTRIBUTEINFO_H
#define BAG_ATTRIBUTEINFO_H

#include "bag_types.h"


namespace H5 {

class PredType;

}  // namespace H5

namespace BAG {

struct AttributeInfo
{
    AttributeInfo(const char* inMinName, const char* inMaxName,
        const char* inPath, const ::H5::PredType& inH5type)
        : minName(inMinName)
        , maxName(inMaxName)
        , path(inPath)
        , h5type(inH5type)
    {}

    const char* minName = nullptr;
    const char* maxName = nullptr;
    const char* path = nullptr;
    const ::H5::PredType& h5type;
};

AttributeInfo getAttributeInfo(LayerType layerType);

}  // namespace BAG

#endif  //BAG_ATTRIBUTEINFO_H

