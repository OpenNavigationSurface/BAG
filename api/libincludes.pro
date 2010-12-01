XERCESCROOT = $(PWD)/../extlibs/xerces-c-src_2_6_0

# Setup the OpenNS software development includes. Normally application projects 
# will just need to include INC_OPENNSALL.
win32 {
    XERCESCROOT = ../extlibs/xerces-c-src_2_6_0
	INC_OPENNSALL   = ../../api ../extlibs/hdf5-1.8.5-patch1 ../extlibs/beecrypt ../extlibs ../extlibs/xerces-c-src_2_6_0/include 
}
unix {
	INC_OPENNSALL   = ../api ../extlibs/hdf5-1.6.1/src $(XERCESROOT)/src ../extlibs/beecrypt-4.1.2 ../extlibs
}

