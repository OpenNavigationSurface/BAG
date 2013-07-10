/*! \file onscrypto.c
 * \brief Implement cryptographic service for the ONS Digital Signature Scheme.
 *
 * This file implements cryptographic services for the ONS Digital Signature Scheme,
 * which uses the FIPS DSA, SHA and AES algorithms.  The actual computation primitives
 * are implemented via the BeeCrypt library.
 */

/*
 * $Id: onscrypto.c,v 1.1 2006/02/02 13:11:21 openns Exp $
 * $Log: onscrypto.c,v $
 * Revision 1.1  2006/02/02 13:11:21  openns
 * MP: Moving source files from the API sub-directories so they all reside in the main API directory. First part of several cleanup checkins.
 *
 * Revision 1.2  2005/10/26 17:36:06  openns
 * Updates after testing following the 2nd ONSWG meeting.  Checked in from
 * Webb McDonald's implementation of 2005-09-07.
 *
 * Revision 1.1  2005/09/07 13:48:19  mcdonaldjrw
 * Initial revision
 *
 * Revision 1.3  2005/03/15 22:55:11  openns
 * Updated to include some general user data in the ons_gen_digest() call, so that
 * the user can add more information to the hash in addition to the file.  This is used in
 * the rest of the code to add the sequence ID to the hash before finalisation, so that
 * the SID can't be modified without invalidating the signature (previously this was
 * possible with appropriate binary manipulation of the BAG and caused an obvious
 * security loophole).  Modifications to the remainder of the call sequence to support this.
 *
 * Revision 1.2  2005/01/20 16:28:32  openns
 * (brc)  Added a little more debug code to check message digest construction.
 *
 * Revision 1.1  2004/11/01 22:55:46  openns
 * Open Navigation Surface cryptographic code for signing BAGs.
 *
 *
 * File:	onscrypto.c
 * Purpose:	Code for OpenNavigationSurface cryptographic code
 * Date:	2004-09-26
 *
 * This is free source code, courtesy of the OpenNavigationSurface project.  Visit
 * the website http://www.opennavsurf.org
 *
 */
 
#include <stdio.h>
#include <stdlib.h>
#include "crc32.h"
#include "beecrypt/aes.h"
#include "beecrypt/dsa.h"
#include "beecrypt/sha1.h"
#include "beecrypt/sha256.h"
#include "beecrypt/mpnumber.h"
#include "beecrypt/mpbarrett.h"

#define DEFAULT_MD_BUFFER_LEN	40960			/*!< The default length of blocks to digest in the Message Digest */
#define DEFAULT_KEY_LEN		1024				/*!< Maximum bit length allowed for asymmetric keys */

#define ONS_CRYPTO_BLOCK_MAGIC		0x4F4E5343	/*!< Magic ID for ONSCrypto blocks, a.k.a., 'ONSC' */
#define ONS_CRYPTO_BLOCK_CUR_VER	1			/*!< Current version byte for ONSCrypto blocks */
#define ONS_CRYPTO_SIG_BLOCK_SIZE	1024		/*!< Maximum size of an ONSCrypto block, padded if required */

#undef __DEBUG__

/* The ONSCryptoBlock signature block is a fixed length block at the end of a file, as defined above,
 * broken out as follows:
 *	Offset		Length	Description
 *  0			4		Magic number identifying the block
 *	4			1		Version number
 *	5			4		Signature ID reference number
 *	9			1		Number of elements in signature (currently always 2)
 *	10			1		R-Element bytes (=: R)
 *	11			R		R-Element data
 *	11+R		1		S-Element bytes (=: S)
 *	11+R+1		S		S-Element data
 *	11+R+S+1	4		Signature CRC32 (covers SigID -> S-Element data)
 *	11+R+S+5	(N-n)	Padding (zeros)
 */

#define ONS_CRYPTO_SIG_MAGIC_OFF	0	/*!< Offset of magic number in a signature bytestream */
#define ONS_CRYPTO_SIG_VER_OFF		4	/*!< Offset to version number in a signature bytestream */
#define ONS_CRYPTO_SIG_SIGID_OFF	5	/*!< Offset to the sequential signature ID in a signature bytestream */
#define ONS_CRYPTO_SIG_SIG_OFF		9	/*!< Offset to the binary signature data in a signature bytestream */

/* Internally, a key is represented by a raw u8 sequence, pre-pended with a count of the number
 * of numbers to follow:
 *	Offset		Length	Description
 *	0			1		Number of key numbers following (=: K)
 *	1			1		Length of key to follow (=: L1)
 *	2			L1		Key 1 of K
 *	2+L1		1		Length of key to follow (=: L2)
 *	2+L1+1		L2		Key 2 of K
 *  ...			...		...
 *	N			4		CRC32 of everything from start to end of last key
 */

#define ONS_CRYPTO_KEY_NKEYS_OFF	0	/*!< Offset to the key integer count in a key bytestream */
#define ONS_CRYPTO_KEY_KEY1_OFF		1	/*!< Offset to the start of key 1 binary data in a key bytestream */

#include "onscrypto.h"

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

/*! \brief Determine whether a particular file has an ONSCrypto block present
 *
 * This checks for the presence of an ONSCrypto block in \a file.  The ONSCrypto block is a
 * fixed size block of data at the end of a file, and therefore can always be checked by running
 * backwards by this size from the end of the file and looking for the magic number above.  This
 * only checks that the block exists, and doesn't say anything about whether the block is valid.
 * Use ons_read_file_sig() to read and validate.
 *
 * \param	*file		Name of the file to check for a block.
 * \return				\a True if the ONSCrypto block exists in \a file, or \a False if not.  Also
 *						returns \a False if the file doesn't exist, or can't be opened as required, etc.
 */

Bool ons_check_cblock(char *file)
{
	FILE	*f;
	u32	file_magic;
	
	if ((f = fopen(file, "rb")) == NULL) {
		fprintf(stderr, "error: failed to open \"%s\" for ONSCryptoBlock check.\n",
			file);
		return(False);
	}
	fseek(f, -ONS_CRYPTO_SIG_BLOCK_SIZE, SEEK_END);
	fread(&file_magic, sizeof(u32), 1, f);
#ifdef __BAG_BIG_ENDIAN__
	swap_4((void*)&file_magic);
#endif
	fclose(f);
	return(file_magic == ONS_CRYPTO_BLOCK_MAGIC);
}

/*! \brief Compute a Message Digest from a file, including arbitrary user data.
 *
 * This computes a Message Digest from \a file (minus the ONSCrypto block, if it exists), including
 * \a user_data[] if non-NULL, of length \a user_data_len.  For the ONS DSS, this should be a U32
 * containing a sequential signature ID (in LSB format) to tie the signature event to the meta-data in the file.  The
 * length of the MD depends on the algorithm being used, and the returned buffer has no sentinals; use
 * \a *nBytes as a length count.  The memory returned should be free()d by the caller after use.
 *
 * \param	*file			Name of the file to digest.
 * \param	*user_data		Pointer to some user data to digest along with \a file.
 * \param	user_data_len	Number of bytes of user data to digest.
 * \param	*nbytes			Number of bytes returned to the user on success.
 * \return					Returns a pointer to a bytestream of Message Digest on success, or NULL
 *							on failure.
 */

