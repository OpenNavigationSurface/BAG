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
class Attribute;
class CompType;
class DataSet;
class H5File;
class PredType;

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

BAG_API const ::H5::AtomType& getH5fileType(DataType type);

BAG_API const ::H5::AtomType& getH5memoryType(DataType type);

::H5::Attribute createAttribute(const ::H5::DataSet& h5dataSet,
    const ::H5::PredType& attributeType, const char* path);

void createAttributes(const ::H5::DataSet& h5dataSet,
    const ::H5::PredType& attributeType, const std::vector<const char*>& paths);

template <typename T>
void writeAttribute(const ::H5::DataSet& h5dataSet,
    const ::H5::PredType& attributeType, T value, const char* path);

}  // namespace BAG

#endif  // BAG_HDFHELPER_H

