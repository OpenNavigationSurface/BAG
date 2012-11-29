/*! \file bag_crypto.c
 * \brief Implement translation interface to onscrypto.c.
 *
 * The only reason this is required is because the cryptographic library
 * was developed before the API fully developed, and it was easier to wrap
 * it than to rebuild to the new API (for now).  In the future, this will
 * be the primary interface to avoid call-through costs.
 */

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

#ifdef __BAG_BIG_ENDIAN__
/*! \brief Swap four-byte entities end for end.
 *
 * This swaps 32-bit numbers endian-ness (LSB <-> MSB) if required.  The swap is done in-place.
 * The internal code of all ONS crypto entities are LSB by default, so this code only gets
 * compiled in if the code is being compiled for MSB systems).
 *
 * \param	*elem	Pointer to the data structure to swap.
 * \return			None.
 */
 
static void swap_4(void *elem)
{
	u8	swap, *buffer = (u8*)elem;
	
	swap = buffer[0]; buffer[0] = buffer[3]; buffer[3] = swap;
	swap = buffer[1]; buffer[1] = buffer[2]; buffer[2] = swap;
}
#endif

/*! \brief Compute the cryptographic Message Digest for a named file.
 *
 * This computes the MD for \a file, using \a signatureID as the unique sequential integer ID to
 * link this signature event back to the meta-data in the remainder of the file.  The MD is returned,
 * but since this is just a binary sequence without coding, the length is returned in \a nBytes.
 * This is a call-through to ons_gen_digest().
 *
 * \param	*file		Name of the file to be digested.
 * \param	signatureID	Sequential integer ID mixed with the data to link this event to the meta-data.
 * \param	*nBytes		Number of bytes returned to the user (on success)
 * \return				A pointer to the message digest bytestream on success, or NULL on failure.
 */

u8 *bagComputeMessageDigest(char *file, u32 signatureID, u32 *nBytes)
{
#ifdef __BAG_BIG_ENDIAN__
	swap_4((void*)&signatureID);
#endif
	return(ons_gen_digest(file, (u8*)&signatureID, sizeof(u32), nBytes));
}

/*! \brief Sign a cryptographic Message Digest using a suitable secret key bytestream
 *
 * This signs a computed MD (or any general bytestream) in \a md, of length \a md_len with a
 * user-provided secret key bytestream in \a secKey and returns the signature bytestream required
 * for verification of the signature.  This has information on lengths and numbers of integers
 * encoded in it, and therefore no length is required.  The value of \a errcode is set to
 * indicate any error condition, or \a BAG_SUCCESS if the signature was computed OK.  This is
 * a call-through to ons_sign_digest().
 *
 * \param	*md			Message Digest (or arbitrary bytestream) to be signed.
 * \param	mdLen		Length (bytes) of the Message Digest bytestream.
 * \param	*secKey		Secret Key bytestream (in ONS internal format) to sign with.
 * \param	*errcode	Enumerated error code, or \a BAG_SUCCESS on success.
 * \return				Returns a pointer to the signature bytestream (in ONS internal format) on
 *						success, and NULL on failure.
 */

u8 *bagSignMessageDigest(u8 *md, u32 mdLen, u8 *secKey, bagError *errcode)
{
	OnsCryptErr	rc;
	u8			*rtn;
	
	rtn = ons_sign_digest(md, mdLen, secKey, &rc);
	*errcode = bagTranslateCryptoError(rc);
	return(rtn);
}


/*! \brief Read signature stream from a file, if a certification block exists.
 *
 * Read the certification block in \a file, if it exists, and return the stored
 * signature (in user-supplied buffer \a sig with \a nBuffer bytes available) and
 * the sequential signature ID stored in the certification block in \a sigID.
 * The code returns \a BAG_SUCCESS if the certification block exists, the signature
 * can be read and is in the correct format, and otherwise returns a suitable error
 * code.  This is a call-through to ons_read_file_sig().
 *
 * \param	*file		Name of the BAG file to read a certification block from.
 * \param	*sig		User-supplied bytestream buffer to store signature into.
 * \param	nBuffer		Space available in the buffer for the signature.
 * \param	*sigID		Sequential signature ID (cross-reference to meta-data) (on success).
 * \return				Returns an appropriate error code, or \a BAG_SUCCESS if
 *						everything works out.
 */