u8 *ons_gen_digest(char *file, u8 *user_data, u32 user_data_len, u32 *nbytes)
{
	static sha1Param	mdparam;
	FILE		*f;
	u8   		*rtn;
    const u8    *buffer;
	u32			n_read, tot_read, buffer_len = DEFAULT_MD_BUFFER_LEN;
	u32			len;
	Bool		done = False, has_crypto_block;
    size_t elsize;
	
#ifdef __DEBUG_HASH__
	fprintf(stderr, "debug: computing message digest for \"%s\"\n", file);
#endif

	if ((buffer = (const u8 *)malloc(buffer_len)) == NULL) {
		fprintf(stderr, "error: failed to get memory for MD hash buffer (%d bytes).\n",
			buffer_len);
		return(NULL);
	}
	has_crypto_block = ons_check_cblock(file);
	if ((f = fopen(file, "rb")) == NULL) {
		fprintf(stderr, "error: failed to open \"%s\" for input MD computation.\n",
			file);
		return(NULL);
	}
	
	fseek(f, 0, SEEK_END); 
    len = ftell(f); 
    rewind(f);

	if (has_crypto_block) 
        len -= ONS_CRYPTO_SIG_BLOCK_SIZE;

#ifdef __DEBUG_HASH__
	fprintf(stderr, "debug: file \"%s\" active length %d bytes (%s crypto block).\n",
		file, len, has_crypto_block ? "has" : "has no");
#endif

	if (sha1Reset(&mdparam)) {
		fprintf(stderr, "error: failed to set up SHA-256 for MD of \"%s\".\n", file);
		return(NULL);
	}

	tot_read = 0;
	while (!done && !feof(f)) {
		n_read = (u32)fread((void *)buffer, 1, buffer_len, f);
		tot_read += n_read;
		if (n_read < buffer_len || tot_read > len) {
			if (ferror(f)) {
				fprintf(stderr, "error: failed reading \"%s\" for MD.\n", file);
				return(NULL);
			}

			if (tot_read > len) 
                n_read = len - (tot_read - n_read);

#ifdef __DEBUG_HASH__
			fprintf(stderr, "debug: tot_read = %d, n_read = %d.\n", tot_read, n_read);
#endif
			done = True;
		}

        elsize = (size_t)(n_read * sizeof(u8));

        
		if (sha1Update(&mdparam, buffer, (size_t)elsize)) {
			fprintf(stderr, "error: failed to add block to MD for \"%s\".\n", file);
			return(NULL);
		}
		/* ungetc(getc(f), f); */
	}

    
	fclose(f);
	*nbytes = 20;
	if ((rtn = (u8 *)malloc(*nbytes)) == NULL) {
		fprintf(stderr, "error: failed to get memory for MD of \"%s\" (%d bytes).\n",
			file, *nbytes);
		return(NULL);
	}
	if (user_data != NULL && sha1Update(&mdparam, user_data, user_data_len)) {
		fprintf(stderr, "error: failed to add user data to MD for \"%s\".\n", file);
		return(NULL);
	}
	if (sha1Digest(&mdparam, rtn)) {
		fprintf(stderr, "error: failed to compute MD from \"%s\".\n", file);
		free(rtn);
		return(NULL);
	}
#ifdef __DEBUG_HASH__
	fprintf(stderr, "debug: hash of \"%s\" is: ", file);
	for (len = 0; len < 20; ++len)
		fprintf(stderr, "%02X", rtn[len]);
	fprintf(stderr, "\n");
#endif
	return(rtn);
}

/*! \brief Compute the length in bytes of an internal ONS format bytestream
 *
 * This computes the length in bytes of an internal ONS format bytestream (i.e., either a key
 * or a signature).  The internal format is a byte containing the number of elements (basically
 * long-format integers) in the stream, and then for each object a single byte containing the
 * length in bytes of the object, then the object.  This code just runs the list and counts
 * the declared lengths.
 *
 * \param	*data		Pointer to the ONS format bytestream to count.
 * \return				Length of the bytestream in bytes.
 */

static u32 ons_compute_int_len(u8 *data)
{
	u32	n_nums, num_len, num, tot_len;

	n_nums = (u32)data[0];
	tot_len = 1;	/* Number of numbers following = 1 byte */
	for (num = 0; num < n_nums; ++num) {
		num_len = (u32)data[tot_len];
		tot_len += 1 + num_len;
	}
	return(tot_len);
}	

/*! \brief Read an ONSCrypto signature block from a file, verify contents, and return signature.
 *
 * This checks for the presence of an ONSCrypto signature block on the end of \a file, and if it
 * exists, checks that the format is valid.  If so, the signature bytestream is read and converted
 * into internal ONS format, and the sequential signature ID is read and converted.
 *
 * \param	*file		Name of the file to check for an ONSCrypto block.
 * \param	*sig		Pointer to some user-allocated buffer space for the signature to be returned.
 * \param	*sigid		Pointer to some user-allocated space to store the sequential signature ID to be returned.
 * \param	nbuf		Space available in \a *sig for the signature to be returned.
 * \return				An appropriate error code, or \a ONS_CRYPTO_SIG_OK on success.
 */

OnsCryptErr ons_read_file_sig(char *file, u8 *sig, u32 *sigid, u32 nbuf)
{
	FILE	*f;
	u8		buffer[ONS_CRYPTO_SIG_BLOCK_SIZE];
	u32		file_magic, file_version, file_crc, sig_crc, sig_len;
#ifdef __DEBUG__
	u32		i;
#endif

	if (nbuf < ONS_CRYPTO_MAX_SIG_LEN) {
		fprintf(stderr, "error: signature buffer no long enough (need %d bytes).\n",
			ONS_CRYPTO_MAX_SIG_LEN);
		return(ONS_CRYPTO_ERR);
	}
	if ((f = fopen(file, "rb")) == NULL) {
		fprintf(stderr, "error: failed to open \"%s\" for signature block read.\n",
			file);
		return(ONS_CRYPTO_FILE_ERR);
	}
	fseek(f, -ONS_CRYPTO_SIG_BLOCK_SIZE, SEEK_END);
	fread(buffer, sizeof(u8), ONS_CRYPTO_SIG_BLOCK_SIZE, f);
	fclose(f);
	
	file_magic = *((u32*)(buffer + ONS_CRYPTO_SIG_MAGIC_OFF));
#ifdef __BAG_BIG_ENDIAN__
	swap_4((void*)&file_magic);
#endif
	if (file_magic != ONS_CRYPTO_BLOCK_MAGIC) {

#ifdef __DEBUG__
		fprintf(stderr, "debug: found \"%x\" for crypto block magic number, not \"%x\".\n",
			file_magic, ONS_CRYPTO_BLOCK_MAGIC);
#endif

		return(ONS_CRYPTO_NO_SIG_FOUND);
	}
	file_version = *(buffer + ONS_CRYPTO_SIG_VER_OFF);
	if (file_version != ONS_CRYPTO_BLOCK_CUR_VER) {
		fprintf(stderr, "error: ONSCryptoBlock not correct version (input %d, current %d).\n",
			(u32)file_version, (u32)ONS_CRYPTO_BLOCK_CUR_VER);
		return(ONS_CRYPTO_BAD_SIG_BLOCK);
	}
	sig_len = ons_compute_int_len(buffer + ONS_CRYPTO_SIG_SIG_OFF);

#ifdef __DEBUG__
	fprintf(stderr, "debug: signature length = %d bytes\n", sig_len);
#endif

	file_crc = *((u32*)(buffer + ONS_CRYPTO_SIG_SIG_OFF + sig_len));
#ifdef __BAG_BIG_ENDIAN__
	swap_4((void*)&file_crc);
#endif
	sig_crc = crc32_calc_buffer((char *)buffer+ONS_CRYPTO_SIG_SIGID_OFF, sig_len+sizeof(u32));
#ifdef __DEBUG__
	fprintf(stderr, "debug: signature ID is: 0x%08x\n", *((u32*)(buffer+ONS_CRYPTO_SIG_SIGID_OFF)));
	fprintf(stderr, "debug: signature is: ");
	for (i = 0; i < sig_len; ++i)
		fprintf(stderr, "%02x", buffer[ONS_CRYPTO_SIG_SIG_OFF + i]);
	fprintf(stderr, "\n");
#endif
	if (file_crc != sig_crc) {
		fprintf(stderr, "error: CRC32 failure for ONSCryptoBlock (bad signature block).\n");

#ifdef __DEBUG__
		fprintf(stderr, "debug: file CRC32 = 0x%x, recomputed CRC32 = 0x%x\n", file_crc, sig_crc);
#endif

		return(ONS_CRYPTO_BAD_SIG_BLOCK);
	}
	*sigid = *((u32*)(buffer + ONS_CRYPTO_SIG_SIGID_OFF));

#ifdef __BAG_BIG_ENDIAN__
	swap_4((void*)sigid);
#endif

	*((u32*)(buffer+ONS_CRYPTO_SIG_SIG_OFF+sig_len)) = crc32_calc_buffer((char *)buffer + ONS_CRYPTO_SIG_SIG_OFF, sig_len);

#ifdef __BAG_BIG_ENDIAN__
	swap_4((void*)(buffer+ONS_CRYPTO_SIG_SIG_OFF+sig_len));
#endif

	memcpy(sig, buffer + ONS_CRYPTO_SIG_SIG_OFF, sig_len + 4 /* CRC */);
	return(ONS_CRYPTO_SIG_OK);
}

