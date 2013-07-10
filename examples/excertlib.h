/*
 * $Id: excertlib.h,v 1.2 2005/10/26 17:38:26 openns Exp $
 * $Log: excertlib.h,v $
 * Revision 1.2  2005/10/26 17:38:26  openns
 * Updates after testing following the 2nd ONSWG meeting.  Checked in from
 * Webb McDonald's implementation of 2005-09-07.
 *
 * Revision 1.2  2005/03/15 22:44:25  openns
 * Added XML certificate functionality, and slightly different call sequence to
 * ons_gen_digest() to support new hashing scheme.
 *
 * Revision 1.1  2004/11/01 22:27:03  openns
 * Example certificate construction and mangement code for use with simple text
 * files for public certificates and HASP keys for secret keys.
 *
 *
 * File:	excertlib.h
 * Purpose:	Support routines for the example key and certificate management scheme.
 * Date:	2004-10-23
 *
 * This is free source code, courtesy of the OpenNavigationSurface project.  Visit
 * the website http://www.opennavsurf.org
 *
 */

#ifndef __EXCERTLIB_H__
#define __EXCERTLIB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stdtypes.h"

typedef enum _excert_rc {
	EXCERT_OK = 0,				/* No error */
	EXCERT_INTERNAL_ERROR,		/* This never happens ... */

	EXCERT_NO_MEM,				/* No memory for the requested operation */
	EXCERT_BAD_FILE,			/* File is wrong format, or corrupt */
	EXCERT_NO_FILE,				/* File does not exist or cannot be made */

	EXCERT_NO_HASP,				/* No HASP found attached to computer */
	EXCERT_WRONG_HASP,			/* Wrong type of HASP attached to computer */
	EXCERT_HASP_READ_ERROR,		/* Failed to read memory from the attached HASP */
	EXCERT_HASP_WRITE_ERROR,	/* Failed to write memory to the attached HASP */

	EXCERT_CRYPTO_FAILED,		/* A cryptographic operation failed */
	EXCERT_HASH_FAILED,			/* Hash construction for a certificate failed */
	EXCERT_BAD_CERTIFICATE,		/* A mal-formed certificate was passed */
	EXCERT_BAD_SECKEY			/* A bad secret key was passed */
} ExcertErr;

typedef enum _objtype {
	EXCERT_PUBLIC_KEY,
	EXCERT_SECRET_KEY,
	EXCERT_SIGNATURE
} ExcertObject;

typedef struct _cert_tag {
	char	*tag;
	char	*value;
} CertTag;

typedef struct _user_info {
	char	*name;				/* Name of the user who ownes the certificate */
	char	*organization;		/* Organization name of the certificate's owner */
	u32		idnum;				/* Serial number within organization's generation sequence */
	char	*keysource;			/* String indicating where to obtain this key */
} xcrtUserInfo;

typedef struct _key_info {
	Bool	is_public;			/* Flag: True => the following key is a public key (else private) */
	Bool	is_encrypted;		/* Flag: True => the following key is encrypted somehow (else it's an internal bytestream) */
	char	*algorithm;			/* String for the algorithm that computed the key */
	char	*version;			/* String for the version of the algorithm that computed the key */
	u8		*key;				/* Binary internal format bytestream for the key */
} xcrtKeyInfo;

typedef struct _sig_info {
	xcrtUserInfo	signer;		/* Information for the person that signed the certificate */
	char			*algorithm;	/* String for the algorithm the computed the signature */
	char			*version;	/* String for the version of the algorithm that computed the signature */
	u8				*signature;	/* Binary internal format bytestream for the signature */
} xcrtSigInfo;

typedef struct _certificate {
	xcrtUserInfo	user;		/* User who owns the certificate */
	xcrtKeyInfo		key;		/* Key associated with the certificate */
	xcrtSigInfo		signature;	/* The signature attesting to the veracity of the certificate */
} xcrtCertificate;

/* Routine:	excert_get_hasp_key
 * Purpose:	Extract the secret key from an attached hasp USB and decode
 * Inputs:	*phrase		Passphrase to hash for AES key
 * Outputs:	*rc			Set to indicate the result of the operation
 *			Returns pointer to the u8 key ready for use with signing operations,
 *			or NULL on failure (with rc set appropriately)
 * Comment:	This checks for the key, extracts the memory and then decodes
 *			it using the AES key computed from the passphrase.  Errors can occur
 *			if there is no hasp attached, if it isn't a memo-hasp, or if it isn't
 *			made with the correct passwords (i.e., doesn't belong to the agency that
 *			made this code into an executable and issued the key).
 */

