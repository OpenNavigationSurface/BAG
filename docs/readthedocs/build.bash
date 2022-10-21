#!/usr/bin/bash
wget -q https://repo.continuum.io/miniconda/Miniconda3-latest-Linux-x86_64.sh -O miniconda.sh
bash miniconda.sh -b -p $(pwd)/miniconda
source "$(pwd)/miniconda/etc/profile.d/conda.sh"
hash -r
conda env create -q --file docs/readthedocs/environment.yml
conda activate readthedocs
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -B build -S . -DBAG_BUILD_SWIG:BOOL=ON -DBAG_BUILD_PYTHON:BOOL=ON -DBAG_BUILD_DOCS:BOOL=ON
cmake --build build -v
mkdir -p _readthedocs/html/
ls -R build/docs/sphinx
cp -r build/docs/sphinx/* _readthedocs/html/
ls -R _readthedocs/html
