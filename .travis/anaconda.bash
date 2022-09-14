#!/bin/bash

set -ev

curl https://repo.continuum.io/miniconda/Miniconda3-latest-MacOSX-x86_64.sh -o miniconda.sh;

# Install miniconda
bash miniconda.sh -b -p $HOME/miniconda
export PATH="$HOME/miniconda/bin:$PATH"
hash -r
conda config --set always_yes yes --set changeps1 no
conda config --add channels conda-forge
conda update -q conda

# Useful for debugging any issues with conda
conda info -a

# Create conda test environment
conda create -q -n test-environment python=$PYTHON_VERSION
source activate test-environment
conda install cmake hdf5 libxml2
conda list --show-channel-urls
