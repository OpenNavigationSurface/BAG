
.. image:: https://github.com/OpenNavigationSurface/BAG/actions/workflows/testreporting.yml/badge.svg
    :target: https://github.com/OpenNavigationSurface/BAG/actions/workflows/testreporting.yml

.. image:: https://github.com/OpenNavigationSurface/BAG/actions/workflows/testmatrix.yml/badge.svg
    :target: https://github.com/OpenNavigationSurface/BAG/actions/workflows/testmatrix.yml
	
.. image:: https://ci.appveyor.com/api/projects/status/b4y9lmrhvhlntgo2?svg=true
    :target: https://ci.appveyor.com/project/giumas/bag

|
	
This directory contains the necessary source files and examples required to build and
work with data in the BAG format:

* **api** - This is the primary API directory and contains the source for the Bathymetric Attributed Grid format (BAG).
* **configdata** - Required XML support files. You must have an environment variable called BAG_HOME mapped to this directory in order to run the API functions.
* **docs** - As the documentation evolves it will reside here. Currently this contains some initial draft documentation.
* **examples** - Contains programs to demonstrate some of the API functionality. In particular bag_create and bag_read are good starting points.
* **python** - Contains Python tests that make use of the SWIG interface.
* **tests** - Contains C++ tests.

*Important Building Notes*: The build instructions can be found here: docs/build_instructions.html

Building documentation
----------------------
Doxygen is needed to produce the documentation.  Get it at http://www.doxygen.nl/

::
    cd docs/api
    doxygen docs_config.dox

  Browse to html/index.html

To generate API docs for Python bindings, you will need to install Sphinx (see requirements-docs.txt).
To build the docs, first build the Python bindings:

::
    cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -B build -S . -DCMAKE_INSTALL_PREFIX=/usr/local -DBAG_BUILD_SWIG:BOOL=ON -DBAG_BUILD_PYTHON:BOOL=ON && \
    cmake --build build

Then, generate .rst files using Sphinx:

::
    sphinx-apidoc -o docs/python/source build/api/swig/python

Finally, build HTML API docs:

::
    cd docs/python
    make html

Then open docs/python/build/html/index.html.

Running C++ tests in Visual Studio
----------------------------------
The C++ unit tests are written with Catch2.  There is no test harness at the moment.  Using the Standard toolbar, choose bag_tests.exe as the Select Startup Item.  Press F5 to run the tests.

Running Python tests in Visual Studio
-------------------------------------
The Python tests do not use any unit test framework at the moment.  Using the Standard toolbar, choose Current Document as the Select Startup Item.  Open test_all.py (or any other test_*.py) in the python folder, and press F5 to run the tests.
