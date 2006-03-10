#/bin/bash

export UNTAR='tar -xvzf'
export CP='cp -f'
export CD='cd'

export XERCESCROOT=$PWD/xerces-c-src_2_6_0
export GEOTRANS_LIB=geotrans
export BEECRYPT_LIB=beecrypt-4.1.2
export HDF_LIB=hdf5-1.6.1
export SZIP_LIB=szip
export ZLIB_LIB=zlib


$UNTAR $SZIP_LIB.tgz
$CD $SZIP_LIB
make
$CD ..

if [ ! -e lib/libszip.so ] ; then
	echo "failed to build libszip.so"
	exit
fi

$UNTAR $ZLIB_LIB.tgz
$CD $ZLIB_LIB
make
$CD ..

if [ ! -e lib/libzlib.so ] ; then
	echo "failed to build libzlib.so"
	exit
fi

$UNTAR $GEOTRANS_LIB.tgz
$CD $GEOTRANS_LIB
make
$CD ..

if [ ! -e lib/libgeotrans.so ] ; then
	echo "failed to build libgeotrans.so"
	exit
fi


$UNTAR $BEECRYPT_LIB.tar.gz
$CD $BEECRYPT_LIB
configure
make
$CP .libs/libbeecrypt.so* ../lib
$CD ..

if [ ! -e lib/libbeecrypt.so ] ; then
	echo "failed to build libbeecrypt.so"
	exit
fi

$UNTAR $HDF_LIB.tar.gz
$CD $HDF_LIB
configure
gmake
make install
$CP src/.libs/libhdf5.so* ../lib
$CD ..

if [ ! -e lib/libhdf5.so ] ; then
	echo "failed to build libhdf5.so"
	exit
fi

$UNTAR $XERCESCROOT.tar.gz
$CD $XERCESCROOT/src/xercesc
runConfigure -plinux -cgcc -xg++ -minmem -nsocket -tnative -rpthread
gmake
$CD $XERCESCROOT/..
$CP $XERCESCROOT/lib/libxerces-c.so* lib

if [ ! -e lib/libxerces-c.so ] ; then
	echo "failed to build libxerces-c.so"
	exit
fi
