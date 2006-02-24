####
# Compiler declarations
####

#Sun SPARCworks
#cc = cc
#CC = CC
#Gnu
cc = gcc
CC = g++

####
# Support declarations
####

DEBUG = -g

CFLAGS = $(DEBUG)

####
# Directories
####

DTCCDIR = ..

####
# Libraries
####

LIB =  libdtcc.a

####
# Sources
####

SRCS = \
	albers.c \
	azeq.c \
	bonne.c \
	bng.c \
	cassini.c \
	cyleqa.c \
	datum.c \
	eckert4.c \
	eckert6.c \
	ellipse.c \
	eqdcyl.c \
	geocent.c \
	geoid.c \
	georef.c \
	gnomonic.c \
	grinten.c \
        lambert.c \
	loccart.c \
	mercator.c \
	mgrs.c \
	miller.c \
	mollweid.c \
	neys.c \
	nzmg.c \
	omerc.c \
	orthogr.c \
	polarst.c \
	polycon.c \
	sinusoid.c \
	stereogr.c \
	trcyleqa.c \
	tranmerc.c \
	ups.c \
	utm.c
	
####
# Includes
####

INCLUDES = \
	-I$(DTCCDIR)/albers \
	-I$(DTCCDIR)/azeq \
	-I$(DTCCDIR)/bonne \
	-I$(DTCCDIR)/bng \
	-I$(DTCCDIR)/cassini \
	-I$(DTCCDIR)/cyleqa \
	-I$(DTCCDIR)/datum \
	-I$(DTCCDIR)/eckert4 \
	-I$(DTCCDIR)/eckert6 \
	-I$(DTCCDIR)/ellipse \
	-I$(DTCCDIR)/eqdcyl \
	-I$(DTCCDIR)/geocent \
	-I$(DTCCDIR)/geoid \
	-I$(DTCCDIR)/georef \
	-I$(DTCCDIR)/gnomonic \
	-I$(DTCCDIR)/grinten \
	-I$(DTCCDIR)/lambert \
	-I$(DTCCDIR)/loccart \
	-I$(DTCCDIR)/mercator \
	-I$(DTCCDIR)/mgrs \
	-I$(DTCCDIR)/miller \
	-I$(DTCCDIR)/mollweid \
	-I$(DTCCDIR)/neys \
	-I$(DTCCDIR)/nzmg \
	-I$(DTCCDIR)/omerc \
	-I$(DTCCDIR)/orthogr \
	-I$(DTCCDIR)/polarst \
	-I$(DTCCDIR)/polycon \
	-I$(DTCCDIR)/sinusoid \
	-I$(DTCCDIR)/stereogr \
	-I$(DTCCDIR)/trcyleqa \
	-I$(DTCCDIR)/tranmerc \
	-I$(DTCCDIR)/ups \
	-I$(DTCCDIR)/utm

####
# Suffix rules
####

.SUFFIXES: .c .o

.c.o:
	$(cc) -c $(CFLAGS) $(INCLUDES) $<

####
# Targets
####

all : $(LIB)

$(SRCS):
	ln -s $(DTCCDIR)/$(@:.c=)/$@ .

$(LIB): $(SRCS:.c=.o)
	ar -rv $(LIB) $(SRCS:.c=.o)

clean:
	$(RM) *.c *.o *.a
