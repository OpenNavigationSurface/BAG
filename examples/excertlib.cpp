/*
 * $Id: excertlib.c,v 1.2 2005/10/26 17:38:26 openns Exp $
 * $Log: excertlib.c,v $
 * Revision 1.2  2005/10/26 17:38:26  openns
 * Updates after testing following the 2nd ONSWG meeting.  Checked in from
 * Webb McDonald's implementation of 2005-09-07.
 *
 * Revision 1.3  2005/03/15 22:44:25  openns
 * Added XML certificate functionality, and slightly different call sequence to
 * ons_gen_digest() to support new hashing scheme.
 *
 * Revision 1.2  2005/01/20 16:27:07  openns
 * (brc) Fixed typos in certificate hashing and checking (failed because it wasn't looking
 * at the right certificate).
 *
 * Revision 1.1  2004/11/01 22:27:03  openns
 * Example certificate construction and mangement code for use with simple text
 * files for public certificates and HASP keys for secret keys.
 *
 *
 * File:	excertlib.c
 * Purpose:	Support routines for the example key and certificate management scheme.
 * Date:	2004-10-23
 *
 * This is free source code, courtesy of the OpenNavigationSurface project.  Visit
 * the website http://www.opennavsurf.org
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory>
#include <string>
#include <sstream>
#include <vector>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include "stdtypes.h"
#include "onscrypto.h"
#include "crc32.h"
#include "excertlib.h"

static char *modname = "excertlib";
static char *modrev = "$Revision: 1.2 $";

#undef __DEBUG_HASH__
#undef __DEBUG__

//#define __WITH_HASP__	/*! < Define if HASP interface should be compiled in */

#ifdef __WITH_HASP__
#include "hasp/hasp.h"

#define HASP_MEMORY_SIZE	248
#define HASP_SEED			100		/* Essentially random value for interface */
#define HASP_PASS1			15417	/* Standard value for DemoMA M4 HASP */
#define HASP_PASS2			9632	/* Standard value for DemoMA M4 HASP */
#endif

#define STRING_BREAK_LENGTH	60	/* Number of bytes to output before doing a newline */

#define PUBLIC_KEY_STRING		"------ OpenNavigationSurface Public Key ------"
#define SECRET_KEY_STRING		"------ OpenNavigationSurface Secret Key ------"
#define SIGNATURE_KEY_STRING	"------ OpenNavigationSurface Signature ------"

namespace {
//! Utility class to convert an encoded XML string.
class EncodedString
{
public:
    //************************************************************************
    //! Constructor
    /*!
    \param doc
        \li The XML document that the string is from.
    \param string
        \li The string to be converted.
    */
    //************************************************************************
    EncodedString(const xmlDoc &doc, const char *string)
    {
        this->m_pEncodedString = xmlEncodeEntitiesReentrant((xmlDoc *)&doc, (const xmlChar *)string);
    }

    //************************************************************************
    //! Destructor.
    //************************************************************************
    ~EncodedString()
    {
        xmlFree(this->m_pEncodedString);
    }

    //************************************************************************
    //! Conversion operator.
    /*!
    \return 
        \li The encoded string.
    */
    //************************************************************************
    operator xmlChar*() const
    {
        return this->m_pEncodedString;
    }

private:
    //! The encoded string.
    xmlChar* m_pEncodedString;
};

//************************************************************************
//! Convert a string to a double value.
/*!
\param value
    \li The intput string to be converted.
\return
    \li The doulbe value.
*/
//************************************************************************
double toDouble(const std::string &value)
{
    std::stringstream lineStream;
    lineStream.imbue(std::locale::classic());
    lineStream << value;

    double dblValue = 0.0;
    lineStream >> dblValue;

    return dblValue;
}

//************************************************************************
//! Get the name of a node.
/*!
\param value
    \li The intput string to be converted.
\return
    \li The doulbe value.
*/
//************************************************************************
std::string getNodeName(const xmlNode &node)
{
    std::string name;

    // append the namespace prefix
    const xmlNs* nameSpace = node.ns;
    if (nameSpace)
    {
        name = (const char*)nameSpace->prefix;
        name += ':';
    }

    name += (const char*)node.name;
    return name;
}

//************************************************************************
//! Find all the nodes matching the supplied xpath pattern.
/*!
\param relativeNode
    \li The node to start searching from.
\param searchString
    \li The pattern to be searched for.  The string conforms to the xpath
        searching syntax.  For detailed information, see http://www.w3schools.com/xpath/xpath_syntax.asp
\return
    \li The list of XML nodes that match \e searchString
*/
//************************************************************************
std::vector<const xmlNode*> findNodes(const xmlNode &relativeNode, const char *searchString)
{
    std::vector<const xmlNode*> retList;

    //Get the root node of the document.
    const xmlNode *pRoot = xmlDocGetRootElement(relativeNode.doc);
    if (pRoot == NULL)
        return std::vector<const xmlNode*>();

    //If the xPath context has not been initialized yet, do it now.
    xmlXPathContext *pContext = xmlXPathNewContext(relativeNode.doc);
    if (pContext == NULL)
        return std::vector<const xmlNode*>();

    pContext->node = const_cast<xmlNode*>(&relativeNode);

    //Register any namespaces with the xPath context.
    const xmlNs *xmlNameSpace = pRoot->nsDef;
    while (xmlNameSpace != NULL)
    {
        if (xmlNameSpace->prefix != NULL)
        {
            const int ret = xmlXPathRegisterNs(pContext, xmlNameSpace->prefix, xmlNameSpace->href);
            if (ret != 0)
            {
                //Error
                 xmlXPathFreeContext(pContext);
                return std::vector<const xmlNode*>();
            }
        }

        xmlNameSpace = xmlNameSpace->next;
    }

    //Encode the specified search string.
    const EncodedString encodedSearch(*relativeNode.doc, searchString);

    //Evaluate the expression.
    xmlXPathObject *pPathObject = xmlXPathEvalExpression(encodedSearch, pContext);
    if (pPathObject == NULL)
    {
        //Error
         xmlXPathFreeContext(pContext);
        return std::vector<const xmlNode*>();
    }

    //Add each value that was returned.
    if (pPathObject->nodesetval != NULL)
    {
        for (int i = 0; i < pPathObject->nodesetval->nodeNr; i++)
            retList.push_back(pPathObject->nodesetval->nodeTab[i]);
    }

    xmlXPathFreeObject(pPathObject);
    xmlXPathFreeContext(pContext);

    return retList;
}

//************************************************************************
//! Find a single node matching the given xpath pattern.
/*!
\param relativeNode
    \li The node to start searching from.
\param searchString
    \li The pattern to be searched for.  The string conforms to the xpath
        searching syntax.  For detailed information, see http://www.w3schools.com/xpath/xpath_syntax.asp
\return
    \li The first XML node that matches \e searchString
*/
//************************************************************************
const xmlNode* findNode(const xmlNode &relativeNode, const char *searchString)
{
    std::vector<const xmlNode*> retList = findNodes(relativeNode, searchString);
    if (retList.empty())
        return NULL;

    return retList.front();
}

//************************************************************************
//! Get the named property from an XML node.
/*!
\param current
    \li The node to retreive the property from.
\param propertyName
    \li The name of the property to be retreived.
\return
    \li The property maching \e propertyName from \e current.
*/
//************************************************************************
std::string getProperty(const xmlNode &current, const char *propertyName)
{
     // Retrieve the property.
    xmlChar * temp = xmlGetProp(const_cast<xmlNode *>(&current), EncodedString(*current.doc, propertyName));
    if(temp == NULL)
        return std::string();

    const std::string value((const char *)temp);

    // Free the memory allocated by xmlGetProp().
    xmlFree(temp);

    return value;
}

//************************************************************************
//! Get the contents of an XML node.
/*!
\param current
    \li The node to retreive the contents from.
\return
    \li The contents of \e current.
*/
//************************************************************************
std::string getContents(const xmlNode &current)
{
    std::string contents;

    // Get the children of the current element.
    const xmlNode * text = current.children;

    // Concatenate all the text elements.
    while(text != NULL)
    {
        if (text->type == XML_TEXT_NODE && text->content != NULL)
            contents += (const char *)text->content;

        text = text->next;
    }

    return contents;
}

//************************************************************************
//! Get the named property from an XML node as a string.
/*!
\param node
    \li The node to begin searching from.
\param searchPath
    \li The pattern to be searched for.  The string conforms to the xpath
        searching syntax.  For detailed information, see http://www.w3schools.com/xpath/xpath_syntax.asp
\param propertyName
    \li The name of the property to be retreived from the node maching 
        \e searchPath
\return
    \li The specified property as a string value.  NULL is returned if
        a node matching \e searchPath cannot be found.
*/
//************************************************************************
u8* getPropertyAsString(const xmlNode &node, const char *searchPath, const char * propertyName)
{
    const xmlNode *pNode = findNode(node, searchPath);
    if (pNode == NULL)
        return NULL;

    const std::string &value = getProperty(*pNode, propertyName);
    return (u8*)_strdup(value.c_str());
}

//************************************************************************
//! Get the contents of an XML node as a string.
/*!
\param node
    \li The node to begin searching from.
\param searchPath
    \li The pattern to be searched for.  The string conforms to the xpath
        searching syntax.  For detailed information, see http://www.w3schools.com/xpath/xpath_syntax.asp
\return
    \li The contents of \e node as a string value.  NULL is returned if
        a node matching \e searchPath cannot be found.
*/
//************************************************************************
u8* getContentsAsString(const xmlNode &node, const char *searchPath)
{
    const xmlNode *pNode = findNode(node, searchPath);
    if (pNode == NULL)
        return NULL;

    const std::string &value = getContents(*pNode);
    return (u8*)_strdup(value.c_str());
}

//************************************************************************
//! Get the contents of an XML node as an integer.
/*!
\param node
    \li The node to begin searching from.
\param searchPath
    \li The pattern to be searched for.  The string conforms to the xpath
        searching syntax.  For detailed information, see http://www.w3schools.com/xpath/xpath_syntax.asp
\return
    \li The contents of \e node as an integer value.  0 is returned if
        a node matching \e searchPath cannot be found.
*/
//************************************************************************
s32 getContentsAsInt(const xmlNode &node, const char *searchPath)
{
    const xmlNode *pNode = findNode(node, searchPath);
    if (pNode == NULL)
        return 0;

    const std::string &value = getContents(*pNode);
    return (s32)toDouble(value);
}


} // anonymous namespace

