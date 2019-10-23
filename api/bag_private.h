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
//TODO ignore?
//#define NUM_HYPOTHESES_PATH             ROOT_PATH "/num_hypotheses"
#define AVERAGE_PATH			        ROOT_PATH "/average"
#define NODE_GROUP_PATH			        ROOT_PATH "/node"
#define ELEVATION_SOLUTION_GROUP_PATH   ROOT_PATH "/elevation_solution"
//TODO ignore?
//#define STANDARD_DEV_PATH	              ROOT_PATH "/standard_dev"

/*! Names for BAG Attributes */
#define BAG_VERSION_NAME                "Bag Version"                /*!< Name for version attribute, value set in bag.h */
#define	MIN_ELEVATION_NAME              "Minimum Elevation Value"    /*!< Name for min elevation attribute, value stored in bagData */
#define	MAX_ELEVATION_NAME              "Maximum Elevation Value"    /*!< Name for max elevation attribute, value stored in bagData */
#define	MIN_UNCERTAINTY_NAME            "Minimum Uncertainty Value"  /*!< Name for min uncertainty attribute, value stored in bagData */
#define	MAX_UNCERTAINTY_NAME            "Maximum Uncertainty Value"  /*!< Name for max uncertainty attribute, value stored in bagData */
#define TRACKING_LIST_LENGTH_NAME       "Tracking List Length"       /*!< Name for the tracking list length attribute */
#define VERT_DATUM_CORR_NSX             "Node Spacing X"             /*!<Name for the node spacing X attribute for vert datum set */
#define VERT_DATUM_CORR_NSY             "Node Spacing Y"             /*!<Name for the node spacing Y attribute for vert datum set */
#define VERT_DATUM_CORR_SWX             "SW Corner X"                /*!<Name for the sw corner X attribute for vert datum set */
#define VERT_DATUM_CORR_SWY             "SW Corner Y"                /*!<Name for the sw corner Y attribute for vert datum set */
#define VERT_DATUM_CORR_SURFACE_TYPE    "surface_type"               /*!<Name for the surface type */
#define VERT_DATUM_CORR_VERTICAL_DATUM  "vertical_datum"             /*!<Name for the vertical datum */

}   //namespace BAG

#endif  //BAG_PRIVATE_H

