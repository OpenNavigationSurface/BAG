all: build

build: Makefile_$(HOSTMACHINE)
	$(MAKE) -f Makefile_$(HOSTMACHINE)

Makefile_$(HOSTMACHINE): bagcreate.pro
	qmake -o Makefile_$(HOSTMACHINE) bagcreate.pro

clean: Makefile_$(HOSTMACHINE)
	$(MAKE) -f Makefile_$(HOSTMACHINE) clean
	rm Makefile_$(HOSTMACHINE)