/*! \brief Write a signature and sequential signature ID into a file.
 *
 * This takes the internal ONS format bytestream from \a *sig, and the sequential signature ID in
 * \a sigid, and write them into \a file, appending a new ONSCrypto signature block if required.
 *
 * \param	*file		Name of the file to write information into.
 * \param	*sig		Internal ONS bytestream containing the signture to write.
 * \param	sigid		Sequential signature ID number used to link this event to the meta-data.
 * \return				A suitable error code, or \a ONS_CRYPTO_OK on success.
 */

OnsCryptErr ons_write_file_sig(char *file, u8 *sig, u32 sigid)
{
	FILE	*f;
	u32		file_magic, sig_crc, crc, sig_len;
	u8		buffer[ONS_CRYPTO_SIG_BLOCK_SIZE];
#ifdef __DEBUG__
	u32		i;
#endif
	
	sig_len = ons_compute_int_len(sig);
#ifdef __DEBUG__
	fprintf(stderr, "debug: signature length = %d bytes.\n", sig_len);
#endif
	crc = crc32_calc_buffer((char *)sig, sig_len);
#ifdef __DEBUG__
	fprintf(stderr, "debug: signature CRC32 = 0x%x\n", crc);
#endif
	sig_crc = *((u32*)(sig + sig_len));

#ifdef __BAG_BIG_ENDIAN__
	swap_4((void*)&sig_crc);
#endif

	if (sig_crc != crc) {
		fprintf(stderr, "error: CRC passed in for signature doesn't match (IN(%08X) != %08X).\n",
			sig_crc, crc);
		return(ONS_CRYPTO_BAD_SIG_BLOCK);
	}
	memset(buffer, 0, ONS_CRYPTO_SIG_BLOCK_SIZE);
	*((u32*)(buffer + ONS_CRYPTO_SIG_MAGIC_OFF)) = ONS_CRYPTO_BLOCK_MAGIC;

#ifdef __BAG_BIG_ENDIAN__
	swap_4((void*)(buffer + ONS_CRYPTO_SIG_MAGIC_OFF));
#endif

	buffer[ONS_CRYPTO_SIG_VER_OFF] = ONS_CRYPTO_BLOCK_CUR_VER;
	*((u32*)(buffer + ONS_CRYPTO_SIG_SIGID_OFF)) = sigid;

#ifdef __BAG_BIG_ENDIAN__
	swap_4((void*)(buffer + ONS_CRYPTO_SIG_SIGID_OFF));
#endif

	memcpy(buffer + ONS_CRYPTO_SIG_SIG_OFF, sig, sig_len);

#ifdef __DEBUG__
	fprintf(stderr, "debug: signature ID is 0x%08x\n", sigid);
	fprintf(stderr, "debug: signature is: ");
	for (i = 0; i < sig_len; ++i)
		fprintf(stderr, "%02x", sig[i]);
	fprintf(stderr, "\n");
#endif
	
	if ((f = fopen(file, "r+b")) == NULL) {
		fprintf(stderr, "error: failed to open \"%s\" for ONSCryptoBlock write.\n",
			file);
		/*free(buffer);*/
		return(ONS_CRYPTO_FILE_ERR);
	}
	fseek(f, -ONS_CRYPTO_SIG_BLOCK_SIZE, SEEK_END);
	fread(&file_magic, sizeof(u32), 1, f);

#ifdef __BAG_BIG_ENDIAN__
	swap_4((void*)&file_magic);
#endif

	if (file_magic != ONS_CRYPTO_BLOCK_MAGIC) {
		/* No signature in file, wind to end and write magic number */
		fseek(f, 0, SEEK_END);
	} else {
		fseek(f, -4, SEEK_CUR);	/* Rewind the size of the magic ID and then write block */
	}
	/* Compute the file-based CRC32 to protect the SigID and signature */
	*((u32*)(buffer + ONS_CRYPTO_SIG_SIG_OFF + sig_len)) =
          crc32_calc_buffer((char *)buffer + ONS_CRYPTO_SIG_SIGID_OFF, sig_len + sizeof(u32));

#ifdef __BAG_BIG_ENDIAN__
	swap_4((void*)(buffer + ONS_CRYPTO_SIG_SIG_OFF + sig_len));
#endif

#ifdef __DEBUG__
	fprintf(stderr, "debug: file CRC32 = 0x%x\n", *((u32*)(buffer+ONS_CRYPTO_SIG_SIG_OFF+sig_len)));
#endif
	fwrite(buffer, sizeof(u8), ONS_CRYPTO_SIG_BLOCK_SIZE, f);
	fclose(f);
	return(ONS_CRYPTO_OK);
}

/*! \brief Simple converter from binary data to ASCII hexstring for output.
 *
 * Convert from binary data into an ASCII 7-bit clean printable hexstring
 * for output to text files.  Output is zero terminated.  No interpretation of
 * the binary data is done.
 *
 * \param	*data		Pointer to data buffer to transcribe.
 * \param	n_bytes		Number of bytes of data in the buffer to transcribe.
 * \return				Pointer to memory buffer containing the ASCII string for the
 *						binary data, or NULL on failure.
 */

char *ons_bin_to_ascii(u8 *data, u32 n_bytes)
{
	char	*rtn;
	u32	i, n_out_bytes;
	
	n_out_bytes = 2*n_bytes + 1;
	if ((rtn = (char *)malloc(n_out_bytes)) == NULL) {
		fprintf(stderr, "error: no memory for bin to ASCII conversion.\n");
		return(NULL);
	}
	for (i = 0; i < n_bytes; ++i)
		sprintf(rtn + i*2, "%02X", (u32)data[i]);
	return(rtn);
}

/*! \brief Convert from ONS internal format bytestream to ASCII hexstring for output.
 *
 * This converts an ONS internal format bytestream into an ASCII hexstring for output.  The
 * code also checks the internal CRC32 used to protect the bytestream in order to ensure that
 * the data is still valid.  This assumes that a CRC32 follows the basic data, and that the \a len
 * value does not include this extra four bytes.  The output stream has an ASCII CRC32 appended,
 * in order to protect the data in transit, and is zero terminated.  The caller should free() this
 * memory when it's been used.
 *
 * \param	*intdat		Pointer to the ONS internal format bytestream.
 * \param	len			Length of the bytestream in bytes (not including the CRC32 at the end).
 * \return				Pointer to the ASCII hexstring representation of the bytestream, or NULL
 *						on failure.
 */

