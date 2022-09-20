# Ubuntu 20.04 C++ development quick start

This is aimed at a developer the needs to write a C++ application that uses the BAG library to read a bag file.

## Build the library

Make sure dependencies are installed.

    sudo apt install libxml2-dev libhdf5-dev

Follow usual cmake procedures, disabling testing by setting BAG_BUILD_TESTS to OFF.

    mkdir build
    cd build
    cmake-gui ../

Modify CMAKE_INSTALL_PREFIX if desired, build, and install.

    make
    make install

## Using the library

The Dataset class represents a bag file. The static open method can be used to open a bag file.

    #include "bag_dataset.h"

    auto bag = BAG::Dataset::open(filename);

Data is organized in layers with equired layers being "Elevation" and "Uncertainty". Obtaining size information of the elevation grid can be done with the Dataset's Metadata object.

    const auto& meta = bag->getMetadata();

TODO: what about BAG::Descriptor? Does it contain the same metadata?


To extract elevation data from a bag file, the layer is obtained with the getLayer method specifiying the layer type.

    auto elevationLayer = bag.getLayer(BAG::LayerType::Elevation, "");

The data as a byte array can now be extracted from the layer. A subset of the data can be requested by specifying appropriate bounds.

    auto data = elevationLayer->read(startRow, startCol, endRow, endCol);

Note: to obtain usable values, the returned data should be cast to floats.

### Variable resolution bags


