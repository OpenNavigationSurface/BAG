###############################################################################
#                      OpenNS Library Master Project                          #
# Version 1.0                                                                 #
###############################################################################
#                                                                             #                                                                       
# This file contains core definitions that all OpenNS libraries will need     #
# to in order to create the platform independent Makefiles/MS Projecs. Be     #
# careful editing this file unless you know what you are doing.               #
#                                                                             #
###############################################################################

# Setup the OpenNS software development includes. Normally application projects 
# will just need to include INC_IVSALL.

# include (libincludes.pro)
INC_OPENNSALL   = ../../extlibs

# Define the basic template for the application
win32 {
	TEMPLATE = lib
}
unix {
	TEMPLATE = lib
}

# Setup shared definitions across all platforms
LANGUAGE = C++
CONFIG  -= qt

QMAKE_CFLAGS_DEBUG = 
QMAKE_CXXFLAGS_DEBUG = $$QMAKE_CFLAGS_DEBUG

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
}


unix {
	message( In libmaster.pro for the general unix platform )
    CONFIG += shared
	OBJECTS_DIR = .obj/$(HOSTMACHINE)
	QMAKE_CFLAGS += ${IVSCOPTS}  -Wall
	QMAKE_CXXFLAGS += ${IVSCCOPTS2}  -Wall
	DESTDIR = ../lib/$(HOSTMACHINE)/
    macx {
	    message( In libmaster.pro for the Macintosh platform )
    	DEFINES += _SYS_TIMESPEC_H
    }
}

