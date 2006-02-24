/********************************************************************
 *
 * Module Name : bag_crypto.c
 *
 * Authors/Date : 2005-07-18
 *     Brian Calder      (CCOM/JHC)
 *     Rick Brennan      (CCOM/JHC)
 *     Bill Lamey        (CARIS Ltd)
 *     Mark Paton        (IVS Ltd)
 *     Shannon Byrne     (SAIC, Newport)
 *     Jim Case          (SAIC, Newport)
 *     Dave Fabre        (NAVOCEANO)
 *     Jan Depner        (NAVOCEANO)
 *     Wade Ladner       (NAVOCEANO)
 *     Barry Gallagher   (NOAA HSTP)
 *     Friedhelm Moggert (7Cs GmbH)
 *     Shep Smith        (NOAA)
 *     Jack Riley        (NOAA HSTP)
 *
 * Initial concepts developed during The Open Navigation Surface Workshop
 *   attended by above contributors meeting at CCOM/JHC.
 *
 * Description : 
 *               This is the API interface to the onscrypto library.
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 * who when      what
 * --- ----      ----
 *
 * Classification : 
 *  Unclassified
 *
 * Distribution :
 * 
 * Usage And Licensing :
 *
 * References :
 *     Bathymetric Attributed Grid Format Specification Dcoument
 *     http://hdf.ncsa.uiuc.edu/HDF5/
 * 
 ********************************************************************/

/* Get the required standard C include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#include "bag.h"

#include "onscrypto.h"

static bagError bagTranslateCryptoError(OnsCryptErr errcode)
{
	bagError	rc;
	
	switch (errcode) {
    case ONS_CRYPTO_SIG_OK:			rc = BAG_CRYPTO_SIGNATURE_OK; break;
    case ONS_CRYPTO_NO_SIG_FOUND:	rc = BAG_CRYPTO_NO_SIGNATURE_FOUND; break;
    case ONS_CRYPTO_BAD_SIG_BLOCK:	rc = BAG_CRYPTO_BAD_SIGNATURE_BLOCK; break;
    case ONS_CRYPTO_BAD_KEY:		rc = BAG_CRYPTO_BAD_KEY; break;
    case ONS_CRYPTO_WRONG_KEY:		rc = BAG_CRYPTO_WRONG_KEY; break;
    case ONS_CRYPTO_FILE_ERR:		rc = BAG_BAD_FILE_IO_OPERATION; break;
    case ONS_CRYPTO_OK:				rc = BAG_SUCCESS; break;
    case ONS_CRYPTO_ERR:			rc = BAG_CRYPTO_GENERAL_ERROR; break;
    default:
        rc = BAG_CRYPTO_INTERNAL_ERROR;	/* This, of course, never happens */
        break;
	}
	return(rc);
}

/* Routine:	bagComputeMessageDigest
 * Purpose:	Compute, from a BAG file, the Message Digest needed for the signature construction
 * Inputs:	*file		Name of the BAG file to digest (see comment)
 *			signatureID	Sequential signature ID --- the reference number from the corresponding metadata element
 * Outputs:	*nBytes		Set to the number of bytes generated in the Message Digest
 *			Returns pointer to the message digest byte sequence
 * Comment:	This calls through to ons_gen_digest(), and supplies the appropriate user data to make the MD
 *			compatible with the ONS Digital Security Scheme.
 */

u8 *bagComputeMessageDigest(char *file, u32 signatureID, u32 *nBytes)
{
	return(ons_gen_digest(file, (u8*)&signatureID, sizeof(u32), nBytes));
}

/* Routine:	bagSignMessageDigest
 * Purpose:	Compute, from a Message Digest and a Secret Key, the Signature sequence
 * Inputs:	*md			Message Digest to be signed
 *			md_len		Message Digest length in bytes
 *			*secKey		Secret key byte sequence to use in signing
 * Outputs:	*errcode	Error code (or BAG_SUCCESS if all's OK)
 *			Returns pointer to the signature stream on success, otherwise NULL on failure
 * Comment:	This calls through to ons_sign_digest() and does appropriate error code translation
 */

u8 *bagSignMessageDigest(u8 *md, u32 mdLen, u8 *secKey, bagError *errcode)
{
	OnsCryptErr	rc;
	u8			*rtn;
	
	rtn = ons_sign_digest(md, mdLen, secKey, &rc);
	*errcode = bagTranslateCryptoError(rc);
	return(rtn);
}

/* Routine:	bagReadCertification
 * Purpose:	Read signature stream from file, if it exists
 * Inputs:	*file	BAG file to read signature stream from
 *			*sig	Buffer space to read signature into
 *			nBuffer	Number of spaces in the *sig buffer
 * Outputs:	*sigID	The signature's Sequential ID (cross-ref to meta-data)
 *			Returns appropriate error code, BAG_SUCCESS on successful completion.
 * Comment:	This reads the BAG file for an ONSCrypto block, if it exists.  If the
 *			block exists, the signature is read, validated, and then returned.  If
 *			the block doesn't exist, or doesn't validate, error codes are returned.
 */

bagError bagReadCertification(char *file, u8 *sig, u32 nBuffer, u32 *sigID)
{
	return(bagTranslateCryptoError(ons_read_file_sig(file, sig, sigID, nBuffer)));
}

