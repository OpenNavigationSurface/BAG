#ifndef BAG_METADATAIMPORT_H
#define BAG_METADATAIMPORT_H

#include "bag_types.h"
#include "bag_metadatatypes.h"


namespace BAG {

BAG_API BagError bagImportMetadataFromXmlFile(const char* fileName,
    BagMetadata& metadata, bool doValidation);
BAG_API BagError bagImportMetadataFromXmlBuffer(const char* xmlBuffer, int bufferSize,
    BagMetadata& metadata, bool doValidation);

BAG_API void bagSetHomeFolder(const char* homeFolder);
BAG_API std::string bagGetHomeFolder();

}  // namespace BAG

#endif  // BAG_METADATAIMPORT_H

