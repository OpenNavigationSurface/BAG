[![image](https://github.com/OpenNavigationSurface/BAG/actions/workflows/testreporting.yml/badge.svg)](https://github.com/OpenNavigationSurface/BAG/actions/workflows/testreporting.yml)
[![image](https://github.com/OpenNavigationSurface/BAG/actions/workflows/testmatrix.yml/badge.svg)](https://github.com/OpenNavigationSurface/BAG/actions/workflows/testmatrix.yml)
[![image](https://github.com/OpenNavigationSurface/BAG/actions/workflows/testwindows.yml/badge.svg)](https://github.com/OpenNavigationSurface/BAG/actions/workflows/testwindows.yml)
[![image](https://raw.githubusercontent.com/OpenNavigationSurface/BAG/badges/master/coverage.svg)](https://github.com/OpenNavigationSurface/BAG/actions/workflows/testreporting.yml)
[![Documentation Status](https://readthedocs.org/projects/bag/badge/?version=stable)](https://bag.readthedocs.io/en/latest/?badge=stable)

# Bathymetric Attributed Grid (BAG) - Open Navigation Surface Project

This repository contains the specification of BAG data format and the necessary library code and examples required
to build and work with data in the BAG format:

- [api](/api) - This is the primary API directory and contains the source
    for the Bathymetric Attributed Grid format (BAG).
- [configdata](/configdata) - Required XML support files. You must have an
    environment variable called BAG_HOME mapped to this directory in
    order to run the API functions.
- [docs](/docs) - Miscellaneous and historical documentation resides here
- [examples](/examples) - Contains programs to demonstrate some of the API
    functionality. In particular bag_create and bag_read are good
    starting points.
- [python](/python) - Contains Python units tests and examples that make
    use of the SWIG interface.
- [tests](/tests) - Contains C++ tests.

The BAG specification and library are produced by the 
[Open Navigation Surface project](https://opennavigationsurface.github.io).

# Documentation

Documentation for the BAG specification and library can be found [here](https://bag.readthedocs.io/en/stable/index.html), in particular:

- [Format Specification Document](https://bag.readthedocs.io/en/stable/fsd/index.html)
- [How-to Guide](https://bag.readthedocs.io/en/stable/howto-guide/index.html)
- [C++ API Reference](https://bag.readthedocs.io/en/stable/cpp-api/index.html)
- [Python API Reference](https://bag.readthedocs.io/en/stable/python-api/index.html)

# Installing and using the BAG library

The BAG library, and its dependencies, can be installed in a Conda environment
(for example, [Anaconda](https://www.anaconda.com/download) or
[Miniconda](https://docs.conda.io/projects/miniconda/en/latest/)).

If you only want the C++ library, install 
[libbaglib](https://anaconda.org/conda-forge/libbaglib).

To install the Python bindings (along with the C++ library) install 
[bagpy](https://anaconda.org/conda-forge/bagpy).

Once installed, you can test C++ library by building the
[BAG examples](examples/README.md) as a standalone project.

> Note: You can use [FindBAG.cmake](CMakeModules/FindBAG.cmake) in your
> own projects to locate libbaglib installed via conda.

Likewise, you can run the Python tests using the Conda-provided `bagpy`
bindings by first installing the test dependencies into your conda environment:
```shell
pip install -r requirements.txt
```

Then run the tests (Linux and macOS):
```shell
BAG_SAMPLES_PATH=./examples/sample-data python -m pytest python/test_*.py
```

Under Windows, run:
```shell
set BAG_SAMPLES_PATH=examples\sample-data
python -m pytest python\test_*.py
```

# Building and using the BAG library

Comprehensive build instructions can be found [here](docs/BUILDING.md).


For a Quick Start using `make` to build C++ applications on Linux, see [QUICKSTART.MD](docs/QUICKSTART.md).
