#!/bin/bash
set -ex # Abort on error.

# Note: This script is meant to be run within the development container defined by ../Dockerfile.dev.

# Configure CMake
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -B build -S . \
  -DCMAKE_INSTALL_PREFIX=/usr -DBAG_BUILD_TESTS:BOOL=ON -DBAG_BUILD_EXAMPLES:BOOL=ON
# Build
cmake --build build
python3 -m pip wheel -w ./wheel/ ./build/api/swig/python
# Install
cmake --install build
python3 -m pip install --break-system-packages --force-reinstall ./wheel/bagPy-*.whl
# Run tests
export BAG_SAMPLES_PATH=/tmp/bag/examples/sample-data
./build/tests/bag_tests_d
python3 -m pytest python/test_*.py
