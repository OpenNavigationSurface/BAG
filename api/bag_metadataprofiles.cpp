
#include "bag_metadataprofiles.h"
#include "bag_record.h"

namespace BAG {

    const Record CreateRecord_NOAA_NBS_2022_06(uint32_t dataAssessment,
                                               bool significantFeatures,
                                               bool featureLeastDepth,
                                               float featureSize,
                                               bool coverage,
                                               bool bathyCoverage,
                                               float horizontalUncertFixed,
                                               float horizontalUncertVar,
                                               float verticalUncertFixed,
                                               float verticalUncertVar,
                                               std::string licenseName,
                                               std::string licenseURL,
                                               std::string sourceSurveyID,
                                               std::string sourceInstitution,
                                               std::string surveyDataStart,
                                               std::string surveyDateEnd) {
        return BAG::Record{
            BAG::CompoundDataTypeVector{
                    CompoundDataType{dataAssessment},
                    CompoundDataType{significantFeatures},
                    CompoundDataType{featureLeastDepth},
                    CompoundDataType{featureSize},
                    CompoundDataType{coverage},
                    CompoundDataType{bathyCoverage},
                    CompoundDataType{horizontalUncertFixed},
                    CompoundDataType{horizontalUncertVar},
                    CompoundDataType{verticalUncertFixed},
                    CompoundDataType{verticalUncertVar},
                    CompoundDataType{licenseName},
                    CompoundDataType{licenseURL},
                    CompoundDataType{sourceSurveyID},
                    CompoundDataType{sourceInstitution},
                    CompoundDataType{surveyDataStart},
                    CompoundDataType{surveyDateEnd}
            }
        };
    }

}