#!/bin/sh
GITHUB_WORKSPACE=$1
PYTHON_VERSION=$2
sudo apt-get update
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
conda install clang gxx_linux-64 cmake ninja hdf5-static libxml2 swig catch2
conda list --show-channel-urls
