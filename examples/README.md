# BAG C++ Sample Programs

## Building standalone
First install libbaglib from the
[conda-forge](https://anaconda.org/conda-forge/libbaglib).

Then install the following additional conda-forge packages:
```shell
conda install -c conda-forge ninja gcc gxx
```

Finally, build BAG examples (make sure you are in the `examples` sub-directory
when running these commands:
```shell
export CC=gcc	# Windows: set CC=gcc
export CXX=g++	# Windows: set GXX=g++
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -B build -S .
cmake --build build
```

## sample-data
Contains a sample XML file and a prebuild BAG file as trivial
examples.

## bag_read
A sample reading of a Bag file. See the readme.txt file inside 
the sample-data directory for more information to test this 
program.

## bag_create
Creates a sample 10x10 row/column BAG file. See the readme.txt 
file inside the sample-data directory for more information on 
how to test this program.

## bag_compoundlayer
Creates and reads a GeorefMetadataLayer.

## bag_vr_create
Creates a variable resolution BAG.

## bag_vr_read
Reads a variable resolution BAG.