extern u8 *excert_get_hasp_key(u8 *phrase, ExcertErr *rc);

/* Routine:	excert_put_hasp_key
 * Purpose:	Write a secret key to HASP memory using the pass-phrase as a source of AES encryption key
 * Inputs:	*sec_key_ptext	Plaintext for the secret key
 *			*phrase			Pass-phrase to use for AES256 key generation
 * Outputs:	Returns an appropriate error code, EXCERT_OK on success.
 * Comment:	This hashes the pass-phrase to form an AES256 key, and then uses this key
 *			to encrypt the secret key.  The secret key is then written to the memory of the attached
 *			HASP key if possible.
 */

extern ExcertErr excert_put_hasp_key(u8 *sec_key_ptext, u8 *phrase);

/* Routine:	excert_write_xml_seckey
 * Purpose:	Write the secret key, encrypted with AES-256, to an output XML filename
 * Inputs:	*sec_key_ptext	Secret key plain-text
 *			*phrase			Pass-phrase used for the symmetric encryption process
 *			*filename		File on which to write the XML for the secret key
 * Outputs:	EXCERT_OK on success, otherwise and appropriate error
 * Comment:	This is an alternative to using HASP keys to do the secret key part of
 *			the process.  Since the key is encrypted within the file, it should be
 *			about as secure as the HASP key solution.
 */

extern ExcertErr excert_write_xml_seckey(u8 *sec_key_ptext, u8 *phrase, char *filename);

/* Routine:	excert_get_object
 * Purpose:	Get an object from the file specified, as required by the user
 * Inputs:	*ip		FILE to read from
 *			iptype	Type of object to read from file
 * Outputs:	Returns pointer to byte-stream for the object, or NULL on failure.
 * Comment:	This uses the _iptype_ to find the appropriate search string, and then
 *			runs through the file until it find the first instance of that string,
 *			or runs out of file.  The first instance is converted, if possible, and
 *			returned to the user.  The file is left where it is after the last byte
 *			is read.
 */

extern u8 *excert_get_object(FILE *ip, ExcertObject iptype);

/* Routine:	excert_put_object
 * Purpose:	Output an object description to file
 * Inputs:	*op		FILE to write into
 *			*obj	Object byte-stream to be written
 *			obtype	How to describe the object on the output stream
 *			nbytes	Number of bytes in the object
 * Outputs:	Returns an appropriate error code, EXCERT_OK on success
 * Comment:	This simply writes the appropriate recognition string to the output
 *			and then the object, converted to ASCII hex.
 */

extern ExcertErr excert_put_object(FILE *op, u8 *obj, ExcertObject obtype);

/* Routine:	excert_parse_certificate
 * Purpose:	Search a certificate for particular values in a specific section
 * Inputs:	*cert		Name of the certificate to search
 *			*section	Section tag for the part of the certificate to search
 *			*table		Table of certificate tags to match if possible
 * Output:	Returns count of the number of tags matched, including possibly 0, and
 *			-1 on error.
 * Comment:	This is a fairly simple parser, and could be readily fooled ...  The
 *			table must have a terminating tag with recognition string set to NULL.
 *			As each element is parsed, the value element of the tag is set to point to
 *			a dynamically allocated string.  It is the user's responsibility to ensure
 *			that the value elements are NULL on entry (or at least not pointing to
 *			anything important) and that the memory is freed after use.
 */

extern s32 excert_parse_certificate(char *cert, char *section, CertTag *table);

/* Routine:	excert_hash_certificate
 * Purpose:	Compute the signature hash of the certificate
 * Inputs:	*cert	Name of the certificate to be hashed
 * Outputs:	*nbytes	Number of bytes in the certificate's hash
 *			Returns pointer to hash byte-stream or NULL on failure
 * Comment:	This extracts all of the information from the certificate into a temporary file,
 *			and then hashes it with the standard OpenNS signature algorithm.  This allows
 *			the code to compute the hash either with or without a signature section.
 */

extern u8 *excert_hash_certificate(char *cert, u32 *nbytes);
extern u8 *excert_hash_xml_certificate(xcrtCertificate *user, u32 *digest_len);