char *ons_int_to_ascii(u8 *intdat, u32 len)
{
	u32		n_out_bytes, crc;
	char	*rtn, *asc;
	
	n_out_bytes = len*2 + sizeof(u32)*2 /* CRC */ + sizeof(u32)*2 /* ASCII CRC */ + 1;
	if ((rtn = (char*)malloc(n_out_bytes)) == NULL) {
		fprintf(stderr, "error: failed to get memory for ASCII output (need %d bytes).\n",
			n_out_bytes);
		return(NULL);
	}
	if ((asc = ons_bin_to_ascii(intdat, len + sizeof(u32))) == NULL) {
		fprintf(stderr, "error: failed to convert internal format to ASCII.\n");
		free(rtn);
		return(NULL);
	}
	crc = crc32_calc_buffer(asc, (len+sizeof(u32))*2);
	memcpy(rtn, asc, (len+sizeof(u32))*2);
	free(asc);
	sprintf(rtn + (len+sizeof(u32))*2, "%08X", crc);
	return(rtn);
}

/*! \brief Convenience function to convert from a signature bytestream to ASCII.
 *
 * This is a convenience call-through to ons_int_to_ascii() which also computes the
 * length of the signature bytestream.  User should free() the returned memory once
 * it has been used.  Note that the ASCII string has an ASCII CRC32 appended to the
 * data so that the bytestream is protected in transit, and is zero terminated so
 * that it can be fprintf()d directly if required.
 *
 * \param	*sig		Pointer to the ONS internal format signature bytestream.
 * \return				Pointer to the ASCII buffer on success, or NULL on failure.
 */

char *ons_sig_to_ascii(u8 *sig)
{
	return(ons_int_to_ascii(sig, ons_compute_int_len(sig)));
}

/*! \brief Convenience function to convert from a key bytestream to ASCII.
 *
 * This is a convenience call-through to ons_int_to_ascii() which also computes the
 * length of the key bytestream.  User should free() the returned memory once
 * it has been used.  Note that the ASCII string has an ASCII CRC32 appended to the
 * data so that the bytestream is protected in transit, and is zero terminated so
 * that it can be fprintf()d directly if required.
 *
 * \param	*key		Pointer to the ONS internal format key bytestream.
 * \return				Pointer to the ASCII buffer on success, or NULL on failure.
 */

char *ons_key_to_ascii(u8 *key)
{
	return(ons_int_to_ascii(key, ons_compute_int_len(key)));
}

/*! \brief Convert from ASCII hexstring representations of objects into internal ONS bytestream format.
 *
 * This converts from ASCII hexstring format into ONS internal bytestream format.  The ASCII
 * string is assumed to have been generated by ons_int_to_ascii() so that it has an ASCII format
 * CRC32 on the end to check for validity.  This code checks for the CRC's validity but does not
 * make any other interpretations of the data.
 *
 * \param	*ascii		Pointer to the ASCII input string.
 * \param	*errcd		On output, error code for the conversion, or \a ONS_CRYPTO_OK on success.
 * \return				Pointer to internal format bytestream.
 */

static u8 *ons_ascii_to_int(char *ascii, OnsCryptErr *errcd)
{
	u32		ascii_len, i, crc, digit;
	u8		*rtn;
	char	ascii_crc[9];
	
	ascii_len = (u32)strlen(ascii) - sizeof(u32)*2; /* Less size of CRC */
	crc = crc32_calc_buffer(ascii, ascii_len);
	sprintf(ascii_crc, "%08X", crc);
	if (strcmp(ascii_crc, ascii + ascii_len) != 0) {
		fprintf(stderr, "error: CRC check failure on input signature.\n");
		*errcd = ONS_CRYPTO_BAD_SIG_BLOCK;
		return(NULL);
	}
	if ((rtn = (u8*)malloc(ascii_len/2)) == NULL) {
		fprintf(stderr, "error: no memory for signature (need %d bytes).\n",
			ascii_len/2);
		*errcd = ONS_CRYPTO_ERR;
		return(NULL);
	}
	for (i = 0; i < ascii_len/2; ++i) {
		sscanf(ascii + i*2, "%02X", &digit);
		rtn[i] = (u8)(digit & 0xFF);
	}
	*errcd = ONS_CRYPTO_OK;
	return(rtn);
}

/*! \brief Convert from an ASCII hexstring into internal ONS bytestream format.
 *
 * This is a convenience function call-through to ons_ascii_to_int(), used to convert from
 * an ASCII hexstring into an internal format bytestream.  The ASCII hexstring is assumed to
 * have been generated by ons_int_to_ascii() so that it has a trailing ASCII CRC32 element
 * that can be used to check for validity of the ASCII representation.
 *
 * \param	*sig		Pointer to the signature ASCII hexstring to convert
 * \param	*errcd		Pointer to space for error code, which is set to \a ONS_CRYPTO_OK on
 *						success, or appropriately on failure.
 * \return				Pointer to internal format ONS bytestream, or NULL on failure.
 */

u8 *ons_ascii_to_sig(char *sig, OnsCryptErr *errcd)
{
	return(ons_ascii_to_int(sig, errcd));
}

/*! \brief Convert from an ASCII hexstring into internal ONS bytestream format.
 *
 * This is a convenience function call-through to ons_ascii_to_int(), used to convert from
 * an ASCII hexstring into an internal format bytestream.  The ASCII hexstring is assumed to
 * have been generated by ons_int_to_ascii() so that it has a trailing ASCII CRC32 element
 * that can be used to check for validity of the ASCII representation.
 *
 * \param	*key		Pointer to the key ASCII hexstring to convert
 * \param	*errcd		Pointer to space for error code, which is set to \a ONS_CRYPTO_OK on
 *						success, or appropriately on failure.
 * \return				Pointer to internal format ONS bytestream, or NULL on failure.
 */

u8 *ons_ascii_to_key(char *key, OnsCryptErr *errcd)
{
	return(ons_ascii_to_int(key, errcd));
}

/*! \brief Convert from BeeCrypt Multi-precision Integer to ONS bytestream format.
 *
 * This converts from the format used for Multi-precision Integers in the support cryptographic
 * library to the format used for ONS output and formatting.  The external format has a pre-pended
 * length byte, and then the data as generated by i2osp().  This only works for unsigned numbers,
 * and therefore won't work for all MPIs.
 *
 * \param *mpi	Pointer to the MPI number for BeeCrypt
 * \return		Pointer to the ONS internal format number, or NULL on failure.
 */

static u8 *ons_mpn_to_int(mpnumber *mpi)
{
	u8	*buffer;
	u32	nbits, nbytes;
	
	nbits = (u32)mpnbits(mpi);
	nbytes = ((nbits + 7)>>3) + (((nbits & 7) == 0) ? 1 : 0);
	
	if ((buffer = (u8*)malloc(nbytes + 1)) == NULL) {
		fprintf(stderr, "error: no memory for MPI conversion (need %d bytes).\n", nbytes);
		return(NULL);
	}
	i2osp(buffer+1, nbytes, mpi->data, mpi->size);
	buffer[0] = (u8)(nbytes & 0xFF);
	return(buffer);
}

/*! \brief Convert from BeeCrypt MPB ormat to ONS internal format
 *
 * This converts from BeeCrypt's internal MPB format into ONS internal format.  The format has
 * a pre-pended length byte, and then the data from i2osp().
 *
 * \param	*mpb	Pointer to the Barrett format to convert
 * \return			Pointer to the ONS internal format bytestream, or NULL on failure.
 */

static u8 *ons_mpb_to_int(mpbarrett *mpb)
{
	u8	*buffer;
	u32	nbits, nbytes;
	
	nbits = (u32)mpbbits(mpb);
	nbytes = ((nbits + 7)>>3) + (((nbits & 7) == 0) ? 1 : 0);
	
	if ((buffer = (u8*)malloc(nbytes + 1)) == NULL) {
		fprintf(stderr, "error: no memory for MPI conversion (need %d bytes).\n", nbytes);
		return(NULL);
	}
	i2osp(buffer+1, nbytes, mpb->modl, mpb->size);
	buffer[0] = (u8)(nbytes & 0xFF);
	return(buffer);
}

