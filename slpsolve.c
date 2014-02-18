
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "slplib.h"


#define N nmk[0]
#define M nmk[1]
#define K nmk[2]
#define PROG_NAME "SLPSolve"



int symb_solve(t_bastack*, t_clause*, int);
bool check(t_bastack*, t_clause*);
bool is_linear(t_clause**);
void cleanmem();
void usage();


/* GLOBAL DATA STRUCTURES */

int nmk[3]={0,0,0};
t_clause **X = NULL;
t_clause **T = NULL;
t_clause **Y = NULL;
t_bastack *stack = NULL;

int main(int argc, char **argv) {

	char *fin = NULL;
	char *fout = NULL;
	char *solve = NULL;
	FILE *pfile = NULL;
	int f, h;


	/* parse command line options (see the getopt tool) */
	while ((f=getopt(argc, argv, "f:o:s:h")) != -1) {
		switch(f) {
		case 'f':
			fin = optarg;
			break;
		case 'h':
			usage();
			exit(0);
		case 'o':
			fout = optarg;
			break;
		case 's':
			solve = optarg;
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


	/* parse the program and populate X, T, Y */
	compile(pfile, &X, &T, &Y, nmk, solve);
	fclose(pfile);

	printf("\n\n>> N=%d M=%d, K=%d\n", N, M, K);

	/* (try to) open output file and print dimensions */
	if (fout) {
		pfile = fopen(fout, "w");
		if (!pfile) {
			fprintf(stderr, "--%s : unable to write to '%s'", PROG_NAME, fout);
			exit(ERR_IO);
		}
		fprintf(pfile, "%d %d\n", M, N);
	}


	/* calculate outputs */
	printf("\n\n>> Calculating output signals <<\n\n");
	int m, r, w;
	bool print = TRUE;
	bool and_flg = is_linear(T);
	char *cname;

	// for each output signal...
	for (m = 0; m < M; m++) {

		// signal buffer
		t_bitarray *ba;

		// allocate and prepare stack
		wipestack(stack);
		stack = empty_stack();
		if (!stack) {
			fprintf(stderr, "-- %s : could not create stack for output signal y%d\n", PROG_NAME, m);
			continue;
		}

		// symbolically solve Y[m]
		r = symb_solve(stack, Y[m], 0);

		printf("%s = ", get_clause_name(Y[m]));

		// is the AND flag set (non-linear case) ?
		if (TRUE-and_flg)
		{
			if (fout) 	fprintf(pfile, "%s = ", get_clause_name(Y[m]));


			// if the XNOR flag is set, print a "+1"
			if (stack_is_xn_flg_set(stack)) {
				printf("1 + ");
				if (fout)
					fprintf(pfile, "1 + ");
			}


			while (r) {
				ba = pop(stack);
				h = hamming(ba);
				for (w=0; w<N; w++) {
					if (getbit(ba, w)) {
						cname = get_clause_name(X[w]);
						(--h) ? printf("%s*", cname) : printf("%s", cname);
						if (fout)
							(h) ? fprintf(pfile, "%s*", cname) : fprintf(pfile, "%s", cname);
					}
				}
				wipe(ba);
				r--;
				r ? printf(" + ") : printf("\n");
				if (fout)
					r ? fprintf(pfile, " + ") : fprintf(pfile, "\n");
			}

		} 	// end of non-linear case

		else

		{
			// linear case (XOR/XNOR)
			if (fout && print)
			{
				for (w=0; w<N; w++) {
					cname = get_clause_name(X[w]);
					fprintf(pfile, "%s ", cname);
				}
				fprintf(pfile, "\n");
				print = FALSE;
			}

			if (fout)	fprintf(pfile, "%s ", get_clause_name(Y[m]));


			// bitwise-xor the bitarrays
			t_bitarray *buf1, *buf2;
			ba = bitarray(N);
			while (r) {
				buf1 = ba;
				buf2 = pop(stack);
				ba = ba_xor(buf1, buf2);
				wipe(buf1);
				wipe(buf2);
				r--;
			}


			// check for the XNOR flag
			if (stack_is_xn_flg_set(stack)) 	printf("1 + ");


			h = hamming(ba);
			for (w=0; w<N; w++) {
				if (fout)	fprintf(pfile, "%d ", getbit(ba, w));
				if (getbit(ba, w)) {
					cname = get_clause_name(X[w]);
					h-1 ? printf("%s + ", cname) : printf("%s\n", cname);
					h--;
				}
			}
			if (fout)	fprintf(pfile, "\n");

			wipe(ba);
		}

	} // end for loop



	/* if the user requests it, validate results */
	if (solve) {
		printf("\n\n>> Evaluating results <<\n");
		for (m=0; m<M; m++) {
			evaluate(Y[m]);
			printf("\n%s = %d (", get_clause_name(Y[m]), get_value(Y[m]));
			stack = empty_stack();
			if (check(stack, Y[m]))
				printf("output match");
			else
				printf("output MISMATCH #### ");
			printf(")\n");
		}


	}



	/* clean up before exiting */
	cleanmem();
	//fclose(pfile);
	return 0;
}




/*
 * Cleans up memory
 */
void cleanmem() {
	int i;
	for (i=0; i<N; i++) wipe_clause(X[i]);
	for (i=0; i<M; i++) wipe_clause(Y[i]);
	for (i=0; i<K; i++)	wipe_clause(T[i]);
	free(X);
	free(Y);
	free(T);
	wipestack(stack);
}



/*
 * Symbolically solves the equations for a given output signal
 */
int symb_solve(t_bastack *stack, t_clause *y, int count) {
	int retval = count;
	t_bitarray *ba = NULL;

	if (isinput(y))
	{
		// input clause. In this case just generate the bitarray
		// and push it onto stack
		ba = bitarray(N);
		bitset(ba, enumerate(y));
		push(stack, ba);
		retval++;
		return retval;
	}
	else
	{
		// this is a full clause. Need to recursively solve the
		// corresponsing sub-tree first
		int nleft, nright;
		nleft = symb_solve(stack, get_child(y, 0), retval);
		nright = symb_solve(stack, get_child(y, 1), retval);
		retval += nright;
		retval += nleft;

		// next, look at the operator and act accordingly
		// if it is AND, I need to do nright+nleft pop()s
		// otherwise, just return to caller
		char *op = get_operator(y);
		if (!strcasecmp(op, AND)) {
			int a, b;
			stack_set_and_flg(stack, TRUE);
			t_bitarray **signals = (t_bitarray**)malloc((nright+nleft)*sizeof(t_bitarray*));
			for (a=0; a < nright+nleft; a++) {
				signals[a]=pop(stack);
				retval--;
			}

			for (a=nright+nleft-1; a >= nright; a--)
				for (b=nright-1; b >= 0; b--) {
					push(stack, ba_or(signals[a], signals[b]));
					retval++;
				}

			// clean memory from unnecessary bitarrays before returning
			for (a=0; a < nright+nleft; a++)
				wipe(signals[a]);
			free(signals);
		}
		else
		{
			// operator is XOR/XNOR
			// do nothing, just pass back
			if (!strcasecmp(op, XNOR))
				stack_set_xn_flg(stack,
						TRUE-stack_is_xn_flg_set(stack));
		}
	}

	return retval;
}



/*
 * Returns 0 iff the SLP contains at leats one non linear clause
 */
bool is_linear(t_clause **T) {
	bool b = TRUE;
	int i;
	for (i=0; i<K; i++) {
		if (streq(get_operator(T[i]), AND)) {
			b = FALSE;
			break;
		}
	}
	return b;
}



/*
 * Checks whether the symbolic result for the given output clause
 * is numerically correct by using the input values provided
 * (assumes the -s option is set)
 */
bool check(t_bastack *stack, t_clause *clause) {
	bool y = get_value(clause);
	stack = empty_stack();
	int nel = symb_solve(stack, clause, 0);
	printf("\n*** signals = %d\n", nel);
	t_bitarray *ba;
	int el, i;
	bool r;
	bool f_x = FALSE;
	for (el=0; el < nel; el++) {
		r = TRUE;
		ba = pop(stack);
		printf("*** ba = ");
		ba_print(ba);
		printf("\t");

		for (i=0; i < N; i++) {
			if (getbit(ba, i)) {
				if (get_value(X[i]) == FALSE) {
					printf("*** setting r=0 (ba[%d]=%d, x[%d]=%d)\t", i, getbit(ba, i), i, get_value(X[i]));
					r = FALSE;
					break;
				}
			}
		}
		f_x ^= r;
		printf("r=%d, f(x)=%d\n", r, f_x);
	}
	return f_x == y;
}



/*
 * Simple usage message
 */
void usage() {
	printf("\n%s %s -- Simple solver for straight-line programs\n\n", PROG_NAME, PROG_VERSION);
	printf("slpsolve [optlist]\n\n");
	printf("List of options:\n");
	printf("  -f <file>\tuse <file> as input file\n");
	printf("  -o <file>\tprint output to <file>\n");
	printf("  -s (0|1)+\tnumerically solve equations using the provided input values\n");
	printf("  -h\t\tshows this message\n\n");
}



