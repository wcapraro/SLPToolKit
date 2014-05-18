/*
 * 	xldg.c - Extended version of the BP-LDG heuristic. This
 * 	version allows for fine-tuning the behavior as to the tie
 * 	breaking criterion as well as other parameters.
 *
 * 	Created on: 26/4/2014
 * 	Revisited:  18/5/2014
 *  Author: Wiliam Capraro - wiliam.capraro@studenti.unimi.it
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "slplib.h"



#define PROG_NAME "Extended BP-LDG Heuristic"
#define SPACES " \n\r\t"


t_slp *scanMatrix(int);
void xLowDepthGreedy(t_slp*, int, int, int, int);
void pickInputs(int, int*, int*, int);
void updateRows(t_slp*, int, int, int, int);
int countRows(int, int, int);
int inferMaxDepth(int*, int);
float computeUpdatedNorm(int, int);
void add_outputs(t_slp*, int, int);
void usage();
void cleanup();


/*
 * Input dimensions
 */
int numRows = 0;
int numCols = 0;

/*
 * All rows have hamming weight at most 2^k,
 * therefore the depth of the resulting circuit
 * is at most k
 */
int k = 0;

/*
 * We keep a collection of bit arrays corresponding
 * to the input matrix columns
 */
t_bitarray **columns = NULL;

/*
 * Also, we keep the Hamming weight for each row
 * of the input matrix for future usage
 */
int *H = NULL;

/*
 * How much to increase the Hamming
 * weight of each row after every iteration
 * of the heuristic
 */
int *deltaH = NULL;

/*
 * Pointers to input and output files
 */
FILE *inFile = NULL;
FILE *outFile = NULL;

/*
 * Store resulting SLP
 */
t_slp *program;


int main(int argc, char **argv) {

	// arg buffer
	int f;

	// verbose flag
	int verbose = 0;

	// preprocess output signals at the beginning
	// of each new phase. Enabled by default
	int preproc = 1;

	// saturate each level before continuing
	// to the next phase. Disabled by default
	int saturate = 0;

	// input and output filename
	char *fin = NULL;
	char *fout = NULL;

	// check for mandatory args
	int _k;
	for (_k=1; k <= argc; _k++) {
		if (!streq(argv[_k], "-f"))
			break;
	}
	if (argc < 3 || _k > argc) {
		usage();
		exit(EXIT_SUCCESS);
	}

	// parse command line args (@see getopt)
	while ((f=getopt(argc, argv, "f:o:p:s:vh")) != -1) {
		switch(f) {
		case 'h':
			usage();
			exit(0);
		case 'f':
			fin = optarg;
			break;
		case 'o':
			fout = optarg;
			break;
		case 'p':
			preproc = streq(optarg, "off") ? 0 : 1;
			break;
		case 's':
			saturate = streq(optarg, "on") ? 1 : 0;
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			printf("Unknown option '%c'\n", f);
			exit(ERR_PARSE);
		}
	}

	// Checks for the input file
	if (fin) {
		inFile = fopen(fin, "r");
		if (!inFile) {
			fprintf(stderr, "@! %s: Could not read file %s\n", PROG_NAME, fin);
			exit(ERR_IO);
		}
	}

	// Checks for the output file
	if (fout) {
		outFile = fopen(fout, "w");
		if (!outFile) {
			fprintf(stderr, "@! %s: Could not read file %s\n", PROG_NAME, fout);
			exit(ERR_IO);
		}
	}

	// Set on to parse input file
	program = scanMatrix(verbose);
	if (!program) {
		cleanup();
		exit(ERR_ALLOC);
	}

	// Add description to SLP
	program->desc = "Boyar-Peralta LowDepthGreedy, extended version";

	// Run the heuristic
	xLowDepthGreedy(program, k, preproc, saturate, verbose);

	// Adds output signals to SLP and print
	add_outputs(program, numRows, numCols);
	slp_print(program, outFile);


	// Clean up
	if (inFile) 	fclose(inFile);
	if (outFile)	fclose(outFile);
	cleanup();

	exit(0);

}


