#ifndef BAG_METADATAEXMPORT_H
#define BAG_METADATAEXMPORT_H

#include "bag_config.h"
#include "bag_metadatatypes.h"

namespace BAG {

BAG_API std::string exportMetadataToXML(const BagMetadata& metadata);

}  // BAG

#endif  //BAG_METADATAEXPORT_H

