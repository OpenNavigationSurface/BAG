This directory contains the necessary source files and examples required to build and
work with data in the BAG format:

api        - This is the primary API directory and contains the source for the
             Bathymetric Attributed Grid format (BAG).
configdata - Required XML support files. You must have an environment variable called
             BAG_HOME mapped to this directory in order to run the API functions.
docs       - As the documentation evolves it will reside here. Currently this contains some
             initial draft documentation.
examples   - Contains programs to demonstrate some of the API functionality. In 
             particular bagcreate and bagread are good starting points.
extlibs    - Contains the 3rd-party libraries upon which the API depends. The examples will
             also need to link with the libraries in this directory in order to
             build properly.

Important Building Notes:

1) On the linux platform to compile the software you must set an environment variable
called HOSTMACHINE to the value of "linux". 
     ie. export HOSTMACHINE=linux     (for a bash shell), or
         setenv HOSTMACHINE linux     (for a csh or tcsh shell)

2) In order to run any of the examples, or to properly use the API, you must also define
the environment variable, BAG_HOME, to point to the location of the configdata directory.
For linux, that would be something like:
       export BAG_HOME=/usr/local/openns/configdata
On the Windows platform, environment variables are set via the "Advanced" tab in the System 
Properties control panel.


3) Be aware that the xerces-c library requires an environment variable to
be set. The API module needs the header files of xerces-c, which can
be found where the xerces-c library was installed under extlibs. 

(may only be required with Linux).
Something like the following should be set:

export	XERCESCROOT=$PWD/extlibs/xerces-c-src_2_6_0
setenv  XERCESCROOT $PWD/extlibs/xerces-c-src_2_6_0

Note that for building on the linux platform a toplevel build script called:
build-linux.mak is provided. You can run it with the command:
     sh build-linux.mak
It is important when running this script that you have qmake in the current 
path and also dot (.) in the current path. You might need to add it with a 
command similar to:
     export PATH=$PATH:.

Another toplevel build script called:
build-win.mak is provided.  This script is very similar to build-linux.mak, but is
intended for use with a MINGW32 build environment.  It will run the extlibs/buildlibs-win.mak.

