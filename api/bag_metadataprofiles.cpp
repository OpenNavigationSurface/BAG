
#include "bag_metadataprofiles.h"
#include "bag_compounddatatype.h"

namespace BAG {

    const Record CreateRecord_NOAA_OCS_2022_10(bool significantFeatures,
                                               bool featureLeastDepth,
                                               float featureSize,
                                               float featureSizeVar,
                                               bool coverage,
                                               bool bathyCoverage,
                                               float horizontalUncertFixed,
                                               float horizontalUncertVar,
                                               std::string surveyDateStart,
                                               std::string surveyDateEnd,
                                               std::string sourceInstitution,
                                               std::string sourceSurveyID,
                                               uint32_t sourceSurveyIndex,
                                               std::string licenseName,
                                               std::string licenseURL) {
        return BAG::Record{
            CompoundDataType{significantFeatures},
            CompoundDataType{featureLeastDepth},
            CompoundDataType{featureSize},
            CompoundDataType{featureSizeVar},
            CompoundDataType{coverage},
            CompoundDataType{bathyCoverage},
            CompoundDataType{horizontalUncertFixed},
            CompoundDataType{horizontalUncertVar},
            CompoundDataType{surveyDateStart},
            CompoundDataType{surveyDateEnd},
            CompoundDataType{sourceInstitution},
            CompoundDataType{sourceSurveyID},
            CompoundDataType{sourceSurveyIndex},
            CompoundDataType{licenseName},
            CompoundDataType{licenseURL}
        };
    }

}