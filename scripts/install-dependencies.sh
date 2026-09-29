#!/bin/bash
set -ex # Abort on error.
GITHUB_WORKSPACE=$1
PYTHON_VERSION=$2
echo "GITHUB_WORKSPACE: ${GITHUB_WORKSPACE}"
echo "PYTHON_VERSION: ${PYTHON_VERSION}"
pushd .

sudo apt-get update -y
sudo apt-get install -y cmake g++ ninja-build swig zlib1g-dev libproj-dev
# Install a recent version of Catch2 version 3
cd /tmp
wget https://github.com/catchorg/Catch2/archive/refs/tags/v3.16.0.tar.gz
echo "0957cae5821b17ce07f0833aaa52b5137643a8382203221f363a8303c109af34  v3.16.0.tar.gz" > catch2.sum
shasum -a 256 -c catch2.sum
tar xf v3.16.0.tar.gz
cd Catch2-3.16.0
cmake -B build -G Ninja -S . -DCMAKE_INSTALL_PREFIX:PATH=/usr -DBUILD_TESTING:BOOL=OFF
sudo cmake --build build --target install

# Install libxml2
cd /tmp
wget https://download.gnome.org/sources/libxml2/2.15/libxml2-2.15.1.tar.xz
echo "c008bac08fd5c7b4a87f7b8a71f283fa581d80d80ff8d2efd3b26224c39bc54c  libxml2-2.15.1.tar.xz" > libxml2.sum
shasum -a 256 -c libxml2.sum
tar xf libxml2-2.15.1.tar.xz
cd libxml2-2.15.1
cmake -B build -G Ninja -S . -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_INSTALL_PREFIX:PATH=/usr \
  -DLIBXML2_WITH_ZLIB=ON -DLIBXML2_WITH_ICONV=OFF -DLIBXML2_WITH_LZMA=OFF -DLIBXML2_WITH_PYTHON=OFF
sudo cmake --build build --target install

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
python -m venv python-venv --system-site-packages
source python-venv/bin/activate
pip install setuptools 'setuptools-scm[toml]' wheel cmake-build-extension \
  unittest-xml-reporting pytest pytest-cov pytest-xdist numpy
# Install GDAL after numpy is installed so that gdal_array bindings will be installed.
pip install 'GDAL==3.9.3'
deactivate