/* Routine:	excert_write_tabs
 * Purpose:	Write a given number of tabs to output, to make things purty
 * Inputs:	*op		File on which to write
 *			ntabs	Number of tabs to write
 * Output:	-
 * Comment:	-
 */

static void excert_write_tabs(FILE *op, u32 ntabs)
{
	u32	i;

	for (i = 0; i < ntabs; ++i) fprintf(op, "\t");
}

/* Routine:	excert_write_hex_xml
 * Purpose:	Write out a binary sequence in hex, with linewrap and tabs
 * Inputs:	*op			File on which to write
 *			*hexstring	ASCII version of the sequence to write
 *			base_tab	Number of tabs to the parent node of this data
 * Outputs:	-
 * Comment:	This routine can't take the binary internal structure directly since it
 *			can only be converted, at present, by a routine in ons_() that needs to
 *			know what type of string it is.  Hmmm ... I feel some API-change coming on.
 */

static void excert_write_hex_xml(FILE *op, char *hexstring, u32 base_tab)
{
	u32		wrap_length = STRING_BREAK_LENGTH - 4*base_tab, c, line_out,
			tot_len = (u32)strlen(hexstring);

	excert_write_tabs(op, base_tab+1);
	line_out = 0;
	for (c = 0; c < tot_len; ++c) {
		fprintf(op, "%c", hexstring[c]);
		++line_out;
		if (line_out >= wrap_length && c != tot_len-1) {
			fprintf(op, "\n");
			line_out = 0;
			excert_write_tabs(op, base_tab+1);
		}
	}
	fprintf(op, "\n");
}

#ifdef __WITH_HASP__
/* Routine:	excert_check_hasp
 * Purpose:	Check that the correct M4 HASP key is attached to the computer
 * Inputs:	seed	Seed value to use for the HASP protocol
 * Outputs:	*port	Port that the HASP is using for communication
 *			*id		The HASP's unique ID number
 * Comment:	This checks that the hasp is attached, that it is an M4 hasp (i.e., with
 *			memory to hold a key), and that it belongs to us (i.e., uses the correct
 *			password keys).
 */

static ExcertErr excert_check_hasp(u32 seed, u32 *port, u32 *id)
{
	s32	hasp1, hasp2, hasp3, hasp4, pass1 = HASP_PASS1, pass2 = HASP_PASS2;
	s32	hasp_major, hasp_minor;

	/* Check whether the hasp is in place */
	hasp1 = hasp2 = hasp3 = hasp4 = 0; /* no input parameters */
	*port = 0;
	hasp(LOCALHASP_ISHASP, 0, *port, 0, 0, &hasp1, &hasp2, &hasp3, &hasp4);
	if (hasp3) {
		fprintf(stderr, "%s: error: failed to check the HASP is present.\n", modname);
		return(EXCERT_NO_HASP);
	}

	/* Check for the correct type of HASP being in place */
	hasp1 = hasp2 = hasp3 = hasp4 = 0; /* no input parameters */
	hasp(LOCALHASP_HASPSTATUS, seed, *port, pass1, pass2, &hasp1, &hasp2, &hasp3, &hasp4);
	if (hasp3 == 0) {
		fprintf(stderr, "%s: error: failed to find HASP required (passwd incorrect).\n", modname);
		return(EXCERT_WRONG_HASP);
	}
	
	*port = hasp3;	/* p3 is the port number where the HASP was found */

	hasp_major = hasp4/1000; hasp_minor = hasp4 % 1000;
	if (hasp2 != 1 || hasp1 != 4) {
		fprintf(stderr, "%s: error: correct HASP, but wrong type (not M4).\n", modname);
		return(EXCERT_WRONG_HASP);
	}

	/* Check that we have a real HASP4 */
	hasp1 = hasp2 = hasp3 = hasp4 = 0;
	hasp(LOCALHASP_HASPGENERATION, seed, *port, pass1, pass2, &hasp1, &hasp2, &hasp3, &hasp4);
	if (hasp3) {
		fprintf(stderr, "%s: error: failed to check HASP generation.\n", modname);
		return(EXCERT_WRONG_HASP);
	}
	if (hasp1 != 1) {
		fprintf(stderr, "%s: error: HASP3 is connected, not HASP4.\n", modname);
		return(EXCERT_WRONG_HASP);
	}

	/* Find HASP4 ID number */
	hasp(MEMOHASP_HASPID, seed, *port, pass1, pass2, &hasp1, &hasp2, &hasp3, &hasp4);
	if (hasp3) {
		fprintf(stderr, "%s: error: failed to read HASP ID.\n", modname);
		return(EXCERT_HASP_READ_ERROR);
	}
	*id = (u32)hasp1 + ((u32)hasp2 << 16);

	return(EXCERT_OK);
}
#endif

#ifdef __WITH_HASP__
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

u8 *excert_get_hasp_key(u8 *phrase, ExcertErr *rc)
{
	u8			*aes_key, *sec_key_ctext, *sec_key_ptext;
	s32			sec_key_clen, hasp1, hasp2, hasp3, hasp4, hasp_id;
	OnsCryptErr	cryprc;

	s32	seed = HASP_SEED, port, pass1 = HASP_PASS1, pass2 = HASP_PASS2;

	if ((*rc = excert_check_hasp(seed, &port, &hasp_id)) != EXCERT_OK) {
		fprintf(stderr, "%s: error: failed to ensure HASP is present.\n", modname);
		return(NULL);
	}
	if ((sec_key_ctext = (u8*)malloc(HASP_MEMORY_SIZE*sizeof(u16))) == NULL) {
		fprintf(stderr, "%s: error: failed to get memory for secret key readback.\n", modname);
		*rc = EXCERT_NO_MEM;
		return(NULL);
	}
	
	/* Try to read some memory from the HASP */
	hasp1 = 0; /* Start of block */
	hasp2 = HASP_MEMORY_SIZE; /* Size in words */
	hasp3 = 0;	/* High word of address */
	hasp4 = (int)sec_key_ctext;
	hasp(MEMOHASP_READBLOCK, seed, port, pass1, pass2, &hasp1, &hasp2, &hasp3, &hasp4);
	if (hasp3) {
		fprintf(stderr, "%s: error: failed to read memory buffer from HASP.\n", modname);
		free(sec_key_ctext);
		*rc = EXCERT_HASP_READ_ERROR;
		return(NULL);
	}

#ifdef __DEBUG_HASP__
	printf("HASP memory contents (on read):\n");
	for (hasp1 = 0; hasp1 < HASP_MEMORY_SIZE*sizeof(u16); ++hasp1)
		printf("%02X", sec_key_ctext[hasp1]);
	printf("\n");
#endif

#ifdef __BAG_BIG_ENDIAN__
	{
		u32	w;
		
		for (w = 0; w < HASP_MEMORY_SIZE; ++w) {
			swap_2((void*)(sec_key_ctext + w*sizeof(u16)));
		}
	}
#endif

	sec_key_clen = ((u16*)sec_key_ctext)[0];

	if ((aes_key = ons_phrase_to_key(phrase)) == NULL) {
		fprintf(stderr, "%s: error: failed to convert pass-phrase to AES256 key.\n", modname);
		free(sec_key_ctext);
		*rc = EXCERT_CRYPTO_FAILED;
		return(NULL);
	}

	if ((sec_key_ptext = ons_decrypt_key(sec_key_ctext+2, sec_key_clen, aes_key, &cryprc)) == NULL) {
		fprintf(stderr, "%s: error: failed to decrypt secret key with pass-phrase (code %d).\n", modname, (u32)cryprc);
		free(sec_key_ctext); free(aes_key);
		*rc = EXCERT_CRYPTO_FAILED;
		return(NULL);
	}

	free(aes_key);
	return(sec_key_ptext);
}
#endif

/* Routine:	excert_read_xml_seckey
 * Purpose:	Read the encrypted secret key from an XML file and decrypt
 * Inputs:	*filename	File to read XML from
 *			*phrase		Pass-phrase for AES-256 symmetric encryption decrypt
 * Outputs:	Returns pointer to secret key in internal format, or NULL on error
 *			*errcode	Error code appropriate
 * Comment:	This routine parses an XML file and looks for an encrypted secret key in it.  If present, the
 *			key is decoded by hashing the pass-phrase into an AES-256 key and then decoding.
 */

