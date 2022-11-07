================
API How-to Guide
================
.. TODO: Change links to files in the BAG repo to refer to the main branch once v2 PR is merged

.. reStructuredText newbie examples:
.. A `regular link`_.
.. .. _regular link: http://ccom.unh.edu
.. See C++ API Reference :doc:`../cpp-api/index` for more information.
.. See Python API Reference :doc:`../python-api/index` for more information.
.. Link to a C++ entities: :cpp:class:`BAG::Dataset`, :cpp:func:`BAG::Dataset::open`

This how-to guide described how to use the BAG API, from either C++ or Python, to perform some common tasks, including:

* Opening an existing BAG file:
    * Opening read-only
    * Opening read-write
* Creating a BAG file
* Working with variable-resolution BAGs
* Working with georeferenced metadata

For more example usage, please refer to these sections of the BAG source code:

* Tests:
    * `C++ unit tests`_
    * `Python unit tests`_
* Examples:
    * `C++ examples`_
    * `Python examples`_

.. pull-quote::

    Note: if you have experience using `GDAL`_, the `BAG-GDAL compatibility tests`_ may be particularly useful as they
    show how to use the BAG API side-by-side with the GDAL API.

.. _GDAL: https://gdal.org
.. _BAG-GDAL compatibility tests: https://github.com/OpenNavigationSurface/BAG/blob/v2/python/test_compat_gdal.py
.. _C++ unit tests: https://github.com/OpenNavigationSurface/BAG/blob/v2/tests
.. _Python unit tests: https://github.com/OpenNavigationSurface/BAG/blob/v2/python
.. _C++ examples: https://github.com/OpenNavigationSurface/BAG/blob/v2/examples
.. _Python examples: https://github.com/OpenNavigationSurface/BAG/blob/v2/examples/python

Opening an existing BAG file
============================

Opening read-only
-----------------
**C++**: To open a BAG file read-only and print the number of rows and columns use the
:cpp:func:`BAG::Dataset::open` function with :cpp:type:`BAG::OpenMode` set to
:cpp:enumerator:`BAG_OPEN_READONLY`:

.. code-block:: cpp

    #include <stdlib.h>
    #include <iostream>
    #include <bag_dataset.h>

    using BAG::Dataset;

    int main(int argc, char** argv) {
        auto dataset = Dataset::open(argv[1], BAG_OPEN_READONLY);
        const auto& descriptor = dataset->getDescriptor();
        std::cout << "Dataset details:" << std::endl;
        uint64_t numRows = 0, numCols = 0;
        std::tie(numRows, numCols) = descriptor.getDims();
        std::cout << "\trows, columns == " << numRows << ", " << numCols << std::endl;
        return EXIT_SUCCESS;
    }

**Python**: To open a BAG file read-only and print the number of rows and columns use the
:py:func:`bagPy.Dataset.openDataset` function with ``openMode`` set to ``BAG_OPEN_READONLY``:

.. code-block:: python

    import sys
    import bagPy as BAG

    dataset = BAG.Dataset.openDataset(sys.argv[1], BAG.BAG_OPEN_READONLY)
    descriptor = dataset.getDescriptor()
    print('Dataset details:')
    numRows, numCols = descriptor.getDims()
    print(f"\trows, columns == {numRows}, {numCols}")

Opening read-write
------------------
**C++**: To open a BAG file read-write and add a new layer use the :cpp:func:`BAG::Dataset::open` function with
:cpp:type:`BAG::OpenMode` set to :cpp:enumerator:`BAG_OPEN_READ_WRITE`:

