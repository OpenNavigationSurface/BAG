#                                                                             
###                       DO NOT EDIT THIS SECTION                          
#                                                                             
### Retrieve the application master project which contains shared definations,
### includes, and other parameters to support platform independent building.
#
#
# Create the master include path
INCLUDEPATH += $$INC_OPENNSALL
###############################################################################
#
###                  LIBRARY SPECIFIC SECTION (Edit Here)    
#             

# Define the basic template for the application
win32 {
	TEMPLATE = vclib
}
unix {
	TEMPLATE = lib
}

DEFINES -= UNICODE

CONFIG -= qt

win32 {
	message( libmaster.pro - WIN32 --> Setting up basic windows parameters )
	# The defines should define the OS, GUI, and Hardware platform among others
	DEFINES	+= WIN32 _WINDOWS _MBCS
    CONFIG += warn_off
    CONFIG -= QT_DLL
    CONFIG(debug, debug|release) {
            message( libmaster.pro - WIN32 --> Setting lib\debug directory )
    	DESTDIR = ../../lib/Debug/
	    OBJECTS_DIR = ./DebugObj/
    } else {
            message( libmaster.pro - WIN32 --> setting lib\release directory )
    	DESTDIR = ../../lib/Release/
	    OBJECTS_DIR = ./ReleaseObj/
    }
    CONFIG += staticlib
}

unix {
	message( In libmaster.pro for the general unix platform )
#	MOC_DIR = .moc/$(HOSTMACHINE)
	OBJECTS_DIR = .obj/$(HOSTMACHINE)
	QMAKE_CFLAGS += ${IVSCOPTS} 
	QMAKE_CXXFLAGS += ${IVSCCOPTS2} 
	DESTDIR = ./lib/$(HOSTMACHINE)/
    macx {
	    message( In libmaster.pro for the Macintosh platform )
    	DEFINES += _SYS_TIMESPEC_H
    }
}

INCLUDEPATH  += ../../extlibs ../../api

DEFINES += 

SOURCES	+= excertlib.c getopt.c

HEADERS	+= excertlib.h getopt.h
