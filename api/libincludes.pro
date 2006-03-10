XERCESCROOT = $(PWD)/../extlibs/xerces-c-src_2_6_0

# Setup the OpenNS software development includes. Normally application projects 
# will just need to include INC_OPENNSALL.
#INC_OPENNSALL   = ../../extlibs/hdf5 ../../extlibs/geotrans ../../extlibs/beecrypt ../../extlibs
INC_OPENNSALL   = ../api ../extlibs/hdf5 ../extlibs/geotrans ../extlibs/beecrypt ../extlibs $(XERCESCROOT)/src
