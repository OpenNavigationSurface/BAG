#ifndef BAG_PRIVATE_H
#define BAG_PRIVATE_H

#include "bag_dataset.h"

#include <memory>


struct BagHandle {
    std::shared_ptr<BAG::Dataset> pDataset;
};

namespace BAG
{

constexpr auto RANK = 2;
constexpr auto TRACKING_LIST_BLOCK_SIZE = 10;

/*! Path names for BAG entities */
#define ROOT_PATH                       "/BAG_root"
#define METADATA_PATH                   ROOT_PATH "/metadata"
#define ELEVATION_PATH                  ROOT_PATH "/elevation"
#define UNCERTAINTY_PATH                ROOT_PATH "/uncertainty"
#define TRACKING_LIST_PATH              ROOT_PATH "/tracking_list"

/*! Path names for optional BAG entities */
#define NOMINAL_ELEVATION_PATH          ROOT_PATH "/nominal_elevation"
#define VERT_DATUM_CORR_PATH            ROOT_PATH "/vertical_datum_corrections"
#define HYPOTHESIS_STRENGTH_PATH        ROOT_PATH "/hypotheses_strength"  //TODO Discuss this.
#define NUM_HYPOTHESES_PATH             ROOT_PATH "/num_hypotheses"
#define AVERAGE_PATH			        ROOT_PATH "/average"
#define NODE_GROUP_PATH			        ROOT_PATH "/node"
#define ELEVATION_SOLUTION_GROUP_PATH   ROOT_PATH "/elevation_solution"
#define SHOAL_ELEVATION_PATH            ROOT_PATH "/shoal_elevation"  //TODO Discuss this.
#define STANDARD_DEV_PATH	            ROOT_PATH "/standard_dev"
#define NUM_SOUNDINGS_PATH	            ROOT_PATH "/num_soundings"  //TODO Disucuss this.

/*! Names for BAG Attributes */
#define BAG_VERSION_NAME                "Bag Version"                /*!< Name for version attribute, value set in bag.h */

#define	MIN_ELEVATION_NAME              "Minimum Elevation Value"    /*!< Name for min elevation attribute, value stored in bagData */
#define	MAX_ELEVATION_NAME              "Maximum Elevation Value"    /*!< Name for max elevation attribute, value stored in bagData */
#define	MIN_UNCERTAINTY_NAME            "Minimum Uncertainty Value"  /*!< Name for min uncertainty attribute, value stored in bagData */
#define	MAX_UNCERTAINTY_NAME            "Maximum Uncertainty Value"  /*!< Name for max uncertainty attribute, value stored in bagData */
#define MIN_STANDARD_DEV_NAME           "min_stddev"                 /*!< Name for min standard deviation attribute value */
#define MAX_STANDARD_DEV_NAME           "max_stddev"                 /*!< Name for max standard deviation attribute value */
#define MIN_NUM_SOUNDINGS               "min_num_soundings"          /*!< Name for min number of soundings attribute value */
#define MAX_NUM_SOUNDINGS               "max_num_soundings"          /*!< Name for max number of soundings attribute value */
#define MIN_SHOAL_ELEVATION             "min_shoal_elevation"        /*!< Name for min shoal elevation attribute value */
#define MAX_SHOAL_ELEVATION             "max_shoal_elevation"        /*!< Name for max shoal elevation attribute value */
#define MIN_HYPOTHESIS_STRENGTH         "min_hyp_strength"           /*!< Name for min hypothesis strength attribute value */
#define MAX_HYPOTHESIS_STRENGTH         "max_hyp_strength"           /*!< Name for max hypothesis strength attribute value */
#define MIN_NUM_HYPOTHESES              "min_num_hypotheses"         /*!< Name for min num hypotheses attribute value */
#define MAX_NUM_HYPOTHESES              "max_num_hypotheses"         /*!< Name for max num hypotheses attribute value */
#define MIN_NOMINAL_ELEVATION           "min_value"                  /*!< Name for min nominal elevation attribute value*/
#define MAX_NOMINAL_ELEVATION           "max_value"                  /*!< Name for max nominal elevation attribute value*/
#define MIN_AVERAGE                     "min_value"                  /*!< Name for min average attribute value */
#define MAX_AVERAGE                     "max_value"                  /*!< Name for max average attribute value */

#define TRACKING_LIST_LENGTH_NAME       "Tracking List Length"       /*!< Name for the tracking list length attribute */

#define VERT_DATUM_CORR_NSX             "Node Spacing X"             /*!<Name for the node spacing X attribute for vert datum set */
#define VERT_DATUM_CORR_NSY             "Node Spacing Y"             /*!<Name for the node spacing Y attribute for vert datum set */
#define VERT_DATUM_CORR_SWX             "SW Corner X"                /*!<Name for the sw corner X attribute for vert datum set */
#define VERT_DATUM_CORR_SWY             "SW Corner Y"                /*!<Name for the sw corner Y attribute for vert datum set */
#define VERT_DATUM_CORR_SURFACE_TYPE    "surface_type"               /*!<Name for the surface type */
#define VERT_DATUM_CORR_VERTICAL_DATUM  "vertical_datum"             /*!<Name for the vertical datum */

}   //namespace BAG

#endif  //BAG_PRIVATE_H

