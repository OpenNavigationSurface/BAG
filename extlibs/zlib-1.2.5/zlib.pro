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

win32 {
    CONFIG += staticlib
}

HEADERS += crc32.h \
        deflate.h \
        gzguts.h \
        inffast.h \
        inffixed.h \
        inflate.h \
        inftrees.h \
        trees.h \
        zconf.h \
        zlib.h \
        zutil.h
        
SOURCES += adler32.c \
           compress.c \
           crc32.c \
           deflate.c \
           gzclose.c \
           gzlib.c \
           gzread.c \
           gzwrite.c \
           infback.c \
           inffast.c \
           inflate.c \
           inftrees.c \
           trees.c \
           uncompr.c \
           zutil.c
