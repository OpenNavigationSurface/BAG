#include <stdio.h>
#include <string.h>

char	*optarg = NULL;
int		optind = 1;
int		opterr = 1;
char	optopt;

#define _next_char(string) (char)(*(string+1))

int getopt(int argc, char *argv[], char *optstring)
{
	static char *pIndexPosition = NULL;	// Place inside current argv string
	char *pArgString = NULL;	// Where to start from next
	char *pOptString;			// The string in out program

	if (pIndexPosition != NULL) {
		// We last left off inside an argv string
		if (*(++pIndexPosition)) {
			// There is more to come in the most recent argv
			pArgString = pIndexPosition;
		}
	}
	if (pArgString == NULL) {
		// Didn't leave off in the middle of an argv string
		if (optind >= argc) {
			// More command line arguments that the argument count
			pIndexPosition = NULL; // Not in the middle of anything
			return EOF;
		}
		// If the next argv[] is not an option, there can be no more options
		pArgString = argv[optind++];	// Set this to the next argument ptr
		if (('/' != *pArgString) && ('-' != *pArgString)) {
			// Doesn't start with a slash or a dash - not an option
			--optind;				// Point to current arg once we're done
			optarg = NULL;			// No argument follows the option
			pIndexPosition = NULL;	// Not in the middle of anything
			return EOF;				// Used up all of the command line flags
		}
		// Check for special 'end-of-flags' markers
		if ((strcmp(pArgString, "-") == 0) || (strcmp(pArgString, "--") == 0)) {
			optarg = NULL;	// No argument follows the option
			pIndexPosition = NULL;	// Not in the middle of anything
			return EOF;				// Encountered the special flag
		}
		pArgString++;
	}
	if (':' == *pArgString) {	// is it a colon?
		// Rare case: if opterr is non-zero, return a question mark, otherwise ':'
		optopt = ':';
		return (opterr ? (int)'?' : (int)':');
	} else if ((pOptString = strchr(optstring, *pArgString)) == 0) {
		// The letter on the command line wasn't any good
		optarg = NULL;
		pIndexPosition = NULL;
		optopt = *pArgString;
		return (opterr ? (int)'?' : (int)*pArgString);
	} else {
		// The letter on the command line matches one we expect to see
		if (':' == _next_char(pOptString)) {	// Is the next letter a colon?
			// It is a colon.  Look for an argument string
			if ('\0' != _next_char(pArgString)) {	// Argument in this argv?
				optarg = &pArgString[1];	// Yes, it is
			} else {
				// The argument string must be in the next argv.  But, what if there
				// is none (bad input from the user)?  In that case, return the letter,
				// and optarg as NULL.
				if (optind < argc)
					optarg = argv[optind++];
				else {
					optarg = NULL;
					optopt = *pArgString;
					return (opterr ? (int)'?' : (int)*pArgString);
				}
			}
			pIndexPosition = NULL; // Not in the middle of anything
		} else {
			// It's not a colon, so just return the letter
			optarg = NULL;
			pIndexPosition = pArgString;	// Point to the letter we're on
		}
		return (int)*pArgString;
	}
}
