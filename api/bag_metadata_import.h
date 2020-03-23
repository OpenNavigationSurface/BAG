#ifndef BAG_METADATAIMPORT_H
#define BAG_METADATAIMPORT_H

#include "bag_types.h"
#include "bag_metadatatypes.h"


BagError bagImportMetadataFromXmlFile(const char* fileName,
    BagMetadata& metadata, bool doValidation);
BagError bagImportMetadataFromXmlBuffer(const char* xmlBuffer, int bufferSize,
    BagMetadata& metadata, bool doValidation);

void bagSetHomeFolder(const char* homeFolder);

#endif  // BAG_METADATAIMPORT_H