u8 *excert_read_xml_seckey(char *filename, u8 *phrase, ExcertErr *errcode)
{
	xcrtCertificate *cert;
	u8	*aes_key, *sec_key_ptext, *sec_key_ctext;
	u32	sec_key_clen, digit, hex;
	OnsCryptErr		cryprc;

	if ((cert = excert_read_xml_certificate(filename, errcode)) == NULL) {
		fprintf(stderr, "%s: error: failed to parse \"%s\" for XML certificate.\n", modname, filename);
		return(NULL);
	}
	if (cert->key.is_public) {
		fprintf(stderr, "%s: error: the key in file \"%s\" is not a secret key (and we need one here).\n", modname, filename);
		*errcode = EXCERT_BAD_CERTIFICATE;
		excert_release_xml_certificate(cert);
		return(NULL);
	}
	sec_key_clen = (u32)strlen((const char *)cert->key.key);
	if ((sec_key_clen %2) == 1) {
		fprintf(stderr, "%s: error: length of key is not even --- corrupt key?\n", modname);
		*errcode = EXCERT_BAD_SECKEY;
		excert_release_xml_certificate(cert);
		return(NULL);
	}
	sec_key_clen /= 2;
	if ((sec_key_ctext = (u8*)malloc(sec_key_clen)) == NULL) {
		fprintf(stderr, "%s: error: failed to get memory for secret key de-crypt.\n", modname);
		excert_release_xml_certificate(cert);
		*errcode = EXCERT_NO_MEM;
		return(NULL);
	}
	for (digit = 0; digit < sec_key_clen; ++digit) {
		if (sscanf((const char *)(cert->key.key + digit*2), "%02X", &hex) != 1) {
			fprintf(stderr, "%s: error: failed to convert digit %d of encrypted secret key.\n", modname, digit);
			excert_release_xml_certificate(cert);
			free(sec_key_ctext);
			*errcode = EXCERT_BAD_SECKEY;
			return(NULL);
		}
		sec_key_ctext[digit] = (u8)(hex & 0xFFU);
	}
	if ((aes_key = ons_phrase_to_key((char *)phrase)) == NULL) {
		fprintf(stderr, "%s: error: failed to convert pass-phrase to AES256 key.\n", modname);
		free(sec_key_ctext);
		excert_release_xml_certificate(cert);
		*errcode = EXCERT_CRYPTO_FAILED;
		return(NULL);
	}
	if ((sec_key_ptext = ons_decrypt_key(sec_key_ctext, sec_key_clen, aes_key, &cryprc)) == NULL) {
		fprintf(stderr, "%s: error: failed to decrypt secret key with pass-phrase (code %d).\n", modname, (u32)cryprc);
		free(sec_key_ctext);
		free(aes_key);
		*errcode = EXCERT_CRYPTO_FAILED;
		excert_release_xml_certificate(cert);
		return(NULL);
	}

	free(aes_key);
	return(sec_key_ptext);
}

#ifdef __WITH_HASP__
/* Routine:	excert_put_hasp_key
 * Purpose:	Write a secret key to HASP memory using the pass-phrase as a source of AES encryption key
 * Inputs:	*sec_key_ptext	Plaintext for the secret key
 *			*phrase			Pass-phrase to use for AES256 key generation
 * Outputs:	Returns an appropriate error code, EXCERT_OK on success.
 * Comment:	This hashes the pass-phrase to form an AES256 key, and then uses this key
 *			to encrypt the secret key.  The secret key is then written to the memory of the attached
 *			HASP key if possible.
 */

ExcertErr excert_put_hasp_key(u8 *sec_key_ptext, u8 *phrase)
{
	u8			*aes_key, *sec_key_ctext, *membuf;
	u32			sec_key_clen, hasp1, hasp2, hasp3, hasp4, hasp_id;
	s32			seed = HASP_SEED, port, pass1 = HASP_PASS1, pass2 = HASP_PASS2;
	ExcertErr	rc;

	if ((aes_key = ons_phrase_to_key(phrase)) == NULL) {
		fprintf(stderr, "%s: error: failed to generate AES key from pass-phrase.\n", modname);
		return(EXCERT_CRYPTO_FAILED);
	}
	if ((sec_key_ctext = ons_encrypt_key(sec_key_ptext, aes_key, &sec_key_clen)) == NULL) {
		fprintf(stderr, "%s: error: failed to encrypt secret key with AES256.\n", modname);
		free(aes_key);
		return(EXCERT_CRYPTO_FAILED);
	}
	if ((sec_key_clen+2) > HASP_MEMORY_SIZE*sizeof(u16)) {
		fprintf(stderr, "%s: error: encrypted secret key is %d bytes, more than the HASP's %d bytes memory.\n",
			modname, sec_key_clen, HASP_MEMORY_SIZE);
		free(aes_key); free(sec_key_ctext);
		return(EXCERT_CRYPTO_FAILED);
	}
	
	free(aes_key);

	if ((rc = excert_check_hasp(seed, &port, &hasp_id)) != EXCERT_OK) {
		fprintf(stderr, "%s: error: failed talking to HASP.\n", modname);
		free(sec_key_ctext);
		return(rc);
	}
	if ((membuf = (u8*)malloc(sizeof(u16)*HASP_MEMORY_SIZE)) == NULL) {
		fprintf(stderr, "%s: error: failed getting memory for HASP write.\n", modname);
		free(sec_key_ctext);
		return(EXCERT_NO_MEM);
	}
	memset(membuf, 0, HASP_MEMORY_SIZE*sizeof(u16));
	((u16*)membuf)[0] = (u16)sec_key_clen;
	memcpy(membuf+2, sec_key_ctext, sec_key_clen);

#ifdef __BAG_BIG_ENDIAN__
	{
		u32	w;
		
		for (w = 0; w < HASP_MEMORY_SIZE; ++w) {
			swap_2((void*)(membuf + w*sizeof(u16)));
		}
	}
#endif

#ifdef __DEBUG_HASP__
	printf("HASP memory contents (on write):\n");
	for (hasp1 = 0; hasp1 < HASP_MEMORY_SIZE*sizeof(u16); ++hasp1)
		printf("%02X", membuf[hasp1]);
	printf("\n");
#endif

	hasp1 = 0;
	hasp2 = HASP_MEMORY_SIZE;	/* Size in words, not bytes */
	hasp3 = 0;
	hasp4 = (int)membuf;
	hasp(MEMOHASP_WRITEBLOCK, seed, port, pass1, pass2, &hasp1, &hasp2, &hasp3, &hasp4);
	if (hasp3) {
		fprintf(stderr, "%s: error: failed to write memory buffer to HASP.\n", modname);
		free(sec_key_ctext);
		return(EXCERT_HASP_WRITE_ERROR);
	}

	free(sec_key_ctext);
	return(EXCERT_OK);
}
#endif

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

ExcertErr excert_write_xml_seckey(u8 *sec_key_ptext, u8 *phrase, char *filename)
{
	u8		*aes_key, *sec_key_ctext;
	char	*sec_key_ctext_ascii;
	u32		sec_key_clen, samp;
	FILE	*op;

	if ((aes_key = ons_phrase_to_key((char *)phrase)) == NULL) {
		fprintf(stderr, "%s: error: failed to generate AES key from pass-phrase.\n", modname);
		return(EXCERT_CRYPTO_FAILED);
	}
	if ((sec_key_ctext = ons_encrypt_key((u8 *)sec_key_ptext, aes_key, &sec_key_clen)) == NULL) {
		fprintf(stderr, "%s: error: failed to encrypt secret key with AES256.\n", modname);
		free(aes_key);
		return(EXCERT_CRYPTO_FAILED);
	}
	free(aes_key);
	if ((sec_key_ctext_ascii = (char*)malloc(sec_key_clen*2 + 1)) == NULL) {
		fprintf(stderr, "%s: error: no memory for secret key processing.\n", modname);
		return(EXCERT_NO_MEM);
	}
	for (samp = 0; samp < sec_key_clen; ++samp)
		sprintf(sec_key_ctext_ascii + samp*2, "%02X", sec_key_ctext[samp]);
	sec_key_ctext_ascii[sec_key_clen*2] = '\0';
	free(sec_key_ctext);
	if ((op = fopen(filename, "w")) == NULL) {
		fprintf(stderr, "%s: error: failed to open \"%s\" for encrypted secret key output.\n", modname, filename);
		return(EXCERT_NO_FILE);
	}
	fprintf(op, "<?xml version=\"1.0\"?>\n");
	fprintf(op, "<entity>\n");
	fprintf(op, "\t<key type=\"private\" algorithm=\"openns\" version=\"1.0\" encoding=\"crypthex\">\n");
	excert_write_hex_xml(op, sec_key_ctext_ascii, 2);
	fprintf(op, "\t</key>\n</entity>\n");
	fclose(op);
	free(sec_key_ctext_ascii);
	return(EXCERT_OK);
}

/* Routine:	excert_bytes_to_ext
 * Purpose:	Encode bytes into human-readable format on output stream
 * Inputs:	*bytes	Bytes to write to output
 *			*op		FILE to write onto
 * Outputs:	Returns an appropriate error code, EXCERT_OK on success.
 * Comment:	This encodes the bytes as simple hex, rather than something more
 *			compact like base64, mostly because it's simple.
 */

static ExcertErr excert_bytes_to_ext(char *bytes, FILE *op)
{
	u32	b, n_out;
	u32	nbytes = (u32)strlen(bytes);

	fprintf(op, "%04X", nbytes); n_out = 4;
	for (b = 0; b < nbytes; ++b) {
		fprintf(op, "%c", bytes[b]); ++n_out;
		if (n_out >= STRING_BREAK_LENGTH) {
			fprintf(op, "\n");
			n_out = 0;
		}
	}
	fprintf(op, "=\n");
	return(EXCERT_OK);
}

/* Routine: excert_ext_to_bytes
 * Purpose:	Decode human-readable form from input stream to bytes
 * Inputs:	*ip		FILE to read from
 * Outputs:	*nbytes	Number of bytes decoded from the input stream
 *			Returns pointer to the decoded bytes, or NULL on failure
 * Comment:	This assumes that the data was written as excert_bytes_to_ext(),
 *			so that the line lengths and termination conditions are known.
 */

