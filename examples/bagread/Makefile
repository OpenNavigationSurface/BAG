all: build

build: Makefile_$(HOSTMACHINE)
	$(MAKE) -f Makefile_$(HOSTMACHINE)

Makefile_$(HOSTMACHINE): bagread.pro
	qmake -o Makefile_$(HOSTMACHINE) bagread.pro

clean: Makefile_$(HOSTMACHINE)
	$(MAKE) -f Makefile_$(HOSTMACHINE) clean
	rm Makefile_$(HOSTMACHINE)
