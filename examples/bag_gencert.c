/*
 * $Id: gencert.c,v 1.3 2006/02/08 17:42:50 openns Exp $
 * $Revision: 1.3 $
 *
 * File:	gencert.c
 * Purpose:	Generate a certificate and private key to HASP for OpenNavigationSurface signatures
 * Date:	2004-10-24
 *
 * This is free source code, courtesy of the OpenNavigationSurface project.  Visit
 * the website http://www.opennavsurf.org
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "getopt.h"

#include "excertlib.h"

static char *modname = "gencert";
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
	PROTO_CERT = 1,
	OUTPUT_CERT,
	SYMCRY_PASSPHRASE,
	ARGC_EXPECTED
} Cmd;

static void Syntax(void)
{
	char	*p = get_rcs_rev(modrev);

	printf("gencert V%s [%s, %s] - Generate OpenNavigationSurface certificate & HASP key from prototype.\n",
			p, __DATE__, __TIME__);
	printf("Syntax: gencert [opt] <prototype><outcert><passphrase>\n");
	printf(" Prototype certificate ----^         ^          ^\n");
	printf(" Output certificate -----------------'          |\n");
	printf(" String to use for secret key encryption -------'\n");
	printf(" Options:\n");
	printf("  -x         Use XML certificates rather than plain.\n");
	printf("  -f <file>  Write private key certificate to <file> instead of HASP.\n");
	printf(" Notes:\n");
	printf("  1.  This code assumes that a suitable Memo-HASP M4 is attached to\n");
	printf("      the system, ready to receive the encrypted secret key for this\n");
	printf("      certificate.\n");

	free(p);
}

int main(int argc, char **argv)
{
	int			c;
	Bool		use_xml = False;
	ExcertErr	success;
	xcrtCertificate	*user_cert = NULL;
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
		if ((user_cert = excert_read_xml_certificate(argv[PROTO_CERT], &success)) == NULL) {
			fprintf(stderr, "%s: error: failed to read prototype certificate from \"%s\".\n", modname, argv[PROTO_CERT]);
			return(1);
		}
		success = excert_build_xml_certificate(user_cert, argv[SYMCRY_PASSPHRASE], private_file == NULL ? True : False, private_file);
	} else {
		success = excert_build_certificate(argv[PROTO_CERT], argv[OUTPUT_CERT], argv[SYMCRY_PASSPHRASE]);
	}
	if (success != EXCERT_OK) {
		fprintf(stderr, "%s: error: failed to build certificate to \"%s\".\n",
			modname, argv[OUTPUT_CERT]);
		return(1);
	} else if (use_xml) {
		if (!excert_write_xml_certificate(user_cert, argv[OUTPUT_CERT])) {
			fprintf(stderr, "%s: error: failed to write output certificate to \"%s\".\n", modname, argv[OUTPUT_CERT]);
			return(1);
		}
	}
	return(0);
}
