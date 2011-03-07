OS := $(shell uname)

ifeq ($(OS),Linux)

    CC = gcc

    LINKER = gcc

    OBJS = bag_xml_meta.o

    .c.o:
	$(CC) $(CFLAGS) $*.c

    TGT = libbagxml.so

    MAKEFILE = Makefile

    CFLAGS = -fPIC -O -ansi -Wall -c -DNVLinux -I $(PFM_INCLUDE)
    CXXFLAGS = -fPIC -O -ansi -Wall -DNVLinux -I $(PFM_INCLUDE)

    LINK_FLAGS = -shared -fPIC -Wl,-soname,$(TGT) -o $(TGT)

all: $(TGT)
{-c $(LINKER) $(LINK_FLAGS)} $(TGT) : $(OBJS) $(MAKEFILE)
	$(LINKER) $(LINK_FLAGS) $(OBJS)
	rm -f *~
	mv $(TGT) $(PFM_LIB)
	cp bag_xml_meta.h bag_xml_meta.hpp $(PFM_INCLUDE)


else


    ARFLAGS=r

    CC = gcc

    OS := $(shell uname)

    CFLAGS = -ansi -O -Wall -c -DXML_LIBRARY -DNVWIN3X -I $(PFM_INCLUDE)
    CXXFLAGS = -ansi -Wall -O -DXML_LIBRARY -DNVWIN3X -I $(PFM_INCLUDE)


    .c.o:
	$(CC) -c $(CFLAGS) -I$(PFM_INCLUDE) $*.c

    .cpp.o:
	$(CXX) -c $(CXXFLAGS) -I$(PFM_INCLUDE) -o $@ $<


    TGT = libbagxml.a


    $(TGT):	$(TGT)(bag_xml_meta.o)
	rm -f *~
	cp $(TGT) $(PFM_LIB)
	rm $(TGT)
	cp bag_xml_meta.h bag_xml_meta.hpp $(PFM_INCLUDE)

endif


bag_xml_meta.o : 	bag_xml_meta.hpp bag_xml_meta.h
