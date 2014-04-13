/*
 * 	slpdpth.c - Computes the depth of a given straight-line
 * 	program over GF(2)
 *
 *  Created on: 29/01/2014
 *  Revisited:	13/04/2014
 *  Author: Wiliam Capraro - wiliam.capraro@studenti.unimi.it
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "slplib.h"


#define PROG_NAME "SLPDepth"


int lightweight_compile(FILE*, int*, int);
void usage();


int main(int argc, char **argv) {

	char *fin = NULL;
	char *fout = NULL;
	FILE *pfile = NULL;
	int f, h;
	int verbose = 0;

	/* Keep track of circuit size */
	int circuit_size = 0;


	/* parse command line options (see the getopt tool) */
	while ((f=getopt(argc, argv, "f:o:s:hv")) != -1) {
		switch(f) {
		case 'f':
			fin = optarg;
			break;
		case 'h':
			usage();
			exit(0);
		case 'v':
			verbose = 1;
			break;
		case 'o':
			fout = optarg;
			break;
		default:
			printf("Unknown option '%c'\n", f);
			exit(ERR_PARSE);
		}
	}


	/* (try to) open and read input file */
	if (!fin) {
		usage();
		exit(ERR_IO);
	}
	pfile = fopen(fin, "r");
	if (!pfile) {
		fprintf(stderr, "--%s : could not open file '%s'", PROG_NAME, fin);
		exit(ERR_IO);
	}


	/* parse each equation one by one */
	int max_depth = lightweight_compile(pfile, &circuit_size, verbose);

	if (max_depth < 0) {
		fprintf(stderr, "--%s : Ooops...! Something went wrong :-(\n", PROG_NAME);
		exit (1);
	}

	/* Output to file and/or stdout */
	if (fout) {

		fclose(pfile);
		pfile = fopen(fout, "w");

		if (!pfile)
		{
			fprintf(stderr, "--%s : could not open file '%s'", PROG_NAME, fin);
			pfile = stdout;
		}

		fprintf(pfile, ">> Depth=%d, Size=%d,\t@ %s\n", max_depth, circuit_size, fin);
	}

	if (!fout)
		printf(">> Depth=%d, Size=%d,\t@ %s\n", max_depth, circuit_size, fin);


	/* clean up before exiting */
	fclose(pfile);
	exit (0);

}


/*
 * Parses the input file, generates the necessary
 * clauses and computes their depth on-the-fly. Also
 * takes responsibility for memory management of clauses.
 */
int lightweight_compile(FILE *fin, int *size_buffer, int verbose) {

	char line[LINLEN];
	char copy[LINLEN];
	char *y;
	char *op;
	char *a1;
	char *a2;
	t_clause *cl;
	t_clause *cl1;
	t_clause *cl2;
	t_hashtable *known_symbols;

	/* Tells whether to parse lines or not */
	int parse = 0;
	int err = 0;

	/* Keep track of the depth for each clause */
	short d;

	/* Keep track of max-depth among all clauses */
	int max_depth = -1;

	/* Initialize clause dictionary */
	known_symbols = hashtable(IN_HASHSIZE);
	if (!known_symbols) {
		fprintf(stderr, "-- lightweight_compile() : could not init hashtable\n");
		return (max_depth);
	}

	/* process each line at a time */
	while (fgets(line, LINLEN, fin)) {

		/* Make a stripped copy of the line and count chars */
		int l = lincpy(line, copy, "\n\t\r;");

		if (l < 2)		
			continue;

		if (streq(copy, "begin") && !parse) {
			/* Begin parsing */
			parse = 1;
			continue;
		}

		else if (streq(copy, "end") && parse) {
			/* Return */
			parse = 0;
			break;
		}

		if (parse) {

			/* Here is actual program parsing code */
			y =  strtok(copy, "= ");

			/*
			 * Accounts for the 'output' keyword
			 * which is not always used
			 */
			if (streq(y, "output"))
				y = strtok(NULL, "= ");

			a1 = strtok(NULL, "= ");
			op = strtok(NULL, "= ");
			a2 = strtok(NULL, "= ");

			if (y && streq(y, "err")) {

				// handle error from heuristics
				// set the err flag and break loop

				fprintf(stderr, "-- lightweight_compile() : encountered error 200. Aborting...\n");
				err = 1;
				break;
			}

			else if (y && a1 && op && a2) {

				// lookup a1 and a2 in the hashtable
				// and compute y's depth. Note that
				// if neither a1 nor a2 are known symbols
				// they are assumed to be input variables
				// so their depth is zero

				d = 0;
				cl1 = find_clause(known_symbols, a1);
				cl2 = find_clause(known_symbols, a2);

				if (cl1) 		d = ((short)cl1->value > d) ? (short)cl1->value : d;
				if (cl2)		d = ((short)cl2->value > d) ? (short)cl2->value : d;

				d++;

				cl = clause(NULL, NULL, NULL, y, 0);
				cl->value = d;
				put_clause(known_symbols, cl);

				if (verbose)
					printf("0x%x\t[depth=%d]\t%s = %s %s %s\n", cl, d, y, a1, op, a2);

				max_depth = (max_depth > d) ? max_depth : d;

				(*size_buffer)++;

			}			

			else if (y && a1 && !op && !a2) {

				// same as before, but clause has only
				// one child. In this case y's depth is
				// either a1's depth or zero

				d = 0;
				cl1 = find_clause(known_symbols, a1);

				if (cl1)		d = (short)cl1->value;

				cl = clause(NULL, NULL, NULL, y, 0);
				cl->value = d;
				put_clause(known_symbols, cl);

				if (verbose)
					printf("0x%x\t[depth=%d]\t%s = %s\n", cl, d, y, a1);

				max_depth = (max_depth > d) ? max_depth : d;

			}

			else {

				// handle any other error. Set the
				// flag and break loop

				fprintf(stderr, "-- lightweight_compile() : invalid syntax \'%s\'\n", line);
				err = 1;
				break;

			}

		}

	}

	/* Clean up before returning */
	if (known_symbols) {
		wipe_hashtable(known_symbols, TRUE);
		free(known_symbols);
	}

	return (max_depth);

}



/*
 * Simple usage message
 */
void usage() {
	printf("\n%s %s -- Computes the depth of a straight-line program over GF(2)\n\n", PROG_NAME, PROG_VERSION);
	printf("slpdpth [optlist]\n\n");
	printf("List of options:\n");
	printf("  -f <file>\tuse <file> as input file (mandatory)\n");
	printf("  -o <file>\tprint output to <file>\n");
	printf("  -v \t\tverbose\n");
	printf("  -h\t\tshows this message\n\n");
}



