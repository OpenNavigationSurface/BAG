#!/bin/bash
set -ex # Abort on error.
pushd .
sudo apt-get update -y
sudo apt-get install -y llvm clang lcov cmake ninja-build libhdf5-dev libxml2-dev python3-venv
# Install Catch2 version 3 (Ubuntu 22.04 only packages version 2)
cd /tmp
wget https://github.com/catchorg/Catch2/archive/refs/tags/v3.4.0.tar.gz
echo "122928b814b75717316c71af69bd2b43387643ba076a6ec16e7882bfb2dfacbb  v3.4.0.tar.gz" > catch2.sum
shasum -a 256 -c catch2.sum
tar xf v3.4.0.tar.gz
cd Catch2-3.4.0
cmake -B build -G Ninja -S . -DCMAKE_INSTALL_PREFIX:PATH=/usr -DBUILD_TESTING:BOOL=OFF
sudo cmake --build build --target install
popd
python3 -m venv python-venv --system-site-packages
source python-venv/bin/activate
pip install 'Pillow==9.1.1'
pip install 'genbadge[coverage]==1.0.6'
deactivate
