/*! \file onscrypto.h
 * \brief Declarations of Open Navigation Surface cryptographic primitives.
 *
 * This file provides some enumerated types and definitons for the asymmetric cryptographic
 * primitives used to implement the Open Navigation Surface Digital Signature Scheme.
 */

/*
 * $Id: onscrypto.h,v 1.1 2006/02/02 13:11:21 openns Exp $
 * $Log: onscrypto.h,v $
 * Revision 1.1  2006/02/02 13:11:21  openns
 * MP: Moving source files from the API sub-directories so they all reside in the main API directory. First part of several cleanup checkins.
 *
 * Revision 1.2  2005/10/26 17:36:06  openns
 * Updates after testing following the 2nd ONSWG meeting.  Checked in from
 * Webb McDonald's implementation of 2005-09-07.
 *
 * Revision 1.2  2005/03/15 22:55:11  openns
 * Updated to include some general user data in the ons_gen_digest() call, so that
 * the user can add more information to the hash in addition to the file.  This is used in
 * the rest of the code to add the sequence ID to the hash before finalisation, so that
 * the SID can't be modified without invalidating the signature (previously this was
 * possible with appropriate binary manipulation of the BAG and caused an obvious
 * security loophole).  Modifications to the remainder of the call sequence to support this.
 *
 * Revision 1.1  2004/11/01 22:55:46  openns
 * Open Navigation Surface cryptographic code for signing BAGs.
 *
 *
 * File:	onscrypto.h
 * Purpose:	Types and prototypes for OpenNavigationSurface cryptographic code
 * Date:	2004-08-24
 *
 * This is free source code, courtesy of the OpenNavigationSurface project.  Visit
 * the website http://www.opennavsurf.org
 *
 */

#ifndef __ONSCRYPTO_H__
#define __ONSCRYPTO_H__

#include "stdtypes.h"
#include "bag.h"

/*! \brief An enum to describe the errors codes that can be returned by this code module.
 *
 * Describe the error codes that can be returned from this module.  Note that since this module
 * is only used internal and is wrapped by bag_crypto.c, these are not exported to the user level
 * of the API.
 */
typedef enum _ons_cryp_err {
	ONS_CRYPTO_SIG_OK = 0,		/*!< The signature was found and is valid */
	ONS_CRYPTO_NO_SIG_FOUND,	/*!< No signature was found in a file */
	ONS_CRYPTO_BAD_SIG_BLOCK,	/*!< The \a ONSCrypto block in a file was not in the correct format */
	ONS_CRYPTO_BAD_KEY,			/*!< The key passed is not a valid ONS internal format */
	ONS_CRYPTO_WRONG_KEY,		/*!< The key passed is not correct for the operation */
	ONS_CRYPTO_FILE_ERR,		/*!< A general file error occured during access */
	ONS_CRYPTO_OK,				/*!< All's well */
	ONS_CRYPTO_ERR				/*!< An unknown error has occurred */
} OnsCryptErr;

/*! Define the maximum length (bytes) that an internal ONS format bytestream signature can have */
#define ONS_CRYPTO_MAX_SIG_LEN	518

/*! Define the maximum length (bytes) that an internal ONS format bytestream key can have */
#define ONS_CRYPTO_MAX_KEY_LEN 1286

/* Routine:	ons_gen_digest
 * Purpose:	Generate a message digest from the named file, less the crypto block, if it exists
 * Inputs:	*file			Filename of the file to read and hash
 *			*user_data		User-supplied data to add to the digest (see comment)
 *			user_data_len	Length of the user-supplied data
 * Outputs:	*nbytes	Number of bytes in the digest
 *			Returns pointer to message digest on success, otherwise False
 * Comment:	The message digest protocol may change from time to time, so users should not assume
 *			that the nbytes value won't change.  The user-supplied data 
 */

BAG_EXTERNAL u8 *ons_gen_digest(char *file, u8 *user_data, u32 user_data_len, u32 *nbytes);

/* Routine:	ons_check_cblock
 * Purpose:	Determine whether the given file has an ONSCryptoBlock at the end
 * Inputs:	*file	Name of the file to read
 * Outputs:	Returns True if file has a ONSCryptoBlock at the end, otherwise False
 * Comment:	Note that this only checks that the ONSCryptoBlock exists in the correct place,
 *			and says nothing about whether the block is valid --- use ons_read_file_sig() to
 *			read and validate.
 */

BAG_EXTERNAL Bool ons_check_cblock(char *file);

/* Routine:	ons_read_file_sig
 * Purpose:	Verify a crypto signature block, and return signature if it exists
 * Inputs:	*file	Name of the file to read
 *			*sig	Buffer space for signature to be returned
 *			*sigid	Signature block reference ID
 *			nbuf	Number of spaces in the signature buffer
 * Outputs:	Returns ONS_CRYPTO_SIG_OK on success, otherwise ONS_CRYPTO_ERR
 * Comment:	This checks the end of the specified file for a crypto signature block, and returns the
 *			signature and block ID if it exists.
 */

