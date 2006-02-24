
# Setup the OpenNS software development includes. Normally application projects 
# will just need to include INC_OPENNSALL.
win32 {
	INC_OPENNSALL   = ../../api ../../api/crypto ../../extlibs/hdf5 ../../extlibs/geotrans ../../extlibs/beecrypt ../../extlibs
}
unix {
	INC_OPENNSALL   = ../../api ../../api/crypto ../../extlibs/hdf5-linux/include ../../extlibs/geotrans ../../extlibs/beecrypt ../../extlibs
}

#INC_OPENNSALL   = ../api ../extlibs/hdf5 ../extlibs/geotrans ../extlibs/beecrypt ../extlibs
