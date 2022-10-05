[![image](https://github.com/OpenNavigationSurface/BAG/actions/workflows/testreporting.yml/badge.svg)](https://github.com/OpenNavigationSurface/BAG/actions/workflows/testreporting.yml)
[![image](https://github.com/OpenNavigationSurface/BAG/actions/workflows/testmatrix.yml/badge.svg)](https://github.com/OpenNavigationSurface/BAG/actions/workflows/testmatrix.yml)
[![image](https://ci.appveyor.com/api/projects/status/b4y9lmrhvhlntgo2?svg=true)](https://ci.appveyor.com/project/giumas/bag)
[![image](coverage.svg)](https://github.com/OpenNavigationSurface/BAG/actions/workflows/testreporting.yml)

# Bathymetric Attributed Grid (BAG) - Open Navigation Surface Project

This repository contains the necessary library code and examples required
to build and work with data in the BAG format:

- **api** - This is the primary API directory and contains the source
    for the Bathymetric Attributed Grid format (BAG).
- **configdata** - Required XML support files. You must have an
    environment variable called BAG_HOME mapped to this directory in
    order to run the API functions.
- **docs** - As the documentation evolves it will reside here.
    Currently this contains some initial draft documentation.
- **examples** - Contains programs to demonstrate some of the API
    functionality. In particular bag_create and bag_read are good
    starting points.
- **python** - Contains Python units tests and examples that make
    use of the SWIG interface.
- **tests** - Contains C++ tests.

# Building the library

The build instructions can be found [here](docs/BUILDING.md).


For a C++ Quick Start, see [quickstart.md](./quickstart.md).

# Building documentation

## C++ API

Doxygen is needed to produce the documentation. Get it at
<http://www.doxygen.nl/>

To build documentation:

```
doxygen docs/api/docs_config.dox
```

Then browse to docs/api/html/index.html.

## Python bindings

To generate API docs for Python bindings, you will need to install
Sphinx (see requirements-docs.txt). To build the docs, first build the
Python bindings and install them into a virtual environment:

```
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -B build -S . \
    -DCMAKE_INSTALL_PREFIX=/usr/local -DBAG_BUILD_SWIG:BOOL=ON \
    -DBAG_BUILD_PYTHON:BOOL=ON && \
cmake --build build
python build/api/swig/python/setup.py install
```

> Note: Make sure you initialize your virtual environment before running the final command.

Build HTML API docs:

```
cd docs/python
make html
```

Then open docs/python/build/html/index.html.

# Running C++ tests in Visual Studio

The C++ unit tests are written with Catch2. There is no test harness at
the moment. Using the Standard toolbar, choose bag_tests.exe as the
Select Startup Item. Press F5 to run the tests.

If you are unable to run tests directly withing Visual Studio, the tests
can be run manually. See [appveyor.yml](appveyor.yml) for details.

# Running Python tests in Visual Studio

The Python tests do not use any unit test framework at the moment. Using
the Standard toolbar, choose Current Document as the Select Startup
Item. Open test_all.py (or any other `test_*.py`) in the python
folder, and press F5 to run the tests.

If you are unable to run tests directly withing Visual Studio, the tests
can be run manually. See [appveyor.yml](appveyor.yml) for details.
