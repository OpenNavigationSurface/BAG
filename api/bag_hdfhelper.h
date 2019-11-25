#ifndef BAG_HDFHELPER_H
#define BAG_HDFHELPER_H

#include "bag_types.h"
#include "bag_config.h"

#include <string>


namespace H5 {

class CompType;
class H5File;
class PredType;

}  // namespace H%


namespace BAG {

BAG_API uint64_t getChunkSize(const ::H5::H5File& h5file,
    const std::string& path);

BAG_API unsigned int getCompressionLevel(const ::H5::H5File& h5file,
    const std::string& path);

BAG_API ::H5::CompType getH5compType(LayerType layerType, GroupType groupType);

BAG_API const ::H5::PredType& getH5MemoryType(DataType type);

}  // namespace BAG

#endif  // BAG_HDFHELPER_H