BAG_EXTERNAL OnsCryptErr ons_read_file_sig(char *file, u8 *sig, u32 *sigid, u32 nbuf);

/* Routine:	ons_write_file_sig
 * Purpose:	Write a signature into the specified file, appending if it doesn't have one yet
 * Inputs:	*file	Name of file to write/append
 *			*sig	Signature to add to the file
 *			sigid	Reference ID to add to the signature block
 * Outputs:	Writes block to file, or appends if there isn't one already.
 *			Returns ONS_CRYPTO_OK if success, otherwise error code
 * Comment:	-
 */

BAG_EXTERNAL OnsCryptErr ons_write_file_sig(char *file, u8 *sig, u32 sigid);

/* Routine:	ons_sig_to_ascii
 * Purpose:	Convert a binary signature into ASCII for output, with CRC
 * Inputs:	*sig	Signature to convert for writing
 * Outputs:	Returns pointer to ASCII signature (zero terminated) with CRC appended,
 *			or NULL on failure.
 * Comment:	The code appends a CRC for the ASCII signature, also in ASCII so that the
 *			output can be fprintf()ed to output directly; on input, the CRC is used as a check
 *			for correctness of the transmission, not of signature.
 */

BAG_EXTERNAL char *ons_sig_to_ascii(u8 *sig);

/* Routine:	ons_key_to_ascii
 * Purpose:	Convert a binary key into ASCII for output, with CRC
 * Inputs:	*key	Key element to convert for writing
 * Outputs:	Returns pointer to ASCII signature (zero terminated) with CRC appended, or
 *			NULL on failure.
 * Comment:	The code appends a CRC for the ASCII key, also in ASCII so that the
 *			output can be fprintf()ed to output directly; on input, the CRC is used as a check
 *			for correctness of the transmission, not of key.
 */

BAG_EXTERNAL char *ons_key_to_ascii(u8 *key);

/* Routine:	ons_ascii_to_sig
 * Purpose:	Converts an ASCII signature to binary internal format, checking for validity
 * Inputs:	*sig	Input ASCII signature with CRC appended
 * Outputs:	Returns pointer to internal format signature on success, else NULL.
 *			*errcd	Error code for conversion, ONS_CRYPTO_OK on success.
 * Comment:	Converts signature to internal format, checking the CRC as it does so.  Errors
 *			are reported with return of NULL and appropriate setting of errcd.
 */

BAG_EXTERNAL u8 *ons_ascii_to_sig(char *sig, OnsCryptErr *errcd);

/* Routine:	ons_ascii_to_key
 * Purpose:	Convert an ASCII key to binary internal format, checking for validity
 * Inputs:	*key	Input ASCII key with CRC appended
 * Outputs:	Returns pointer to internal format key on success, else NULL
 *			*errcd	Error code for conversion, ONS_CRYPTO_OK on success
 * Comment:	Converts key to internal format, checking the CRC as it does so.  Errors are reported
 *			with return of NULL and appropriate setting of errcd.
 */

BAG_EXTERNAL u8 *ons_ascii_to_key(char *key, OnsCryptErr *errcd);

/* Routine:	ons_generate_keys
 * Purpose:	Generate keys for signature
 * Inputs:	-
 * Outputs:	*pkey	Pointer to the public key string in internal format
 *			*skey	Pointer to the secret key string in internal format
 *			Returns error information, or ONS_CRYPTO_OK on success.
 * Comment:	Generates internal format keys for signature operations.  The secret component should, of
 *			course, be treated with a degree of respect.
 */

BAG_EXTERNAL OnsCryptErr ons_generate_keys(u8 **pkey, u8 **skey);
	
/* Routine:	ons_sign_digest
 * Purpose:	Use secret key to sign the digest passed in
 * Inputs:	*md		Digest to sign (can send arbitrary data if required)
 *			nbytes	Size of the digest in bytes
 *			*skey	Secret key to use for signing (in internal format)
 * Outputs:	Returns pointer to signature in internal format, or NULL on error
 *			*errcd	Error code, or ONS_CRYPTO_OK on success.
 * Comment:	-
 */

BAG_EXTERNAL u8 *ons_sign_digest(u8 *md, u32 nbytes, u8 *skey, OnsCryptErr *errcd);

/* Routine:	ons_compute_signature
 * Purpose:	Compute the signature for a file
 * Inputs:	*name			Name of the file to read & compute signature
 *			*skey			Secret key to use in signature
 *			*user_data		User-supplied data to add to the message digest (see comment)
 *			user_data_len	Length of the user-supplied data to add
 * Outputs:	Returns pointer to the signature to use for the file, or NULL on error
 *			*errcd	Error code for the signature process
 * Comment:	This computes the signature, CRC32 etc.  The user-supplied data is passed on
 *			to the message digest computation code; set NULL if none is required.
 */