bagError bagReadCertification(char *file, u8 *sig, u32 nBuffer, u32 *sigID)
{
	return(bagTranslateCryptoError(ons_read_file_sig(file, sig, sigID, nBuffer)));
}

/*! \brief Write the certification block to a BAG file with signature information.
 *
 * This writes a signature from bytestream in \a sig and sequential signature ID \a sigID into
 * \a file, adding a \a ONSCrypto block if one doesn't already exists.  If the block exists, it
 * is overwritten with the new information.  This is a call-through to ons_write_file_sig().
 *
 * \param	file		Name of the BAG file to write signature stream info
 * \param	*sig		Signature bytestream (in ONS internal format) to write.
 * \param	sigID		Sequential signature ID (meta-data link) to write.
 * \return				Appropriate error code, or \a BAG_SUCCESS on success.
 */

bagError bagWriteCertification(char *file, u8 *sig, u32 sigID)
{
	return(bagTranslateCryptoError(ons_write_file_sig(file, sig, sigID)));
}

/*! \brief Carry out verification of a signature held in a memory buffer.
 *
 * Run the DSS verification algorithm on the signature in \a sig using the user public
 * key in \a pubKey (in ONS internal format) and computed message digest in bytestream
 * \a md with length \a mdLen bytes.  Returns \a True if the signature, public key and
 * message digest match, and otherwise returns \a False.  This is a call-through to
 * ons_verify_signature().
 *
 * \param	*sig		Signature bytestream (in ONS internal format) to be verified.
 * \param	*pubKey		Signature Authority's public key bytestream (in ONS internal format).
 * \param	*md			Message digest for the file being verified.
 * \param	mdLen		Length (bytes) of the message digest in \a md.
 * \return				Returns \a True if the signature, public key and message digest
 *						all match, and otherwise \a False.
 */

Bool bagVerifyCertification(u8 *sig, u8 *pubKey, u8 *md, u32 mdLen)
{
	return(ons_verify_signature(sig, pubKey, md, mdLen));
}

/*! \brief Compute a signature from a file given the Signature Authority's secret key and a
*			signature ID.
*
* A convenience function to do all of the processing required to compute a signature from
* a file (named \a name) given the Sigature Authority's secret key (from buffer \a secKey,
* in ONS internal format) and a sequential signature ID in \a sigID.  The returned signature
* bytestream is in ONS internal format.  This is a call-through ons_compute_signature().
*
* \param	*name		Name of the file to compute the signature from.
* \param	sigID		Sequential signature ID to link this event to the meta-data.
* \param	*secKey		Signature Authority's secret key bytestream (in ONS internal format).
* \return				Pointer to the signature bytestream on success, or NULL on failure.
*/
 
u8 *bagComputeFileSignature(char *name, u32 sigID, u8 *secKey)
{
#ifdef __BAG_BIG_ENDIAN__
	swap_4((void*)&sigID);
#endif
 	return(ons_compute_signature(name, (u8*)&sigID, sizeof(u32), secKey));
}

/*! \brief Convenience function to sequence all functions required to sign a BAG file ab initio.
 *
 * A convenience function to do all of the tasks required to sign a BAG file ab initio.  This
 * computes the message digest for file \a name, signs it with the Signature Authority secret key
 * in \a secKey and the sequential signature ID in \a sigID, and then writes the \a ONSCrypto block
 * back to \a name.  This is a call-through to ons_sign_file().
 *
 * \param	*name		Name of the file to sign.
 * \param	*secKey		Signature Authority's secret key bytestream (in ONS internal format).
 * \param	sigID		Sequential signature ID (link to meta-data).
 * \return				\a True on success, otherwise \a False.
 */

