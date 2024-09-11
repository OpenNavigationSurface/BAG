#!/usr/bin/bash
set -eu -o pipefail
conda init bash
source ${HOME}/.bashrc
conda env create -q --file docs/readthedocs/environment.yml
conda activate readthedocs
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -B build -S . -DBAG_BUILD_PYTHON:BOOL=ON -DBAG_BUILD_DOCS:BOOL=ON
cmake --build build -v
mkdir -p _readthedocs/html/
ls -R build/docs/sphinx
cp -r build/docs/sphinx/* _readthedocs/html/
ls -R _readthedocs/html
