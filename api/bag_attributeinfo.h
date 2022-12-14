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

//! This structure contains simple layer attribute information.
struct AttributeInfo
{
    AttributeInfo(const char* inMinName, const char* inMaxName,
        const char* inPath, const ::H5::PredType& inH5type)
        : minName(inMinName)
        , maxName(inMaxName)
        , path(inPath)
        , h5type(inH5type)
    {}

    //! The minimum value attribute name.
    const char* minName = nullptr;
    //! The maximum value attribute name.
    const char* maxName = nullptr;
    //! The HDF5 path to the attribute.
    const char* path = nullptr;
    //! The HDF5 type the attribute is.
    const ::H5::PredType& h5type;
};

//! Retrieve the simple layer attribute information.
AttributeInfo getAttributeInfo(LayerType layerType);

}  // namespace BAG

#endif  //BAG_ATTRIBUTEINFO_H


