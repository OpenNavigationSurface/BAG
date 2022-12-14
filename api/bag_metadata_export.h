#ifndef BAG_METADATAEXPORT_H
#define BAG_METADATAEXPORT_H

#include "bag_config.h"
#include "bag_metadatatypes.h"

#include <string>


namespace BAG {

BAG_API std::string exportMetadataToXML(const BagMetadata& metadata);

}  // BAG

#endif  // BAG_METADATAEXPORT_H

