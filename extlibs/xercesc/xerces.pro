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
INCLUDEPATH += $$INC_IVSALL
###############################################################################
#
###                  LIBRARY SPECIFIC SECTION (Edit Here)                          
#

DEFINES += _CRTDBG_MAP_ALLOC PROJ_SAX2
DEFINES += PROJ_XMLPARSER PROJ_XMLUTIL PROJ_PARSERS PROJ_SAX4C PROJ_DOM
DEFINES += PROJ_VALIDATORS XML_SINGLEDLL XML_USE_WIN32_TRANSCODER 
DEFINES += XML_USE_WIN32_MSGLOADER XML_USE_NETACCESSOR_WINSOCK

# _DEBUG
# WIN32
#_WINDOWS


HEADERS += deflate.h \
        
SOURCES += adler32.c \
