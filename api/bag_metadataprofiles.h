//
// Created by Brian C Miles on 5/24/22.
//

#ifndef OPENNS_BAG_METADATAPROFILE_H
#define OPENNS_BAG_METADATAPROFILE_H

#include "bag_compounddatatype.h"

namespace BAG {

    const std::unordered_map<GeorefMetadataProfile, std::string> kGeorefMetadataProfileMapString {
            {UNKNOWN_METADATA_PROFILE,          UNKOWN_METADATA_PROFILE_NAME},
            {NOAA_OCS_2022_10_METADATA_PROFILE, NOAA_OCS_2022_10_METADATA_PROFILE_NAME}
    };

    const std::unordered_map<std::string, GeorefMetadataProfile> kStringMapGeorefMetadataProfile {
            {UNKOWN_METADATA_PROFILE_NAME,           UNKNOWN_METADATA_PROFILE},
            {NOAA_OCS_2022_10_METADATA_PROFILE_NAME, NOAA_OCS_2022_10_METADATA_PROFILE}
    };

    BAG_API const Record CreateRecord_NOAA_OCS_2022_10(bool significantFeatures,
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
                                                       std::string licenseName,
                                                       std::string licenseURL);

    const RecordDefinition METADATA_DEFINITION_UNKNOWN = RecordDefinition();

    const RecordDefinition METADATA_DEFINITION_NOAA_OCS_2022_10 = {
            {"significant_features", DT_BOOLEAN},
            {"feature_least_depth", DT_BOOLEAN},
            {"feature_size", DT_FLOAT32},
            {"feature_size_var", DT_FLOAT32},
            {"coverage", DT_BOOLEAN},
            {"bathy_coverage", DT_BOOLEAN},
            {"horizontal_uncert_fixed", DT_FLOAT32},
            {"horizontal_uncert_var", DT_FLOAT32},
            {"survey_date_start", DT_STRING},
            {"survey_date_end", DT_STRING},
            {"source_institution", DT_STRING},
            {"source_survey_id", DT_STRING},
            {"license_name", DT_STRING},
            {"license_url", DT_STRING}
    };

    const std::unordered_map<GeorefMetadataProfile, RecordDefinition> kGeorefMetadataProfileMapKnownRecordDefinition {
            {NOAA_OCS_2022_10_METADATA_PROFILE, METADATA_DEFINITION_NOAA_OCS_2022_10}
    };

}

#endif //OPENNS_BAG_METADATAPROFILE_H