/**
 * Parses matrix file and creates data structures
 */
t_slp *scanMatrix(int verbose) {

	int r = -1;
	int c;
	int size;
	char line[LINLEN];
	char *token;
	t_slp *slp;

	// First, create a blank SLP
	slp = new_slp();
	if (!slp) {
		fprintf(stderr, "@! Could not create new SLP. Aborting...\n");
		return (NULL);
	}

	// We expect the first line to be two integers
	if (fgets(line, LINLEN, inFile)) {
		token = strtok(line, SPACES);
		if (token)	numRows = atoi(token);
		token = strtok(NULL, SPACES);
		if (token)	numCols = atoi(token);
	}

	// Here check if numRows and numCols are valid
	// and create main data structure
	if (numRows && numCols)
	{
		slp->inputs = numCols;
		slp->outputs = numRows;

		if (verbose)
			printf("@+ Parsing program: %d rows and %d columns\n", numRows, numCols);

		// Create main data structures. By theorem 1,
		// we're gonna use up to numCols*numRows + numCols - numRows
		// columns
		size = numCols*numRows + numCols - numRows;
		H = malloc(numRows*sizeof(int));
		deltaH = malloc(numRows*sizeof(int));
		columns = malloc(size*sizeof(t_bitarray*));

		if (H && deltaH && columns)
		{
			for (; size--; ) 			columns[size] = bitarray(numRows);
			for (r=0; r<numRows; r++)	H[r] = deltaH[r] = 0;

			// Parse each row, but populate the structure column-wise
			for (r=-1; r<numRows; r++) {

				fgets(line, LINLEN, inFile);
				strtok(line, SPACES);

				// Skip the first line
				if (r == -1) {
					continue;
				}

				for (c=0; c<numCols; c++)
				{
					token = strtok(NULL, SPACES);
					if (token && atoi(token)) {
						bitset(columns[c], r);
						H[r]++;
					}
				}

				// Infer maximum circuit depth
				k = inferMaxDepth(H, numRows);
			}

			if (verbose)
				printf("@+ Goal depth is %d\n", k);

		}
		else
		{
			fprintf(stderr, "@! scanMatrix() :: Allocation error\n");
			if (slp)		wipe_slp(slp);
			if (H)			free(H);
			if (columns)	free(columns);
			exit(ERR_ALLOC);
		}
	}
	else
	{
		fprintf(stderr, "@! scanMatrix() :: parsing error: numRows=%d, numCols=%d\n", numRows, numCols);
		exit(ERR_PARSE);
	}

	return (slp);
}



/*
 * Adds all the output signals to the SLP
 */
void add_outputs(t_slp *slp, int numRows, int numCols) {

	char *name;
	int size = numRows*numCols + numCols - numRows;
	int r;
	int c;

	for (r=0; r<numRows; r++) {
		for (c=0; c<size; c++) {
			if (slp && getbit(columns[c], r)) {
				name = malloc(20*sizeof(char));
				if (!name) {
					fprintf(stderr, "@! Could not add output signal Y%d = X%d to SLP\n", r, c);
					continue;
				}
				sprintf(name, "Y%d = X%d", r, c);
				slp_add_signal(slp, name);
			}
		}
	}
}



/**
 * Extended version of the Boyar-Peralta LowDepthGreedy
 * heuristic for computing linear straight-line programs
 * over GF(2). The input matrix is supposed
 * to have Hamming weight at most 2^k in every row. This
 * version of the heuristic does not impose a maximum number
 * of rounds to be performed; rather, it goes on until all
 * rows of the matrix have Hamming weight 1.
 */
