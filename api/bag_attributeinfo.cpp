
#include "bag_attributeinfo.h"
#include "bag_exceptions.h"
#include "bag_private.h"

#include <H5Cpp.h>


namespace BAG {

//! Retrieve the attribute information about the specified layer type.
/*!
    This function retrieves the attribute information about the specified Simple
    Layer.  If the specified layer is not supported, an UnknownSimpleLayerType
    exception is thrown.

\param layerType
    The type of simple layer.
    Supported types: Elevation, Uncertainty, Hypothesis_Strength, Num_Hypotheses,
        Shoal_Elevation, Std_Dev, Num_Soundings, Average_Elevation, and
        Nominal_Elevation.

\return
    The attribute information about the specified simple layer.
*/
AttributeInfo getAttributeInfo(LayerType layerType)
{
    switch (layerType)
    {
    case Elevation:
        return AttributeInfo(MIN_ELEVATION_NAME, MAX_ELEVATION_NAME,
            ELEVATION_PATH, ::H5::PredType::NATIVE_FLOAT);
    case Uncertainty:
        return AttributeInfo(MIN_UNCERTAINTY_NAME, MAX_UNCERTAINTY_NAME,
            UNCERTAINTY_PATH, ::H5::PredType::NATIVE_FLOAT);
    case Hypothesis_Strength:
        return AttributeInfo(MIN_HYPOTHESIS_STRENGTH, MAX_HYPOTHESIS_STRENGTH,
            HYPOTHESIS_STRENGTH_PATH, ::H5::PredType::NATIVE_FLOAT);
    case Num_Hypotheses:
        return AttributeInfo(MIN_NUM_HYPOTHESES, MAX_NUM_HYPOTHESES,
            NUM_HYPOTHESES_PATH, ::H5::PredType::NATIVE_UINT32);
    case Shoal_Elevation:
        return AttributeInfo(MIN_SHOAL_ELEVATION, MAX_SHOAL_ELEVATION,
            SHOAL_ELEVATION_PATH, ::H5::PredType::NATIVE_FLOAT);
    case Std_Dev:
        return AttributeInfo(MIN_STANDARD_DEV_NAME, MAX_STANDARD_DEV_NAME,
            STANDARD_DEV_PATH, ::H5::PredType::NATIVE_FLOAT);
    case Num_Soundings:
        return AttributeInfo(MIN_NUM_SOUNDINGS, MAX_NUM_SOUNDINGS,
            NUM_SOUNDINGS_PATH, ::H5::PredType::NATIVE_UINT32);
    case Average_Elevation:
        return AttributeInfo(MIN_AVERAGE, MAX_AVERAGE, AVERAGE_PATH,
            ::H5::PredType::NATIVE_FLOAT);
    case Nominal_Elevation:
        return AttributeInfo(MIN_NOMINAL_ELEVATION, MAX_NOMINAL_ELEVATION,
            NOMINAL_ELEVATION_PATH, ::H5::PredType::NATIVE_FLOAT);
    default:
        throw UnsupportedSimpleLayerType{};
    }
}

}  // namespace BAG


