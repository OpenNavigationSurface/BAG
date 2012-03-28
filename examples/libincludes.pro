
# Setup the OpenNS software development includes. Normally application projects 
# will just need to include INC_OPENNSALL.


win32 {
        XERCESCROOT = ../extlibs/xerces-c-src_2_6_0
	INC_OPENNSALL   = ../../api ../../extlibs/hdf5-1.8.3/src ../../extlibs/geotrans ../../extlibs/beecrypt ../../extlibs ../extlibs/xerces-c-src_2_6_0/include 
}
unix {
        XERCESCROOT = ../extlibs/xerces-c-src_2_6_0
	INC_OPENNSALL   = ../../api ../../extlibs/hdf5-1.8.3/src $(XERCESROOT)/src ../../extlibs/geotrans ../../extlibs/beecrypt-4.1.2 ../../extlibs
}

#INC_OPENNSALL   = ../api ../extlibs/hdf5 ../extlibs/geotrans ../extlibs/beecrypt ../extlibs
