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
To open a BAG file read-only and print the number of rows and columns use the
:cpp:func:`BAG::Dataset::open` function with :cpp:type:`BAG::OpenMode` set to
:cpp:enumerator:`BAG_OPEN_READONLY`:

C++::

    #include <stdlib.h>
    #include <bag_dataset.h>

    using BAG::Dataset;

    int main(int argc, char** argv) {
        auto pDataset = Dataset::open(argv[1], BAG_OPEN_READONLY);
        const auto& descriptor = pDataset->getDescriptor();
        std::cout << "Dataset details:\n";
        uint64_t numRows = 0, numCols = 0;
        std::tie(numRows, numCols) = descriptor.getDims();
        std::cout << "\trows, columns == " << numRows << ", " << numCols << '\n';
        return EXIT_SUCCESS;
    }

Python::

    import sys
    from bagPy import *

    dataset = Dataset.openDataset(sys.argv[1], BAG_OPEN_READONLY)
    descriptor = dataset.getDescriptor()
    print('Dataset details:')
    numRows, numCols = descriptor.getDims()
    print(f"\trows, columns == {numRows}, {numCols}")


Opening read-write
------------------
To open a BAG file read-write and add a new layer use the :cpp:func:`BAG::Dataset::open` function with
:cpp:type:`BAG::OpenMode` set to :cpp:enumerator:`BAG_OPEN_READ_WRITE`:

C++::


Python::


Creating a BAG file
===================






