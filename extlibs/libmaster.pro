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

include (libincludes.pro)

# Define the basic template for the application
win32 {
	TEMPLATE = vclib
}
unix {
	TEMPLATE = lib
}

# Setup shared definitions across all platforms
DEFINES	+= 
LANGUAGE = C++
CONFIG	+= debug
CONFIG  -= qt

QMAKE_CFLAGS_DEBUG = 
QMAKE_CXXFLAGS_DEBUG = $$QMAKE_CFLAGS_DEBUG

win32 {
	message( libmaster.pro - WIN32 --> Setting up basic windows parameters )
	# The defines should define and IVS OS, GUI, and Hardware platform among others
	DEFINES	+= WIN32 _WINDOWS _MBCS
	OBJECTS_DIR = Debug
	QMAKE_LIBDIR = ../../libsrc/lib/debug
	DESTDIR = ../lib/debug	
}

#win32:debug {
#	CONFIG += console
#}

unix {
	message( In libmaster.pro for the general unix platform )
	OBJECTS_DIR = .obj/$(HOSTMACHINE)
	QMAKE_CFLAGS += ${IVSCOPTS}  -Wall
	QMAKE_CXXFLAGS += ${IVSCCOPTS2}  -Wall
	DESTDIR = ../lib/$(HOSTMACHINE)/
    macx {
	    message( In libmaster.pro for the Macintosh platform )
    	DEFINES += _SYS_TIMESPEC_H
    }
}