/*! \brief Convert from ONS internal format to BeeCrypt MPN format
 *
 * This converts from ONS internal format to BeeCrypt MPN format for further
 * computation.  This assumes that the number is an unsigned form so that it can be converted
 * into an MPN at all.
 *
 * \param	*num	Pointer to ONS internal format number to convert.
 * \return			Pointer to the BeeCrypt MPN, or NULL on failure.
 */

static mpnumber *ons_int_to_mpn(u8 *num)
{
	u32			nbytes = (u32)(num[0]);
	mpnumber	*mpi;
	
	if ((mpi = (mpnumber *)malloc(sizeof(mpnumber))) == NULL) {
		fprintf(stderr, "error: failed to get memory for MPI.\n");
		return(NULL);
	}
	
	mpnzero(mpi);
	mpnsetbin(mpi, num+1, nbytes);

	
	return(mpi);
}

/*! \brief Convert from ONS internal format number to BeeCrypt MPB number.
 *
 * Convert from ONS internal format number to BeeCrypt MPB number format.  This works
 * for general numbers.
 *
 * \param	*num	Pointer to the ONS internal format number.
 * \return			Pointer to the BeeCrypt MPB number, or NULL on failure.
 */

static mpbarrett *ons_int_to_mpb(u8 *num)
{
	u32			nbytes = (u32)(num[0]);
	mpbarrett	*mpb;
	
	if ((mpb = (mpbarrett *)malloc(sizeof(mpbarrett))) == NULL) {
		fprintf(stderr, "error: failed to get memory for MPB.\n");
		return(NULL);
	}
	
	mpbzero(mpb);
	mpbsetbin(mpb, num+1, nbytes);
	
	return(mpb);
}

/*! \brief Convert from an BeeCrypt format into ONS internal key bytestreamm.
 *
 * This converts from the FIPS Digital Signature Algorithm format parameter structure and
 * public/private number for the key into an ONS internal bytestream format.  The output
 * code consists of the (p,q,g) components of the DSA domain, and then the key.  The output
 * bytestream therefore has four integers, with a pre-pended integer count of one byte, and
 * a post-fix CRC32 number of four bytes to protect the contents of the bytestream in transit.
 *
 * \param	*param	Pointer to the DSA domain parameters for the signature stream.
 * \param	*key	Pointer to either public or private key MPN for the signature stream.
 * \return			Pointer to the ONS internal format bytestream, or NULL on failure.
 */

static u8 *ons_key_to_int(dsaparam *param, mpnumber *key)
{
	u32		nbytes, offset;
	u8		*inp, *inq, *ing, *ink, *rtn;
	
	inp = inq = ing = ink = NULL;
	if ((inp = ons_mpb_to_int(&param->p)) == NULL || (inq = ons_mpb_to_int(&param->q)) == NULL ||
		(ing = ons_mpn_to_int(&param->g)) == NULL || (ink = ons_mpn_to_int(key)) == NULL) {
		fprintf(stderr, "error: failed to convert key to internal format.\n");
		free(inp); free(inq); free(ing); free(ink);
		return(NULL);
	}
	nbytes = 1 + (inp[0] + 1) + (inq[0] + 1) + (ing[0] + 1) + (ink[0] + 1);
	if ((rtn = (u8*)malloc(nbytes + 4 /* CRC */)) == NULL) {
		fprintf(stderr, "error: failed to get memory for internal format key.\n");
		free(inp); free(inq); free(ing); free(ink);	
		return(NULL);
	}
	rtn[0] = 4; offset = 1;
	memcpy(rtn + offset, inp, inp[0] + 1); offset += inp[0] + 1;
	memcpy(rtn + offset, inq, inq[0] + 1); offset += inq[0] + 1;
	memcpy(rtn + offset, ing, ing[0] + 1); offset += ing[0] + 1;
	memcpy(rtn + offset, ink, ink[0] + 1); offset += ink[0] + 1;
	*((u32*)(rtn + offset)) = crc32_calc_buffer((char *)rtn, nbytes);

#ifdef __BAG_BIG_ENDIAN__
	swap_4((void*)(rtn + offset));
#endif

	free(inp); free(inq); free(ing); free(ink);	
	return(rtn);
}

/*! \brief Convert from ONS internal bytestream key to BeeCrypt key parameters.
 *
 * This converts from the ONS internal format bytestream to separate BeeCrypt compatible
 * individual numbers ready to do DSA verification.  This assumes that the \a in[] bytestream
 * was converted fromm ons_key_to_int().
 *
 * \param	*in		Pointer to the input ONS internal format bytestream.
 * \param	**p		Anchor for output DSA component (i.e., *p is set to point to the converted item).
 * \param	**q		Anchor for output DSA component (i.e., *q is set to point to the converted item).
 * \param	**g		Anchor for output DSA component (i.e., *g is set to point to the converted item).
 * \param	**key	Anchor for output key component (i.e., *key is set to point to the converted item).
 * \return			True on success, otherwise False.
 */

static Bool ons_int_to_key(u8 *in, mpbarrett **p, mpbarrett **q, mpnumber **g, mpnumber **key)
{
	u32	offset;
	
	if (in[0] != 4) {
		fprintf(stderr, "error: internal format doesn't have four integers!\n");
		return(False);
	}
	offset = 1;
	if ((*p = ons_int_to_mpb(in + offset)) == NULL) {
		fprintf(stderr, "error: failed to convert P element for DSA signature.\n");
		return(False);
	}
	offset += in[offset] + 1;
	if ((*q = ons_int_to_mpb(in + offset)) == NULL) {
		fprintf(stderr, "error: failed to convert Q element for DSA signature.\n");
		mpbfree(*p); free(*p);
		return(False);
	}
	offset += in[offset] + 1;
	if ((*g = ons_int_to_mpn(in + offset)) == NULL) {
		fprintf(stderr, "error: failed to convert G element for DSA signature.\n");
		mpbfree(*p); free(*p); mpbfree(*q); free(*q);
		return(False);
	}
	offset += in[offset] + 1;
	if ((*key = ons_int_to_mpn(in + offset)) == NULL) {
		fprintf(stderr, "error: failed to convert key element for DSA signature.\n");
		mpbfree(*p); free(*p); mpbfree(*q); free(*q); mpnfree(*g); free(*g);
		return(False);
	}

	return(True);
}

/*! \brief Generate an asymmetric key-pair for DSA usage.
 *
 * This generates an asymmetric cryptography key-pair suitable for use with the Digital Signature
 * Algorithm.  The strength of the key-pair depends on the strength of the cryptographically
 * secure PRNG system, which may vary according to the amount of entropy that the system has
 * gathered (this depends on the system being used, but is generally data from the audio inputs
 * on the computer).  The keys are split and converted into internal format before being returned.
 * The secret component of the key-pair should, of course, be treated with an appropriate degree
 * of respect.
 *
 * \param	**pkey	Anchor for the public key of the pair (i.e., *pkey is set to point to the key)
 * \param	**skey	Anchor for the secret key of the pair (i.e., *skey is set to point to the key)
 * \return			Suitable error code on failure, or \a ONS_CRYPTO_OK on success.
 */

