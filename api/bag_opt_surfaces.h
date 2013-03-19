extern bagError bagReadOptDatasetPos (bagHandle bagHandle, s32 type, f64 **x, f64 **y);
extern bagError bagReadOptDataset (bagHandle bagHandle, s32 type);
extern bagError bagGetOptDatasetInfo(bagHandle *bag_handle, s32 type);
extern bagError bagWriteOptDataset (bagHandle bagHandle, s32 type);
extern bagError bagUpdateOptMinMax (bagHandle hnd, u32 type);
extern bagError bagUpdateOptSurface (bagHandle hnd, u32 type);

