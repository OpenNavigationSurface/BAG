###############################################################################
#                    OpenNS Application Master Project                        #
# Version 1.0                                                                 #
###############################################################################
#                                                                             #                                                                       
# This file contains core definitions that all OpenNS applications will need  #
# to in order to create the platform independent Makefiles. Be careful        #
# editing this file unless you know what you are doing.                       #
#                                                                             #
###############################################################################

# Setup the OpenNS software development includes. Normally application projects 
# will just need to include INC_OPENNSALL.

# one way or the another...!
# Webb McDonald -- Mon Mar 27 14:36:23 2006
include (libincludes.pro)
include (../libincludes.pro)

# Define the basic template for the application
win32 {
	TEMPLATE = vcapp
}
unix {
	TEMPLATE = app
}

# Setup shared definitions across all platforms
DEFINES	+= 
LANGUAGE = C++
CONFIG	+= warn_off 
CONFIG  -= qt

win32 {
	message( appmaster.pro - WIN32 --> Setting up basic windows parameters )
	# The defines should define and IVS OS, GUI, and Hardware platform among others
	DEFINES	+= WIN32 _WINDOWS _MBCS PLATFORM_WIN32
	OBJECTS_DIR = Debug
	QMAKE_LIBDIR = ../../extlibs/lib/Debug ../../api/lib/debug
	DESTDIR = ../bin/debug	
        SYSOBJFILES=
}

#win32:debug {
#	CONFIG += console
#}

unix {
	message( In appmaster.pro for the general unix platform )
	OBJECTS_DIR = .obj/$(HOSTMACHINE)
	DESTDIR = ../bin/$(HOSTMACHINE)/
	QMAKE_CFLAGS += ${IVSCOPTS} 
	QMAKE_CXXFLAGS += ${IVSCCOPTS2} 
        EXTLIBDIR= ../../extlibs/lib/$(HOSTMACHINE) ../../lib/$(HOSTMACHINE)
        SYSOBJFILES= 
	QMAKE_LIBDIR += ../../libsrc/lib/$(HOSTMACHINE) $$EXTLIBDIR
    macx {
	    message( In appmaster.pro for the Macintosh platform )
	    DEFINES += IVSMACXOS IVSMAC _SYS_TIMESPEC_H
    }
}

###
###                   SETUP LIBRARY DEFINATIONS
### Don't Edit this unless you absolutely know what you are doing as
### there are many subtle library dependencies especially on the UNIX
### platforms. If you are not sure please ask someone before making changes.
###
unix {
	EXTRAUNIXLIBS   = ${IVSEXTRAUNIXLIBS}
	OPENNSLIBS      = 
#	LIBS	+= $$SYSOBJFILES -lsdatb -lsd -ltargets -lpolygon $$IVSSFLIBS $$IVSRASTERGLLIBS -lqtkit $$IVSLICLIBS $$IVSSVGLLIBS $$IVSSYSLIBS
    macx {
	    LIBS += -framework IOKit -framework CoreFoundation -framework ApplicationServices
    }
}
win32 {
	EXTRAWINLIBS    = netapi32.lib comctl32.lib ws2_32.lib
	OPENNSLIBS      = 
#	OPENNSLIBS      = bag.lib XercesLib.lib
}
