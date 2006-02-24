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
#include "aes.h"
#include "dsa.h"
#include "sha1.h"
#include "sha256.h"
#include "crc32.h"
#include "mpnumber.h"
#include "mpbarrett.h"

#define DEFAULT_MD_BUFFER_LEN	40960
#define DEFAULT_MD_ALGO		GCRY_MD_SHA256
#define DEFAULT_PK_ALGO		GCRY_AC_DSA
#define DEFAULT_KEY_LEN		1024
#define DEFAULT_SYM_ALGO	GCRY_CIPHER_AES256

#define ONS_CRYPTO_BLOCK_MAGIC		0x4F4E5343	/* 'ONSC' */
#define ONS_CRYPTO_BLOCK_CUR_VER	1			/* Version byte */
#define ONS_CRYPTO_SIG_BLOCK_SIZE	1024		/* Maximum size, padded if required */

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

#define ONS_CRYPTO_SIG_MAGIC_OFF	0
#define ONS_CRYPTO_SIG_VER_OFF		4
#define ONS_CRYPTO_SIG_SIGID_OFF	5
#define ONS_CRYPTO_SIG_SIG_OFF		9

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

#define ONS_CRYPTO_KEY_NKEYS_OFF	0
#define ONS_CRYPTO_KEY_KEY1_OFF		1

#include "onscrypto.h"

/**
	Check whether a Cryptographic block exists for the specified file.
	
*/

/* Routine:	ons_check_cblock
 * Purpose:	Determine whether the given file has an ONSCryptoBlock at the end
 * Inputs:	*file	Name of the file to read
 * Outputs:	Returns True if file has a ONSCryptoBlock at the end, otherwise False
 * Comment:	Note that this only checks that the ONSCryptoBlock exists in the correct place,
 *			and says nothing about whether the block is valid --- use ons_read_file_sig() to
 *			read and validate.
 */

Bool ons_check_cblock(char *file)
{
	FILE	*f;
	u32	file_magic;
	
	if ((f = fopen(file, "rb")) == NULL) {
		fprintf(stderr, "error: failed to open \"%s\" for ONSCryptoBlock check.\n",
			file);
		return(ONS_CRYPTO_FILE_ERR);
	}
	fseek(f, -ONS_CRYPTO_SIG_BLOCK_SIZE, SEEK_END);
	fread(&file_magic, sizeof(u32), 1, f);
	fclose(f);
	return(file_magic == ONS_CRYPTO_BLOCK_MAGIC);
}

/* Routine:	ons_gen_digest
 * Purpose:	Generate a message digest from the named file, less the crypto block, if it exists
 * Inputs:	*file			Filename of the file to read and hash
 *			*user_data		User-supplied data to add to the digest (see comment)
 *			user_data_len	Length of the user-supplied data
 * Outputs:	*nbytes	Number of bytes in the digest
 *			Returns pointer to message digest on success, otherwise False
 * Comment:	The message digest protocol may change from time to time, so users should not assume
 *			that the nbytes value won't change.  The user-supplied data is added to the digest after
 *			all of the file information has been added.  If user_data == NULL, no information
 *			is added to the digest.
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

    /* fprintf(stderr, "sha1reset succeeded - bufferlen = %d - spoffset: %d\n",  */
/*             buffer_len, mdparam.offset); */
/*     fflush(stderr); */

	tot_read = 0;
	while (!done && !feof(f)) {
		n_read = fread((void *)buffer, 1, buffer_len, f);
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
        /* fprintf(stderr, "china is not the issuer: %d - offset: %d\n", n_read, mdparam.offset); */
/*         fflush(stderr); */

        elsize = (size_t)(n_read * sizeof(u8));

        
		if (sha1Update(&mdparam, buffer, (size_t)elsize)) {
			fprintf(stderr, "error: failed to add block to MD for \"%s\".\n", file);
			return(NULL);
		}
		/* ungetc(getc(f), f); /\* WTF!? *\/ */
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

/* Routine:	ons_compute_int_len
 * Purpose:	Compute the size, in bytes, of the internal element passed
 * Inputs:	*data	Internal format data string
 * Ouputs:	Returns number of bytes in internal string
 * Comment:	-
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
	sig_crc = crc32_calc_buffer(buffer+ONS_CRYPTO_SIG_SIGID_OFF, sig_len+sizeof(u32));
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
	*((u32*)(buffer+ONS_CRYPTO_SIG_SIG_OFF+sig_len)) = crc32_calc_buffer(buffer + ONS_CRYPTO_SIG_SIG_OFF, sig_len);
	memcpy(sig, buffer + ONS_CRYPTO_SIG_SIG_OFF, sig_len + 4 /* CRC */);
	return(ONS_CRYPTO_SIG_OK);
}

