/*
 * $Id: signfile.c,v 1.3 2006/02/08 17:43:39 openns Exp $
 * $Log: signfile.c,v $
 * Revision 1.3  2006/02/08 17:43:39  openns
 * brc: optional compile for _strdup() to fix Win32 heap problems, and static
 * library linkage to simpify Win32 build.
 *
 * Revision 1.2  2005/10/26 17:38:50  openns
 * Updates after testing following the 2nd ONSWG meeting.  Checked in from
 * Webb McDonald's implementation of 2005-09-07.
 *
 * Revision 1.2  2005/03/15 22:45:57  openns
 * Added facility for sequence number of have a number base qualifier (e.g., 0x... or
 * 0...) instead of being just decimal.
 *
 * Revision 1.1  2004/11/01 22:52:24  openns
 * Driver to sign a BAG using a user's secret key from a USB HASP.
 *
 *
 * File:	signfile.c
 * Purpose:	Sign a file using high-level certificates and HASP keys.
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
	INPUT_FILE = 1,
	PASS_PHRASE,
	SEQ_NUM,
	ARGC_EXPECTED
} Cmd;

void Syntax(void)
{
	char	*rev = get_rcs_rev(modrev);

	printf("signfile V%s [%s] - Sign a file using OpenNavigationSurface algorithms.\n", rev, __DATE__);
	printf("Syntax: signfile [opt] <input><phrase><idnum>\n");
	printf(" Input file to sign ------^       ^      ^\n");
	printf(" Pass-phrase to decode HASP key --'      |\n");
	printf(" Signature sequence ID number -----------'\n");
	printf(" Options:\n");
	printf("  -f <seckey>  Read the secret key from XML file <seckey> and decrypt.\n");

	free(rev);
}

int main(int argc, char **argv)
{
	int		c;
	u32		sig_id;
	char	*dummyptr, *private_file = NULL;

	opterr = 0;
	while ((c = getopt(argc, argv, "hf:")) != EOF) {
		switch(c) {
			case 'f':
				private_file = strdup(optarg);
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
	if (excert_sign_ons(argv[INPUT_FILE], argv[PASS_PHRASE], sig_id, private_file == NULL ? True : False, private_file) != EXCERT_OK) {
		fprintf(stderr, "%s: error: failed to sign input \"%s\".\n", modname, argv[INPUT_FILE]);
		return(1);
	}
	return(0);
}
