/*
 * $Id: verifyfile.c,v 1.3 2006/02/08 17:43:56 openns Exp $
 * $Log: verifyfile.c,v $
 * Revision 1.3  2006/02/08 17:43:56  openns
 * brc: optional compile for _strdup() to fix Win32 heap problems, and static
 * library linkage to simpify Win32 build.
 *
 * Revision 1.2  2005/10/26 17:39:05  openns
 * Updates after testing following the 2nd ONSWG meeting.  Checked in from
 * Webb McDonald's implementation of 2005-09-07.
 *
 * Revision 1.2  2005/03/15 22:47:09  openns
 * Added facility for sequence number of have a number base qualifier (e.g., 0x... or
 * 0...) instead of being just decimal, and added support for XML certificates.
 *
 * Revision 1.1  2004/11/01 22:53:16  openns
 * Driver to verify the signature in a BAG given a user's public key.
 *
 *
 * File:	verifyfile.c
 * Purpose:	Verify a file using high-level certificates and HASP keys.
 * Date:	2004-10-24
 *
 * This is free source code, courtesy of the OpenNavigationSurface project.  Visit
 * the website http://www.opennavsurf.org
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "excertlib.h"
#include "getopt.h"

static char *modname = "verifyfile";
static char *modrev = "$Revision: 1.3 $";

static char *get_rcs_rev(const char *rev)
{
	char *p = strchr(rev, ' ');
#ifdef WIN32
	/* For some reason, strdup() causes heap corruption problems, but
	 * explicitly using _strdup() does not.  This may just be about where
	 * these get linked from, but for now, this solves the problem, even
	 * if it is egregiously ugly.
	 */
	if (p != NULL) {
		char *b = _strdup(++p);
		if ((p = b) != NULL && (b = strchr(p, ' ')) != NULL) *b = '\0';
	} else p = _strdup("1.0");
#else
	if (p != NULL) {
		char *b = strdup(++p);
		if ((p = b) != NULL && (b = strchr(p, ' ')) != NULL) *b = '\0';
	} else p = strdup("1.0");
#endif
	return(p);
}

typedef enum {
	INPUT_FILE = 1,
	AUTH_CERT,
	SEQ_NUM,
	ARGC_EXPECTED
} Cmd;

void Syntax(void)
{
	char	*rev = get_rcs_rev(modrev);

	printf("verifyfile V%s [%s] - Verify a file using OpenNavigationSurface algorithms.\n", rev, __DATE__);
	printf("Syntax: verifyfile [opt] <input><authcert><idnum>\n");
	printf(" Input file to sign --------^       ^      ^\n");
	printf(" Signing authority's certificate ---'      |\n");
	printf(" Signature sequence ID number -------------'\n");
	printf(" Options:\n");
	printf("  -x  Use XML certificates rather than plain text\n");

	free(rev);
}

int main(int argc, char **argv)
{
	u32		sig_id;
	char	*dummyptr;
	CertTag	auth[2] = { { "name", NULL }, { NULL, NULL } };
	Bool	use_xml = False, verified;
	xcrtCertificate	*sa_cert;
	ExcertErr		rc;
	int		c;
	u32		retval;

	opterr = 0;
	while ((c = getopt(argc, argv, "xh")) != EOF) {
		switch(c) {
			case 'x':
				use_xml = True;
				break;
			case '?':
				printf("%s: unknown option '%c'\n", modname, optopt);
			default:
				Syntax();
				return(1);
		}
	}
	argc -= optind-1; argv += optind-1;
	if (argc != ARGC_EXPECTED) {
		Syntax();
		return(1);
	}
	sig_id = strtoul(argv[SEQ_NUM], &dummyptr, 0);
	if (use_xml) {
		if ((sa_cert = excert_read_xml_certificate(argv[AUTH_CERT], &rc)) == NULL) {
			fprintf(stderr, "%s: error: failed to read XML certificate from \"%s\".\n", modname, argv[AUTH_CERT]);
			return(1);
		}
		verified = excert_verify_xml_ons(sa_cert, argv[INPUT_FILE], sig_id);
	} else {
		if (excert_parse_certificate(argv[AUTH_CERT], "user", auth) != 1) {
			fprintf(stderr, "%s: error: failed to parse authority identity from \"%s\".\n", modname, argv[AUTH_CERT]);
			return(1);
		}
		verified = excert_verify_ons(argv[AUTH_CERT], argv[INPUT_FILE], sig_id);
	}
	if (verified) {
		printf("%s: verified file \"%s\" as signed by authority \"%s\" and unmodified.\n",
			modname, argv[INPUT_FILE], use_xml ? sa_cert->user.name : auth[0].value);
		retval = 0;
	} else {
		fprintf(stderr, "%s: error: failed to verify file \"%s\" as signed by authority \"%s\" with certificate \"%s\".\n",
			modname, argv[INPUT_FILE], use_xml ? sa_cert->user.name : auth[0].value, argv[AUTH_CERT]);
		retval = 1;
	}
	return(retval);
}
