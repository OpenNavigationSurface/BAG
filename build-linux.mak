#/bin/bash
#################################
# Sample master build/test script
# Webb McDonald -- Sun Mar 19 10:50:44 2006
#################################

export ECHO='echo -e'
export UNTAR='tar -xvzf'
export CP='cp -f'
export CD='cd'

if [ ! -n "$PWD" ] ; then
	$ECHO "PWD env variable expected to be set, trying CWD instead."
	export PWD=$CWD
fi

##################################################################
# if developing with libbag, you might want to add these permanently:
##################################################################
export LIBRARY_PATH=$LIBRARY_PATH:$PWD/lib/$HOSTMACHINE:$PWD/extlibs/lib/$HOSTMACHINE
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PWD/lib/$HOSTMACHINE:$PWD/extlibs/lib/$HOSTMACHINE
export LD_RUN_PATH=$LD_RUN_PATH:$PWD/lib/$HOSTMACHINE:$PWD/extlibs/lib/$HOSTMACHINE
export BAG_HOME=$PWD/configdata
##################################################################

export XERCESCROOT=$PWD/extlibs/xerces-c-src_2_6_0


$ECHO "\nstarting $0..."

$ECHO "Begin building of the external libraries if necessary..."
$CD extlibs; buildlibs.mak

$ECHO "External libs complete.\n Compiling libbag.so from the api/ sources..."
$CD ../api;  make

if [ -e ../lib/$HOSTMACHINE/libbag.so ] ; then
	$ECHO "libbag.so exists"
else
	$ECHO "FAILED to build libbag.so"
	exit
fi

$ECHO "Building the examples programs..."
$CD ../examples/bagcreate; make
$CD ../bagread; make
$CD ../bin/$HOSTMACHINE

if [ -e bagread ] && [ -e bagcreate ] ; then
	$ECHO "Bag example programs exist"
else
	$ECHO "\n\tFAILED to build example programs\n"
	exit
fi

$ECHO "\n\t$0 completed succesfully, exiting.\n"
