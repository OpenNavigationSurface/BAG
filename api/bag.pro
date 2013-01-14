###############################################################################
#                OpenNS Application Build Project (v1.5)                      #
###############################################################################
#                                                                             
###                       DO NOT EDIT THIS SECTION                          
#                                                                             
### Retrieve the application master project which contains shared OpenNS definitions,
### includes, and other parameters to support platform independent building.
#
include (libmaster.pro)
#
INCLUDEPATH += $$INC_OPENNSALL
###############################################################################
#
###                  LIBRARY SPECIFIC SECTION (Edit Here)                          
#

TARGET = libbag

CONFIG -= qt
CONFIG += 

unix {
         VERSION=1.5.0
}

QMAKE_CFLAGS_DEBUG = 
QMAKE_CXXFLAGS_DEBUG = $$QMAKE_CFLAGS_DEBUG

HEADERS += stdtypes.h \
        crc32.h \
        onscrypto.h \
        ons_xml.h \
        ons_xml_error_handler.h \
	bag_metadata_def.h	\
	bag_xml_meta.hpp	\
	bag_private.h \
        bag.h
        
SOURCES += crc32.c \
        onscrypto.c \
        ons_xml.cpp \
        bag_xml_meta.cpp \
        bag_attr.c \
        bag_crypto.c \
        bag_surface_correct.c \
        bag_reference_system.cpp \
        bag_hdf.c  \
        bag_epsg.c  \
        bag_surfaces.c \
        bag_opt_group.c \
        bag_opt_surfaces.c \
        bag_tracking_list.c \
        bag_xml.c
           
