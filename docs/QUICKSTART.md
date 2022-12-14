# Ubuntu 20.04 C++ api quick start

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

See the [How-to Guide](https://bag.readthedocs.io/en/stable/howto-guide/index.html) and
[C++ API Reference](https://bag.readthedocs.io/en/stable/cpp-api/index.html) and 
[Python API Reference](https://bag.readthedocs.io/en/stable/python-api/index.html) for 
tips on how to use the BAG library to read and create BAG files.

