XERCESCROOT = $(PWD)/../extlibs/xerces-c-src_2_6_0

# Setup the OpenNS software development includes. Normally application projects 
# will just need to include INC_OPENNSALL.
INC_OPENNSALL   = ../api ../extlibs/hdf5-1.6.1/src ../extlibs/geotrans ../extlibs/beecrypt-4.1.2 ../extlibs $(XERCESCROOT)/src
