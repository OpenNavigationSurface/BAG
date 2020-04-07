#ifndef GETOPT_H
#define GETOPT_H

extern "C" char *optarg;	// Points to option arguments when required
extern "C" int optind;		// Index into the argv vector reached at end of parse
extern "C" int opterr;		// Flag: 0 => ignore errors, 1 => return '?' for errors and set...
extern "C" char optopt;		// optopt equal to the flag causing the error.

extern "C" int getopt(int argc, char *argv[], char *optstring);

#endif  // GETOPT_H

