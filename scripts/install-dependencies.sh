#!/bin/bash
set -ex # Abort on error.
GITHUB_WORKSPACE=$1
PYTHON_VERSION=$2
echo "GITHUB_WORKSPACE: ${GITHUB_WORKSPACE}"
echo "PYTHON_VERSION: ${PYTHON_VERSION}"
pushd .

sudo apt-get update -y
sudo apt-get install -y cmake g++ ninja-build libxml2-dev swig4.0 zlib1g-dev libproj-dev
# Install Catch2 version 3 (Ubuntu 22.04 only packages version 2)
cd /tmp
wget https://github.com/catchorg/Catch2/archive/refs/tags/v3.4.0.tar.gz
echo "122928b814b75717316c71af69bd2b43387643ba076a6ec16e7882bfb2dfacbb  v3.4.0.tar.gz" > catch2.sum
shasum -a 256 -c catch2.sum
tar xf v3.4.0.tar.gz
cd Catch2-3.4.0
cmake -B build -G Ninja -S . -DCMAKE_INSTALL_PREFIX:PATH=/usr -DBUILD_TESTING:BOOL=OFF
sudo cmake --build build --target install

# Install HDF5
cd /tmp
wget https://github.com/HDFGroup/hdf5/releases/download/hdf5-1_14_2/hdf5-1_14_2.tar.gz
echo "120641d3ffedd4c730dc7862f544dc0d33382730841cebfcdc78fb9d0869b410  hdf5-1_14_2.tar.gz" > hdf5.sum
shasum -a 256 -c hdf5.sum
tar xf hdf5-1_14_2.tar.gz
cd hdfsrc
cmake -B build -G Ninja -S . -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_INSTALL_PREFIX:PATH=/usr \
  -DHDF5_BUILD_CPP_LIB=ON -DHDF5_BUILD_TOOLS:BOOL=OFF -DBUILD_TESTING:BOOL=OFF -DBUILD_SHARED_LIBS:BOOL=ON \
  -DHDF5_BUILD_HL_LIB:BOOL=ON -DHDF5_ENABLE_Z_LIB_SUPPORT:BOOL=ON
sudo cmake --build build --target install

# Install GDAL
cd /tmp
wget https://github.com/OSGeo/gdal/releases/download/v3.6.4/gdal-3.6.4.tar.gz
echo "f98a654f348a08ef2a09ac78bc9ac0707d7de2b7f942685f5953041399ee6959  gdal-3.6.4.tar.gz" > gdal.sum
shasum -a 256 -c gdal.sum
tar xf gdal-3.6.4.tar.gz
cd gdal-3.6.4
cmake -B build -G Ninja -S . -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_INSTALL_PREFIX:PATH=/usr \
  -DBUILD_APPS=OFF -DBUILD_TESTING=OFF -DGDAL_ENABLE_DRIVER_BAG=ON  \
  -DGDAL_USE_PARQUET=OFF -DGDAL_USE_ARROW=OFF -DGDAL_USE_ARROWDATASET=OFF \
  -DGDAL_ENABLE_HDF5_GLOBAL_LOCK:BOOL=ON -DBUILD_PYTHON_BINDINGS:BOOL=OFF -DBUILD_JAVA_BINDINGS:BOOL=OFF \
  -DBUILD_CSHARP_BINDINGS:BOOL=OFF
sudo cmake --build build --target install

popd
# Create Python venv and install dependencies
python3 -m venv python-venv --system-site-packages
source python-venv/bin/activate
pip install setuptools 'setuptools-scm[toml]' wheel cmake-build-extension \
  unittest-xml-reporting pytest pytest-cov pytest-xdist numpy
# Install GDAL after numpy is installed so that gdal_array bindings will be installed.
pip install 'GDAL==3.6.4'
deactivate
