============
How-to Guide
============
.. reStructuredText newbie examples:
.. A `regular link`_.
.. .. _regular link: http://ccom.unh.edu
.. See C++ API Reference :doc:`../cpp-api/index` for more information.
.. See Python API Reference :doc:`../python-api/index` for more information.
.. Link to a C++ entity: :cpp:class:`BAG::Dataset`

Opening a BAG file
==================

Opening read-only
-----------------
To use the :cpp:class:`BAG::Dataset` class to open a BAG file and print the number of rows
and columns do the following:

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



Opening read-write
------------------


Creating a BAG file
===================






