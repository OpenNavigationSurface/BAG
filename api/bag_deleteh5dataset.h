/*!
\file bag_deleteh5dataset.h
\brief Custom deleter to avoid needing a definition for ::H5::DataSet::~DataSet().
*/
#ifndef BAG_DELETEH5DATASET_H
#define BAG_DELETEH5DATASET_H

#include "bag_config.h"

namespace H5 
{
    class DataSet;
}

namespace BAG 
{
    struct BAG_API DeleteH5dataSet final
    {
        void operator()(::H5::DataSet* ptr) noexcept;
    };
};

#endif  //BAG_DELETEH5DATASET_H

