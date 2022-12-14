
# This directory contains pairs of files for testing.

For instance:

sample.xml and sample.bag

The sample.xml file can be used with the bag_create program to create an initial
sample bag file. From a console, assuming you are in the sample-data
directory then the command to do this under Windows is:

```
..\..\build\stage\bag_create.exe sample.xml testoutput.bag
```

This will combine the XML file with some internally generated data to create
a bag file.

This directory also includes a produced bag file called sample.bag which is in
fact the output of bag_create with the sample.xml file. You can either use it
to test the bag_read program or create your own as described above first.
For example:

```
..\..\build\stage\bag_read.exe sample.bag
```

This will print some of the bag information out on the command line.

The single example_w_qc_layers.bag contains an example of the NODE and ELEVATION
groups, that the InterleavedLegacyLayer class works with.

## Georeferenced metadata layer
`bag_georefmetadata_layer.bag` can be created by running:
```
bag_georefmetadata_layer bag_georefmetadata_layer.xml bag_georefmetadata_layer.bag
```

> Node: The example app `bag_georefmetadata_layer` is not feature-rich and will 
> simply crash if the output file, e.g. `bag_georefmetadata_layer.cpp`, already exists.