void xLowDepthGreedy(t_slp *slp, int k, int preproc, int saturate, int verbose) {

	// pointer to the next slot in the array
	int s = numCols;

	// the current max depth
	int i = 0;

	// columns up to s-1 have depth at most i
	int ip = s-1;

	int _s;
	int d;
	int l;
	int j1;
	int j2;
	int mh;

	if (verbose) {
		if (preproc)	printf("@+ Using output preprocessing\n");
		if (saturate)	printf("@+ Using level saturation\n");
	}

	// Go
	while (TRUE) {

		if (verbose)
			printf("@+ Beginning phase %d [k=%d, s=%d, ip=%d]\n", (i+1), k, s, ip);

		// Pre-process output signal first if the
		// corresponding option is enabled
		l = j1 = j2 = -1;
		while (preproc && (l = findRowIndexHamming2(ip, &j1, &j2, verbose)) != -1) {
			updateRows(slp, j1, j2, s++, verbose);
		}

		// Otherwise, find the two input variables
		// that occur most often in the current rows
		mh = (saturate) ? 0 : k-i-1;
		while (findRowIndexMaxHamming(mh) != -1) {
			l = j1 = j2 = -1;
			pickInputs(ip, &j1, &j2, verbose);
			if (j1 != -1 && j2 != -1)
				updateRows(slp, j1, j2, s++, verbose);
			else
				break;

		}

		// End of i-th phase. Update counters
		_s = 0;
		for (d=0; d<numRows; d++) {
			H[d]+=deltaH[d];
			deltaH[d]=0;
			_s += H[d];
		}
		ip = s-1;
		i++;

		// Are we done yet ?!
		if (_s <= slp->outputs)
			return;

	}

}



/*
 * Add a new signal corresponding to an XOR gate
 * of the given two inputs j1 and j2 and update all rows
 * of the matrix accordingly
 */
void updateRows(t_slp *slp, int j1, int j2, int s, int verbose) {

	int l;
	int delta;
	bool can_proceed = 0;
	columns[s] = bitarray(numRows);
	char *signal_name = malloc(20*sizeof(char));

	if (slp && signal_name) {
		sprintf(signal_name, "X%d = X%d + X%d", s, j1, j2);
		slp_add_signal(slp, signal_name);
	}

	for (l=0; l<numRows; l++) {

		// We update all rows in which either there occur both j1 and j2
		can_proceed = 	(getbit(columns[j1], l) && getbit(columns[j2], l));

		if (can_proceed) {
			bittgl(columns[j1], l);
			bittgl(columns[j2], l);
			bitset(columns[s], l);
			delta = (getbit(columns[j1], l) == getbit(columns[j2], l)) ? 2 : 0;
			H[l] = H[l] - delta;
			deltaH[l]++;

			if (verbose)
				printf("@+ Updating row %d: H[%d] = %d\tdeltaH[%d]=%d\n", l, l, H[l], l, deltaH[l]);

		}
	}
}


/*
 * Yields the index of a row in the matrix
 * with Hamming weight greater than 2^k, provided
 * one such row exists. Conversely, it returns -1.
 */
int findRowIndexMaxHamming(int k, int verbose) {

	int kk = (k < 0) ? 0 : k;
	int h = (int)pow(2, kk);
	int index = -1;
	int j;

	if (verbose)
		printf("@+ Looking for rows with Hamming weight > %d ... ", h);

	kk = 0;
	for (j=0; j<numRows && H; j++) {
		if (H[j] > h) {
			index = j;
			kk++;
		}
	}

	if (verbose)
		if (kk)	printf("Got %d of them\n", kk);
		else	printf("None found\n");

	return (index);

}



/**
 * Returns the value of the norm of H as if the gate
 * (j1 XOR j2) were added to the matrix
 */
float computeUpdatedNorm(int j1, int j2) {

	int newH[numRows];
	float norm;
	int i;

	for (i=0; i<numRows; i++) {
		newH[i]=H[i];
		if (getbit(columns[j1], i) && getbit(columns[j2], i))
			newH[i]-=2;
	}

	norm = norm2(newH, numRows);
	return (norm);
}




/*
 * Chooses two inputs j1 and j2 that occur most often
 * in the current rows, i.e which maximize
 * the set S = { r : M[r, j1] = M[r, j2] = 1 }
 */
