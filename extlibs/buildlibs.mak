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
export GEOTRANS_LIB=geotrans
export BEECRYPT_LIB=beecrypt-4.1.2
export HDF5_LIB=hdf5-1.6.1
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
if [ ! -e lib/libszip.so ] ; then
	$ECHO "Running make for $SZIP_LIB"
	$CD $SZIP_LIB
	make
	$CD ..

	if [ ! -e lib/libszip.so ] ; then
		$ECHO "FAILED to build libszip.so"
		exit
	else
		$ECHO "built lib/libszip.so successfully"
	fi
else
	$ECHO "lib/libszip.so  exists"
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
if [ ! -e lib/libzlib.so ] ; then
	$ECHO "Running make for $ZLIB_LIB"
	$CD $ZLIB_LIB
	make
	$CD ..

	if [ ! -e lib/libzlib.so ] ; then
		$ECHO "FAILED to build libzlib.so"
		exit
	else
		$ECHO "built lib/libzlib.so successfully"
	fi
else
	$ECHO "lib/libzlib.so  exists"
fi

#################################
#  GEOTRANS library build
#################################
$ECHO "Checking for $GEOTRANS_LIB"
if [ ! -d $GEOTRANS_LIB ] ; then
	$UNTAR $GEOTRANS_LIB.tgz
else
	$ECHO "$GEOTRANS_LIB  exists"
fi
if [ ! -e lib/libgeotrans.so ] ; then
	$ECHO "Running make for $GEOTRANS_LIB"
	$CD $GEOTRANS_LIB
	make
	$CD ..
	if [ ! -e lib/libgeotrans.so ] ; then
		$ECHO "FAILED to build libgeotrans.so"
		exit
	else
		$ECHO "built lib/libgeotrans.so successfully"
	fi
else
	$ECHO "lib/libgeotrans.so  exists"
fi	

#################################
#  BEECRYPT library build
#################################
$ECHO "Checking for $BEECRYPT_LIB"
if [ ! -d $BEECRYPT_LIB ] ; then
	$UNTAR $BEECRYPT_LIB.tar.gz
else
	$ECHO "$BEECRYPT_LIB  exists"
fi
if [ ! -e lib/libbeecrypt.so ] ; then
	$ECHO "Configuring and running make for $BEECRYPT_LIB"
	$CD $BEECRYPT_LIB
	configure
	make
	$CP .libs/libbeecrypt.so* ../lib
	$CD ..
	if [ ! -e lib/libbeecrypt.so ] ; then
		$ECHO "FAILED to build libbeecrypt.so"
		exit
	else
		$ECHO "built lib/libbeecrypt.so successfully"
	fi
else
	$ECHO "lib/libbeecrypt.so  exists"
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
if [ ! -e lib/libhdf5.so ] ; then
	$ECHO "Configuring and running gmake for $HDF5_LIB"
	$CD $HDF5_LIB
	configure
	gmake
	make install
	$CP src/.libs/libhdf5.so* ../lib
	$CD ..
	if [ ! -e lib/libhdf5.so ] ; then
		$ECHO "FAILED to build libhdf5.so"
		exit
	else
		$ECHO "built lib/libhdf5.so successfully"
	fi	
else
	$ECHO "lib/libhdf5.so  exists"
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
if [ ! -e lib/libxerces-c.so ] ; then
	$ECHO "Configuring and running gmake for $XERCESCROOT"
	$CD $XERCESCROOT/src/xercesc
	autoconf
	runConfigure -plinux -cgcc -xg++ -minmem -nsocket -tnative -rpthread
	gmake
	$CD $XERCESCROOT/..
	$CP $XERCESCROOT/lib/libxerces-c.so* lib
	if [ ! -e lib/libxerces-c.so ] ; then
		$ECHO "FAILED to build libxerces-c.so"
		exit
	else
		$ECHO "built lib/libxerces-c.so successfully"
	fi	
else
	$ECHO "lib/libxerces-c.so  exists"
fi
