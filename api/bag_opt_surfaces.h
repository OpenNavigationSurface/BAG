extern bagError bagAlignOptRow (bagHandle bagHandle, u32 row, u32 start_col, 
                      u32 end_col, s32 type, s32 read_or_write, void *data);
extern bagError bagAlignOptRegion (bagHandle bagHandle, u32 start_row, u32 start_col, 
                    u32 end_row, u32 end_col, s32 type, s32 read_or_write, hid_t xfer);
extern bagError bagReadOptRegionPos (bagHandle bag, u32 start_row, u32 start_col, 
                           u32 end_row, u32 end_col, s32 type, f64 **x, f64 **y);
extern bagError bagReadOptDatasetPos (bagHandle bagHandle, s32 type, f64 **x, f64 **y);
extern bagError bagReadOptDataset (bagHandle bagHandle, s32 type);
extern bagError bagGetOptDatasetInfo(bagHandle *bag_handle, s32 type);
extern bagError bagWriteOptDataset (bagHandle bagHandle, s32 type);
extern bagError bagUpdateOptMinMax (bagHandle hnd, u32 type);
extern bagError bagUpdateOptSurface (bagHandle hnd, u32 type);

