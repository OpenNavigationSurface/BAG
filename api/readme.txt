   API
   =======
   
Be aware that the xerces-c library requires an environment variable to
be set. The API module needs the header files of xerces-c, which can
be found where the xerces-c library was installed under extlibs.
You will also need header files for beecrypt, geotrans, and hdf5 libraries.

Something like the following should be set:

export	XERCESCROOT=$PWD/../extlibs/xerces-c-src_2_6_0
setenv  XERCESCROOT $PWD/../extlibs/xerces-c-src_2_6_0


Also, don't forget the requirement for BAG_HOME before trying the examples:

export	BAG_HOME=$PWD/../configdata
setenv  BAG_HOME $PWD/../configdata
