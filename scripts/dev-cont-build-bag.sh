#!/bin/bash
set -ex # Abort on error.

# Note: This script is meant to be run within the development container defined by ../Dockerfile.dev.

rm -rf venv-docker
python3 -m venv --system-site-packages venv-docker
source venv-docker/bin/activate
pip install -r requirements-dev.txt
pip install mypy

# Create our own lcov config file to make sure branch coverage is always included
printf 'branch_coverage = 1\nno_exception_branch = 1\n' > ~/.lcovrc
# Configure CMake
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -B build -S . \
  -DCMAKE_INSTALL_PREFIX=/usr -DBAG_BUILD_EXAMPLES:BOOL=ON -DBAG_BUILD_TESTS:BOOL=ON -DBAG_CODE_COVERAGE:BOOL=ON
# Build
cmake --build build
pip wheel -w ./wheel/ ./build/api/swig/python
# Install
cmake --install build
pip install --force-reinstall ./wheel/bagPy-*.whl
# Generate PEP484 stub file
stubgen -m bagPy -o ./python
# Run tests
export BAG_HOME=/tmp/bag/configdata
export BAG_SAMPLES_PATH=/tmp/bag/examples/sample-data
# To run tests without coverage, run: ./build/tests/bag_tests_d
ninja -C build ccov-bag_tests
pytest python/test_*.py