void pickInputs(int limit, int* j1, int* j2, int verbose) {

	int buf1[numCols*numCols*numRows];
	int buf2[numCols*numCols*numRows];
	int i = 0;

	int jj1 = 0;
	int jj2 = 0;
	int count = 0;
	int c;

	// Pre-compute the norm of H
	float normH = 0.0f;

	// choose a pair j1 and j2 that occur most
	// often in the current rows, resolving ties
	// by choosing the pair that minimizes the
	// Euclidean norm of H

	for (jj1=0; jj1<limit; jj1++)
		for (jj2=jj1+1; jj2<=limit; jj2++) 
		{
			c = countRows(jj1, jj2, 0);

			if (c && c >= count)
			{
				if (c > count || computeUpdatedNorm(jj1, jj2) < normH)
				{
					count = c;
					normH = computeUpdatedNorm(jj1, jj2);
					i = 0;
				}

				// c == count and norm(H) == norm(newH)
				buf1[i] = jj1;
				buf2[i] = jj2;
				i++;
			}
		}

	// end loop, set j1 and j2
	if (i>0) {
		(*j1) = buf1[0];
		(*j2) = buf2[0];

		if (verbose) {
			printf("@+ Picked inputs [j1=%d, j2=%d]\n", *j1, *j2);
		}
	}

}




/**
 * Returns the index of a row in the matrix
 * having Hamming weight 2, taking into account
 * uniquely the inputs up to limit. If no such row
 * exists, returns -1.
 */
int findRowIndexHamming2(int limit, int* j1, int*j2, int verbose) {

	int t = -1;
	int jj1 = 0;
	int jj2 = limit;

	// check for any row with hamming weight 2
	for (t=0; t<numRows && H; t++) {
		if (H[t] == 2) {
			for (; !getbit(columns[jj1], t) && jj1<jj2; jj1++) ;
			for (; !getbit(columns[jj2], t) && jj1<jj2; jj2--) ;
			(*j1) = jj1;
			(*j2) = jj2;

			if (verbose)
				printf("@+ Preprocessing row %d whose Hamming weight is 2 [j1=%d, j2=%d]\n", t, *j1, *j2);

			return (t);
		}

	}

	return (-1);

}



/*
 * Scans through all rows of the matrix and
 * counts how many rows r satisfy the condition
 * M[r, j1] = M[r, j2] = 1
 */
int countRows(int j1, int j2, int verbose) {

	int count = 0;
	int r;

	for (r=0; r<numRows; r++) {
		if (getbit(columns[j1], r) && getbit(columns[j2], r))
			count++;
	}

	if (verbose)
		printf("@+ There are %d rows having bits %d=1 and %d=1\n", count, j1, j2);

	return (count);

}




/*
 * Computes the optimal circuit's depth
 * as max{log2(hamming(M_r) | r=0...numRows}
 */
int inferMaxDepth(int *H, int numRows) {

	int max = 0;
	int i;

	for (i=0; i<numRows; i++) {
		if (H[i] > max) {
			max = H[i];
		}
	}

	return ((int)ceil(log2(max)));
}



/*
 * Deallocates memory for data structures
 */
void cleanup() {

	while (columns && numRows--)	wipe(columns[numRows]);
	if (program)					wipe_slp(program);
	if (columns)					free(columns);
	if (H)							free(H);

}



/*
 * Simple usage message
 */
void usage() {
	printf("\n%s -- Boyar and Peralta's Heuristic for minimizing linear components' depth\n\n", PROG_NAME);
	printf("ldg -f <file> [optlist]\n\n");
	printf("List of options:\n");
	printf("  -o <file>\tPrint result to <file>\n");
	printf("  -p [on|off]\tPreprocess outputs at the beginning of each phase [default=on]\n");
	printf("  -s [on|off]\tEnable or disable level saturation [default=off]\n");
	printf("  -v\t\tVerbose\n");
	printf("  -h\t\tShows this message\n\n");
}


