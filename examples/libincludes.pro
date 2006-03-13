
# Setup the OpenNS software development includes. Normally application projects 
# will just need to include INC_OPENNSALL.
win32 {
	INC_OPENNSALL   = ../../api ../../extlibs/hdf5 ../../extlibs/geotrans ../../extlibs/beecrypt ../../extlibs
}
unix {
	INC_OPENNSALL   = ../../api ../../extlibs/hdf5-1.6.1/src ../../extlibs/geotrans ../../extlibs/beecrypt-4.1.2 ../../extlibs
}

#INC_OPENNSALL   = ../api ../extlibs/hdf5 ../extlibs/geotrans ../extlibs/beecrypt ../extlibs