static u8 *excert_ext_to_bytes(FILE *ip, u32 *nbytes)
{
	u8		*buffer, *op;
	u32		buf_len, ip_len;
	Bool	found;

	if ((buffer = (u8*)malloc(STRING_BREAK_LENGTH + 10)) == NULL) {
		fprintf(stderr, "%s: error: failed to get memory for input transcription.\n", modname);
		return(NULL);
	}

	fgets((char *)buffer, STRING_BREAK_LENGTH+10, ip);
	buffer[(buf_len = (u32)strlen((const char *)buffer))-1] = '\0'; --buf_len; /* chomp */
	sscanf((const char *)buffer, "%04X", &ip_len);
	memmove(buffer, buffer+4, buf_len-3);
	buf_len -= 4;

	if ((op = (u8*)malloc(ip_len+1)) == NULL) {
		fprintf(stderr, "%s: error: no memory for input string transcription.\n", modname);
		return(NULL);
	}

	found = False;
	*nbytes = 0;
	while (!feof(ip) && !found) {
		if (buffer[buf_len-1] == '=') {
			--buf_len;
			found = True;
		}
		memcpy(op + *nbytes, buffer, buf_len);
		*nbytes += buf_len;
		if (fgets((char *)buffer, STRING_BREAK_LENGTH+10, ip) == NULL) {
			if (ferror(ip)) {
				fprintf(stderr, "%s: error: hard read error on input.\n", modname);
				free(buffer);
				return(NULL);
			}
			continue;	/* End of file otherwise, so no data to work */
		}
		buffer[(buf_len = (u32)strlen((const char *)buffer))-1] = '\0'; --buf_len; /* chomp */
	}
	free(buffer);
	op[*nbytes] = '\0';
	return(op);
}

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

u8 *excert_get_object(FILE *ip, ExcertObject iptype)
{
    u8      *rtn;
	char	*search_string;
	char	buffer[1024];
	Bool	found;
	u32		n_read;

	switch(iptype) {
		case EXCERT_PUBLIC_KEY:
			search_string = PUBLIC_KEY_STRING;
			break;
		case EXCERT_SECRET_KEY:
			search_string = SECRET_KEY_STRING;
			break;
		case EXCERT_SIGNATURE:
			search_string = SIGNATURE_KEY_STRING;
			break;
		default:
			fprintf(stderr, "%s: error: internal error - object type not recognised.\n", modname);
			return(NULL);
	}

	found = False;
	while (!feof(ip) && !found) {
		fgets(buffer, 1024, ip);
		buffer[strlen(buffer)-1] = '\0'; /* chomp */
		if (strcmp(buffer, search_string) == 0) {
			found = True;
		}
		ungetc(fgetc(ip), ip);
	}
	if (!found) {
		fprintf(stderr, "%s: error: failed to find required object in input file.\n", modname);
		return(NULL);
	}
	if ((rtn = excert_ext_to_bytes(ip, &n_read)) == NULL) {
		fprintf(stderr, "%s: error: failed to convert object to internal format.\n", modname);
		return(NULL);
	}
	return(rtn);
}

/* Routine:	excert_put_object
 * Purpose:	Output an object description to file
 * Inputs:	*op		FILE to write into
 *			*obj	Object byte-stream to be written
 *			obtype	How to describe the object on the output stream
 * Outputs:	Returns an appropriate error code, EXCERT_OK on success
 * Comment:	This simply writes the appropriate recognition string to the output
 *			and then the object, converted to ASCII hex.
 */

ExcertErr excert_put_object(FILE *op, u8 *obj, ExcertObject obtype)
{
	u8			*recog_string;
	char		*op_string;
	ExcertErr	rc;

	switch(obtype) {
		case EXCERT_PUBLIC_KEY:
			recog_string = (u8*)PUBLIC_KEY_STRING;
			op_string = ons_key_to_ascii(obj);
			break;
		case EXCERT_SECRET_KEY:
			recog_string = (u8*)SECRET_KEY_STRING;
			op_string = ons_key_to_ascii(obj);
			break;
		case EXCERT_SIGNATURE:
			recog_string = (u8*)SIGNATURE_KEY_STRING;
			op_string = ons_sig_to_ascii(obj);
			break;
		default:
			fprintf(stderr, "%s: error: internal error - object type not known.\n", modname);
			return(EXCERT_INTERNAL_ERROR);
	}
	fprintf(op, "%s\n", recog_string);
	if ((rc = excert_bytes_to_ext(op_string, op)) != EXCERT_OK) {
		fprintf(stderr, "%s: error: failed to write object to output.\n", modname);
		free(op_string);
		return(rc);
	}
	free(op_string);
	return(EXCERT_OK);
}

/* Routine:	excert_hash_certificate
 * Purpose:	Compute the signature hash of the certificate
 * Inputs:	*cert	Name of the certificate to be hashed
 * Outputs:	*nbytes	Number of bytes in the certificate's hash
 *			Returns pointer to hash byte-stream or NULL on failure
 * Comment:	This extracts all of the information from the certificate into a temporary file,
 *			and then hashes it with the standard OpenNS signature algorithm.  This allows
 *			the code to compute the hash either with or without a signature section.
 */

u8 *excert_hash_certificate(char *cert, u32 *nbytes)
{
	FILE	*ip, *tmp;
	char	*tmp_file;
	u8		buffer[1024], *hash;
	Bool	in_sig;

	if ((ip = fopen(cert, "r")) == NULL) {
		fprintf(stderr, "%s: error: failed to open \"%s\" for signature hash.\n", modname, cert);
		return(NULL);
	}
	if ((tmp_file = tmpnam(NULL)) == NULL) {
		fprintf(stderr, "%s: error: failed to generate temporary file name for signature hash.\n", modname);
		fclose(ip);
		return(NULL);
	}
	if ((tmp = fopen(tmp_file, "w")) == NULL) {
		fprintf(stderr, "%s: error: failed to open temporary file for signature hash.\n", modname);
		fclose(ip);
		return(NULL);
	}
	in_sig = False;
	while (!feof(ip)) {
		fgets((char *)buffer, 1024, ip);
		if (strncmp((const char *)buffer, "signature {", 11) == 0) {
			/* In the signature section, mark for no output */
			in_sig = True;
		} else if (strncmp((const char *)buffer, "}", 1) == 0) {
			if (!in_sig) fprintf(tmp, "%s", buffer);
			in_sig = False;
		} else if (!in_sig) {
			fprintf(tmp, "%s", buffer);
		}
		ungetc(fgetc(ip), ip);
	}
	fclose(ip); fclose(tmp);
#ifdef __DEBUG_HASH__
	fprintf(stderr, "%s: debug: temporary file name \"%s\" will be reported for \"%s\" in debug code from onscrypto.\n",
		modname, tmp_file, cert);
#endif
	if ((hash = ons_gen_digest(tmp_file, NULL, 0, nbytes)) == NULL) {
		fprintf(stderr, "%s: error: failed to construct signature hash for \"%s\".\n", modname, cert);
		return(NULL);
	}
	remove(tmp_file);
	return(hash);
}

u8 *excert_hash_xml_certificate(xcrtCertificate *user, u32 *digest_len)
{
	char	*tmp_file;
	u8		*digest;

	if ((tmp_file = tmpnam(NULL)) == NULL) {
		fprintf(stderr, "%s: error: failed to get temporary file for certificate digest.\n", modname);
		return(NULL);
	}
	if (!excert_write_xml_certificate(user, tmp_file)) {
		fprintf(stderr, "%s: error: failed to write temporary file of certificate for digest.\n", modname);
		remove(tmp_file);
		return(NULL);
	}
	if ((digest = ons_gen_digest(tmp_file, NULL, 0, digest_len)) == NULL) {
		fprintf(stderr, "%s: error: failed to compute digest for certificate.\n", modname);
		remove(tmp_file);
		return(NULL);
	}
	remove(tmp_file);
	return(digest);
}

static u8 *excert_hash_xml_certificate_file(char *name, u32 *digest_len)
{
	char	*tmp_file, buffer[1024];
	FILE	*ip, *tmp;
	u8		*digest;
	Bool	in_sigstream;

	if ((tmp_file = tmpnam(NULL)) == NULL) {
		fprintf(stderr, "%s: error: failed to get name for temporary certificate file.\n", modname);
		return(NULL);
	}
	if ((ip = fopen(name, "r")) == NULL) {
		fprintf(stderr, "%s: error: failed to open input certificate file \"%s\".\n", modname, name);
		return(NULL);
	}
	if ((tmp = fopen(tmp_file, "w")) == NULL) {
		fprintf(stderr, "%s: error: failed to open temporary certificate file.\n", modname, name);
		fclose(ip);
		return(NULL);
	}
	in_sigstream = False;
	while (!feof(ip)) {
		fgets(buffer, 1024, ip);
		if (strstr(buffer, "<sigstream") != NULL) {
			/* In the signature section, mark for no output */
			in_sigstream = True;
			fprintf(tmp, "%s", buffer);
		} else if (strstr(buffer, "</sigstream>") != NULL) {
			in_sigstream = False;
			fprintf(tmp, "%s", buffer);
		} else if (!in_sigstream) {
			fprintf(tmp, "%s", buffer);
		}
		ungetc(fgetc(ip), ip);
	}
	fclose(ip); fclose(tmp);
	if ((digest = ons_gen_digest(tmp_file, NULL, 0, digest_len)) == NULL) {
		fprintf(stderr, "%s: error: failed to compute digest for certificate.\n", modname);
		remove(tmp_file);
		return(NULL);
	}
	remove(tmp_file);
	return(digest);	
}

