#!/bin/bash
GITHUB_WORKSPACE=$1
PYTHON_VERSION=$2
echo "GITHUB_WORKSPACE: ${GITHUB_WORKSPACE}"
echo "PYTHON_VERSION: ${PYTHON_VERSION}"
sudo apt-get update -y
sudo apt-get install -y llvm clang lcov
wget -q https://repo.continuum.io/miniconda/Miniconda3-latest-Linux-x86_64.sh -O miniconda.sh
bash miniconda.sh -b -p ${GITHUB_WORKSPACE}/miniconda
source "${GITHUB_WORKSPACE}/miniconda/etc/profile.d/conda.sh"
hash -r
conda config --set always_yes yes --set changeps1 no
conda config --add channels conda-forge
conda update -q conda
conda info -a
conda create -q -n test-environment python=${PYTHON_VERSION}
conda activate test-environment
conda install gxx_linux-64 cmake ninja hdf5 libxml2 swig
conda list --show-channel-urls
