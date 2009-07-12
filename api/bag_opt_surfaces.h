extern bagError bagAlignOptRow (bagHandle bagHandle, bagHandle_opt bagHandle_opt, u32 row, u32 start_col, 
                      u32 end_col, s32 type, s32 read_or_write, void *data);
extern bagError bagAlignOptRegion (bagHandle bagHandle, bagHandle_opt bagOptHandle, u32 start_row, u32 start_col, 
                    u32 end_row, u32 end_col, s32 type, s32 read_or_write, hid_t xfer);
extern bagError bagReadOptRegionPos (bagHandle bag, bagHandle_opt bag_opt, u32 start_row, u32 start_col, 
                           u32 end_row, u32 end_col, s32 type, f64 **x, f64 **y);
extern bagError bagReadOptDatasetPos (bagHandle bagHandle, bagHandle_opt bagHandle_opt, s32 type, f64 **x, f64 **y);
extern bagError bagReadOptDataset (bagHandle bagHandle,  bagHandle_opt bagHandle_opt, s32 type);
extern bagError bagGetOptDatasetInfo(bagHandle_opt *bag_handle_opt, s32 type);
extern bagError bagWriteOptDataset (bagHandle bagHandle, bagHandle_opt bagHandle_opt, s32 type);
extern bagError bagUpdateOptMinMax (bagHandle hnd, bagHandle_opt hnd_opt, u32 type);
extern bagError bagUpdateOptSurface (bagHandle hnd, bagHandle_opt hnd_opt, u32 type);