OnsCryptErr ons_generate_keys(u8 **pkey, u8 **skey)
{
	dsaparam				param;
	randomGeneratorContext	rgc;
	mpnumber				priv, pub;
	
	if (randomGeneratorContextInit(&rgc, randomGeneratorDefault()) != 0) {
		fprintf(stderr, "error: failed to initialise RNG for DSA signature.\n");
		return(ONS_CRYPTO_ERR);
	}
	dldp_pInit(&param);
	if (dsaparamMake(&param, &rgc, DEFAULT_KEY_LEN)) {
		fprintf(stderr, "error: failed to make key for DSA signature (length %d bits).\n", DEFAULT_KEY_LEN);
		randomGeneratorContextFree(&rgc);
		return(ONS_CRYPTO_ERR);
	}
	mpnzero(&priv); mpnzero(&pub);
	if (dldp_pPair(&param, &rgc, &priv, &pub)) {
		fprintf(stderr, "error: failed to make keys for DSA signature (length %d bits).\n", DEFAULT_KEY_LEN);
		randomGeneratorContextFree(&rgc);
		return(ONS_CRYPTO_ERR);
	}
	randomGeneratorContextFree(&rgc);
	
	*pkey = *skey = NULL;
	if ((*pkey = ons_key_to_int(&param, &pub)) == NULL || (*skey = ons_key_to_int(&param, &priv)) == NULL) {
		fprintf(stderr, "error: failed to convert keys into internal format for DSA algorithm.\n");
		free(*pkey); free(*skey);
		return(ONS_CRYPTO_ERR);
	}
	return(ONS_CRYPTO_OK);
}

/*! \brief Convert from a BeeCypt signature into an ONS internal format bytestream.
 *
 * This converts from the components (r,s) of a DSA style signature into an ONS internal
 * format bytestream.  This consists of a byte containing the number of components in
 * the signature (always 2) and then the components numbers (r, then s).  A CRC32 is
 * appended to the end of the sequence to provide some protection in transit.
 *
 * \param	*r	Pointer to the first signature component.
 * \param	*s	Pointer to the second signature component.
 * \return		Pointer to the ONS internal format bytestream for the signature, or NUL
 *				on failure.
 */

static u8 *ons_sig_to_int(mpnumber *r, mpnumber *s)
{
	u32	nbytes, offset;
	u8	*inr, *ins, *rtn;
	
	inr = ins = NULL;
	if ((inr = ons_mpn_to_int(r)) == NULL || (ins = ons_mpn_to_int(s)) == NULL) {
		fprintf(stderr, "error: failed to convert signature to internal format.\n");
		free(inr); free(ins);
		return(NULL);
	}
	nbytes = 1 + (inr[0] + 1) + (ins[0] + 1);
	if ((rtn = (u8*)malloc(nbytes + sizeof(u32))) == NULL) {
		fprintf(stderr, "error: failed to get memory for internal format signature.\n");
		return(NULL);
	}
	rtn[0] = 2; offset = 1;
	memcpy(rtn + offset, inr, inr[0] + 1); offset += inr[0] + 1;
	memcpy(rtn + offset, ins, ins[0] + 1); offset += ins[0] + 1;
	*((u32*)(rtn + offset)) = crc32_calc_buffer((char *)rtn, nbytes);
#ifdef __BAG_BIG_ENDIAN__
	swap_4((void*)(rtn + offset));
#endif
	return(rtn);
}

/*! \brief Convert from ONS internal bytestream format to BeeCrypt compatible numbers.
 *
 * This converts from ONS internal bytestream format to broken-out numbers compatible
 * with BeeCrypt for signature verification purposes.
 *
 * \param	*sig	Pointer to the ONS internal format bytestream.
 * \param	**r		Anchor for BeeCrypt output component (i.e., *r is set to the output value).
 * \param	**s		Anchor for BeeCrypt output component (i.e., *s is set to the output value).
 * \return			Suitable error code, or ONS_CRYPTO_OK on success.
 */

static OnsCryptErr ons_int_to_sig(u8 *sig, mpnumber **r, mpnumber **s)
{
	u32	n_ints, in_crc, sig_crc, sig_len, offset;
	
	sig_len = ons_compute_int_len(sig);
	sig_crc = *((u32*)(sig + sig_len));

#ifdef __BAG_BIG_ENDIAN__
	swap_4((void*)&sig_crc);
#endif

	in_crc = crc32_calc_buffer((char *)sig, sig_len);
	if (sig_crc != in_crc) {
		fprintf(stderr, "error: failed CRC32 check on signature on input.\n");
		return(ONS_CRYPTO_ERR);
	}
	n_ints = (u32)sig[0];
	if (n_ints != 2) {
		fprintf(stderr, "error: internal data is not a signature (not two elements).\n");
		return(ONS_CRYPTO_ERR);
	}
	offset = 1; *r = *s = NULL;
	if ((*r = ons_int_to_mpn(sig + offset)) == NULL) {
		fprintf(stderr, "error: failed to convert R component of DSA signature.\n");
		return(ONS_CRYPTO_ERR);
	}
	offset += sig[offset] + 1;
	if ((*s = ons_int_to_mpn(sig + offset)) == NULL) {
		fprintf(stderr, "error: failed to convert S component of DSA signature.\n");
		mpnfree(*r);
		return(ONS_CRYPTO_ERR);
	}
	return(ONS_CRYPTO_OK);
}

/*! \brief Sign a digest (or other data) using the Digital Signature Algorithm.
 *
 * This computes the signature for the data in \a md[] using the Digital Signature
 * Algorithm and secret key in \a skey[] (in internal ONS format).  In general,
 * the data in \a md[] should be the message digest for the file to be signed,
 * but there is no real restriction on this.
 *
 * \param	*md		Pointer to the message digest to sign.
 * \param	nbytes	Length of the message digest to sign.
 * \param	*skey	Pointer to the secret key to use, in ONS internal bytestream format.
 * \param	*errcd	Set to an appropriate error code, or ONS_CRYPTO_OK on success.
 * \return			Pointer to signature stream in ONS internal bytestream format, or
 *					NULL on failure.
 */

u8 *ons_sign_digest(u8 *md, u32 nbytes, u8 *skey, OnsCryptErr *errcd)
{
	u8			*rtn;
	mpbarrett	*p, *q;
	mpnumber	*x, *g, r, s, digest;
	randomGeneratorContext	rgc;
	
	if (!ons_int_to_key(skey, &p, &q, &g, &x)) {
		fprintf(stderr, "error: failed to convert key to gcrypt format.\n");
		return(NULL);
	}
	if (randomGeneratorContextInit(&rgc, randomGeneratorDefault()) != 0) {
		fprintf(stderr, "error: failed to initialise RNG for DSA signature.\n");
		mpnfree(x); free(x);
		mpbfree(p); free(p); mpbfree(q); free(q); mpnfree(g); free(g);
		return(NULL);
	}

	mpnzero(&digest);
	mpnsetbin(&digest, md, nbytes);
	mpnzero(&r); mpnzero(&s);

	if (dsasign(p, q, g, &rgc, &digest, x, &r, &s) != 0) {
		fprintf(stderr, "error: failed to sign digest with DSA method.\n");
		mpnfree(x); free(x); mpnfree(&digest); mpnfree(&r); mpnfree(&s);
		mpbfree(p); free(p); mpbfree(q); free(q); mpnfree(g); free(g);
		return(NULL);
	}

	randomGeneratorContextFree(&rgc);
	mpnfree(x); free(x); mpnfree(&digest);
	mpbfree(p); free(p); mpbfree(q); free(q); mpnfree(g); free(g);
	if ((rtn = ons_sig_to_int(&r, &s)) == NULL) {
		fprintf(stderr, "error: failed to convert signature into internal format.\n");
		*errcd = ONS_CRYPTO_ERR;
		return(NULL);
	}
	mpnfree(&r); mpnfree(&s);
	return(rtn);
}

/*! \brief Compute the signature for a file, avoiding the ONSCrypto block if reqquired.
 *
 * This digests a file, adds the \a user_data[] element, and then computes the signature
 * for the block using the key in \a skey[].  The signature is returned in internal ONS
 * bytestream format, including the trailing CRC32 to protect the contents in transit.  For
 * the ONS signature structure, the user data should be an unsigned integer reference in LSB format
 * that ties this signature event into the meta-data for the file.
 *
 * \param	*name			Name of the file to read and digest.
 * \param	*user_data		Pointer to the user-data to add to the digest.
 * \param	user_data_len	Length of the user-data to add to the digest.
 * \param	*skey			Pointer to the secret key to sign with (in ONS bytestream format).
 * \return					Pointer to signature in ONS bytestream format, or NULL on failure.
 */

