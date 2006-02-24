#ifndef __GETOPT__
#define __GETOPT__

extern char *optarg;	// Points to option arguments when required
extern int optind;		// Index into the argv vector reached at end of parse
extern int opterr;		// Flag: 0 => ignore errors, 1 => return '?' for errors and set...
extern char optopt;		// optopt equal to the flag causing the error.

extern int getopt(int argc, char *argv[], char *optstring);

#endif
