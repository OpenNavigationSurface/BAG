###############################################################################
#                   IVS Application Build Project (v1.0)                      #
###############################################################################
#                                                                             
###                       DO NOT EDIT THIS SECTION                          
#                                                                             
### Retrieve the application master project which contains shared IVS definations,
### includes, and other parameters to support platform independent building.
#
include (../libmaster.pro)
#
INCLUDEPATH += $$INC_OPENNSALL
###############################################################################
#
###                  LIBRARY SPECIFIC SECTION (Edit Here)                          
#

CONFIG += debug

QMAKE_CFLAGS_DEBUG = 
QMAKE_CXXFLAGS_DEBUG = $$QMAKE_CFLAGS_DEBUG

HEADERS += deflate.h \
        trees.h \
        zutil.h \
        infblock.h \
        inftrees.h \
        infcodes.h \
        infutil.h \
        inffast.h \
        inffixed.h \
        zconf.h
        
SOURCES += adler32.c \
           compress.c \
           crc32.c \
           gzio.c \
           uncompr.c \
           deflate.c \
           trees.c \
           zutil.c \
           inflate.c \
           infblock.c \
           inftrees.c \
           infcodes.c \
           infutil.c \
           inffast.c
