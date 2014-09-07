/*
 * slpchk.c
 *
 *  Created on: 06/giu/2013
 *  Author: Wiliam Capraro - wiliam.capraro@studenti.unimi.it
 */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "slplib.h"



#define PROG_NAME "SLPChk"
#define MAX_N_INPUTS 64  // max allowable number of input variables



void usage();
void assign(t_clause**, int, t_clause**, int, unsigned long int, bool);
void hijack(t_clause**, t_clause**, int);
void cleanup(FILE*, t_clause**, t_clause**, t_clause**, int, int);




void usage() {
	printf("\n%s %s - Checks equivalency of any two given straight-line programs\n\n", PROG_NAME, PROG_VERSION);
	printf("slpchk <file1> <file2> [optlist]\n\n");
	printf("List of options:\n");
	printf("  -p\tevaluate clauses positionally\n\n");
	exit(EXIT_SUCCESS);
}



int main(int argc, char **argv) {

	/* used for parsing */
	FILE *fa, *fb;
	int parama[3] = {0, 0, 0};
	int paramb[3] = {0, 0, 0};
	t_clause **Xa, **Xb, **Ya, **Yb, **Ta, **Tb;
	bool fail = FALSE;
	bool positional = FALSE;
	int i, f;


	if (argc < 3 || argc > 4) 	usage();

	else

	{

		fa = fopen(argv[1], "r");
		fb = fopen(argv[2], "r");
		if (!fa || !fb)
		{
			fprintf(stderr, "--%s : input file error\n", PROG_NAME);
			fa ? fclose(fa) : NULL ;
			fb ? fclose(fb) : NULL ;
			exit(ERR_IO);
		}

		/* parse options */
		while ((f=getopt(argc, argv, "ph")) != -1) {
			switch(f) {
			case 'h':
				usage();
				exit(EXIT_SUCCESS);
			case 'p':
				positional = TRUE;
				break;
			default:
				printf("Unknown option '%c'\n", f);
				exit(ERR_PARSE);
			}
		}

		/* (try to) compile each input program individually */
		compile(fa, &Xa, &Ta, &Ya, parama, NULL);
		compile(fb, &Xb, &Tb, &Yb, paramb, NULL);

		/* do a simple preliminary check on the parameters */
		if 	(parama[0] != paramb[0] || parama[1] != paramb[1])
		{
			printf("\n--%s : input files do not match!\n", PROG_NAME);
			cleanup(fa, Xa, Ya, Ta, parama[0], parama[2]);
			cleanup(fb, Xb, Yb, Tb, paramb[0], paramb[2]);
			exit(EXIT_SUCCESS);
		}

		if (!positional) {
			hijack(Xa, Xb, parama[0]);
			hijack(Ya, Yb, parama[1]);
		} else {
			printf("++ %s : using positional clause evaluation\n", PROG_NAME);
		}


		/* now we test all possible assignments on the input variables */
		unsigned long int l=0;
		printf("\n\n>> Beginning test <<\n\n");
		while (l < pow(2, parama[0]) && !fail) {

			printf("@ 0x%x\n", l);
			assign(Xa, parama[0], Ya, parama[1], l, positional);
			assign(Xb, paramb[0], Yb, paramb[1], l, positional);
			l++;

			/* Reset clause values */
			for (i=0; i<parama[2]; i++) 	reset_clause(Ta[i]);
			for (i=0; i<paramb[2]; i++) 	reset_clause(Tb[i]);
			for (i=0; i<parama[1]; i++) 	reset_clause(Ya[i]);
			for (i=0; i<paramb[1]; i++)		reset_clause(Yb[i]);

			/* evaluate all clauses */
			for (i=0; i<parama[2]; i++) {
				evaluate(Ta[i]);
			}

			for (i=0; i<paramb[2]; i++) {
				evaluate(Tb[i]);
			}

			for (i=0; i<parama[1]; i++) {
				evaluate(Ya[i]);
				evaluate(Yb[i]);
			}

			/* check */
			for (i=0; i<parama[1]; i++) {
				if (get_value(Ya[i]) != get_value(Yb[i])) {
					printf("  (%s=%d, %s=%d)",
							get_clause_name(Ya[i]),
							get_value(Ya[i]),
							get_clause_name(Yb[i]),
							get_value(Yb[i]));
					printf(" ### OUTPUT MISMATCH\n", i);
					fail = TRUE;
				}
			}

		}

		/* test passed? */
		printf("\n>> Test finished. Result: ");
		fail ?
				printf("*** FAIL *** <<\n\n") :
				printf("pass <<\n\n");


		/* remember to close files and dealloc memory */
		cleanup(fa, Xa, Ya, Ta, parama[0], parama[2]);
		cleanup(fb, Xb, Yb, Tb, paramb[0], paramb[2]);
	}

	exit(EXIT_SUCCESS);
	return 0;
}



void assign(t_clause **X, int nx, t_clause **Y, int ny, unsigned long int value, bool positional) {
	int i, j, v;
	bool b;
	if ( nx <= 0 || nx > MAX_N_INPUTS )
		return;
	else
	{
		for (i=0; i<nx; i++) {
			// X[i] = value's rightmost bit
			b = (bool)(value & 1L);
			set_value(X[i], b);
			value = value >> 1;
		}

		// if any output clause is also an input clause
		// ( i.e. y(k) = x(n) ) then reassign x(n)'s value
		// to y(k)
		for (i=0; i<ny; i++) {
			if (isinput(Y[i])) {
				v = get_value(X[enumerate(Y[i])]);
				if (!positional) {
					char *cname = get_clause_name(get_child(Y[i], 0));
					for (j=0; j<nx; j++)
						if (streq(cname, get_clause_name(X[j])))
							break;
					v = get_value(X[j]);
				}
				set_value(Y[i], v);
			}

		}
	}
}



void hijack(t_clause **A, t_clause **B, int num) {
	if (A && B) {
		t_clause *target = NULL;
		int i, n=num;

		while (num) {
			for (i=0; i<num; i++) {
				if (streq(get_clause_name(A[num-1]), get_clause_name(B[i]))) {
					target = B[num-1];
					B[num-1] = B[i];
					B[i] = target;
					break;
				}
			}

			num--;
		}
	}
}



void cleanup(FILE *f, t_clause **X, t_clause **Y, t_clause **T, int nx, int nt)
{
	int i;
	fclose(f);
	for (i=0; i<nx; i++) 	free(X[i]);
	for (i=0; i<nt; i++) 	free(T[i]);
	free(X);
	free(Y);
	free(T);
}
