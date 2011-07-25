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

	TEMPLATE = lib

# Define the basic template for the application
win32 {
    CONFIG -= embed_manifest_exe
    CONFIG -= embed_manifest_dll
}

# Setup shared definitions across all platforms
DEFINES	+= H5_USE_16_API
LANGUAGE = C++
CONFIG  -= qt
CONFIG  += exceptions warn_off

win32 {
	message( libmaster.pro - WIN32 --> Setting up basic windows parameters )
	# The defines should define and GUI, and Hardware platform among others
	DEFINES	+= WIN32 _WINDOWS _MBCS
	OBJECTS_DIR = .obj/$(HOSTMACHINE)
	QMAKE_CFLAGS += -ansi ${IVSCOPTS} 
	QMAKE_CXXFLAGS += -ansi ${IVSCCOPTS2} 
	OPENNSLIBS += -lhdf5 -lxerces-c -lbeecrypt -lszip -lzlib
	LIBS	+= $$SYSOBJFILES $$OPENNSLIBS
    DESTDIR = ../lib/$(HOSTMACHINE)/
}

unix {
	message( In libmaster.pro for the general unix platform )
#	MOC_DIR = .moc/$(HOSTMACHINE)
	OBJECTS_DIR = .obj/$(HOSTMACHINE)
	QMAKE_CFLAGS += -ansi ${IVSCOPTS} 
	QMAKE_CXXFLAGS += -ansi ${IVSCCOPTS2} 
	OPENNSLIBS += -lhdf5 -lxerces-c -lbeecrypt -lszip -lzlib -lpthread
	LIBS	+= $$SYSOBJFILES $$OPENNSLIBS $$EXTRAUNIXLIBS
    DESTDIR = ../lib/$(HOSTMACHINE)/
    macx {
	    message( In libmaster.pro for the Macintosh platform )
    	DEFINES += _SYS_TIMESPEC_H
    }
}

