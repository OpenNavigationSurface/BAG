============
How-to Guide
============
.. reStructuredText newbie examples:
.. A `regular link`_.
.. .. _regular link: http://ccom.unh.edu
.. See C++ API Reference :doc:`../cpp-api/index` for more information.
.. See Python API Reference :doc:`../python-api/index` for more information.
.. Link to a C++ entities: :cpp:class:`BAG::Dataset`, :cpp:func:`BAG::Dataset::open`

Opening a BAG file
==================

Opening read-only
-----------------
**C++**: To open a BAG file read-only and print the number of rows and columns use the
:cpp:func:`BAG::Dataset::open` function with :cpp:type:`BAG::OpenMode` set to
:cpp:enumerator:`BAG_OPEN_READONLY`::

    #include <stdlib.h>
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
:py:func:`bagPy.Dataset.openDataset` function with ``openMode`` set to ``BAG_OPEN_READONLY``::

    import sys
    from bagPy import *

    dataset = Dataset.openDataset(sys.argv[1], BAG_OPEN_READONLY)
    descriptor = dataset.getDescriptor()
    print('Dataset details:')
    numRows, numCols = descriptor.getDims()
    print(f"\trows, columns == {numRows}, {numCols}")

Opening read-write
------------------
**C++**: To open a BAG file read-write and add a new layer use the :cpp:func:`BAG::Dataset::open` function with
:cpp:type:`BAG::OpenMode` set to :cpp:enumerator:`BAG_OPEN_READ_WRITE`::

    #include <stdlib.h>
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
with ``openMode`` set to ``BAG_OPEN_READ_WRITE``::


    import sys
    from bagPy import *

    dataset = Dataset.openDataset(sys.argv[1], BAG_OPEN_READ_WRITE)
    numLayerTypes = len(dataset.getLayerTypes())
    print(f"Number of layer types: {numLayerTypes}")
    chunkSize = 100
    compressionLevel = 6
    dataset.createSimpleLayer(Average_Elevation, chunkSize, compressionLevel);
    print(f"Number of layer types: {len(dataset.getLayerTypes())}")

Creating a BAG file
===================






