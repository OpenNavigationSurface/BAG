#include <stddef.h>
#include <stdint.h>
#include <unistd.h>

#include "bag_dataset.h"

using BAG::Dataset;


extern int LLVMFuzzerTestOneInput(const uint8_t *buf, size_t len) {

    char filename[256];
    sprintf(filename, "/tmp/libfuzzer.%d", getpid());

    // Save input file to temporary file so that we can read it.
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        return 0;
    }
    fwrite(buf, len, 1, fp);
    fclose(fp);

    auto pDataset = Dataset::open(filename, BAG_OPEN_READONLY);
    if (pDataset != NULL) {
        pDataset->close();
    }

    return 0;
}
