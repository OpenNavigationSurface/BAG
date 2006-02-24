#                                                                             
###                       DO NOT EDIT THIS SECTION                          
#                                                                             
### Retrieve the application master project which contains shared definations,
### includes, and other parameters to support platform independent building.
#
include (../appmaster.pro)
#
# Create the master include path
INCLUDEPATH += $$INC_OPENNSALL ..\..\excertlib
###############################################################################
#
###                  APPLICATION SPECIFIC SECTION (Edit Here)    
#             

INCLUDEPATH  += ../excertlib

CONFIG += console debug
CONFIG -= qt

DEFINES += 

SOURCES	+= signfile.c

HEADERS	+= 

OPENNSLIBS += bag.lib hdf5.lib beecrypt.lib szip.lib zlib.lib excert.lib

unix {
	QMAKE_LIBDIR += ../excertlib/lib/${HOSTMACHINE}
	LIBS	+= $$SYSOBJFILES $$OPENNSLIBS $EXTRAUNIXLIBS
}

win32 {
	QMAKE_LIBDIR += ../excertlib/lib/Debug
	LIBS	+= $$OPENNSLIBS $$EXTRAWINLIBS haspw32.lib winmm.lib libxml2.lib iconv.lib
	SOURCES += $$SYSOBJFILES
	QMAKE_LFLAGS_WINDOWS += /nodefaultlib:"libcmt.lib"
}