/* Routine:	excert_sign_xml_certificate
 * Purpose:	Sign an XML certificate file using a HASP secret key, and a text public key
 * Inputs:	*user				Name of the certificate file to be signed
 *			*csa				Name of the CSA's certificate file
 *			*csa_passphrase		CSAPass-phrase to decode the secret key
 *			seckey_from_hasp	Flag: True => read information for the CSA secret key from file, rather than HASP
 *			csa_seckey_file		File from which to read the CSA secret key, if required.
 * Outputs:	Returns appropriate error code, EXCERT_OK on success.
 * Comment:	This uses the pass-phrase to decode the secret key (either from an attached HASP or from
 *			the secret key XML file associated with the CSA's signature credentials), computes the
 *			digest from the SA's XML certificate file, and signs the XML certificate.
 */

extern ExcertErr excert_sign_certificate(char *cert, char *auth, char *phrase);
extern ExcertErr excert_sign_xml_certificate(xcrtCertificate *user, xcrtCertificate *csa, char *csa_passphrase,
											 Bool seckey_from_hasp, char *csa_seckey_file);

/* Routine:	excert_get_public_key
 * Purpose:	Extract the public key element from a certificate, if one exists
 * Inputs:	*cert	Certificate file to check
 * Outputs:	Returns pointer to public key in OpenNavSurf format, or NULL on failure.
 * Comment:	-
 */

extern u8 *excert_get_public_key(char *cert);

/* Routine:	excert_get_signature
 * Purpose:	Extract the signature element of the certificate
 * Inputs:	*cert	Certificate file to check
 * Outputs:	Returns the signature from the certificate in OpenNavSurf format, or NULL on failure
 * Comment:	-
 */

extern u8 *excert_get_signature(char *cert);

/* Routine:	excert_verify_certificate
 * Purpose:	Verify that a certificate is genuine, given the public key certificate of the
 *			signing authority.
 * Inputs:	*cert	The certificate to check
 *			*auth	The signing authority's public key certificate
 * Outputs:	True if certificate is valid, otherwise False.
 * Comment:	Note that this does not determine whether the signing authority's certificate
 *			is valid: that's something that should be done on retrieval by verifying it
 *			against its signing authority.
 */

extern Bool excert_verify_certificate(char *cert, char *auth);
extern Bool excert_verify_xml_certificate(xcrtCertificate *cert, char *cert_file, xcrtCertificate *auth);

/* Routine:	excert_construct_certificate
 * Purpose:	Construct a certificate in standard form given user information and public key
 * Inputs:	*user_name	Full name of the user for whom the certificate is being constructed
 *			*user_org	Organisation with which the user is associated
 *			*user_id	User identification string
 *			*opt_arg	Optional CertTag structures to be included in the user section
 *			*pub_key	Public key byte-stream in OpenNavSurf format to be output
 *			*cert		Filename for the certificate
 * Outputs:	Appropriate error code, EXCERT_OK on success.
 * Comment:	This writes a basic certificate, including the mandatory user information and the
 *			public key.  If the user wants extra information, specify opt_arg[] as a valid
 *			pointer; otherwise set it to NULL.
 */

extern ExcertErr excert_construct_certificate(char *user_name, char *user_org, char *user_id,
											  CertTag *opt_arg, u8 *pub_key, char *cert);

/* Routine:	excert_generate_keys
 * Purpose:	Generate an OpenNavSurf key-pair and output appropriately
 * Inputs:	*phrase			Pass-phrase to use for encrypting the secret key to the HASP device
 *			output_to_hasp	Flag: True => output will go to HASP; otherwise to file
 *			*filename		Name of the file to write on, if using file output
 * Outputs:	Appropriate error code, EXCERT_OK on success
 *			**pkey	Public key anchor
 * Comment:	This generates the keys, constructs an AES256 key from the pass-phrase and
 *			encrypts the secret key, and writes the secret key to the attached HASP device
 *			before returning the public key for certification.
 */

extern ExcertErr excert_generate_keys(char *phrase, u8 **pkey, Bool output_to_hasp, char *filename);

