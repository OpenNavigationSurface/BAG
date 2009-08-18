#/bin/bash
#################################
# Windows script for external libraries
# Webb McDonald -- Fri Aug 03 08:59:05 2007
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
#export BEECRYPT_LIB=beecrypt-4.1.2
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
if [ ! -e lib/$HOSTMACHINE/libszip.dll ] ; then
	$ECHO "Running make for $SZIP_LIB"
	$CD $SZIP_LIB
	make
	$CD ..
        if [ ! -e lib/$HOSTMACHINE/libszip.dll ] ; then
		$ECHO "FAILED to build libszip.dll"
		exit
	else
		$ECHO "built lib/$HOSTMACHINE/libszip.dll successfully"
	fi
else
	$ECHO "lib/$HOSTMACHINE/libszip.dll  exists"
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
if [ ! -e lib/$HOSTMACHINE/libzlib.dll ] ; then
	$ECHO "Running make for $ZLIB_LIB"
	$CD $ZLIB_LIB
	make
	$CD ..

	if [ ! -e lib/$HOSTMACHINE/libzlib.dll ] ; then
		$ECHO "FAILED to build libzlib.dll"
		exit
	else
		$ECHO "built lib/$HOSTMACHINE/libzlib.dll successfully"
	fi
else
	$ECHO "lib/$HOSTMACHINE/libzlib.dll  exists"
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
if [ ! -e lib/$HOSTMACHINE/libgeotrans.dll ] ; then
	$ECHO "Running make for $GEOTRANS_LIB"
	$CD $GEOTRANS_LIB
	make
	$CD ..
	if [ ! -e lib/$HOSTMACHINE/libgeotrans.dll ] ; then
		$ECHO "FAILED to build libgeotrans.dll"
		exit
	else
		$ECHO "built lib/$HOSTMACHINE/libgeotrans.dll successfully"
	fi
else
	$ECHO "lib/$HOSTMACHINE/libgeotrans.dll  exists"
fi	

#################################
#  BEECRYPT library build
#################################
#$ECHO "Checking for $BEECRYPT_LIB"
#if [ ! -d $BEECRYPT_LIB ] ; then
#	 $UNTAR $BEECRYPT_LIB.tar.gz
#else
#	 $ECHO "$BEECRYPT_LIB  exists"
#fi
#if [ ! -e lib/$HOSTMACHINE/libbeecrypt.dll ] ; then
#	 $ECHO "Configuring and running make for $BEECRYPT_LIB"
#	 $CD $BEECRYPT_LIB
#	 configure
#	 make
#	 $CP .libs/libbeecrypt.dll* ../lib/$HOSTMACHINE
#	 $CD ..
#	 if [ ! -e lib/$HOSTMACHINE/libbeecrypt.dll ] ; then
#		 $ECHO "FAILED to build libbeecrypt.dll"
#		 exit
#	 else
#		 $ECHO "built lib/$HOSTMACHINE/libbeecrypt.dll successfully"
#	 fi
#else
#	 $ECHO "lib/$HOSTMACHINE/libbeecrypt.dll  exists"
#fi	

#################################
#  HDF5 library build
#################################
$ECHO "Checking for $HDF5_LIB"
if [ ! -d $HDF5_LIB ] ; then
	 $UNTAR $HDF5_LIB.tar.gz
         $ECHO "Configuring and running make for $HDF5_LIB"
	 $CD $HDF5_LIB
	 sh configure
	 $CD ..
	 $ECHO "built lib/$HOSTMACHINE/libhdf5.dll successfully"
else
	 $ECHO "$HDF5_LIB  exists"
fi
#if [ ! -e lib/$HOSTMACHINE/libhdf5.dll ] ; then
#	 $ECHO "Configuring and running make for $HDF5_LIB"
#	 $CD $HDF5_LIB
#	 configure
#	 make
#	 $CP src/.libs/libhdf5.dll* ../lib/$HOSTMACHINE
#	 $CD ..
#	 if [ ! -e lib/$HOSTMACHINE/libhdf5.dll ] ; then
#		 $ECHO "FAILED to build libhdf5.dll"
#		 exit
#	 else
#		 $ECHO "built lib/$HOSTMACHINE/libhdf5.dll successfully"
#	 fi	
#else
#	 $ECHO "lib/$HOSTMACHINE/libhdf5.dll  exists"
#fi

#################################
#  XERCESC library build
#################################
$ECHO "Checking for $XERCESCROOT"
if [ ! -d $XERCESCROOT/src/xercesc ] ; then
	$UNTAR $XERCESCROOT.tar.gz
else
	$ECHO "$XERCESCROOT  exists"
fi
if [ ! -e lib/$HOSTMACHINE/libxerces-c.dll ] ; then
	$ECHO "Configuring and running make for $XERCESCROOT"
	$CD $XERCESCROOT/src/xercesc
	sh autoconf
	sh runConfigure -pmingw-msys -cgcc -xg++ -minmem -nfileonly -tnative -rnone
	make
	$CD $XERCESCROOT/..
	$CP $XERCESCROOT/lib/libxerces-c.dll* lib/$HOSTMACHINE
	if [ ! -e lib/$HOSTMACHINE/libxerces-c.dll ] ; then
		$ECHO "FAILED to build libxerces-c.dll"
		exit
	else
		$ECHO "built lib/$HOSTMACHINE/libxerces-c.dll successfully"
	fi	
else
	$ECHO "lib/$HOSTMACHINE/libxerces-c.dll  exists"
fi
