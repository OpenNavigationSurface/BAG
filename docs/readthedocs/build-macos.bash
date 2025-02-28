#!/usr/bin/env bash
wget -q https://repo.anaconda.com/miniconda/Miniconda3-latest-MacOSX-arm64.sh -O miniconda.sh
bash miniconda.sh -b -p $(pwd)/miniconda
source "$(pwd)/miniconda/etc/profile.d/conda.sh"
hash -r
conda env create -q --file docs/readthedocs/environment-macos.yml
conda activate readthedocs
rm -rf doc-build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -B doc-build -S . -DPython3_FIND_STRATEGY=LOCATION -DBAG_BUILD_PYTHON:BOOL=ON -DBAG_BUILD_DOCS:BOOL=ON
cmake --build doc-build -v
mkdir -p _readthedocs/html/
ls -R doc-build/docs/sphinx
cp -r doc-build/docs/sphinx/* _readthedocs/html/
ls -R _readthedocs/html