/* Routine:	bagWriteCertification
 * Purpose:	Write signature stream into file, appending if an ONSCrypto block doesn't exist
 * Inputs:	*file	BAG file to write signature stream into
 *			*sig	Signature stream to write to the file
 *			sigID	Signature Sequential ID number to write into ONSCrypto block
 * Outputs:	Returns appropriate error code, BAG_SUCCESS on successful completion.
 * Comment:	This verifies the signature is valid and intact, and then writes into the output
 *			file indicated.  If the output file doesn't have the ONSCrypto block, one is appended.
 */

bagError bagWriteCertification(char *file, u8 *sig, u32 sigID)
{
	return(bagTranslateCryptoError(ons_write_file_sig(file, sig, sigID)));
}

/* Routine:	bagVerifyCertification
 * Purpose:	Verify that a signature, held internally, is valid
 * Inputs:	*sig	The Signature to verify
 *			*pubKey	The putative SA's public key sequence
 *			*md		The message digest of the BAG file that was signed
 *			*mdLen	Length of the message digest in bytes
 * Outputs:	Returns True if the signature, public key and digest match; otherwise False
 * Comment:	-
 */

Bool bagVerifyCertification(u8 *sig, u8 *pubKey, u8 *md, u32 mdLen)
{
	return(ons_verify_signature(sig, pubKey, md, mdLen));
}

/* Routine:	bagComputeFileSignature
 * Purpose:	Convenience function to compute a signature given a file and the SA secret key
 * Inputs:	*name	BAG file name to compute signature over
 *			sigID	Signature Sequential ID number
 *			*secKey	SA's secret key sequence
 * Outputs:	Returns pointer to signature byte sequence, or NULL on failure
 * Comment:	This is primarily for convenience, since it just sequences other calls in the
 *			library.  This computes the MD, then the signature and the associated CRC32, and
 *			returns it for the user.
 */
 
u8 *bagComputeFileSignature(char *name, u32 sigID, u8 *secKey)
{
 	return(ons_compute_signature(name, (u8*)sigID, sizeof(u32), secKey));
}

/* Routine:	bagSignFile
 * Purpose:	Convenience function to sequence all of the functions to sign a file ab initio
 * Inputs:	*name	BAG file name to sign
 *			*secKey	SA's secret key
 *			sigID	Signature Sequential ID number (cross-ref to the metadata)
 * Outputs:	Returns True on success, otherwise False
 * Comment:	This is primarily for convenience --- one call interface to do everything required
 *			to sign a file from scratch.
 */

Bool bagSignFile(char *name, u8 *secKey, u32 sigID)
{
	return(ons_sign_file(name, secKey, sigID));
}

/* Routine:	bagVerifyFile
 * Purpose:	Convenience function to sequence all of the functions required to verify a file ab initio
 * Inputs:	*name	BAG file to check and verify signature
 *			*pubKey	SA's public key byte sequence
 *			sigID	Signature Sequential ID number (cross-ref to metadata)
 * Outputs:	True if the signature is valid, otherwise False
 * Comment:	This is primarily for convenience --- one call interface to do everything required
 *			to read and check the signature, compute the MD for the file, and then verify that
 *			the MD, the signature and the Public Key all agree.
 */

Bool bagVerifyFile(char *name, u8 *pubKey, u32 sigID)
{
	return(ons_verify_file(name, pubKey, sigID));
}

/* Routine:	bagGenerateKeyPair
 * Purpose:	Generate an ONS asymmetric cryptography key pair
 * Inputs:	-
 * Outputs:	**pubKey	*pubKey points to the Public Key byte sequence
 *			**secKey	*secKey points to the Secret Key byte sequence
 *			Returns BAG_SUCCESS on success, otherwise an informative error code
 * Comment:	This generates a DSA key-pair for asymmetric cryptography (e.g., for signature schemes).
 *			The secret key should, of course, be kept secret and not divulged.
 */

bagError bagGenerateKeyPair(u8 **pubKey, u8 **secKey)
{
	return(bagTranslateCryptoError(ons_generate_keys(pubKey, secKey)));
}

/* Routine:	bagConvertCryptoFormat
 * Purpose:	Convert representation format of cryptographic objects (bin <-> ASCII)
 * Inputs:	*object	Cryptographic object to convert
 *			objType	Input object type (bagCryptoObject enum)
 *			convDir	Direction of the conversion to be attempted
 * Outputs:	**converted		*converted points to converted object
 *			Returns BAG_SUCCESS on success, or appropriate error message on failure
 * Comment:	This acts as a switch to convert keys or signatures to and from ASCII and internal
 *			representations.
 */

bagError bagConvertCryptoFormat(u8 *object, bagCryptoObject objType, bagConvDir convDir, u8 **converted)
{
	bagError	rc = BAG_SUCCESS;
    OnsCryptErr onsErr;
	
	if (objType == bagCryptoKey) {
		if (convDir == bagConvertASCIIToBin) {
			*converted = ons_ascii_to_key(object, &onsErr);
		} else if (convDir == bagConvertBinToASCII) {
			*converted = ons_key_to_ascii(object);
		} else {
			*converted = NULL;
			rc = BAG_CRYPTO_INTERNAL_ERROR;
		}
	} else if (objType == bagCryptoSignature) {
		if (convDir == bagConvertASCIIToBin) {
			*converted = ons_ascii_to_sig(object, &onsErr);
		} else if (convDir == bagConvertBinToASCII) {
			*converted = ons_sig_to_ascii(object);
		} else {
			*converted = NULL;
			rc = BAG_CRYPTO_INTERNAL_ERROR;
		}
	} else {
		*converted = NULL;
		rc = BAG_CRYPTO_INTERNAL_ERROR;
	}
	return(rc);
}
