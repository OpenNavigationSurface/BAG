[![image](https://github.com/OpenNavigationSurface/BAG/actions/workflows/testreporting.yml/badge.svg)](https://github.com/OpenNavigationSurface/BAG/actions/workflows/testreporting.yml)
[![image](https://github.com/OpenNavigationSurface/BAG/actions/workflows/testmatrix.yml/badge.svg)](https://github.com/OpenNavigationSurface/BAG/actions/workflows/testmatrix.yml)
[![image](https://ci.appveyor.com/api/projects/status/b4y9lmrhvhlntgo2?svg=true)](https://ci.appveyor.com/project/giumas/bag)
[![image](coverage.svg)](https://github.com/OpenNavigationSurface/BAG/actions/workflows/testreporting.yml)
[![Documentation Status](https://readthedocs.org/projects/bag/badge/?version=stable)](https://bag.readthedocs.io/en/latest/?badge=latest)

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

# Documentation

Documentation for the BAG library can be found [here](https://bag.readthedocs.io/en/stable/index.html), in particular:

- [Format Specification Document](https://bag.readthedocs.io/en/stable/fsd/index.html)
- [How-to Guide](https://bag.readthedocs.io/en/stable/howto-guide/index.html)
- [C++ API Reference](https://bag.readthedocs.io/en/stable/cpp-api/index.html)
- [Python API Reference](https://bag.readthedocs.io/en/stable/python-api/index.html)

# Building and using the BAG library

Comprehensive build instructions can be found [here](docs/BUILDING.md).


For a Quick Start using `make` to build C++ applications on Linux, see [QUICKSTART.MD](docs/QUICKSTART.md).