/* Routine:	ons_write_file_sig
 * Purpose:	Write a signature into the specified file, appending if it doesn't have one yet
 * Inputs:	*file	Name of file to write/append
 *			*sig	Signature to add to the file
 *			sigid	Reference ID to add to the signature block
 * Outputs:	Writes block to file, or appends if there isn't one already.
 *			Returns ONS_CRYPTO_OK if success, otherwise error code
 * Comment:	-
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
	crc = crc32_calc_buffer(sig, sig_len);
#ifdef __DEBUG__
	fprintf(stderr, "debug: signature CRC32 = 0x%x\n", crc);
#endif
	sig_crc = *((u32*)(sig + sig_len));
	if (sig_crc != crc) {
		fprintf(stderr, "error: CRC passed in for signature doesn't match (IN(%08X) != %08X).\n",
			sig_crc, crc);
		return(ONS_CRYPTO_BAD_SIG_BLOCK);
	}
	memset(buffer, 0, ONS_CRYPTO_SIG_BLOCK_SIZE);
	*((u32*)(buffer + ONS_CRYPTO_SIG_MAGIC_OFF)) = ONS_CRYPTO_BLOCK_MAGIC;
	buffer[ONS_CRYPTO_SIG_VER_OFF] = ONS_CRYPTO_BLOCK_CUR_VER;
	*((u32*)(buffer + ONS_CRYPTO_SIG_SIGID_OFF)) = sigid;
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
		free(buffer);
		return(ONS_CRYPTO_FILE_ERR);
	}
	fseek(f, -ONS_CRYPTO_SIG_BLOCK_SIZE, SEEK_END);
	fread(&file_magic, sizeof(u32), 1, f);
	if (file_magic != ONS_CRYPTO_BLOCK_MAGIC) {
		/* No signature in file, wind to end and write magic number */
		fseek(f, 0, SEEK_END);
	} else {
		fseek(f, -4, SEEK_CUR);	/* Rewind the size of the magic ID and then write block */
	}
	/* Compute the file-based CRC32 to protect the SigID and signature */
	*((u32*)(buffer + ONS_CRYPTO_SIG_SIG_OFF + sig_len)) =
		crc32_calc_buffer(buffer + ONS_CRYPTO_SIG_SIGID_OFF, sig_len + sizeof(u32));
#ifdef __DEBUG__
	fprintf(stderr, "debug: file CRC32 = 0x%x\n", *((u32*)(buffer+ONS_CRYPTO_SIG_SIG_OFF+sig_len)));
#endif
	fwrite(buffer, sizeof(u8), ONS_CRYPTO_SIG_BLOCK_SIZE, f);
	fclose(f);
	return(ONS_CRYPTO_OK);
}

/* Routine:	ons_bin_to_ascii
 * Purpose:	Convert binary sequence into ASCII
 * Inputs:	*data	Data to convert
 *			n_bytes	Number of bytes of input data
 * Outputs:	*n_out_bytes	Number of output bytes generated
 *			Returns pointer to buffer with zero terminated string of ASCII characters, suitable
 *			for fprintf().
 * Comment:	A simple hex printer, no more.  Caller is responsible for releasing memory.
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

/* Routine:	ons_int_to_ascii
 * Purpose:	Convert internal format data into ASCII for external representation
 * Inputs:	*intdat	Internal format data, followed by CRC32
 *			len		Number of bytes in internal data, excluding CRC32 on end
 * Outputs:	Pointer to zero terminated ASCII string, with CRC of ASCII data appended, or
 *			NULL on failure.
 * Comment:	This assumes that a CRC32 follows the data to be sent, and that the length specified
 *			on input does not include this four-byte value.
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

/* Routine:	ons_sig_to_ascii
 * Purpose:	Convert a binary signature into ASCII for output, with CRC
 * Inputs:	*sig	Signature to convert for writing
 * Outputs:	Returns pointer to ASCII signature (zero terminated) with CRC appended,
 *			or NULL on failure.
 * Comment:	The code appends a CRC for the ASCII signature, also in ASCII so that the
 *			output can be fprintf()ed to output directly; on input, the CRC is used as a check
 *			for correctness of the transmission, not of signature.
 */

