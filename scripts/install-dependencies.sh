#!/bin/bash
set -ex # Abort on error.
GITHUB_WORKSPACE=$1
PYTHON_VERSION=$2
echo "GITHUB_WORKSPACE: ${GITHUB_WORKSPACE}"
echo "PYTHON_VERSION: ${PYTHON_VERSION}"
pushd .

sudo apt-get update -y
sudo apt-get install -y cmake g++ ninja-build swig4.0 zlib1g-dev libproj-dev
# Install Catch2 version 3 (Ubuntu 22.04 only packages version 2)
cd /tmp
wget https://github.com/catchorg/Catch2/archive/refs/tags/v3.4.0.tar.gz
echo "122928b814b75717316c71af69bd2b43387643ba076a6ec16e7882bfb2dfacbb  v3.4.0.tar.gz" > catch2.sum
shasum -a 256 -c catch2.sum
tar xf v3.4.0.tar.gz
cd Catch2-3.4.0
cmake -B build -G Ninja -S . -DCMAKE_INSTALL_PREFIX:PATH=/usr -DBUILD_TESTING:BOOL=OFF
sudo cmake --build build --target install

# Install libxml2
cd /tmp
wget https://gitlab.gnome.org/GNOME/libxml2/-/archive/v2.13.5/libxml2-v2.13.5.tar.gz
echo "37cdec8cd20af8ab0decfa2419b09b4337c2dbe9da5615d2a26f547449fecf2a  libxml2-v2.13.5.tar.gz" > libxml2.sum
shasum -a 256 -c libxml2.sum
tar xf libxml2-v2.13.5.tar.gz
cd libxml2-v2.13.5
cmake -B build -G Ninja -S . -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_INSTALL_PREFIX:PATH=/usr \
  -DLIBXML2_WITH_ZLIB=ON -DLIBXML2_WITH_ICONV=OFF -DLIBXML2_WITH_LZMA=OFF -DLIBXML2_WITH_PYTHON=OFF
cmake --build build --target install

# Install HDF5
cd /tmp
wget https://github.com/HDFGroup/hdf5/releases/download/hdf5_1.14.5/hdf5-1.14.5.tar.gz
echo "ec2e13c52e60f9a01491bb3158cb3778c985697131fc6a342262d32a26e58e44  hdf5-1.14.5.tar.gz" > hdf5.sum
shasum -a 256 -c hdf5.sum
tar xf hdf5-1.14.5.tar.gz
cd hdf5-1.14.5
cmake -B build -G Ninja -S . -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_INSTALL_PREFIX:PATH=/usr \
  -DHDF5_BUILD_CPP_LIB=ON -DHDF5_BUILD_TOOLS:BOOL=OFF -DBUILD_TESTING:BOOL=OFF -DBUILD_SHARED_LIBS:BOOL=ON \
  -DHDF5_BUILD_HL_LIB:BOOL=ON -DHDF5_ENABLE_Z_LIB_SUPPORT:BOOL=ON
sudo cmake --build build --target install

# Install GDAL
cd /tmp
wget https://github.com/OSGeo/gdal/releases/download/v3.9.3/gdal-3.9.3.tar.gz
echo "f293d8ccc6b98f617db88f8593eae37f7e4b32d49a615b2cba5ced12c7bebdae  gdal-3.9.3.tar.gz" > gdal.sum
shasum -a 256 -c gdal.sum
tar xf gdal-3.9.3.tar.gz
cd gdal-3.9.3
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
pip install 'GDAL==3.9.3'
deactivate
