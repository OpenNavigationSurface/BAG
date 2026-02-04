#!/usr/bin/bash
set -e
wget -q https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh -O miniconda.sh
export CONDA_PLUGINS_AUTO_ACCEPT_TOS=true
bash miniconda.sh -b -p $(pwd)/miniconda
source "$(pwd)/miniconda/etc/profile.d/conda.sh"
hash -r
conda env create -q --file docs/readthedocs/environment.yml
conda activate readthedocs
rm -rf doc-build
echo "Installed conda packages..."
conda list
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -B doc-build -S . \
  -DCMAKE_PREFIX_PATH="${CONDA_PREFIX}" -DCMAKE_IGNORE_PATH='/usr;/usr/local' \
  -DPython3_FIND_STRATEGY=LOCATION -DBAG_BUILD_PYTHON:BOOL=ON -DBAG_BUILD_DOCS:BOOL=ON
cmake --build doc-build -v
mkdir -p _readthedocs/html/
ls -R doc-build/docs/sphinx
cp -r doc-build/docs/sphinx/* _readthedocs/html/
ls -R _readthedocs/html
