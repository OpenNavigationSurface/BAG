# Appendices

## Appendix A: Georeferenced Metadata Profile Definitions

### NOAA Office of Coast Survey Metadata profile, October 2022 (NOAA-OCS-2022.10)

**Table 9.1: Definition of NOAA Office of Coast Survey Metadata profile, October 2022 (NOAA-OCS-2022.10)**

| Column Name               | Column Type | Note                                                                                                                             |
|:--------------------------|:------------|:---------------------------------------------------------------------------------------------------------------------------------|
| significant_features      | Boolean     | See S-101 significant features detected.                                                                                         |
| feature\_least\_depth     | Boolean     | See S-101 least depth of detected feature measured.                                                                              |
| feature_size              | Float       | See S-101 feature size (1)                                                                                                       |
| feature\_size\_var        | Float       | See further discussion (1)                                                                                                       |
| coverage                  | Boolean     | See S-101 full seafloor coverage achieved                                                                                        |
| bathy_coverage            | Boolean     | See further discussion (2)                                                                                                       |
| horizontal\_uncert\_fixed | Float       | See S-101 horizontal position uncertainty fixed                                                                                  |
| horizontal\_uncert\_var   | Float       | See S-101 horizontal position uncertainty variable factor                                                                        |
| survey\_date\_start       | String      | See S-101 Survey date start                                                                                                      |
| survey\_date\_end         | String      | See S-101 Survey date end                                                                                                        |
| source\_institution       | String      | e.g. "NOAA Office of Coast Survey"                                                                                               |
| source\_survey\_id        | String      | e.g. "H99999"                                                                                                                    |
| source\_survey\_index     | UInt32      | A value of 0 indicates the index is uninitialized or unused.                                                                     |
| license\_name             | String      | e.g. "CC0 1.0"                                                                                                                   |
| license\_url              | String      | A URL or DOI (ideally in URL form) referencing the license definition, e.g. "https://creativecommons.org/publicdomain/zero/1.0/" |

#### Further discussion

1. feature_size_var is meant to augment feature_size which corresponds to S-101 size of features detected. As noted in S-101, size of features detected is intended to be described as the smallest size in cubic metres the survey was capable of detecting. Depending on the type of survey this definition might force different depth ranges to have different values. For example, a survey vessel that works at a fixed height off the seafloor could maintain a fixed feature detection size capability over a wide range of depths. A surface vessel working over those same range of depths may have a feature detection capability that varies with depth causing the detection capability to be ambiguous and potentially misrepresented. For this reason feature_size_var is the percentage of depth that a feature of such size could be detected. When both feature_size and feature_size_var are present the greater of the two should be considered valid. The expectation is that feature_size_var will be set to zero if the feature size does not scale with depth. As with feature_size, feature_size_var should be ignored if significant_features is False.

2. When side scan is used to detect features in flat seafloor areas, surveys have coverage that does not contain direct depth measurements. In these cases the nodes with survey coverage but without bathymetry would be set to False. A condition with coverage = True and bathy_coverage = False is a useful indicator for how to work with these nodes within our workflow. If coverage is False, bathy_coverage must also be False.
