all: build

build: Makefile_$(HOSTMACHINE)
	$(MAKE) -f Makefile_$(HOSTMACHINE)

Makefile_$(HOSTMACHINE): bag.pro
	qmake -o Makefile_$(HOSTMACHINE) bag.pro

clean: Makefile_$(HOSTMACHINE)
	$(MAKE) -f Makefile_$(HOSTMACHINE) clean
	rm Makefile_$(HOSTMACHINE)