.. code-block:: cpp

    #include <stdlib.h>
    #include <iostream>
    #include <bag_dataset.h>

    using BAG::Dataset;

    int main(int argc, char** argv) {
        auto dataset = Dataset::open(argv[1], BAG_OPEN_READ_WRITE);
        const auto numLayerTypes = dataset->getLayerTypes().size();
        std::cout << "Number of layer types: " << numLayerTypes << std::endl;
        constexpr uint64_t chunkSize = 100;
        constexpr int compressionLevel = 6;
        dataset->createSimpleLayer(Average_Elevation, chunkSize, compressionLevel);
        std::cout << "Number of layer types: " << dataset->getLayerTypes().size() << std::endl;

        return EXIT_SUCCESS;
    }

**Python**: To open a BAG file read-write and add a new layer use the :py:func:`bagPy.Dataset.openDataset` function
with ``openMode`` set to ``BAG_OPEN_READ_WRITE``:

.. code-block:: python

    import sys
    import bagPy as BAG

    dataset = BAG.Dataset.openDataset(sys.argv[1], BAG.BAG_OPEN_READ_WRITE)
    numLayerTypes = len(dataset.getLayerTypes())
    print(f"Number of layer types: {numLayerTypes}")
    chunkSize = 100
    compressionLevel = 6
    dataset.createSimpleLayer(BAG.Average_Elevation, chunkSize, compressionLevel);
    print(f"Number of layer types: {len(dataset.getLayerTypes())}")

Creating a BAG file
===================
When creating a BAG, you must provide metadata in the form of ISO metadata standards 19915 and 19115-2, as
described in the `Format Specification Document`_. A convenient way to do this is to load metadata from an XML file,
which is the approach illustrated below. However, it should be noted that metadata can be programmatically generated
using :cpp:class:`BAG::Metadata` (Python: :py:class:`bagPy.Metadata`) and its subclasses. For the purposes of this
example, we'll use `sample.xml`_ to populated metadata in the newly created BAG.

.. _Format Specification Document: ../fsd/FSD-BAGStructure.html#metadata
.. _sample.xml: https://github.com/OpenNavigationSurface/BAG/blob/b13ef862a791c6cf776251e6a3e05867acc1fb27/examples/sample-data/sample.xml

.. NOTE::
    The spatial coverage of a BAG is defined by setting the southwest and northeast corners via the metadata element: ``/gmi:MI_Metadata/gmd:spatialRepresentationInfo/gmd:MD_Georectified/gmd:cornerPoints/gml:Point/gml:coordinates``.

**C++**:

.. literalinclude:: howto-create.cpp
    :language: c++

**Python**:

.. literalinclude:: howto-create.py
    :language: python

Working with georeferenced metadata
===================================
Version 2 of the BAG file format allows georeference metadata describing metadata on a node by node basis at the full
resolution of the dataset. This is accomplished by creating a :cpp:class:`BAG::CompoundLayer` (Python:
:py:class:`bagPy.CompoundLayer`) within an existing BAG. To create a compound layer, you first need to create a
:cpp:type:`BAG::RecordDefinition` (Python: :py:class:`bagPy.RecordDefinition`); see lines 122-135 in the example
C++ source code, or lines 88-101 in the example Python code below. Note, to make it easier to create BAGs with
georeferenced metadata of a known format, it is possible to create a named metadata profile, for example
:cpp:enumerator:`NOAA_OCS_2022_10_METADATA_PROFILE`. For information on how to use a named metadata profile, see the
examples `bag_compound_layer.cpp`_ and `bag_compound_layer.py`_ in the BAG source code.

.. _bag_compound_layer.cpp: https://github.com/OpenNavigationSurface/BAG/blob/b13ef862a791c6cf776251e6a3e05867acc1fb27/examples/bag_compound_layer.cpp
.. _bag_compound_layer.py: https://github.com/OpenNavigationSurface/BAG/blob/b13ef862a791c6cf776251e6a3e05867acc1fb27/examples/python/bag_compound_layer.py

**C++**:

.. literalinclude:: howto-create-georef.cpp
    :language: c++
    :linenos:
    :emphasize-lines: 122-135

**Python**:

.. literalinclude:: howto-create-georef.py
    :language: python
    :linenos:
    :emphasize-lines: 88-101
