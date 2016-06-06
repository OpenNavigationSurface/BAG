
This directory contains pairs of files for testing. 

For instance:

sample.xml and sample.bag

The sample.xml file can be used with the bagcreate program to create an initial
sample bag file. From a console, assuming you are in the sample-data 
directory then the command to do this under Windows is:
     ..\bagcreate\debug\bagcreate.exe sample.xml testoutput.bag
     
This will combine the XML file with some internally generated data to create
a bag file. 

This directory also includes a produced bag file called sample.bag which is in
fact the output of bagcreate with the sample.xml file. You can either use it
to test the bagread program or create your own as described above first.
For example:
      ..\bagread\debug\bagread.exe sample.bag
      
This will print some of the bag information out on the command line.
