
Overview of the Sample Programs

sample-data
-----------

   Contains a sample XML file and a prebuild BAG file as trivial 
   examples.

bag_read
-------

   A sample reading of a Bag file. See the readme.txt file inside 
   the sample-data directory for more information to test this 
   program.
   
bag_create
---------

   Creates a sample 10x10 row/column BAG file. See the readme.txt 
   file inside the sample-data directory for more information on 
   how to test this program.
    
bag_gencert
-------

	Generates a digital signature key-pair for use in signing 
	BAGs.  The private key is either stored in a HASP USB device, 
	or in an XML file depending on command-line flags.  The public 
	key is appended to a base certificate (containing name, 
	organization, etc.) and then written as another XML file.  See 
	gencert/example_proto_cert.xml for the contents required in 
	the base certificate.

bag_signcert
--------

	Uses the information in an Entity's private key (plus their 
	passphrase) to compute a Digital Signature for a user 
	certificate, and then appends it to the certificate.  This 
	binds the public key and identity information together (within 
	the belief of the Certificate Signing Agency's veracity).

bag_verifycert
----------

	Uses the information from the Certificate Signing Agency's 
	public key and the user certificate to verify the signature in 
	the user certificate.  A positive result (i.e., that the 
	signature verifies) means that the certificate was signed by 
	the CSA and that it has not been modified since it was signed.

bag_signfile
--------

	Uses the information in a user's private key (and their 
	passphrase) to sign a BAG file.  This computes a Digital 
	Signature for the BAG and then appends it to a control block 
	at the end of the file.  The sequence number is used to link 
	the DS to the metadata in the BAG, and is an arbitrary 
	integer.

bag_verifyfile
----------

	Uses the information in a user certificate to verify the 
	Digital Signature in a BAG file.  A positive result (i.e., the 
	signature verifies) means that the BAG was signed by the 
	person identified in the certificate (within the degree of 
	belief of the CSA's veracity) and that it has not been 
	modified since it was signed (either by transmission or by 
	intent).
	

(from the API readme.txt):

Also, don't forget the requirement for BAG_HOME before trying the examples.

On Linux/OS X:

export	BAG_HOME=$PWD/../configdata
setenv  BAG_HOME $PWD/../configdata

On Windows:

set BAG_HOME=%CD%\..\configdata
