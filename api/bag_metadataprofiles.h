//
// Created by Brian C Miles on 5/24/22.
//

#ifndef OPENNS_BAG_METADATAPROFILE_H
#define OPENNS_BAG_METADATAPROFILE_H

#include "bag_record.h"

namespace BAG {

    const std::unordered_map<GeorefMetadataProfile, std::string> kGeorefMetadataProfileMapString {
            {UNKNOWN_METADATA_PROFILE,          UNKOWN_METADATA_PROFILE_NAME},
            {NOAA_NBS_2022_06_METADATA_PROFILE, NOAA_NBS_2022_06_METADATA_PROFILE_NAME}
    };

    const std::unordered_map<std::string, GeorefMetadataProfile> kStringMapGeorefMetadataProfile {
            {UNKOWN_METADATA_PROFILE_NAME,           UNKNOWN_METADATA_PROFILE},
            {NOAA_NBS_2022_06_METADATA_PROFILE_NAME, NOAA_NBS_2022_06_METADATA_PROFILE}
    };

    BAG_API const Record CreateRecord_NOAA_NBS_2022_06(uint32_t dataAssessment,
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
                                                       std::string surveyDateEnd);

    const RecordDefinition METADATA_DEFINITION_UNKNOWN = RecordDefinition();

    const RecordDefinition METADATA_DEFINITION_NOAA_NBS_2022_06 = {
            {"data_assessment", DT_UINT32},
            {"significant_features", DT_BOOLEAN},
            {"feature_least_depth", DT_BOOLEAN},
            {"feature_size", DT_FLOAT32},
            // TODO: Add feature size var from S101
            {"coverage", DT_BOOLEAN},
            {"bathy_coverage", DT_BOOLEAN},
            {"horizontal_uncert_fixed", DT_FLOAT32},
            {"horizontal_uncert_var", DT_FLOAT32},
            {"vertical_uncert_fixed", DT_FLOAT32},
            {"vertical_uncert_var", DT_FLOAT32},
            {"license_Name", DT_STRING},
            {"license_URL", DT_STRING},
            {"source_Survey_ID", DT_STRING},
            {"source_Institution", DT_STRING},
            {"survey_date_start", DT_STRING},
            {"survey_date_end", DT_STRING}
    };

    const std::unordered_map<GeorefMetadataProfile, RecordDefinition> kGeorefMetadataProfileMapKnownRecordDefinition {
            {NOAA_NBS_2022_06_METADATA_PROFILE, METADATA_DEFINITION_NOAA_NBS_2022_06}
    };

}

#endif //OPENNS_BAG_METADATAPROFILE_H
