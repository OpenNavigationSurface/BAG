# Build instructions

## Linux / macOS

### Configure and build BAG

#### Build C++ library, tests, and example binaries:
```shell
$ cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -B build -S . \
 -DBAG_BUILD_TESTS:BOOL=ON -DBAG_CODE_COVERAGE:BOOL=ON \
 -DBAG_BUILD_PYTHON:BOOL=OFF -DBAG_BUILD_EXAMPLES:BOOL=ON
$ cmake --build build -j 8
$ BAG_SAMPLES_PATH=/ABSOLUTE/PATH/TO/REPO/BAG/examples/sample-data ninja -C build build ccov-all-export-lcov
```

Where `/ABSOLUTE/PATH/TO/REPO/BAG/examples/sample-data` should be replaced by the absolute path
to the `examples/sample-data` directory within the BAG repo.

This will generate an HTML coverage report in `build/ccov/all-merged/index.html`.

> Note: This will build with code coverage report generation for bag_tests when compiled with LLVM or GCC.

> Note: On Linux/macOS, make sure to set `CC=clang` and `CXX=clang++` to compile with clang and
> use llvm-cov. GCC and lcov will work, but the coverage exclusion configuration in the BAG project CMake
> configuration only works properly with LLVM, meaning that lcov coverage will include things like the C++ standard
> library, which will lead to inaccurate coverage percentages for the BAG tests.

> Note: if you want to build documentation, add `-DBAG_BUILD_DOCS:BOOL=ON` to the CMake config. 
> See the Read the Docs [conda environment](readthedocs/environment.yml) for dependencies needed
> to build documentation.

#### Build Python wheel
After building the C++ library in the `build` directory as above, 
you will be able to build a Python wheel for installing `bagPy` as follows:
```shell
$ python -m pip wheel -w ./wheel/ ./build/api/swig/python
```

Then you can install the wheel with:
```shell
$ python -m pip install ./wheel/bagPy-*.whl
```

### Build examples only
Build examples only as follows:
```shell
$ cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -B bag-examples -S . \
 -DBAG_BUILD_BAG_LIB:BOOL=OFF -DBAG_BUILD_EXAMPLES:BOOL=ON
$ cmake --build bag-examples -j 8
```

> Note: This assumes you have either built `baglib` yourself, or installed
> binaries via `conda`.

Then run, for example `bag_georefmetadata_layer`: 
```shell
$ ./bag-examples/examples/bag_georefmetadata_layer \
  examples/sample-data/bag_georefmetadata_layer.xml bag_georefmetadata.bag
```

## Windows: Visual Studio 2022/2019

### Dependencies from Miniconda

#### Install miniconda & BAG dependencies

