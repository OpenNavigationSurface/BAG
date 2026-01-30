#!/bin/bash
set -ex # Abort on error.

# Note: This script is meant to be run within the development container defined by ../Dockerfile.dev.

rm -rf venv-docker
python3 -m venv venv-docker
source venv-docker/bin/activate
pip install -r requirements.txt
pip install mypy numpy GDAL==3.9.3

# Configure CMake
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -B build -S . \
  -DCMAKE_INSTALL_PREFIX=/usr -DBAG_BUILD_TESTS:BOOL=ON -DBAG_BUILD_EXAMPLES:BOOL=ON
# Build
cmake --build build
pip wheel -w ./wheel/ ./build/api/swig/python
# Install
cmake --install build
pip install --force-reinstall ./wheel/bagPy-*.whl
# Generate PEP484 stub file
stubgen -m bagPy -o ./python
# Run tests
export BAG_SAMPLES_PATH=/tmp/bag/examples/sample-data
./build/tests/bag_tests_d
pytest python/test_*.py
