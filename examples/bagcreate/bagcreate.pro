#                                                                             
###                       DO NOT EDIT THIS SECTION                          
#                                                                             
### Retrieve the application master project which contains shared definations,
### includes, and other parameters to support platform independent building.
#
include (../appmaster.pro)
#
# Create the master include path
INCLUDEPATH += $$INC_OPENNSALL
###############################################################################
#
###                  APPLICATION SPECIFIC SECTION (Edit Here)    
#             

CONFIG += console

DEFINES += BEE_STATIC 

SOURCES	+= bagcreate.c

HEADERS	+= 

win32 {
	OPENNSLIBS += bag.lib hdf5.lib xercesLib.lib beecrypt.lib szip.lib zlib.lib
}

unix {
	OPENNSLIBS += -lbag -lhdf5 -lxerces-c -lgeotrans -lbeecrypt -lszip -lzlib -lpthread
	LIBS	+= $$SYSOBJFILES $$OPENNSLIBS $$EXTRAUNIXLIBS
}

win32 {
	LIBS	+= $$OPENNSLIBS $$EXTRAWINLIBS
	SOURCES += $$SYSOBJFILES
	QMAKE_LFLAGS_WINDOWS += /nodefaultlib:"libcmt.lib"
}