Download the latest version of miniconda from [here](https://docs.conda.io/en/latest/miniconda.html#windows-installers) and install to \miniconda (i.e., C:\miniconda).

Next, to install configure miniconda open a Developer Command Prompt in Visual Studio:

```shell
set PATH=\miniconda;\miniconda\Scripts;\miniconda\Library\bin;%PATH%
conda init # Restart your shell
conda update conda
conda config --add channels conda-forge
conda create -n bag-dev-env python=3.9
conda activate bag-dev-env
conda install cmake ninja hdf5 libxml2 swig catch2
```

> Note: if usnig a different version of Python, modify `conda create -q -n bag-dev-environment python=3.9` as needed.

#### Configure and build BAG

```shell
set CC=cl.exe
set CXX=cl.exe
cmake -Bbuild -S. -G "Visual Studio 16 2019" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=\miniconda\envs\bag-dev-env\Library -DBAG_BUILD_PYTHON:BOOL=ON -DBAG_BUILD_TESTS:BOOL=ON
cmake --build build
set PATH=build\api;%PATH%
set BAG_SAMPLES_PATH=examples\sample-data
build\tests\bag_tests.exe
python python\test_all.py
```

### Dependencies from source

> Note: We recommend using Dependencies from Miniconda as per above.

#### Setup output directory for build artifacts

```shell
mkdir \BAG
```

#### zlib

  - Cone repo: https://github.com/madler/zlib.git
  - Check out a release tag, e.g., v1.2.12
  - Build (run in Developer PowerShell in project directory):
  ```
  cmake -B build -G "Visual Studio 16 2019" -S . -DCMAKE_INSTALL_PREFIX:PATH=\BAG -DCMAKE_BUILD_TYPE=Release
  cmake --build build --target install
  ```

  > Note: Use `-G "Visual Studio 17 2022"` for Visual Studio 2022.

#### libxml2

  - Clone repo: https://gitlab.gnome.org/GNOME/libxml2.git
  - Check out a release tag, e.g., v2.9.14
  - Build (run in Developer PowerShell in project directory):
  ```
  cmake -B libxml2-2.9.14-build -G "Visual Studio 16 2019" -S . -DCMAKE_INSTALL_PREFIX:PATH=\BAG -DCMAKE_BUILD_TYPE:STRING=Release -DLIBXML2_WITH_ZLIB=ON -DLIBXML2_WITH_ICONV=OFF -DLIBXML2_WITH_LZMA=OFF -DLIBXML2_WITH_PYTHON=OFF
  cmake --build libxml2-2.9.14-build --config Release
  cmake --install libxml2-2.9.14-build --config Release
  ```
  
  > Note: Use `-G "Visual Studio 17 2022"` for Visual Studio 2022.

#### HDF5

  - Clone repo: https://github.com/HDFGroup/hdf5.git
  - Check out a release tag, e.g., hdf5-1_12_2
  - Build (run in Developer PowerShell in project directory):
  ```
  cmake -B build -G "Visual Studio 16 2019" -S . ^
   -DCMAKE_INSTALL_PREFIX:PATH=\BAG -DCMAKE_BUILD_TYPE:STRING=Release ^
   -DHDF5_BUILD_CPP_LIB=ON -DHDF5_BUILD_TOOLS:BOOL=OFF ^
   -DBUILD_TESTING:BOOL=OFF -DBUILD_SHARED_LIBS:BOOL=ON ^
   -DHDF5_BUILD_HL_LIB:BOOL=ON -DHDF5_ENABLE_Z_LIB_SUPPORT:BOOL=ON ^
   -DZLIB_INCLUDE_DIRS=\BAG\include -DZLIB_LIBRARY:PATH=\BAG\lib\zlibd.lib
  cmake --build build --config Release
  cmake --install build --config Release
  ```
   
  > Note: `BUILD_SHARED_LIBS` must be set to `ON`.

  > Note: Use `-G "Visual Studio 17 2022"` for Visual Studio 2022.
  
#### Catch2

  - Clone repo: https://github.com/catchorg/Catch2.git
  - Check out the latest 3.x+ release, e.g., v3.0.1
  - Build (run in Developer PowerShell in project directory):
  ```
  cmake -B build -G "Visual Studio 16 2019" -S . ^
   -DCMAKE_INSTALL_PREFIX:PATH=\BAG -DBUILD_TESTING:BOOL=OFF
  cmake --build build --target install
  ```
  
  > Note: Use `-G "Visual Studio 17 2022"` for Visual Studio 2022.
  
 #### SWIG (optional; required for Python support)
 
  - Download SWIGWIN 4.x pre-built binary from: https://www.swig.org/download.html
  - Unzip downloaded file
  - Copy to \BAG directory:
  ```
  copy swig.exe \BAG\bin
  xcopy /s Lib \BAG\lib
  ```
 
 ### Configure and build BAG project for developing
 Without Python bindings:
 ```
 cmake -G "Visual Studio 16 2019" -B build -S . -DCMAKE_BUILD_CONFIG=Release ^
  -DCMAKE_PREFIX_PATH=\BAG -DCMAKE_INSTALL_PREFIX=\BAG -DSWIG_DIR=\BAG
 ```

> Note: Use `-G "Visual Studio 17 2022"` for Visual Studio 2019.

Build and install:
```
cmake --build build
cmake --install build 
```

#### Build Python wheels:
After building the C++ library in the `build` directory as above,
you will be able to build a Python wheel for installing `bagPy` as follows:
 ```
python -m pip wheel -w .\wheel\ .\build\api\swig\python
 ```

Then install the wheel as follows:
```
for %%w in (.\wheel\bagPy-*.whl) do python -m pip install %%w
```

### Running C++ tests in Visual Studio

The C++ unit tests are written with Catch2. There is no test harness at
the moment. Using the Standard toolbar, choose bag_tests.exe as the
Select Startup Item. Press F5 to run the tests.

If you are unable to run tests directly withing Visual Studio, the tests
can be run manually. See [appveyor.yml](/appveyor.yml) for details.

### Running Python tests in Visual Studio

The Python tests do not use any unit test framework at the moment. Using
the Standard toolbar, choose Current Document as the Select Startup
Item. Open test_all.py (or any other `test_*.py`) in the python
folder, and press F5 to run the tests.

If you are unable to run tests directly withing Visual Studio, the tests
can be run manually. See [appveyor.yml](/appveyor.yml) for details.
