//************************************************************************
//
//      Open Navigation Surface Working Group, 2013
//
//************************************************************************
#ifndef BAG_OPT_SURFACES_H
#define BAG_OPT_SURFACES_H

#include "bag_config.h"

BAG_EXTERNAL bagError bagReadOptDatasetPos (bagHandle bagHandle, s32 type, f64 **x, f64 **y);
BAG_EXTERNAL bagError bagReadOptDataset (bagHandle bagHandle, s32 type);
BAG_EXTERNAL bagError bagGetOptDatasetInfo(bagHandle *bag_handle, s32 type);
BAG_EXTERNAL bagError bagWriteOptDataset (bagHandle bagHandle, s32 type);
BAG_EXTERNAL bagError bagUpdateOptMinMax (bagHandle hnd, u32 type);
BAG_EXTERNAL bagError bagUpdateOptSurface (bagHandle hnd, u32 type);

#endif
