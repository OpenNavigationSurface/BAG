/*
 * $Id: signcert.c,v 1.3 2006/02/08 17:43:32 openns Exp $
 * $Log: signcert.c,v $
 * Revision 1.3  2006/02/08 17:43:32  openns
 * brc: optional compile for _strdup() to fix Win32 heap problems, and static
 * library linkage to simpify Win32 build.
 *
 * Revision 1.2  2005/10/26 17:38:41  openns
 * Updates after testing following the 2nd ONSWG meeting.  Checked in from
 * Webb McDonald's implementation of 2005-09-07.
 *
 * Revision 1.2  2005/03/15 22:47:33  openns
 * Added facility for XML certificates rather than structured ASCII.
 *
 * Revision 1.1  2004/11/01 22:51:59  openns
 * Driver to use a certificate authority's secret key to sign a user certificate.
 *
 *
 * File:	signcert.c
 * Purpose:	Sign an OpenNavigationSurface certificate to confirm authenticity.
 * Date:	2004-10-24
 *
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

static char *modname = "signcert";
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
	INPUT_CERT = 1,
	AUTH_CERT,
	SYMCRY_PASSPHRASE,
	ARGC_EXPECTED
} Cmd;

static void Syntax(void)
{
	char	*p = get_rcs_rev(modrev);
	printf("gencert V%s [%s] - Sign OpenNavigationSurface certificate with HASP key.\n",
			p, __DATE__);
	printf("Syntax: gencert [opt] <cert><authcert><passphrase>\n");
	printf(" User's certificate ----^        ^         ^\n");
	printf(" Authenticator's certificate ----'         |\n");
	printf(" Authenticator's pass-phrasse -------------'\n");
	printf(" Options:\n");
	printf("  -x          Use XML certificates, rather than plain\n");
	printf("  -f <seckey> Read the secret key from <seckey> rather than HASP, and decrypt.\n");
	printf(" Notes:\n");
	printf("  1.  This code assumes that a suitable Memo-HASP M4 is attached to\n");
	printf("      the system, ready to provide the encrypted secret key for this\n");
	printf("      authenticator's certificate.\n");

	free(p);
}

int main(int argc, char **argv)
{
	int			c;
	Bool		use_xml = False;
	ExcertErr	rc, success;
	xcrtCertificate	*user_cert = NULL, *csa_cert = NULL;
	char		*private_file = NULL;

	opterr = 0;
	while ((c = getopt(argc, argv, "xhf:")) != EOF) {
		switch(c) {
			case 'f':
				private_file = strdup(optarg);
				break;
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
	if (use_xml) {
		if ((user_cert = excert_read_xml_certificate(argv[INPUT_CERT], &rc)) == NULL ||
			(csa_cert = excert_read_xml_certificate(argv[AUTH_CERT], &rc)) == NULL) {
			fprintf(stderr, "%s: error: failed to read %s certificate.\n", modname, csa_cert == NULL ? "user" : "CSA");
			return(1);
		}
			success = excert_sign_xml_certificate(user_cert, csa_cert, argv[SYMCRY_PASSPHRASE], private_file == NULL ? True : False, private_file);
	} else
		success = excert_sign_certificate(argv[INPUT_CERT], argv[AUTH_CERT], argv[SYMCRY_PASSPHRASE]);
	if (rc != EXCERT_OK) {
		fprintf(stderr, "%s: error: failed to sign certificate \"%s\" with authority \"%s\".\n",
			modname, argv[INPUT_CERT], argv[AUTH_CERT]);
		return(1);
	} else if (use_xml) {
		/* Need to save with XML, 'cos it only adds the signature in place */
		if (!excert_write_xml_certificate(user_cert, argv[INPUT_CERT])) {
			fprintf(stderr, "%s: error: failed to write signed certificate back to \"%s\".\n", modname, argv[INPUT_CERT]);
			return(1);
		}
	}
	return(0);
}
