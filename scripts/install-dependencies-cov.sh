#!/bin/bash
pushd .
sudo apt-get update -y
sudo apt-get install -y llvm clang lcov cmake ninja-build libhdf5-dev libxml2-dev catch2
# Install Catch2 version 3 (Ubuntu 22.04 only packages version 2)
cd /tmp
wget https://github.com/catchorg/Catch2/archive/v3.0.1.tar.gz
echo "8c4173c68ae7da1b5b505194a0c2d6f1b2aef4ec1e3e7463bde451f26bbaf4e7  v3.0.1.tar.gz" > catch2.sum
shasum -a 256 -c catch2.sum
tar xf v3.0.1.tar.gz
cd Catch2-3.0.1
cmake -B build -G Ninja -S . -DCMAKE_INSTALL_PREFIX:PATH=/usr -DBUILD_TESTING:BOOL=OFF
sudo cmake --build build --target install
popd