char *ons_sig_to_ascii(u8 *sig)
{
	return(ons_int_to_ascii(sig, ons_compute_int_len(sig)));
}

/* Routine:	ons_key_to_ascii
 * Purpose:	Convert a binary key into ASCII for output, with CRC
 * Inputs:	*key	Key element to convert for writing
 * Outputs:	Returns pointer to ASCII signature (zero terminated) with CRC appended, or
 *			NULL on failure.
 * Comment:	The code appends a CRC for the ASCII key, also in ASCII so that the
 *			output can be fprintf()ed to output directly; on input, the CRC is used as a check
 *			for correctness of the transmission, not of key.
 */

char *ons_key_to_ascii(u8 *key)
{
	return(ons_int_to_ascii(key, ons_compute_int_len(key)));
}

/* Routine:	ons_ascii_to_int
 * Purpose:	Convert from an ASCII string to internal binary format, checking for validity
 * Inputs:	*ascii	Input ASCII string with CRC appended
 * Outputs:	Returns pointer to internal format binary data on success, else NULL
 *			*errcd	Error code for conversion, ONS_CRYPTO_OK on success
 * Comment:	-
 */

static u8 *ons_ascii_to_int(char *ascii, OnsCryptErr *errcd)
{
	u32		ascii_len, i, crc, digit;
	u8		*rtn;
	char	ascii_crc[9];
	
	ascii_len = strlen(ascii) - sizeof(u32)*2; /* Less size of CRC */
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
	return(rtn);
}

/* Routine:	ons_ascii_to_sig
 * Purpose:	Converts an ASCII signature to binary internal format, checking for validity
 * Inputs:	*sig	Input ASCII signature with CRC appended
 * Outputs:	Returns pointer to internal format signature on success, else NULL.
 *			*errcd	Error code for conversion, ONS_CRYPTO_OK on success.
 * Comment:	Converts signature to internal format, checking the CRC as it does so.  Errors
 *			are reported with return of NULL and appropriate setting of errcd.
 */

u8 *ons_ascii_to_sig(char *sig, OnsCryptErr *errcd)
{
	return(ons_ascii_to_int(sig, errcd));
}

/* Routine:	ons_ascii_to_key
 * Purpose:	Convert an ASCII key to binary internal format, checking for validity
 * Inputs:	*key	Input ASCII key with CRC appended
 * Outputs:	Returns pointer to internal format key on success, else NULL
 *			*errcd	Error code for conversion, ONS_CRYPTO_OK on success
 * Comment:	Converts key to internal format, checking the CRC as it does so.  Errors are reported
 *			with return of NULL and appropriate setting of errcd.
 */

u8 *ons_ascii_to_key(char *key, OnsCryptErr *errcd)
{
	return(ons_ascii_to_int(key, errcd));
}

/* Routine:	ons_mpn_to_int
 * Purpose:	Convert an MPI to internal format
 * Inputs:	mpi	The MPI to convert to internal format
 * Outputs:	Pointer to buffer with internal representation, or NULL on failure
 * Comment:	This converts data into internal formats ready for output/storage.  The data
 *			format used only works for unsigned objects, and hence will not work for all MPIs
 */

static u8 *ons_mpn_to_int(mpnumber *mpi)
{
	u8	*buffer;
	u32	nbits, nbytes;
	
	nbits = mpnbits(mpi);
	nbytes = ((nbits + 7)>>3) + (((nbits & 7) == 0) ? 1 : 0);
	
	if ((buffer = (u8*)malloc(nbytes + 1)) == NULL) {
		fprintf(stderr, "error: no memory for MPI conversion (need %d bytes).\n", nbytes);
		return(NULL);
	}
	i2osp(buffer+1, nbytes, mpi->data, mpi->size);
	buffer[0] = (u8)(nbytes & 0xFF);
	return(buffer);
}

/* Routine:	ons_mpb_to_int
 * Purpose:	Convert an MPB to internal format
 * Inputs:	*mpb	The input number to convert
 * Outputs:	Pointer to buffer with internal representation, or NULL on failure
 *			*errcd	Error code on failure
 * Comment:	-
 */