Bool bagSignFile(char *name, u8 *secKey, u32 sigID)
{
	return(ons_sign_file(name, secKey, sigID));
}

/*! \brief Convenience function to sequence all functions required to verify a file ab inito.
 *
 * A convenience function to do all of the tasks required to verify a BAG file ab initio.  This
 * reads the \a ONSCrypto block from file at \a name (if it exists), computes the message digest
 * for the file, and then checks the stored signature against the MD and the Signature Authority
 * public key in bytestream \a pubKey with sequential signature ID \a sigID.  If everything matches
 * then \a True is returned; otherwise \a False.  This is a call-through to ons_verify_file().
 *
 * \param	*name		Name of the file to verify.
 * \param	*pubKey		Signature Authority's public key bytestream (in ONS internal format).
 * \param	sigID		Sequential signature ID (link to meta-data) to check with.
 * \return				Returns \a True if the stored file signature is valid, and \a False otherwise.
 */

Bool bagVerifyFile(char *name, u8 *pubKey, u32 sigID)
{
	return(ons_verify_file(name, pubKey, sigID));
}

/*! \brief Generate ab initio a public/secret key-pair in ONS internal format.
 * 
 * This generates the basic asymmetric (a.k.a., public-key) cryptographic key-pair required
 * for implementation of the Digital Signature Scheme for the Open Navigation Surface project.
 * The bytestreams are in ONS internal format as required for the rest of the library.  The secret
 * key should, of course, be suitably protected.  This is a call-through to ons_generate_keys().
 *
 * \param	**pubKey	Anchor for the public key (code will set *pubKey)
 * \param	**secKey	Anchor for the secret key (code will set *secKey)
 * \return				\a BAG_SUCCESS if key-pair was generated, otherwise a suitable error code.
 */

bagError bagGenerateKeyPair(u8 **pubKey, u8 **secKey)
{
	return(bagTranslateCryptoError(ons_generate_keys(pubKey, secKey)));
}

/*! \brief Carry out conversions between internal and ASCII formats for keys and signatures.
*
* Convert between ONS internal format representations of keys and signatures and ASCII strings
* that can be written to general output.  Both representation formats have CRC checks built in, and
* these are checked before conversion in order to ensure that the objects are valid.  ASCII strings
* are zero terminated and are printable 7-bit ASCII hex-digit strings.  Internal format strings
* have in-built length bytes.  The returned object memory (i.e., \a *converted) should be released
* by the user when it has been utilised.
*
* \param *object		Bytestream for the object (ASCII or binary) to convert.
* \param objType		Enumerated type for the object that's being passed.
* \param convDir		Enumerated type for which conversion is to be attempted.
* \param **converted	On success, \a *converted points to the converted object.
* \return				\a BAG_SUCCESS on success, or an appropriate error code on failure.
*/

bagError bagConvertCryptoFormat(u8 *object, bagCryptoObject objType, bagConvDir convDir, u8 **converted)
{
	bagError	rc = BAG_SUCCESS;
    OnsCryptErr onsErr;
	
	if (objType == bagCryptoKey) {
		if (convDir == bagConvertASCIIToBin) {
                  *converted = (u8 *) ons_ascii_to_key((char *)object, &onsErr);
		} else if (convDir == bagConvertBinToASCII) {
                  *converted = (u8 *) ons_key_to_ascii(object);
		} else {
			*converted = NULL;
			rc = BAG_CRYPTO_INTERNAL_ERROR;
		}
	} else if (objType == bagCryptoSignature) {
		if (convDir == bagConvertASCIIToBin) {
                  *converted = ons_ascii_to_sig((char *)object, &onsErr);
		} else if (convDir == bagConvertBinToASCII) {
                  *converted = (u8 *) ons_sig_to_ascii(object);
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