u8 *ons_compute_signature(char *name, u8 *user_data, u32 user_data_len, u8 *skey)
{
	u8			*digest, *sig;
	u32			digest_len;
	OnsCryptErr	errcd;
	
	if ((digest = ons_gen_digest(name, user_data, user_data_len, &digest_len)) == NULL) {
		fprintf(stderr, "error: failed to generate message digest.\n");
		return(NULL);
	}
	if ((sig = ons_sign_digest(digest, digest_len, skey, &errcd)) == NULL) {
		fprintf(stderr, "error: failed to write digest (error code %d).\n", (u32)errcd);
		free(digest);
		return(NULL);
	}
	free(digest);
	return(sig);
}

/*! \brief Sign the named file with \a skey[] and Signature Sequential ID \a sigid.
 *
 * This carries out all of the operations required to sign the file in \a name using the
 * secret key in \a skey[] and the Sequential Signature ID in \a sigid.  This is essentially
 * a light wrapper on ons_compute_signature() and ons_write_file_signature().
 *
 * \param	*name	Pointer to the name of the file to sign.
 * \param	*skey	Pointer to the signatory secret key.
 * \param	sigid	Sequential Signature ID to combine with the message digest.
 * \return			True on success, otherwise False.
 */

Bool ons_sign_file(char *name, u8 *skey, u32 sigid)
{
	u8			*sig;
	OnsCryptErr	rc;
	
	if ((sig = ons_compute_signature(name, (u8*)&sigid, sizeof(u32), skey)) == NULL) {
		fprintf(stderr, "error: failed to compute signature for file \"%s\".\n", name);
		return(False);
	}
	if ((rc = ons_write_file_sig(name, sig, sigid)) != ONS_CRYPTO_OK) {
		fprintf(stderr, "error: failed to add signature to file \"%s\".\n", name);
		free(sig);
		return(False);
	}
	free(sig);
	return(True);
}

/*! \brief Verify the signature in \a sig[] given public key \a pkey[] and digest \a digest[].
 *
 * This runs the DSA verification process on the given signature, public key and digest.  The
 * code returns True if all three components match, and otherwis False.  There is no direct
 * way, using this method, to determine whether the problem is with the signature, the
 * digest or the public key.
 *
 * \param	*sig	ONS bytestream signature to test.
 * \param	*pkey	ONS bytestream public key to test.
 * \param	*digest	Pointer to message digest to test.
 * \param	dig_len	Length of the message digest in bytes.
 * \return			True if all three components match, otherwise False.
 */

Bool ons_verify_signature(u8 *sig, u8 *pkey, u8 *digest, u32 dig_len)
{
	mpnumber	*r = NULL, *s = NULL, *g = NULL, *y = NULL, md;
	mpbarrett	*p = NULL, *q = NULL;
	Bool		rc;
	
	if (ons_int_to_sig(sig, &r, &s) != ONS_CRYPTO_OK) {
		fprintf(stderr, "error: failed to convert signature to check format.\n");
		return(False);
	}
	if (!ons_int_to_key(pkey, &p, &q, &g, &y)) {
		fprintf(stderr, "error: failed to convert key to check format.\n");
		mpnfree(r); mpnfree(s); free(r); free(s);
		return(False);
	}
	mpnzero(&md);
	mpnsetbin(&md, digest, dig_len);
	if (dsavrfy(p, q, g, &md, y, r, s) == 0) {
		rc = False;
		fprintf(stderr, "error: verification failed.\n");
	} else
		rc = True;
	if (r != NULL) { mpnfree(r); free(r); }
	if (s != NULL) { mpnfree(s); free(s); }
	if (y != NULL) { mpnfree(y); free(y); }
	if (p != NULL) { mpbfree(p); free(p); }
	if (q != NULL) { mpbfree(q); free(q); }
	if (g != NULL) { mpnfree(g); free(g); }
	return(rc);
}

/*! \brief Verify the signature in a file using the public key and Sequentual Signature ID.
 *
 * This sequences all of the components required to verify the signature directly from
 * the file at \a name[].  This re-computes the message digest for the file, reads the signature
 * from the file, and then runs the DSA verification algorithm.  The routine returns True iff
 * (a) the file has an ONSCrypto block at the end of the file, (b) the signature extracted
 * from the block matches the public key and digest, and (c) the Sequential Signature ID
 * extracted matches the \a sig_id passed in.
 *
 * \param	*name	Pointer to the file to be verified.
 * \param	*pkey	Pointer to the public key of the signatory (in ONS internal bytestream format).
 * \param	sig_id	Sequential Signature ID for the signature event in the meta-data being verified.
 * \return			True if siganture in the file, the public key and the SSID all match, otherwise False.
 */

Bool ons_verify_file(char *name, u8 *pkey, u32 sig_id)
{
	u8	signature[ONS_CRYPTO_MAX_SIG_LEN], *digest;
	u32	digest_len, file_sig_id;
	OnsCryptErr	rc;
	Bool	match;
	
	if (!ons_check_cblock(name)) {
		fprintf(stderr, "error: file \"%s\" does not contain a ONSCryptoBlock.\n", name);
		return(False);
	}
	if ((rc = ons_read_file_sig(name, signature, &file_sig_id, ONS_CRYPTO_MAX_SIG_LEN))
			!= ONS_CRYPTO_SIG_OK) {
		fprintf(stderr, "error: failed to read signature from \"%s\".\n", name);
		return(False);
	}
	if (sig_id != file_sig_id) {
		fprintf(stderr, "error: signature ID in file (0x%08X) does not match user ID (0x%08X).\n",
			file_sig_id, sig_id);
		return(False);
	}
	if ((digest = ons_gen_digest(name, (u8*)&sig_id, sizeof(u32), &digest_len)) == NULL) {
		fprintf(stderr, "error: failed to compute message digest for \"%s\".\n", name);
		return(False);
	}
	match = ons_verify_signature(signature, pkey, digest, digest_len);
	free(digest);
	return(match);
}

/*! \brief Convert a pass-phrase to a symmetric key suitable for use in protecting an asymmetric secret key.
 *
 * This converts the user pass-phrase in \a phrase[] into a key suitable for use in a symmetrical
 * cryptography scheme, generally used to protect an asymmetric cryptography secret key in transit to
 * external form.  This is currently implemented using SHA-256 so that the code generates a key
 * suitable for AES-256.  Note that the performance of this key will depend strongly on the length and
 * random nature of the pass-phrase.  To make sense, the pass-phrase has to be fairly long --- Schneier
 * recommends one character per bit of the key that's being generated.  The code does not check that this
 * is the case, however, nor does it enforce any particular standard.
 *
 * \param	*phrase		Pointer to the pass-phrase to convert.
 * \return				Pointer to 32-byte chunk of memory with the converted pass-phrase, or NULL on failure.
 */

u8 *ons_phrase_to_key(char *phrase)
{
	u8			*rtn;
	sha256Param	param;
	
	if ((rtn = (u8*)malloc(32)) == NULL) {
		fprintf(stderr, "error: no memory for symcrypt key buffer.\n");
		return(NULL);
	}
	sha256Reset(&param);
	sha256Update(&param, (unsigned char *) phrase, strlen(phrase));
	sha256Digest(&param, rtn);
	return(rtn);
}