static u8 *ons_mpb_to_int(mpbarrett *mpb)
{
	u8	*buffer;
	u32	nbits, nbytes;
	
	nbits = mpbbits(mpb);
	nbytes = ((nbits + 7)>>3) + (((nbits & 7) == 0) ? 1 : 0);
	
	if ((buffer = (u8*)malloc(nbytes + 1)) == NULL) {
		fprintf(stderr, "error: no memory for MPI conversion (need %d bytes).\n", nbytes);
		return(NULL);
	}
	i2osp(buffer+1, nbytes, mpb->modl, mpb->size);
	buffer[0] = (u8)(nbytes & 0xFF);
	return(buffer);
}

/* Routine:	ons_int_to_mpn
 * Purpose:	Convert internal format to MPI
 * Inputs:	*num	Number in internal format to be converted into MPI
 * Outputs:	Returns MPI formated large integer
 * Comment:	This assumes that the external format is in GCRYMPI_FMT_USG, and hence only works if it
 *			is an unsigned large format integer.
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

/* Routine:	ons_int_to_mpb
 * Purpose:	Convert internal format data into an MPB
 * Inputs:	*num	Number to convert
 * Outputs:	Pointer to the MPB required
 * Comment:	-
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

/* Routine:	ons_key_to_int
 * Purpose:	Convert a key into internal format
 * Inputs:	param	Pointer to the DSA domain parameters structures
 *			key		Public or private key number as required for output
 * Outputs:	Pointer to internal buffer with data, or NULL on failure
 * Comment:	The code generates an output string consisting of the (p,q,g) components of the
 *			DSA domain, and then appends the key component, either public or private, to complete
 *			the output number.  In each case, the output has four integers.  The code adds
 *			a CRC32 at the end of the stream to add some protection.
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
	*((u32*)(rtn + offset)) = crc32_calc_buffer(rtn, nbytes);
	free(inp); free(inq); free(ing); free(ink);	
	return(rtn);
}

/* Routine:	ons_int_to_key
 * Purpose:	Convert internal format to key parameters
 * Inputs:	*in		Input structured number sequence in internal format
 * Outputs:	**p, **q, **g	DSA Parameter structure components
 *			**key			Private/Public key component (depends on input which this is)
 *			Returns True on success, else False.
 * Comment:	This assumes that the key is one associated with DSA, and that it was stored in the
 *			internal format by ons_key_to_int().
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

/* Routine:	ons_generate_keys
 * Purpose:	Generate keys for signature
 * Inputs:	-
 * Outputs:	*pkey	Pointer to the public key string in internal format
 *			*skey	Pointer to the secret key string in internal format
 *			Returns error information, or ONS_CRYPTO_OK on success.
 * Comment:	Generates internal format keys for signature operations.  The secret component should, of
 *			course, be treated with a degree of respect.
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

/* Routine:	ons_sig_to_int
 * Purpose:	Convert a signature to internal format
 * Inputs:	*r, *s	Components of the DSA signature
 * Ouputs:	Returns pointer to the internal signature data, or NULL on failure
 * Comment:	-
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
	*((u32*)(rtn + offset)) = crc32_calc_buffer(rtn, nbytes);
	return(rtn);
}

/* Routine:	ons_int_to_sig
 * Purpose:	Convert an internal format item into a signature
 * Inputs:	*sig	Internal format signature
 * Outputs:	*r, *s	Components of the DSA signature for verification
 *			Returns a suitable error code.
 * Comment:	-
 */

