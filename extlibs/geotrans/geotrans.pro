###############################################################################
#                OpenNS Application Build Project (v1.0)                      #
###############################################################################
#                                                                             
###                       DO NOT EDIT THIS SECTION                          
#                                                                             
### Retrieve the application master project which contains shared OpenNS definations,
### includes, and other parameters to support platform independent building.
#
include (../libmaster.pro)
#
INCLUDEPATH += $$INC_OPENNSALL
###############################################################################
#
###                  LIBRARY SPECIFIC SECTION (Edit Here)                          
#

INCLUDEPATH += azeq bng gnomonic neys nzmg omerc stereogr
INCLUDEPATH += geocent georef mercator datum ellipse tranmerc utm polarst
INCLUDEPATH += ups mgrs lambert bonne cassini cyleqa eqdcyl eckert4 eckert6
INCLUDEPATH += miller mollweid orthogr polycon sinusoid trcyleqa grinten
INCLUDEPATH += geoid albers loccart

win32 {
	DEFINES         += WIN32
}

HEADERS += albers/albers.h \
        azeq/azeq.h \
        bng/bng.h \
        bonne/bonne.h \
        cassini/cassini.h \
        cyleqa/cyleqa.h \
        datum/datum.h \
        eckert4/eckert4.h \
        eckert6/eckert6.h \
        ellipse/ellipse.h \
        eqdcyl/eqdcyl.h \
        geocent/geocent.h \
        geoid/geoid.h \
        georef/georef.h \
        gnomonic/gnomonic.h \
        grinten/grinten.h \
        lambert/lambert.h \
        loccart/loccart.h \
        mercator/mercator.h \
        mgrs/mgrs.h \
        miller/miller.h \
        mollweid/mollweid.h \
        neys/neys.h \
        nzmg/nzmg.h \
        omerc/omerc.h \
        orthogr/orthogr.h \
        polarst/polarst.h \
        polycon/polycon.h \
        sinusoid/sinusoid.h \
        stereogr/stereogr.h \
        tranmerc/tranmerc.h \
        trcyleqa/trcyleqa.h \
        ups/ups.h \
        utm/utm.h \
        engine.h
                
SOURCES += albers/albers.c \
           azeq/azeq.c \
           bng/bng.c \
           bonne/bonne.c \
           cassini/cassini.c \
           cyleqa/cyleqa.c \
           datum/datum.c \
           eckert4/eckert4.c \
           eckert6/eckert6.c \
           ellipse/ellipse.c \
           eqdcyl/eqdcyl.c \
           geocent/geocent.c \
           geoid/geoid.c \
           georef/georef.c \
           gnomonic/gnomonic.c \
           grinten/grinten.c \
           lambert/lambert.c \
           loccart/loccart.c \
           mercator/mercator.c \
           mgrs/mgrs.c \
           miller/miller.c \
           mollweid/mollweid.c \
           neys/neys.c \
           nzmg/nzmg.c \
           omerc/omerc.c \
           orthogr/orthogr.c \
           polarst/polarst.c \
           polycon/polycon.c \
           sinusoid/sinusoid.c \
           stereogr/stereogr.c \
           tranmerc/tranmerc.c \
           trcyleqa/trcyleqa.c \
           ups/ups.c \
           utm/utm.c \
           engine.c
           
           
           
           
           
           