/* Routine:	excert_build_certificate
 * Purpose:	Take a prototype certificate, build keys, and make a certificate ready for signing
 * Inputs:	*proto	Prototype certificate file
 *			*cert	Certificate file to create
 *			*phrase	Pass-phrase to use in managing the secret key part of the pair
 * Outputs:	Appropriate error code, or EXCERT_OK on success
 * Comment:	This generates an OpenNavSurf key-pair, and writes the secret key into a HASP
 *			attached to the computer --- you *must* have a HASP attached before this is called
 *			otherwise it will fail.  The public-key part of the pair gets added to the proto-
 *			type certificate to make the real certificate.
 */

extern ExcertErr excert_build_certificate(char *proto, char *cert, char *phrase);
extern ExcertErr excert_build_xml_certificate(xcrtCertificate *cert, char *phrase, Bool output_to_hasp, char *seckey_filename);

/* Routine:	excert_sign_ons
 * Purpose:	Sign an OpenNavigationSurface file using high-level certificates
 * Inputs:	*ons				OpenNavigationSurface file to sign
 *			*phrase				Pass-phrase associated with the signer's secret key
 *			num					Sequence number to use in signature specification
 *			seckey_from_hasp	Flag: True => read secret key from the HASP; otherwise, from file
 *			seckey_filename		If secret key is coming from file, use this file
 * Outputs:	Appropriate error code, or EXCERT_OK on success
 * Comment:	This is a driver routine designed to make this a one-stop call (easier
 *			to wrap for other drivers).
 */

extern ExcertErr excert_sign_ons(char *ons, char *phrase, u32 num, Bool seckey_from_hasp, char *seckey_filename);

/* Routine:	excert_verify_ons
 * Purpose:	Verify the integrity of an OpenNavigationSurface file using certificates
 * Inputs:	*cert	Signatory's public key certificate
 *			*ons	File to verify
 *			num		Sequence number associated with the signature
 * Outputs:	True on success, otherwise False
 * Comment:	This is a driver routine designed to make this a one-stop call (easier
 *			to wrap for other drivers).
 */

extern Bool excert_verify_ons(char *cert, char *ons, u32 num);
extern Bool excert_verify_xml_ons(xcrtCertificate *sa_cert, char *ons, u32 num);

/* Routine:	excert_write_xml_certificate
 * Purpose:	Write a certificate from internal structure
 * Inputs:	*cert	Certificate data to write
 *			*name	Name of the file on which to write
 * Outputs:	Returns True if wrote OK, otherwise False
 * Comment:	This formats the internal structure as output XML, writing in standard format.
 */

extern Bool excert_write_xml_certificate(xcrtCertificate *cert, char *name);

/* Routine:	excert_read_xml_certificate
 * Purpose:	Extract all information from an XML certificate
 * Inputs:	*name	Name of the XML file to read for the certificate
 * Outputs:	Pointer to an initialised xcrtCertificate structure, or NULL on failure
 *			*rc		Appropriate error code if read fails
 * Comment:	This expects there to be only one certificate in the file, although it will search the file
 *			until it finds the first <entity>, so it doesn't need to be the only thing in the file.  This
 *			routine uses the 'parse to internal' structure so that the whole file in rendered into memory
 *			before anything else happens.  Those with very large XML structures might want to think about
 *			this before use, since it could take a significant length of time.
 */

extern xcrtCertificate *excert_read_xml_certificate(char *name, ExcertErr *rc);

/* Routine:	excert_release_xml_certificate
 * Purpose:	Release all dynamically allocate memory associated with a certificate
 * Inputs:	*cert	xcrtCertificate to release
 * Output:	-
 * Comment:	The routine assumes that the memory has been allocated through a call to
 *			excert_read_xml_certificate() or excert_new_certificate() so that it is
 *			initialised in 'fail safe' mode (i.e., all pointers NULL unless valid).
 *			Deviations from this may result in unpredictable behaviour up to, and possibly
 *			including messy piles of the smelly stuff inappropriately dumped in unexpected
 *			locations within your filesystem.  Don't say you weren't warned.
 */

extern void excert_release_xml_certificate(xcrtCertificate *cert);

/* Routine:	excert_new_xml_certificate
 * Purpose:	Allocate space for a new (empty) XML certificate
 * Inputs:	-
 * Outputs:	Pointer to a certificate structure, or NULL (no memory)
 * Comment: -
 */

extern xcrtCertificate *excert_new_xml_certificate(void);

#ifdef __cplusplus
}
#endif

#endif