static OnsCryptErr ons_int_to_sig(u8 *sig, mpnumber **r, mpnumber **s)
{
	u32	n_ints, in_crc, sig_crc, sig_len, offset;
	
	sig_len = ons_compute_int_len(sig);
	sig_crc = *((u32*)(sig + sig_len));
	in_crc = crc32_calc_buffer(sig, sig_len);
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

/* Routine:	ons_sign_digest
 * Purpose:	Use secret key to sign the digest passed in
 * Inputs:	*md		Digest to sign (can send arbitrary data if required)
 *			nbytes	Size of the digest in bytes
 *			*skey	Secret key to use for signing (in internal format)
 * Outputs:	Returns pointer to signature in internal format, or NULL on error
 *			*errcd	Error code, or ONS_CRYPTO_OK on success.
 * Comment:	-
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

/* Routine:	ons_sign_file
 * Purpose:	Read file, compute digest, and sign a file
 * Inputs:	*name	Name of the file to read & write signature into
 *			*skey	Secret key with which to sign the file
 *			sigid	Signature ID to add to output in order to link identity to the rest of the file
 * Outputs:	True on success, else False.
 * Comment:	A convenience function to sequence all of the elements to add a signature to a file.
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

/* Routine:	ons_verify_signature
 * Purpose:	Determine whether the signature provided matches the digest and public key
 * Inputs:	*sig	Signature to test
 *			*pkey	Public key to use for test
 *			*digest	Message digest to check
 *			dig_len	Digest length
 * Outputs:	Returns True if the signature and digest match, given the key, else False
 * Comment:	-
 */

Bool ons_verify_signature(u8 *sig, u8 *pkey, u8 *digest, u32 dig_len)
{
	mpnumber	*r, *s, *g, *y, md;
	mpbarrett	*p, *q;
	Bool		rc;
	
	if (!ons_int_to_sig(sig, &r, &s)) {
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
	mpnfree(r); free(r); mpnfree(s); free(s); mpnfree(y); free(y);
	mpbfree(p); free(p); mpbfree(q); free(q); mpnfree(g); free(g);
	return(rc);
}

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

/* Routine:	ons_phrase_to_key
 * Purpose:	Construct a 256-bit symmetric crypto. key from a pass phrase
 * Inputs:	*phrase	Pointer to zero-terminated string to use as the pass phrase
 * Outputs:	Returns pointer to key stream of 256-bits (32 bytes), or NULL on error.
 * Comment:	This hashes the pass-phrase to generate a 256-bit key for use in symmetric crypto.
 *			For this key to really make sense, the pass phrase should be fairly long --- Schneier
 *			recommends that there should be one character per bit in the key.  The code does not,
 *			however, check that this is the case, nor does it enforce any particular standard.
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
	sha256Update(&param, phrase, strlen(phrase));
	sha256Digest(&param, rtn);
	return(rtn);
}

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

u8 *ons_encrypt_key(u8 *seckey, u8 *aeskey, u32 *out_len)
{
	u32			seckey_len, in_crc, crc, block, n_blocks, nbytes, residual, b;
	u8			feedback[16], *rtn, *dst;
	aesParam	param;
	randomGeneratorContext	rgc;
	
	seckey_len = ons_compute_int_len(seckey);
	/* Check that the input seckey is a valid internal format (CRC32) */
	crc = crc32_calc_buffer(seckey, seckey_len);
	in_crc = *((u32*)(seckey + seckey_len));
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
	*((u32*)dst) = crc32_calc_buffer(rtn, nbytes-4);
	
	return(rtn);
}

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

u8 *ons_decrypt_key(u8 *ctext, u32 nin, u8 *aeskey, OnsCryptErr *errc)
{
	u8			*rtn, *buffer, feedback[16], *src, *dst;
	u32			crc, in_crc, block, n_blocks, seckey_len, nbytes, b;
	aesParam	param;
	
	crc = crc32_calc_buffer(ctext, nin-4);
	in_crc = *((u32*)(ctext + nin - 4));
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
		return(NULL);
	}
	
	/* Run all blocks, using CBC feedback */
	for (block = 0; block < n_blocks; ++block) {
		if (aesDecrypt(&param, (u32*)dst, (u32*)src) != 0) {
			fprintf(stderr, "error: failed to decrypt block %d of secret key.\n", block);
			free(buffer);
			return(NULL);
		}
		for (b = 0; b < 16; ++b)
			*dst++ ^= feedback[b];
		memcpy(feedback, src, 16); src += 16;
	}	
	
	/* Size output, allocate final return space and compute CRC */
	seckey_len = ons_compute_int_len(buffer);
	nbytes = seckey_len + 4 /* CRC */;
	if ((rtn = (u8*)malloc(nbytes)) == NULL) {
		fprintf(stderr, "error: failed to get memory for output secret key.\n");
		free(buffer);
		return(NULL);
	}
	memcpy(rtn, buffer, seckey_len);
	*((u32*)(rtn + seckey_len)) = crc32_calc_buffer(buffer, seckey_len);
	free(buffer);
	
	return(rtn);
}