/* Routine:	excert_find_section
 * Purpose:	Wind the given file forward to the appropriate section
 * Inputs:	*f			FILE to wind
 *			*section	Section to wind forward to
 * Outputs:	True if section was found, otherwise False
 * Comment:	-
 */

static Bool excert_find_section(FILE *f, char *section)
{
	char	buffer[1024], *recog_string;
	u32		recog_len;

	if ((recog_string = (char *)malloc(strlen(section) + 3)) == NULL) {
		fprintf(stderr, "%s: error: no memory for certificate parser recognition string.\n", modname);
		return(False);
	}
	sprintf(recog_string, "%s {", section);
	recog_len = (u32)strlen(recog_string);

	while (!feof(f)) {
		if (fgets(buffer, 1024, f) == NULL) {
			if (ferror(f)) {
				fprintf(stderr, "%s: error: failed reading input certificate.\n", modname);
			}
			free(recog_string);
			return(False);
		}
		buffer[strlen(buffer)-1] = '\0'; /* chomp */
		if (strncmp(buffer, recog_string, recog_len) == 0) break;
	}
	free(recog_string);
	if (feof(f)) return(False);
	else return(True);
}

/* Routine:	excert_release_tags
 * Purpose:	Release memory associated with tag values
 * Inputs:	*tags	CertTag structures to release
 * Outputs:	-
 * Comment:	-
 */

static void excert_release_tags(CertTag *tags)
{
	while (tags->tag != NULL) {
		free(tags->value);
		++tags;
	}
}

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

s32 excert_parse_certificate(char *cert, char *section, CertTag *table)
{
	FILE	*ip;
	char	buffer[1024], *start;
	CertTag	*tag;
	s32		n_found = 0;
	Bool	in_section;

	if ((ip = fopen(cert, "r")) == NULL) {
		fprintf(stderr, "%s: error: failed to open \"%s\" for parsing.\n", modname, cert);
		return(-1);
	}
	while (!feof(ip)) {
		if (!excert_find_section(ip, section)) break;
		in_section = True;
		while (!feof(ip) && in_section) {
			fgets(buffer, 1024, ip);
			buffer[strlen(buffer)-1] = '\0';
			if (strncmp(buffer, "}", 1) == 0) {
				in_section = False;
				ungetc(fgetc(ip), ip);
				continue;
			}
			start = buffer;
			while (*start == ' ' || *start == '\t') ++start;
			tag = table;
			while (tag->tag != NULL) {
				if (strncmp(start, tag->tag, strlen(tag->tag)) == 0) {
					++n_found;
					while (*start != ' ' && *start != '\t') ++start;
					while (*start == ' ' || *start == '\t') ++start;
					if ((tag->value = strdup(start)) == NULL) {
						fprintf(stderr, "%s: error: failed to parse tag \"%s\" from \"%s\".\n",
							modname, tag->tag, cert);
						return(n_found);
					}
					break;
				}
				++tag;
			}
			ungetc(fgetc(ip), ip);
		}
	}
	fclose(ip);
	return(n_found);
}

/* Routine:	excert_sign_certificate
 * Purpose:	Sign a certificate file using a HASP secret key, and a text public key
 * Inputs:	*cert	Name of the certificate file to be signed
 *			*auth	Name of the authority's certificate file
 *			*phrase	Pass-phrase to decode the HASP secret key
 * Outputs:	Returns appropriate error code, EXCERT_OK on success.
 * Comment:	This uses the pass-phrase to decode the HASP attached to the computer, computes
 *			the digest for the certificate, and then signs the digest using the HASP-derived
 *			secret key.
 */

ExcertErr excert_sign_certificate(char *cert, char *auth, char *phrase)
{
	u8			*sec_key, *sig, *digest;
	char		*sig_op;
	u32			digest_len;
	OnsCryptErr	cryptrc;
	CertTag		authtag[3] = { { "name", NULL }, { "keysource", NULL }, { NULL, NULL } };
	FILE		*op;

    if ((digest = excert_hash_certificate(cert, &digest_len)) == NULL) {
		fprintf(stderr, "%s: failed to compute digest hash for certificate.\n", modname);
		return(EXCERT_HASH_FAILED);
	}

#ifdef __WITH_HASP__
    ExcertErr rc = EXCERT_OK;
	if ((sec_key = excert_get_hasp_key((u8 *)phrase, &rc)) == NULL) {
		fprintf(stderr, "%s: error: failed to get secret key from HASP.\n", modname);
		free(digest);
		return(rc);
	}
#else
    fprintf(stderr, "%s: error: failed to get secret key from HASP.\n", modname);
	free(digest);
	return(EXCERT_NO_HASP);
#endif

	if ((sig = ons_sign_digest(digest, digest_len, sec_key, &cryptrc)) == NULL) {
		fprintf(stderr, "%s: error: failed to sign certificate digest.\n", modname);
		free(digest); free(sec_key);
		return(EXCERT_CRYPTO_FAILED);
	}
	free(digest); free(sec_key);
	if ((sig_op = ons_sig_to_ascii(sig)) == NULL) {
		fprintf(stderr, "%s: error: couldn't convert signature to ASCII value.\n", modname);
		free(sig);
		return(EXCERT_CRYPTO_FAILED);
	}
	free(sig);
	if (excert_parse_certificate(auth, "user", authtag) != 2) {
		fprintf(stderr, "%s: error: failed to find authority's ID tags from authority certificate \"%s\".\n",
			modname, auth);
		free(sig_op);
		excert_release_tags(authtag);
		return(EXCERT_BAD_CERTIFICATE);
	}
	if ((op = fopen(cert, "a+")) == NULL) {
		fprintf(stderr, "%s: error: failed to open \"%s\" for append of signature.\n", modname, cert);
		free(sig_op);
		excert_release_tags(authtag);
		return(EXCERT_BAD_CERTIFICATE);
	}
	fprintf(op, "signature {\n");
	fprintf(op, "\tauth\t\t%s\n", authtag[0].value); free(authtag[0].value);
	fprintf(op, "\tkeysource\t%s\n", authtag[1].value); free(authtag[1].value);
	fprintf(op, "\tvalue\t\t%s\n", sig_op);
	fprintf(op, "}\n");
	free(sig_op);
	fclose(op);

	return(EXCERT_OK);
}

ExcertErr excert_sign_xml_certificate(xcrtCertificate *user, xcrtCertificate *csa, char *csa_passphrase, Bool seckey_from_hasp, char *csa_seckey_file)
{
	u8			*digest, *csa_seckey;
	u32			digest_len;
	OnsCryptErr	cryptrc;
	ExcertErr	rc;
	
	/* Have to reset the signature portion of the user certificate, and determine whether it is valid */
	if (user->signature.signature != NULL) { free(user->signature.signature); user->signature.signature = NULL; }
	if (user->signature.algorithm != NULL) { free(user->signature.algorithm); user->signature.algorithm = NULL; }
	if (user->signature.version != NULL) { free(user->signature.version); user->signature.version = NULL; }
	if (user->signature.signer.keysource != NULL) { free(user->signature.signer.keysource); user->signature.signer.keysource = NULL; }
	if (user->signature.signer.name != NULL) { free(user->signature.signer.name); user->signature.signer.name = NULL; }
	if (user->signature.signer.organization != NULL) {
		free(user->signature.signer.organization);
		user->signature.signer.organization = NULL;
	}
	user->signature.algorithm = strdup("openns");
	user->signature.version = strdup("1.0");
	user->signature.signer.name = strdup(csa->user.name);
	user->signature.signer.organization = strdup(csa->user.organization);
	user->signature.signer.keysource = strdup(csa->user.keysource);
	user->signature.signer.idnum = csa->user.idnum;

	if (user->user.name == NULL || user->user.organization == NULL || user->user.keysource == NULL || user->user.idnum == 0) {
		fprintf(stderr, "%s: error: user information in certificate is incomplete.\n", modname);
		return(EXCERT_BAD_CERTIFICATE);
	}
	if (user->key.key == NULL || strcmp(user->key.algorithm, "openns") != 0 || strcmp(user->key.version, "1.0") != 0) {
		fprintf(stderr, "%s: error: key information in certificate is incomplete.\n", modname);
		return(EXCERT_BAD_CERTIFICATE);
	}

	/* Hash certificate by reconstruction, then sign */
	if ((digest = excert_hash_xml_certificate(user, &digest_len)) == NULL) {
		fprintf(stderr, "%s: error: failed to compute digest from user certificate.\n", modname);
		return(EXCERT_HASH_FAILED);
	}
	if (seckey_from_hasp) {
#ifdef __WITH_HASP__
		if ((csa_seckey = excert_get_hasp_key(csa_passphrase, &rc)) == NULL) {
			fprintf(stderr, "%s: error: failed to get secret key from HASP.\n", modname);
			free(digest);
			return(rc);
		}
#else
		fprintf(stderr, "%s: error: HASP support is not present (and you need it for this).\n",
			modname);
		free(digest);
		return(EXCERT_INTERNAL_ERROR);
#endif
	} else {
		if ((csa_seckey = excert_read_xml_seckey(csa_seckey_file, (u8 *)csa_passphrase, &rc)) == NULL) {
			fprintf(stderr, "%s: error: failed to get secret key from file \"%s\".\n", modname, csa_seckey_file);
			free(digest);
			return(rc);
		}
	}
	if ((user->signature.signature = ons_sign_digest(digest, digest_len, csa_seckey, &cryptrc)) == NULL) {
		fprintf(stderr, "%s: error: failed to sign certificate digest.\n", modname);
		free(digest); free(csa_seckey);
		return(EXCERT_CRYPTO_FAILED);
	}
	free(digest); free(csa_seckey);

	return(EXCERT_OK);
}

