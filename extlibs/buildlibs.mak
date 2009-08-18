#/bin/bash
#################################
# Linux build script for external libraries
# Webb McDonald -- Sat Mar 18 15:02:43 2006
#################################

export ECHO='echo -e'
export UNTAR='tar -xvzf'
export CP='cp -f'
export CD='cd'

if [ ! -n "$PWD" ] ; then
	$ECHO "PWD env variable expected to be set, trying CWD instead."
	export PWD=$CWD
fi

export XERCESCROOT=$PWD/xerces-c-src_2_6_0
export BEECRYPT_LIB=beecrypt-4.1.2
export HDF5_LIB=hdf5-1.8.3
export SZIP_LIB=szip
export ZLIB_LIB=zlib


#################################
# SZIP library build
#################################
$ECHO "Checking for $SZIP_LIB"
if [ ! -d $SZIP_LIB ] ; then
	$UNTAR $SZIP_LIB.tgz
else
	$ECHO "$SZIP_LIB  exists"
fi
if [ ! -e lib/$HOSTMACHINE/libszip.so ] ; then
	$ECHO "Running make for $SZIP_LIB"
	$CD $SZIP_LIB
	make
	$CD ..

	if [ ! -e lib/$HOSTMACHINE/libszip.so ] ; then
		$ECHO "FAILED to build libszip.so"
		exit
	else
		$ECHO "built lib/$HOSTMACHINE/libszip.so successfully"
	fi
else
	$ECHO "lib/$HOSTMACHINE/libszip.so  exists"
fi

#################################
#  ZLIB library build
#################################
$ECHO "Checking for $ZLIB_LIB"
if [ ! -d $ZLIB_LIB ] ; then
	$UNTAR $ZLIB_LIB.tgz
else
	$ECHO "$ZLIB_LIB  exists"
fi
if [ ! -e lib/$HOSTMACHINE/libzlib.so ] ; then
	$ECHO "Running make for $ZLIB_LIB"
	$CD $ZLIB_LIB
	make
	$CD ..

	if [ ! -e lib/$HOSTMACHINE/libzlib.so ] ; then
		$ECHO "FAILED to build libzlib.so"
		exit
	else
		$ECHO "built lib/$HOSTMACHINE/libzlib.so successfully"
	fi
else
	$ECHO "lib/$HOSTMACHINE/libzlib.so  exists"
fi

#################################
#  GEOTRANS library build
#################################
#$ECHO "Checking for $GEOTRANS_LIB"
#if [ ! -d $GEOTRANS_LIB ] ; then
#	$UNTAR $GEOTRANS_LIB.tgz
#else
#	$ECHO "$GEOTRANS_LIB  exists"
#fi
#if [ ! -e lib/$HOSTMACHINE/libgeotrans.so ] ; then
#	$ECHO "Running make for $GEOTRANS_LIB"
#	$CD $GEOTRANS_LIB
#	make
#	$CD ..
#	if [ ! -e lib/$HOSTMACHINE/libgeotrans.so ] ; then
#		$ECHO "FAILED to build libgeotrans.so"
#		exit
#	else
#		$ECHO "built lib/$HOSTMACHINE/libgeotrans.so successfully"
#	fi
#else
#	$ECHO "lib/$HOSTMACHINE/libgeotrans.so  exists"
#fi	
#
#################################
#  BEECRYPT library build
#################################
$ECHO "Checking for $BEECRYPT_LIB"
if [ ! -d $BEECRYPT_LIB ] ; then
	$UNTAR $BEECRYPT_LIB.tar.gz
else
	$ECHO "$BEECRYPT_LIB  exists"
fi
if [ ! -e lib/$HOSTMACHINE/libbeecrypt.so ] ; then
	$ECHO "Configuring and running make for $BEECRYPT_LIB"
	$CD $BEECRYPT_LIB
	configure
	make
	$CP .libs/libbeecrypt.so* ../lib/$HOSTMACHINE
	$CD ..
	if [ ! -e lib/$HOSTMACHINE/libbeecrypt.so ] ; then
		$ECHO "FAILED to build libbeecrypt.so"
		exit
	else
		$ECHO "built lib/$HOSTMACHINE/libbeecrypt.so successfully"
	fi
else
	$ECHO "lib/$HOSTMACHINE/libbeecrypt.so  exists"
fi	

#################################
#  HDF5 library build
#################################
$ECHO "Checking for $HDF5_LIB"
if [ ! -d $HDF5_LIB ] ; then
	$UNTAR $HDF5_LIB.tar.gz
else
	$ECHO "$HDF5_LIB  exists"
fi
if [ ! -e lib/$HOSTMACHINE/libhdf5.so ] ; then
	$ECHO "Configuring and running gmake for $HDF5_LIB"
	$CD $HDF5_LIB
	configure
	gmake
	$CP src/.libs/libhdf5.so* ../lib/$HOSTMACHINE
	$CD ..
	if [ ! -e lib/$HOSTMACHINE/libhdf5.so ] ; then
		$ECHO "FAILED to build libhdf5.so"
		exit
	else
		$ECHO "built lib/$HOSTMACHINE/libhdf5.so successfully"
	fi	
else
	$ECHO "lib/$HOSTMACHINE/libhdf5.so  exists"
fi

#################################
#  XERCESC library build
#################################
$ECHO "Checking for $XERCESCROOT"
if [ ! -d $XERCESCROOT/src/xercesc ] ; then
	$UNTAR $XERCESCROOT.tar.gz
else
	$ECHO "$XERCESCROOT  exists"
fi
if [ ! -e lib/$HOSTMACHINE/libxerces-c.so ] ; then
	$ECHO "Configuring and running gmake for $XERCESCROOT"
	$CD $XERCESCROOT/src/xercesc
	autoconf
	runConfigure -plinux -cgcc -xg++ -minmem -nsocket -tnative -rpthread
	gmake
	$CD $XERCESCROOT/..
	$CP $XERCESCROOT/lib/libxerces-c.so* lib/$HOSTMACHINE
	if [ ! -e lib/$HOSTMACHINE/libxerces-c.so ] ; then
		$ECHO "FAILED to build libxerces-c.so"
		exit
	else
		$ECHO "built lib/$HOSTMACHINE/libxerces-c.so successfully"
	fi	
else
	$ECHO "lib/$HOSTMACHINE/libxerces-c.so  exists"
fi
