# Build instructions

## Windows: Visual Studio 2022/2019

### Dependencies from Miniconda

#### Install miniconda & BAG dependencies
Download the latest version of miniconda from [here](https://docs.conda.io/en/latest/miniconda.html#windows-installers) and install to \miniconda (i.e., C:\miniconda).

Next, to install configure miniconda open an Developer Command Prompt in Visual Studio:
```
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
```
set CC=cl.exe
set CXX=cl.exe
cmake -Bbuild -S. -G "Visual Studio 16 2019" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=\miniconda\envs\bag-dev-env\Library -DBAG_BUILD_PYTHON=ON
cmake --build build
set PATH=build\api;%PATH%
set BAG_SAMPLES_PATH=examples\sample-data
build\tests\bag_tests.exe
python python\test_all.py
```


### Dependencies from source

> Note: We recommend using Dependencies from Miniconda as per above.

#### Setup output directory for build artifacts
```
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
  cmake -B build -G "Visual Studio 16 2019" -S . -DCMAKE_INSTALL_PREFIX:PATH=\BAG -DCMAKE_BUILD_TYPE:STRING=Release -DHDF5_BUILD_CPP_LIB=ON -DHDF5_BUILD_TOOLS:BOOL=OFF -DBUILD_TESTING:BOOL=OFF -DBUILD_SHARED_LIBS:BOOL=ON -DHDF5_BUILD_HL_LIB:BOOL=ON -DHDF5_ENABLE_Z_LIB_SUPPORT:BOOL=ON -DZLIB_INCLUDE_DIRS=\BAG\include -DZLIB_LIBRARY:PATH=\BAG\lib\zlibd.lib
  cmake --build build --config Release
  cmake --install build --config Release
  ```
  
  > Note: Use `-G "Visual Studio 17 2022"` for Visual Studio 2022.
  
#### Catch2

  - Clone repo: https://github.com/catchorg/Catch2.git
  - Check out the latest 3.x+ release, e.g., v3.0.1
  - Build (run in Developer PowerShell in project directory):
  ```
  cmake -B build -G "Visual Studio 16 2019" -S . -DCMAKE_INSTALL_PREFIX:PATH=\BAG -DBUILD_TESTING:BOOL=OFF
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
 
 ### Configure BAG project for developing
 Without Python bindings:
 ```
 cmake -G "Visual Studio 16 2019" -B build -S . -DCMAKE_BUILD_CONFIG=Release -DCMAKE_PREFIX_PATH=\BAG -DCMAKE_INSTALL_PREFIX=\BAG -DSWIG_DIR=\BAG
 ```
 
 With Python bindings:
 ```
 cmake -G "Visual Studio 16 2019" -B build -S . -DCMAKE_BUILD_CONFIG=Release -DCMAKE_PREFIX_PATH=\BAG -DCMAKE_INSTALL_PREFIX=\BAG -DSWIG_DIR=\BAG -DBAG_BUILD_PYTHON=ON
 ```
 
  > Note: Use `-G "Visual Studio 17 2022"` for Visual Studio 2019.