BAG_EXTERNAL u8 *ons_compute_signature(char *name, u8 *user_data, u32 user_data_len, u8 *skey);

/* Routine:	ons_sign_file
 * Purpose:	Read file, compute digest, and sign a file
 * Inputs:	*name	Name of the file to read & write signature into
 *			*skey	Secret key with which to sign the file
 *			sigid	Signature ID to add to output in order to link identity to the rest of the file
 * Outputs:	True on success, else False.
 * Comment:	A convenience function to sequence all of the elements to add a signature to a file.
 */

BAG_EXTERNAL Bool ons_sign_file(char *name, u8 *skey, u32 sigid);

/* Routine:	ons_verify_signature
 * Purpose:	Determine whether the signature provided matches the digest and public key
 * Inputs:	*sig	Signature to test
 *			*pkey	Public key to use for test
 *			*digest	Message digest to check
 *			dig_len	Digest length
 * Outputs:	Returns True if the signature and digest match, given the key, else False
 * Comment:	-
 */

BAG_EXTERNAL Bool ons_verify_signature(u8 *sig, u8 *pkey, u8 *digest, u32 dig_len);

/* Routine:	ons_verify_file
 * Purpose:	Verify that a file is intact
 * Inputs:	*name	Name of the file to read and check
 *			*pkey	Public key of the person reputed to have signed the file
 *			sig_id	Signature ID for the information block being verified
 * Outputs:	True if the file is valid and matches the public key passed
 * Comment:	The file is assumed to be valid if:
 *				(a) It has a valid ONSCryptoBlock at the end of the file
 *				(b) The signature block extracted verifies against the public key passed.
 *				(c) The signature ID extracted matches that passed in.
 */

BAG_EXTERNAL Bool ons_verify_file(char *name, u8 *pkey, u32 sig_id);

/* Routine:	ons_phrase_to_key
 * Purpose:	Construct a 256-bit symmetric crypto. key from a pass phrase
 * Inputs:	*phrase	Pointer to zero-terminated string to use as the pass phrase
 * Outputs:	Returns pointer to key stream of 256-bits (32 bytes), or NULL on error.
 * Comment:	This hashes the pass-phrase to generate a 256-bit key for use in symmetric crypto.
 *			For this key to really make sense, the pass phrase should be fairly long --- Schneier
 *			recommends that there should be one character per bit in the key.  The code does not,
 *			however, check that this is the case, nor does it enforce any particular standard.
 */

BAG_EXTERNAL u8 *ons_phrase_to_key(char *phrase);

/* Routine:	ons_encrypt_key
 * Purpose:	Encrypt a private key with AES to protect it in external representations
 * Inputs:	*seckey	Secret key to encrypt in internal format
 *			*aeskey	Symmetric key to use for AES encryption (must be 256-bits)
 * Outputs:	Pointer to encrypted secret key, or NULL on failure.
 *			*out_len	Set to the length of the output in bytes
 * Comment:	This encrypts the key (or any other internal format integer vector, but it really makes most
 *			sense for private keys, so that they can be written out to some file or device, and still
 *			remain secure) using AES256 (hence the requirement of the AES key).  This should be mostly
 *			secure enough for the intended use, which is to ensure that, should a secret key be
 *			compromised (e.g., a hardware token is stolen), the key cannot be recovered without the pass-
 *			phrase for at least as long as it takes for the compromise to be notices and the asymmetric
 *			key pair to be repudiated by the key-signing authority.
 */

BAG_EXTERNAL u8 *ons_encrypt_key(u8 *seckey, u8 *aeskey, u32 *out_len);

/* Routine:	ons_decrypt_key
 * Purpose:	Decrypt a private key encrypted with AES256 for use in DSA
 * Inputs:	*ctext	Ciphertext of key, with CRC32 appended
 *			nin		Length of ciphertext, including CRC32
 *			*aeskey	AES256 key (must be 32 bytes = 256 bits) to use for decryption.
 * Outputs:	Returns pointer to the decrypted private key, or NULL on failure
 *			*errc	Error code on failure.
 * Comment:	This assumes that the encrypted key was generated by ons_encrypt_key() and includes
 *			a CRC32 on the end to verify that the key hasn't been modified during transmission or
 *			storage.  The key in decrypted, and the output is rebuilt as a valid internal format
 *			key, with appended CRC32 of the decrypted key.
 */

BAG_EXTERNAL u8 *ons_decrypt_key(u8 *ctext, u32 nin, u8 *aeskey, OnsCryptErr *errc);

#endif