/* Routine:	excert_get_public_key
 * Purpose:	Extract the public key element from a certificate, if one exists
 * Inputs:	*cert	Certificate file to check
 * Outputs:	Returns pointer to public key in OpenNavSurf format, or NULL on failure.
 * Comment:	-
 */

u8 *excert_get_public_key(char *cert)
{
	FILE		*ip;
	char		*key;
    u8          *rtn;
	OnsCryptErr	cryptrc;

	if ((ip = fopen(cert, "r")) == NULL) {
		fprintf(stderr, "%s: error: failed to open \"%s\" for public key search.\n", modname, cert);
		return(NULL);
	}
	if (!excert_find_section(ip, "public_key")) {
		fprintf(stderr, "%s: error: failed to find public key in certificate file \"%s\".\n",
			modname, cert);
		fclose(ip);
		return(NULL);
	}
	if ((key = (char*)excert_get_object(ip, EXCERT_PUBLIC_KEY)) == NULL) {
		fprintf(stderr, "%s: error: failed to read public key in certificate file \"%s\".\n",
			modname, cert);
		fclose(ip);
		return(NULL);
	}
	fclose(ip);
	if ((rtn = ons_ascii_to_key(key, &cryptrc)) == NULL) {
		fprintf(stderr, "%s: error: failed to convert key string to public key.\n", modname);
		free(key);
		return(NULL);
	}
	free(key);
	return(rtn);
}

/* Routine:	excert_get_signature
 * Purpose:	Extract the signature element of the certificate
 * Inputs:	*cert	Certificate file to check
 * Outputs:	Returns the signature from the certificate in OpenNavSurf format, or NULL on failure
 * Comment:	-
 */

u8 *excert_get_signature(char *cert)
{
	u8			*sig;
	CertTag		sigtag[2] = { { "value", NULL }, { NULL, NULL } };
	OnsCryptErr	cryptrc;

	if (excert_parse_certificate(cert, "signature", sigtag) != 1) {
		fprintf(stderr, "%s: error: failed to find signature in certificate \"%s\".\n",
			modname, cert);
		excert_release_tags(sigtag);
		return(NULL);
	}
	if ((sig = ons_ascii_to_sig(sigtag[0].value, &cryptrc)) == NULL) {
		fprintf(stderr, "%s: error: failed to convert signature from \"%s\" to internal format.\n",
			modname, cert);
		excert_release_tags(sigtag);
		return(NULL);
	}
	excert_release_tags(sigtag);
	return(sig);
}

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

Bool excert_verify_certificate(char *cert, char *auth)
{
	u8			*public_key = NULL, *signature = NULL, *digest = NULL;
	u32			digest_len;
	Bool        rc;

	if ((public_key = excert_get_public_key(auth)) == NULL ||
		(signature = excert_get_signature(cert)) == NULL ||
		(digest = excert_hash_certificate(cert, &digest_len)) == NULL) {
		fprintf(stderr, "%s: error: failed to extract information from certificates.\n", modname);
		free(public_key); free(signature); free(digest);
		return(False);
	}
	rc = ons_verify_signature(signature, public_key, digest, digest_len);
	free(public_key); free(signature); free(digest);
    return(rc);
}

Bool excert_verify_xml_certificate(xcrtCertificate *cert, char *cert_file, xcrtCertificate *auth)
{
	u8		*digest = NULL;
	u32		digest_len;
	Bool	rc;

	if ((digest = excert_hash_xml_certificate_file(cert_file, &digest_len)) == NULL) {
		fprintf(stderr, "%s: error: failed to compute digest from \"%s\".\n", modname, cert_file);
		return(False);
	}
	rc = ons_verify_signature(cert->signature.signature, auth->key.key, digest, digest_len);
	free(digest);
	return(rc);
}

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

ExcertErr excert_construct_certificate(char *user_name, char *user_org, char *user_id, CertTag *opt_arg,
									   u8 *pub_key, char *cert)
{
	FILE		*op;
	ExcertErr	rc;

	if ((op = fopen(cert, "w")) == NULL) {
		fprintf(stderr, "%s: error: failed to open \"%s\" for certificate.\n", cert);
		return(EXCERT_NO_FILE);
	}
	fprintf(op, "user {\n");
	fprintf(op, "\tname\t\t%s\n", user_name);
	fprintf(op, "\torganisation\t%s\n", user_org);
	fprintf(op, "\tid\t\t%s\n", user_id);
	if (opt_arg != NULL) {
		while (opt_arg->tag != NULL) {
			fprintf(op, "\t%s %s\n", opt_arg->tag, opt_arg->value);
			++opt_arg;
		}
	}
	fprintf(op, "}\n");
	fprintf(op, "public_key {\n");
	if ((rc = excert_put_object(op, pub_key, EXCERT_PUBLIC_KEY)) != EXCERT_OK) {
		fprintf(stderr, "%s: error: failed to write public key to certificate \"%s\".\n",
			modname, cert);
		fclose(op);
		return(EXCERT_BAD_FILE);
	}
	fprintf(op, "}\n");
	fclose(op);
	return(EXCERT_OK);
}

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

ExcertErr excert_generate_keys(char *phrase, u8 **pkey, Bool output_to_hasp, char *filename)
{
	u8			*seckey = NULL;
	OnsCryptErr	cryptrc;
	ExcertErr	rc;

	if ((cryptrc = ons_generate_keys(pkey, &seckey)) != ONS_CRYPTO_OK) {
		fprintf(stderr, "%s: error: failed to generate OpenNavSurf keypair.\n", modname);
		free(seckey);
		return(EXCERT_CRYPTO_FAILED);
	}
	if (output_to_hasp) {
#ifdef __WITH_HASP__
		rc = excert_put_hasp_key(seckey, phrase);
#else
		fprintf(stderr, "%s: error: HASP support is not present (and you need it for this).\n",
			modname);
		free(seckey);
		return(EXCERT_INTERNAL_ERROR);
#endif
	} else
		rc = excert_write_xml_seckey(seckey, (u8 *)phrase, filename);
	free(seckey);
	return(EXCERT_OK);
}

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

ExcertErr excert_build_certificate(char *proto, char *cert, char *phrase)
{
	ExcertErr	rc;
	u8			*pkey = NULL;
	CertTag		user[4] = { { "name", NULL }, { "organisation", NULL }, { "id", NULL }, { NULL, NULL } };

	if (excert_parse_certificate(proto, "user", user) != 3) {
		fprintf(stderr, "%s: error: not enough information in prototype certificate \"%s\" to continue.\n",
			modname, proto);
		return(EXCERT_BAD_FILE);
	}
	if ((rc = excert_generate_keys(phrase, &pkey, True, NULL)) != EXCERT_OK) {
		fprintf(stderr, "%s: error: failed to generate keys for certificate.\n", modname);
		free(pkey);
		return(rc);
	}
	if ((rc = excert_construct_certificate(user[0].value, user[1].value, user[2].value, NULL, pkey, cert)) != EXCERT_OK) {
		fprintf(stderr, "%s: error: failed to construct user certificate in \"%s\".\n", modname);
		free(pkey);
		return(rc);
	}
	free(pkey);
	return(EXCERT_OK);
}

ExcertErr excert_build_xml_certificate(xcrtCertificate *cert, char *phrase, Bool output_to_hasp, char *seckey_filename)
{
	ExcertErr	rc;
	
	if (cert->user.name == NULL || cert->user.organization == NULL ||
		cert->user.keysource == NULL || cert->user.idnum == 0) {
		fprintf(stderr, "%s: error: not enough information in prototype certificate to continue.\n",
			modname);
		return(EXCERT_BAD_FILE);
	}
	if ((rc = excert_generate_keys(phrase, &cert->key.key, output_to_hasp, seckey_filename)) != EXCERT_OK) {
		fprintf(stderr, "%s: error: failed to generate keys for certificate.\n", modname);
		free(cert->key.key); cert->key.key = NULL;
		return(rc);
	}
	cert->key.algorithm = strdup("openns");
	cert->key.version = strdup("1.0");
	cert->key.is_public = True;
	return(EXCERT_OK);
}

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

ExcertErr excert_sign_ons(char *ons, char *phrase, u32 num, Bool seckey_from_hasp, char *seckey_filename)
{
	u8			*seckey;
	ExcertErr	rc;

	if (seckey_from_hasp) {
#ifdef __WITH_HASP__
		if ((seckey = excert_get_hasp_key(phrase, &rc)) == NULL) {
			fprintf(stderr, "%s: error: failed to get secret key from HASP.\n", modname);
			return(rc);
		}
#else
		fprintf(stderr, "%s: error: HASP support not present (and you need it for this).\n",
			modname);
		return(EXCERT_INTERNAL_ERROR);
#endif
	} else {
		if ((seckey = excert_read_xml_seckey(seckey_filename, (u8 *)phrase, &rc)) == NULL) {
			fprintf(stderr, "%s: error: failed to get secret key from file \"%s\".\n", modname, seckey_filename);
			return(rc);
		}
	}
	if (!ons_sign_file(ons, seckey, num)) {
		fprintf(stderr, "%s: error: failed to sign file \"%s\".\n", modname, ons);
		return(EXCERT_BAD_FILE);
	}
	free(seckey);
	return(EXCERT_OK);
}

