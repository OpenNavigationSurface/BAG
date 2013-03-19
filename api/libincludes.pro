XERCESCROOT = $(PWD)/../extlibs/xerces-c-src_2_6_0

# Setup the OpenNS software development includes. Normally application projects 
# will just need to include INC_OPENNSALL.
INC_OPENNSALL   = ../api/include  ../api ../extlibs/hdf5-1.8.3/src ../extlibs/beecrypt-4.1.2  ../extlibs/beecrypt-4.1.2/include  ../extlibs/xerces-c-src_2_6_0/include ../extlibs $(XERCESCROOT)/src
