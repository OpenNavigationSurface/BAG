
Overview of the C++ Sample Programs

Environment
===========

Don't forget the requirement for BAG_HOME before trying the examples.

On Linux/OS X:

export	BAG_HOME=$PWD/../configdata
setenv  BAG_HOME $PWD/../configdata

On Windows:

set BAG_HOME=%CD%\..\configdata


sample-data
-----------

   Contains a sample XML file and a prebuild BAG file as trivial 
   examples.

bag_read
--------

   A sample reading of a Bag file. See the readme.txt file inside 
   the sample-data directory for more information to test this 
   program.

bag_create
----------

   Creates a sample 10x10 row/column BAG file. See the readme.txt 
   file inside the sample-data directory for more information on 
   how to test this program.

bag_compoundlayer
-----------------

    Creates and reads a CompoundLayer.

bag_vr_create
-------------

    Creates a variable resolution BAG.

bag_vr_read
-----------

    Reads a variable resolution BAG.