/* Routine:	excert_verify_ons
 * Purpose:	Verify the integrity of an OpenNavigationSurface file using certificates
 * Inputs:	*cert	Signatory's public key certificate
 *			*ons	File to verify
 *			num		Sequence number associated with the signature
 * Outputs:	True on success, otherwise False
 * Comment:	This is a driver routine designed to make this a one-stop call (easier
 *			to wrap for other drivers).
 */

Bool excert_verify_ons(char *cert, char *ons, u32 num)
{
	u8			*pubkey;
	Bool		sig_ok;

	if ((pubkey = excert_get_public_key(cert)) == NULL) {
		fprintf(stderr, "%s: error: can't get public key from \"%s\".\n",
			modname, cert);
		return(False);
	}
	sig_ok = ons_verify_file(ons, pubkey, num);
	free(pubkey);
	return(sig_ok);
}

Bool excert_verify_xml_ons(xcrtCertificate *sa_cert, char *ons, u32 num)
{
	return(ons_verify_file(ons, sa_cert->key.key, num));
}

/* Routine:	excert_print_user_xml
 * Purpose:	Output all information for the user portion of the cert. in XML
 * Inputs:	*op			File on which to write
 *			*user		xcrtUserInfo for the user (see comment)
 *			base_tab	Number of tabs to parent node of this data
 * Output:	-
 * Comment:	This outputs any user information, and hence can be used for either the
 *			real user of the certificate or the signature authority that signed it.
 */

static void excert_print_user_xml(FILE *op, xcrtUserInfo *user, u32 base_tab)
{
	if (user->name == NULL && user->organization == NULL && user->idnum == 0 && user->keysource == NULL) return;

	excert_write_tabs(op, base_tab+1); fprintf(op, "<user>\n");
	if (user->name != NULL) {
		excert_write_tabs(op, base_tab+2); fprintf(op, "<name>\n");
		excert_write_tabs(op, base_tab+3); fprintf(op, "%s\n", user->name);
		excert_write_tabs(op, base_tab+2); fprintf(op, "</name>\n");
	}
	if (user->organization != NULL) {
		excert_write_tabs(op, base_tab+2); fprintf(op, "<organization>\n");
		excert_write_tabs(op, base_tab+3); fprintf(op, "%s\n", user->organization);
		excert_write_tabs(op, base_tab+2); fprintf(op, "</organization>\n");
	}
	if (user->idnum != 0) {
		excert_write_tabs(op, base_tab+2); fprintf(op, "<idnum>\n");
		excert_write_tabs(op, base_tab+3); fprintf(op, "%d\n", user->idnum);
		excert_write_tabs(op, base_tab+2); fprintf(op, "</idnum>\n");
	}
	if (user->keysource != NULL) {
		excert_write_tabs(op, base_tab+2); fprintf(op, "<keysource>\n");
		excert_write_tabs(op, base_tab+3); fprintf(op, "%s\n", user->keysource);
		excert_write_tabs(op, base_tab+2); fprintf(op, "</keysource>\n");
	}
	excert_write_tabs(op, base_tab+1); fprintf(op, "</user>\n");
}

/* Routine:	excert_write_xml_certificate
 * Purpose:	Write a certificate from internal structure
 * Inputs:	*cert	Certificate data to write
 *			*name	Name of the file on which to write
 * Outputs:	Returns True if wrote OK, otherwise False
 * Comment:	This formats the internal structure as output XML, writing in standard format.
 */

Bool excert_write_xml_certificate(xcrtCertificate *cert, char *name)
{
	FILE	*op;
	char	*ascii_key = NULL, *ascii_sig = NULL;

	if (cert->key.key != NULL && (ascii_key = ons_key_to_ascii(cert->key.key)) == NULL) {
		fprintf(stderr, "%s: error: failed to convert key to ASCII for output.\n", modname);
		return(False);
	}
	if (cert->signature.signature != NULL && (ascii_sig = ons_sig_to_ascii(cert->signature.signature)) == NULL) {
		fprintf(stderr, "%s: error: failed to convert signature to ASCII for output.\n", modname);
		free(ascii_key);
		return(False);
	}
	if ((op = fopen(name, "w")) == NULL) {
		fprintf(stderr, "%s: error: failed to open \"%s\" for output.\n", modname, name);
		free(ascii_key); free(ascii_sig);
		return(False);
	}
	fprintf(op, "<?xml version=\"1.0\"?>\n");
	
	fprintf(op, "<entity>\n");
	excert_print_user_xml(op, &cert->user, 0);

	if (cert->key.key != NULL) {
		excert_write_tabs(op, 1); fprintf(op, "<key type=\"%s\"", cert->key.is_public ? "public" : "private");
		if (cert->key.algorithm != NULL)
			fprintf(op, " algorithm=\"%s\"", cert->key.algorithm);
		if (cert->key.version != NULL)
			fprintf(op, " version=\"%s\"", cert->key.version);
		fprintf(op, " encoding=\"hex\">\n");
		excert_write_hex_xml(op, ascii_key, 1);
		excert_write_tabs(op, 1); fprintf(op, "</key>\n");
	}

	excert_write_tabs(op, 1); fprintf(op, "<signature>\n");
	excert_print_user_xml(op, &cert->signature.signer, 1);
	if (cert->signature.algorithm != NULL && cert->signature.version != NULL) {
		excert_write_tabs(op, 2);
		fprintf(op, "<sigstream algorithm=\"%s\" version=\"%s\" encoding=\"hex\">\n",
					cert->signature.algorithm, cert->signature.version);
		if (ascii_sig != NULL) excert_write_hex_xml(op, ascii_sig, 2);
		excert_write_tabs(op, 2); fprintf(op, "</sigstream>\n");
	}
	excert_write_tabs(op, 1); fprintf(op, "</signature>\n");

	fprintf(op, "</entity>\n");

	free(ascii_key); free(ascii_sig);
	fclose(op);
	return(True);
}

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

void excert_release_xml_certificate(xcrtCertificate *cert)
{
	free(cert->user.name);
	free(cert->user.organization);
	free(cert->user.keysource);
	free(cert->signature.signer.name);
	free(cert->signature.signer.organization);
	free(cert->signature.signer.keysource);
	free(cert->signature.algorithm);
	free(cert->signature.version);
	free(cert->signature.signature);
	free(cert->key.algorithm);
	free(cert->key.version);
	free(cert->key.key);
}

/* Routine:	excert_new_xml_certificate
 * Purpose:	Allocate space for a new (empty) XML certificate
 * Inputs:	-
 * Outputs:	Pointer to a certificate structure, or NULL (no memory)
 * Comment: -
 */

xcrtCertificate *excert_new_xml_certificate(void)
{
	xcrtCertificate *rtn;

	if ((rtn = (xcrtCertificate *)calloc(1, sizeof(xcrtCertificate))) == NULL) {
		fprintf(stderr, "%s: error: no memory for blank xcrtCertificate.\n", modname);
		return(NULL);
	}
	return(rtn);
}

/* Routine:	excert_clean_string
 * Purpose:	Remove any whitespace from the start or end of a line to make it into an internal string
 * Inputs:	*string	String to convert
 * Outputs:	- (in-place conversion)
 * Comment:	This tops-n-tails the string to avoid any leading tabs/whitespace and any trailing cruft
 */

static void excert_clean_string(char *string)
{
	char	*start, *end;

	start = string;
	while (*start != '\0' && (*start == ' ' || *start == '\n' || *start == '\t' || *start == '\r')) ++start;
	end = start + strlen(start) - 1;
	while (end > start && (*end == ' ' || *end == '\n' || *end == '\t' || *end == '\r')) --end;
	*++end = '\0';
	memmove(string, start, strlen(start)+1);
}

/* Routine:	excert_parse_user
 * Purpose:	Extract information on a user from the XML
 * Inputs:	node	Pointer to the child node list of the <user> node
 *			*user	xcrtUserInfo structure to write the data into.
 * Outputs:	Returns True if all data is set, otherwise False
 * Comment:	This does some basic checking of the inputs to ensure that all of the items
 *			necessary are in fact present.
 */

static Bool excert_parse_user(const xmlNode &node, xcrtUserInfo *user)
{
    user->name = (char *)getContentsAsString(node, "name");
    user->organization = (char *)getContentsAsString(node, "organization");
    user->idnum = getContentsAsInt(node, "idnum");
    user->keysource = (char *)getContentsAsString(node, "keysource");

	if (user->name == NULL || user->organization == NULL || user->idnum == 0 || user->keysource == NULL) {
		fprintf(stderr, "%s: error: failed to get all user information from user tag.\n", modname);
		free(user->name); free(user->organization); free(user->keysource);
		return(False);
	}
	excert_clean_string(user->name);
	excert_clean_string(user->organization);
	excert_clean_string(user->keysource);
	return(True);
}

/* Routine:	excert_clean_hex_string
 * Purpose:	Clean up a multi-line hex string ready for conversion
 * Inputs:	*string	String to process
 * Outputs:	- (in place conversion)
 * Comment:	This is the simplest possible implementation, which means that it moves a great
 *			deal more memory than it should really have to.  However, it is simple...
 */

static void excert_clean_hex_string(char *string)
{
	char	*c;

	c = string;
	while (*c != '\0') {
		if (*c == ' ' || *c == '\n' || *c == '\t' || *c == '\r') {
			memmove(c, c+1, strlen(c));
		} else ++c;
	}
}

/* Routine:	excert_parse_signature
 * Purpose:	Extract all signature information from the XML
 * Inputs:	node	Reference to the XML child node of the <signature> node
 *			*sig	xcrtSigInfo structure to write the output into
 * Output:	True on success, else False.  Failure includes not having all of the <user>
 *			information for the signer, or not having a signature.
 * Comment:	This assumes that the user information for the signer is encoded like any other
 *			<user> information (i.e., is just transposed from the <user> section of the
 *			CSA's certificate).
 */

