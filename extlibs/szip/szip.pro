###############################################################################
#                OpenNS Application Build Project (v1.0)                      #
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

HEADERS += rice.h \
        SZconfig.h \
        ricehdf.h \
        szip_adpt.h \
        szlib.h
        
SOURCES += encoding.c \
           rice.c \
           sz_api.c
           