/*! \brief Encrypts the secret key passed with a suitable symmetric crypotgraphy scheme.
 *
 * Encrypt an asymmetric secret key with a symmetric scheme so that it can be passed around
 * externally to the program without compromising the security of the signatures.  The algorithm used
 * is AES-256, and therefore the \a aeskey[] must be 32-bytes long.  This should be secure enough for
 * the intended use, which is to ensure that, should a secret key be compromised (e.g., a hardware token
 * is stolen), the key cannot be recovered without the pass-phrase for at least as long as it takes for
 * the compromise to be noticed, and the asymmetric key pair to be repudiated by the key-signing authority.
 *
 * \param	*seckey		Pointer to the secret key to encrypt, in ONS internal bytestream format.
 * \param	*aeskey		Pointer to 32-bytes of key to encrypt with using AES-256.
 * \param	*out_len	Set on output to the length of the encrypted key generated.
 * \return				Pointer to encrypted secret key, or NULL on failure.
 */

u8 *ons_encrypt_key(u8 *seckey, u8 *aeskey, u32 *out_len)
{
	u32			seckey_len, in_crc, crc, block, n_blocks, nbytes, residual, b;
	u8			feedback[16], *rtn, *dst;
	aesParam	param;
	randomGeneratorContext	rgc;
	
	seckey_len = ons_compute_int_len(seckey);
	/* Check that the input seckey is a valid internal format (CRC32) */
	crc = crc32_calc_buffer((char *)seckey, seckey_len);
	in_crc = *((u32*)(seckey + seckey_len));

#ifdef __BAG_BIG_ENDIAN__
	swap_4((void*)&in_crc);
#endif

	if (crc != in_crc) {
		fprintf(stderr, "error: CRC32 check failed on input secret key for AES encryption.\n");
		return(NULL);
	}
	
	/* Compute number of blocks */
	n_blocks = seckey_len / 16;
	residual = seckey_len - (n_blocks * 16);
	
	nbytes = 16 /* IV */ + n_blocks * 16 + ((residual==0)?0:16) + 4 /* CRC */;
	*out_len = nbytes;
	if ((rtn = (u8*)malloc(nbytes)) == NULL) {
		fprintf(stderr, "error: failed to get memory for output encrypted secret key\n");
		return(NULL);
	}
	
	/* Generate 16-bytes (128-bit) of random input to initialise the CBC feedback register, and
	 * write out to output.
	 */
	randomGeneratorContextInit(&rgc, randomGeneratorDefault());
	randomGeneratorContextNext(&rgc, feedback, 16);
	randomGeneratorContextFree(&rgc);
	
	memcpy(rtn, feedback, 16);
	dst = rtn + 16;
	
	/* Generate the context for AES implementation, then run CBC feedback over all blocks */
	if (aesSetup(&param, aeskey, 256, ENCRYPT) != 0) {
		fprintf(stderr, "error: failed to setup for AES256 encryption.\n");
		return(NULL);
	}
	
	for (block = 0; block < n_blocks; ++block) {
		for (b = 0; b < 16; ++b)
			feedback[b] ^= *seckey++;
		if (aesEncrypt(&param, (u32*)dst, (u32*)feedback) != 0) {
			fprintf(stderr, "error: failed to encrypt block %d of AES words.\n", block);
			free(rtn);
			return(NULL);
		}
		memcpy(feedback, dst, 16);
		dst += 16;
	}
	if (residual != 0) {
		for (b = 0; b < residual; ++b)
			feedback[b] ^= *seckey++;
		if (aesEncrypt(&param, (u32*)dst, (u32*)feedback) != 0) {
			fprintf(stderr, "error: failed to encrypt last block of AES words.\n");
			free(rtn);
			return(NULL);
		}
		dst += 16;
	}
	
	/* Tag end of sequence with CRC32 to provide some transmission protection */
	*((u32*)dst) = crc32_calc_buffer((char *)rtn, nbytes-4);

#ifdef __BAG_BIG_ENDIAN__
	swap_4((void*)dst);
#endif
	
	return(rtn);
}

/*! \brief Decrypt an asymmetric secret key with an AES-256 symmetric key.
 *
 * This decrypts an asymmetric cryptography secret key that was previously encrypted with
 * the AES-256 key in \a aeskey[] (which must therefore be 32-bytes long).  This assumes that
 * the key was generated with ons_encrypt_key() and includes a CRC32 on the end to verify that the
 * key hasn't been modified during transmission or storage.  The key id decrypted, and the output
 * is rebuilt as a valid internal format key, with appended CRC32 of the decrypted key.
 *
 * \param	*ctext		Pointer to ciphertext of key, including CRC32.
 * \param	nin			Length of ciphertext in bytes, including CRC32.
 * \param	*aeskey		Pointer to 32-bytes of AES-256 symmetric crypographic key.
 * \param	*errc		Appropriate error code on failure, or ONS_CRYPTO_OK on success.
 * \return				Pointer to the decrypted asymmetric key, or NULL on failure.
 */

u8 *ons_decrypt_key(u8 *ctext, u32 nin, u8 *aeskey, OnsCryptErr *errc)
{
	u8			*rtn, *buffer, feedback[16], *src, *dst;
	u32			crc, in_crc, block, n_blocks, seckey_len, nbytes, b;
	aesParam	param;
	
	crc = crc32_calc_buffer((char *)ctext, nin-4);
	in_crc = *((u32*)(ctext + nin - 4));

#ifdef __BAG_BIG_ENDIAN__
	swap_4((void*)&in_crc);
#endif

	if (crc != in_crc) {
		fprintf(stderr, "error: AES256 ciphertext CRC32 failed check on input --- corrupt key?\n");
		*errc = ONS_CRYPTO_BAD_KEY;
		return(NULL);
	}
	
	n_blocks = (nin - 4)/16 - 1;
	nbytes = n_blocks*16; /* At most --- may be less with last-block padding */
	if ((buffer = (u8*)malloc(nbytes)) == NULL) {
		fprintf(stderr, "error: failed to get memory for AES output secret key (need %d bytes).\n",
			nbytes);
		*errc = ONS_CRYPTO_ERR;
		return(NULL);
	}
	
	/* Initialise IV from first block of data */
	memcpy(feedback, ctext, 16);
	src = ctext + 16;
	dst = buffer;
	
	if (aesSetup(&param, aeskey, 256, DECRYPT) != 0) {
		fprintf(stderr, "error: failed to initialise AES for decryption of secret key.\n");
		free(buffer);
		*errc = ONS_CRYPTO_ERR;
		return(NULL);
	}
	
	/* Run all blocks, using CBC feedback */
	for (block = 0; block < n_blocks; ++block) {
		if (aesDecrypt(&param, (u32*)dst, (u32*)src) != 0) {
			fprintf(stderr, "error: failed to decrypt block %d of secret key.\n", block);
			free(buffer);
			*errc = ONS_CRYPTO_ERR;
			return(NULL);
		}
		for (b = 0; b < 16; ++b)
			*dst++ ^= feedback[b];
		memcpy(feedback, src, 16); src += 16;
	}	
	
	/* Size output, allocate final return space and compute CRC */
	if ((u32)buffer[0] != 4) {
		fprintf(stderr, "error: decrypted secret key is not a valid secret key.\n");
		free(buffer);
		*errc = ONS_CRYPTO_BAD_KEY;
		return(NULL);
	}
	seckey_len = ons_compute_int_len(buffer);
	nbytes = seckey_len + 4 /* CRC */;
	if ((rtn = (u8*)malloc(nbytes)) == NULL) {
		fprintf(stderr, "error: failed to get memory for output secret key.\n");
		free(buffer);
		*errc = ONS_CRYPTO_ERR;
		return(NULL);
	}
	memcpy(rtn, buffer, seckey_len);
	*((u32*)(rtn + seckey_len)) = crc32_calc_buffer((char *)buffer, seckey_len);

#ifdef __BAG_BIG_ENDIAN__
	swap_4((void*)(rtn + seckey_len));
#endif

	free(buffer);
	*errc = ONS_CRYPTO_OK;

	return(rtn);
}