static Bool excert_parse_signature(const xmlNode &node, xcrtSigInfo *sig)
{
    // Check encoding.  We only want hex encoding.
    char *encoding = (char *)getPropertyAsString(node, "sigstream", "encoding");
    if (encoding == NULL)
        return False;
    if (stricmp(encoding, "hex") != 0)
    {
        free(encoding);
        return False;
    }

    // Check algorithm.  We only want openns.
    char *algorithm = (char *)getPropertyAsString(node, "sigstream", "algorithm");
    if (encoding == NULL)
    {
        free(encoding);
        return False;
    }
    if (stricmp(algorithm, "openns") != 0)
    {
        free(encoding);
        free(algorithm);
        return False;
    }

    // Check version.  We only want 1.0.
    char *version = (char *)getPropertyAsString(node, "sigstream", "version");
    if (version == NULL)
    {
        free(encoding);
        free(algorithm);
        return False;
    }
    if (stricmp(version, "1.0") != 0)
    {
        free(encoding);
        free(algorithm);
        free(version);
        return False;
    }

    // Get the actual signature.
    char *signature = (char *)getContentsAsString(node, "sigstream");
    if (signature == NULL)
    {
        free(encoding);
        free(algorithm);
        free(version);
        return False;
    }

    // Get the signer.
    const xmlNode *pUserNode = ::findNode(node, "user");
    if (pUserNode == NULL)
    {
        free(signature);
        free(encoding);
        free(algorithm);
        free(version);        
        return False;
    }

    // ok we're good to go... set up the return struct.
    sig->algorithm = algorithm;
    sig->version = version;
    excert_parse_user(*pUserNode, &sig->signer);

    excert_clean_hex_string(signature);
#ifdef __DEBUG__
    printf("%s: debug: signature = \"%s\"\n", modname, signature);
#endif
    OnsCryptErr	rc;
    if ((sig->signature = ons_ascii_to_sig(signature, &rc)) == NULL) {
        fprintf(stderr, "%s: error: failed to convert signature string to internal format (rc = %d).\n",
            modname, (u32)rc);

        free(sig->algorithm);
        free(sig->version);
        free(signature);
        free(encoding);
        return(False);
    }
    free(signature);
    free(encoding);

    return(True);
}

/* Routine:	excert_parse_key
 * Purpose:	Extract the key information from the XML
 * Inputs:	node	Pointer to the XML node of the <key>
 *			*key	xcrtKeyInfo structure to write output into
 * Outputs:	True on success, else False.
 * Comment:	The key is defined primarily by the qualifiers in the <key> tag, which
 *			indicate the appropriate algorithm and version.  If these are not correct,
 *			an error is flagged.  Currently, 'correct' means "openns" and "1.0"
 *			respectively.
 */

static Bool excert_parse_key(const xmlNode &node, xcrtKeyInfo *key)
{
	OnsCryptErr	rc;
    std::string type = ::getProperty(node, "type");

	if (type == "public")
    {
#ifdef __DEBUG__
		printf("%s: debug: child:\t(and it's a public key).\n", modname);
#endif
		key->is_public = True;
	}
    else if (type == "private")
    {
#ifdef __DEBUG__
		printf("%s: debug: child:\t(and it's a secret key).\n", modname);
#endif
		key->is_public = False;
	} 
    else 
    {
#ifdef __DEBUG__
		printf("%s: debug: child:\t(and its type is unknown = \"%s\").\n", modname, type);
#endif
		fprintf(stderr, "%s: error: type of key in certificate is not 'public'"
			" or 'private'.\n", modname);
		return(False);
	}

    std::string algorithm = ::getProperty(node, "algorithm");
    std::string version = ::getProperty(node, "version");
    std::string encoding = ::getProperty(node, "encoding");

    if (algorithm.empty() || version.empty() || encoding.empty())
    {
		fprintf(stderr, "%s: error: key does not have algorithm qualifiers.\n", modname);
		return(False);
	}

    key->algorithm = strdup(algorithm.c_str());
    key->version = strdup(version.c_str());

	if (strcmp(key->algorithm, "openns") != 0 || strcmp(key->version, "1.0") != 0 ||
		(encoding != "hex" && encoding != "crypthex"))
    {
		fprintf(stderr, "%s: error: key algorithm qualifiers are invalid.\n", modname);
#ifdef __DEBUG__
		fprintf(stderr, "%s: debug: algorithm = \"%s\"\n", modname, key->algorithm);
		fprintf(stderr, "%s: debug: version = \"%s\"\n", modname, key->version);
		fprintf(stderr, "%s: debug: encoding = \"%s\"\n", modname, enc);
#endif
		return(False);
	}

	if (encoding == "crypthex") 
        key->is_encrypted = True; 
    else 
        key->is_encrypted = False;

	/* Read the key source and convert */
    std::string tmp = ::getContents(node);
    if (tmp.empty())
        return False;

    char *keystring = strdup(tmp.c_str());
    excert_clean_hex_string(keystring);

#ifdef __DEBUG__
	printf("%s: debug: key = \"%s\"\n", modname, keystring);
#endif
	if (!key->is_encrypted) 
    {
		if ((key->key = ons_ascii_to_key(keystring, &rc)) == NULL) {
			fprintf(stderr, "%s: error: failed to convert key to internal format (rc = %d).\n",
				modname, (u32)rc);
			xmlFree(keystring);
			return(False);
		}
	} 
    else 
    {
		/* Just copy the string --- it's encrypted, and we don't know how to fix it */
		key->key = (u8*)strdup(keystring);
	}
	xmlFree(keystring);
#ifdef __DEBUG__
	if (key->is_encrypted) {
		printf("%s: debug: encrypted key = \"%s\"\n", modname, key->key);
	} else {
		printf("%s: debug: converted key = \"%s\"\n", modname, (keystring = ons_key_to_ascii(key->key)));
		free(keystring);
	}
#endif
	return(True);
}

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

xcrtCertificate *excert_read_xml_certificate(char *name, ExcertErr *rc)
{
	xcrtCertificate	*cert;

	if ((cert = excert_new_xml_certificate()) == NULL) 
    {
		fprintf(stderr, "%s: error: failed to get memory for the internal certificate.\n", modname);
		*rc = EXCERT_BAD_FILE;
		return(NULL);
	}

    // parse the file.
    xmlDoc *pDocument = xmlParseFile((const char *)name); 
    if (pDocument == NULL)
    {
        fprintf(stderr, "%s: error: failed to parse \"%s\" for valid XML tree.\n", modname, name);
		*rc = EXCERT_BAD_FILE;
        return(NULL);
    }

    xmlNode *pRoot = pDocument->children;
    if (pRoot == NULL)
    {
        fprintf(stderr, "%s: error: failed to parse \"%s\" for valid XML tree.\n", modname, name);
		*rc = EXCERT_BAD_FILE;
        return(NULL);
    }

    fprintf(stderr, "%s: debug: parsed \"%s\" for input XML tree.\n", modname, name);

    //Look for the 'user' node.
    const xmlNode *userNode = ::findNode(*pRoot, "/entity/user");
    if (!userNode)
    {
        fprintf(stderr, "%s: error: failed to parse user name from \"%s\".\n", modname, name);
	    excert_release_xml_certificate(cert);
		*rc = EXCERT_BAD_FILE;
        return NULL;
    }

    if (!excert_parse_user(*userNode, &cert->user))
    {
        fprintf(stderr, "%s: error: failed to parse user name from \"%s\".\n", modname, name);
	    *rc = EXCERT_BAD_FILE;
	    excert_release_xml_certificate(cert);
	    return(NULL);
    }

#ifdef __DEBUG__
    printf("%s: debug: child:\tuser=\"%s\"\n", modname, cert->user.name);
    printf("%s: debug: child:\torganization=\"%s\"\n", modname, cert->user.organization);
    printf("%s: debug: child:\tid=\"%d\"\n", modname, cert->user.idnum);
    printf("%s: debug: child:\tkeysource=\"%s\"\n", modname, cert->user.keysource);
#endif


    //Look for the 'key' node.
    const xmlNode *keyNode = ::findNode(*pRoot, "/entity/key");
    if (!keyNode)
    {
        fprintf(stderr, "%s: error: failed to parse key information from \"%s\".\n", modname, name);
	    excert_release_xml_certificate(cert);
		*rc = EXCERT_BAD_FILE;
        return NULL;
    }

    if (!excert_parse_key(*keyNode, &cert->key))
    {
        fprintf(stderr, "%s: error: failed to parse key information from \"%s\".\n", modname, name);
	    *rc = EXCERT_BAD_FILE;
	    excert_release_xml_certificate(cert);
	    return(NULL);
    }

    //Look for the 'signature' node.
    const xmlNode *sigNode = ::findNode(*pRoot, "/entity/signature");
    if (!sigNode)
    {
		fprintf(stderr, "%s: error: failed to parse signature information from \"%s\".\n", modname, name);
	    excert_release_xml_certificate(cert);
		*rc = EXCERT_BAD_FILE;
        return NULL;
    }

    if (!excert_parse_signature(*sigNode, &cert->signature))
    {
		fprintf(stderr, "%s: error: failed to parse signature information from \"%s\".\n", modname, name);
		*rc = EXCERT_BAD_FILE;
		excert_release_xml_certificate(cert);
		return(NULL);
	}

	*rc = EXCERT_OK;
	return(cert);
}
