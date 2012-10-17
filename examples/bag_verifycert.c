/*
 * $Id: verifycert.c,v 1.3 2006/02/08 17:43:48 openns Exp $
 * $Log: verifycert.c,v $
 * Revision 1.3  2006/02/08 17:43:48  openns
 * brc: optional compile for _strdup() to fix Win32 heap problems, and static
 * library linkage to simpify Win32 build.
 *
 * Revision 1.2  2005/10/26 17:38:59  openns
 * Updates after testing following the 2nd ONSWG meeting.  Checked in from
 * Webb McDonald's implementation of 2005-09-07.
 *
 * Revision 1.2  2005/03/15 22:47:33  openns
 * Added facility for XML certificates rather than structured ASCII.
 *
 * Revision 1.1  2004/11/01 22:52:52  openns
 * Driver to verify a user's certificate given a certificate authority's public key
 *
 *
 * File:	verifycert.c
 * Purpose:	Verify that a certificate is valid given the signing authority's public certificate
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

#define __DEBUG__

static char *modname = "verifycert";
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
	USER_CERT = 1,
	AUTH_CERT,
	ARGC_EXPECTED
} Cmd;

static void Syntax(void)
{
	char	*p = get_rcs_rev(modrev);
	printf("gencert V%s [%s] - Verify OpenNavigationSurface certificate.\n",
			p, __DATE__);
	printf("Syntax: gencert [opt] <usercert><authcert>\n");
	printf(" User certificate ----^        ^\n");
	printf(" Authority certificate --------'\n");
	printf(" Options:\n");
	printf("  -x   Use XML certificates, rather than simple text.\n");

	free(p);
}

int main(int argc, char **argv)
{
	CertTag			auth[2] = { { "name", NULL }, { NULL, NULL } };
	int				c;
	Bool			use_xml = False, verified;
	xcrtCertificate	*auth_cert, *user_cert;
	ExcertErr		rc;
	int				retval;

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
	if (use_xml) {
		if ((auth_cert = excert_read_xml_certificate(argv[AUTH_CERT], &rc)) == NULL) {
			fprintf(stderr, "%s: error: failed to parse XML certificate from \"%s\".\n", modname, argv[AUTH_CERT]);
			return(1);
		}
		if ((user_cert = excert_read_xml_certificate(argv[USER_CERT], &rc)) == NULL) {
			fprintf(stderr, "%s: error: failed to parse XML certificate from \"%s\".\n", modname, argv[USER_CERT]);
			return(1);
		}
#ifdef __DEBUG__
		excert_write_xml_certificate(auth_cert, "debug_certificate.xml");
#endif
	} else {
		if (excert_parse_certificate(argv[AUTH_CERT], "user", auth) != 1) {
			fprintf(stderr, "%s: error: failed to find signing authority's details from certificate.\n",
				modname);
			return(1);
		}
	}
	if (use_xml) {
		verified = excert_verify_xml_certificate(user_cert, argv[USER_CERT], auth_cert);
	} else {
		verified = excert_verify_certificate(argv[USER_CERT], argv[AUTH_CERT]);
	}
	if (verified) {
		printf("%s: certificate \"%s\" was signed by authority \"%s\", and has not changed.\n",
			modname, argv[USER_CERT], use_xml ? auth_cert->user.name : auth[0].value);
		retval = 0;
	} else {
		printf("%s: certificate \"%s\" was not signed by authority \"%s\", or has been modified.\n",
			modname, argv[USER_CERT], use_xml ? auth_cert->user.name : auth[0].value);
		retval = 1;
	}
	if (use_xml) {
		excert_release_xml_certificate(auth_cert);
		excert_release_xml_certificate(user_cert);
	}
	return(retval);
}
