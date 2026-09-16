
#include "bag_dataset.h"

namespace BAG {
    //! Convert a BAG::CompoundDataType (C++) into a BagCompoundDataType (C).
    /*!
    \param field
        The BAG::CompoundDataType.

    \return
        The BagCompoundDataType created from \e field.
    */
    BagCompoundDataType getValue(
        const BAG::CompoundDataType& field)
    {
        BagCompoundDataType result{};

        result.type = field.getType();

        switch(result.type)
        {
            case DT_FLOAT32:
                result.data.f = field.asFloat();
                break;
            case DT_UINT32:
                result.data.ui32 = field.asUInt32();
                break;
            case DT_BOOLEAN:
                result.data.b = field.asBool();
                break;
            case DT_STRING:  // Copy the string as it will go out of scope.
            {
                const char* const value = field.asString().c_str();
                const auto fieldLen = strlen(value) + 1;
                result.data.c = new char[fieldLen];
                memcpy(result.data.c, value, fieldLen);
                break;
            }
            default:
                result.type = DT_UNKNOWN_DATA_TYPE;
                break;
        }

        return result;
    }

    //! Convert a BagCompoundDataType (C) into a BAG::CompoundDataType (C++).
    /*!
    \param field
        The BagCompoundDataType.

    \return
        The BAG::CompoundDataType created from \e field.
    */
    CompoundDataType getValue(
        const BagCompoundDataType& field)
    {
        switch (field.type)
        {
            case DT_FLOAT32:
                return CompoundDataType{field.data.f};
            case DT_UINT32:
                return CompoundDataType{field.data.ui32};
            case DT_BOOLEAN:
                return CompoundDataType{field.data.b};
            case DT_STRING:
            {
                const char* value = field.data.c;
                return CompoundDataType{std::string{value}};
            }
            default:
                throw InvalidType{};
        }
    }
}
