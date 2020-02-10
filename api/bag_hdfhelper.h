#ifndef BAG_HDFHELPER_H
#define BAG_HDFHELPER_H

#include "bag_compoundlayer.h"
#include "bag_config.h"
#include "bag_fordec.h"
#include "bag_types.h"
#include "bag_valuetable.h"

#include <string>


namespace H5 {

class AtomType;
class CompType;
class H5File;

}  // namespace H%


namespace BAG {

BAG_API ::H5::CompType createH5compType(LayerType layerType,
    GroupType groupType);

BAG_API ::H5::CompType createH5fileCompType(const RecordDefinition& definition);

BAG_API ::H5::CompType createH5memoryCompType(const RecordDefinition& definition);

BAG_API uint64_t getChunkSize(const ::H5::H5File& h5file,
    const std::string& path);

BAG_API unsigned int getCompressionLevel(const ::H5::H5File& h5file,
    const std::string& path);

BAG_API size_t getH5compSize(const RecordDefinition& definition);

BAG_API const::H5::AtomType& getH5fileType(DataType type);

BAG_API const::H5::AtomType& getH5memoryType(DataType type);

}  // namespace BAG

#endif  // BAG_HDFHELPER_H

