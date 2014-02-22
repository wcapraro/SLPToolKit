/*
 * 	ldg.c - Boyar and Peralta's Low Depth Greedy heuristic
 *
 * 	Created on: 21/02/2014
 *  Author: Wiliam Capraro - wiliam.capraro@studenti.unimi.it
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "slplib.h"



#define PROG_NAME "LowDepthGreedy"
#define SPACES " \n\r\t"


void scanMatrix(FILE*, int);
void lowDepthGreedy(int, int);
int pickInputs(int, int*, int*);
int countRows(int, int);
void usage();


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
 * to the input matrix' columns
 */
t_bitarray **columns = NULL;

/*
 * Also, we keep the Hamming weight for each row
 * of the input matrix for future usage
 */
int *H = NULL;


int main(int argc, char **argv) {

	int f;
	int verbose = 0;
	char *fin = NULL;

	FILE *matrix;

	/* parse command line options (see the getopt tool) */
	if (argc < 3) {
		usage();
		exit(EXIT_SUCCESS);
	}

	while ((f=getopt(argc, argv, "f:k:vh")) != -1) {
		switch(f) {
		case 'h':
			usage();
			exit(0);
		case 'f':
			fin = optarg;
			break;
		case 'k':
			k = atoi(optarg);
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
		matrix = fopen(fin, "r");
		if (!matrix) {
			fprintf(stderr, "-- %s: Could not read file %s\n", PROG_NAME, fin);
			exit(ERR_IO);
		}
	}

	// Set on to parse matrix
	scanMatrix(matrix, verbose);

	// Run the heuristic
	lowDepthGreedy(k, verbose);

	int q;

	for (q=0; q<numCols; q++) {
		ba_print(columns[q]);
		printf("\n");
		wipe(columns[q]);
	}
	free(columns);

	free(H);


	if (matrix) {
		fclose(matrix);
	}

	exit(0);

}


/**
 * Parses matrix file and creates data structures
 */
void scanMatrix(FILE *file, int verbose) {

	int r = -1;
	int c;
	int size;
	char line[LINLEN];
	char *token;

	// We expect the first line to be two integers
	if (fgets(line, LINLEN, file)) {
		token = strtok(line, SPACES);
		if (token)	numRows = atoi(token);
		token = strtok(NULL, SPACES);
		if (token)	numCols = atoi(token);
	}

	// Here check if numRows and numCols are valid
	// and create main data structure
	if (numRows && numCols)
	{
		if (!k)
			k = (int)ceil(log2(numCols));

		if (verbose)
			printf("scanMatrix() :: hamming=%d, numRows=%d, numCols=%d\n", k, numRows, numCols);

		// Create main data structures. By theorem 1,
		// we're gonna use up to numCols*numRows + numCols - numRows
		// columns
		//maxDepth = ceil(log2(hamming));
		size = numCols*numRows + numCols - numRows;
		H = malloc(numRows*sizeof(int));
		columns = malloc(size*sizeof(t_bitarray*));

		if (H && columns)
		{
			for (; size--; ) 			columns[size] = bitarray(numRows);
			for (r=0; r<numRows; r++)	H[r]=0;

			// Parse each row, but populate the structure column-wise
			for (r=-1; r<numRows; r++) {

				fgets(line, LINLEN, file);
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
						if (verbose)
							printf("scanMatrix() :: Setting bit r=%d c=%d H[%d]=%d\n", r, c, r, H[r]);
					}
				}

			}
		}
		else
		{
			fprintf(stderr, "scanMatrix() :: Allocation error\n");
			if (H)			free(H);
			if (columns)	free(columns);
			exit(ERR_ALLOC);
		}
	}
	else
	{
		fprintf(stderr, "scanMatrix() :: parsing error: numRows=%d, numCols=%d\n", numRows, numCols);
		exit(ERR_PARSE);
	}

}



/**
 * LowDepthGreedy heuristic. Input matrix is supposed
 * to have Hamming weight at most 2^k in every row
 */
void lowDepthGreedy(int k, int verbose) {

	// pointer to the next slot in the array
	int s = numCols;

	// the current max depth
	int i = 0;

	// columns up to s-1 have depth at most i
	int ip = s-1;

	int l;
	int j1;
	int j2;

	// The i-th phase terminates when there is
	// no more row with hamming weight greater
	// than 2^(k-i-1)
	while (findRowIndexMaxDepth(k-i-1) != -1) {

		// Phase i
		printf("lowDepthGreedy() :: Beginning phase %d [k=%d, s=%d, ip=%d]\n", i, k, s, ip);
		l = j1 = j2 = -1;
		l = pickInputs(ip, &j1, &j2);

		/*
		 * Do stuff here!!
		 */

		s = s + numRows;
		ip = s-1;
		i++;

	}

}


/*
 * Yields the index of a row in the matrix
 * with Hamming weight greater than 2^k, provided
 * one such row exists. Conversely, it returns -1.
 */
int findRowIndexMaxDepth(int k) {

	int h = (int)pow(2, k);
	int index = -1;
	int j;

	printf("findRowIndexMaxDepth() :: Looking for one row with HW > %d (k=%d) ... ", h, k);

	for (j=0; j<numRows && H; j++) {
		if (H[j] > h) {
			h = H[j];
			index = j;
		}
	}

	printf("%d\n", index);

	return (index);

}



/*
 * Considering the first limit (+1) columns, if
 * any row has hamming weight 2 then returns the
 * index of the row and computes the column indexes
 * corresponding to the non-zero elements j1 and j2.
 * Otherwise, returns the row index for the row that
 * maximizes the set S = { r : M[r, j1] = M[r, j2] = 1 }
 */
int pickInputs(int limit, int* j1, int* j2) {

	int t;
	int jj1 = 0;
	int jj2 = limit;
	int count = 0;
	int c;

	// check for any row with hamming weight 2
	for (t=0; t<numRows && H; t++) {
		if (H[t] == 2) {
			for (; !getbit(columns[jj1], t) && jj1<jj2; jj1++) ;
			for (; !getbit(columns[jj2], t) && jj1<jj2; jj2--) ;
			(*j1) = jj1;
			(*j2) = jj2;
			printf("pickInputs() :: Row %d has Hamming weight 2! [j1=%d, j2=%d]\n", t, *j1, *j2);
			return (t);
		}
	}

	// choose a pair j1 and j2 that occur most
	// often in the current rows
	for (jj1=0; jj1<limit; jj1++)
		for (jj2=jj1+1; jj2<=limit; jj2++)
			if ((c = countRows(jj1, jj2)) > count)
			{
				count = c;
				(*j1) = jj1;
				(*j2) = jj2;
			}

	// return -1 to indicate that no row had
	// hamming weight 2
	printf("pickInputs() :: I've picked inputs [j1=%d, j2=%d]\n", *j1, *j2);
	return (-1);

}



/*
 * Scans through all rows of the matrix and
 * counts how many rows r satisfy the condition
 * M[r, j1] = M[r, j2] = 1
 */
int countRows(int j1, int j2) {

	int count = 0;
	int r;

	for (r=0; r<numRows; r++) {
		if (getbit(columns[j1], r) && getbit(columns[j2], r))
			count++;
	}

	printf("countRows() :: There are %d rows having bits %d=1 and %d=1\n", count, j1, j2);

	return (count);

}




/*
 * Simple usage message
 */
void usage() {
	printf("\n%s -- Boyar and Peralta's Heuristic for minimizing linear components' depth\n\n", PROG_NAME);
	printf("ldg [optlist]\n\n");
	printf("List of options:\n");
	printf("  -f <file>\tthe matrix file to use as input (mandatory)\n");
	printf("  -k <int>\tall rows have Hamming weight at most 2^k\n");
	printf("  -v\t\tverbose\n");
	printf("  -h\t\tshows this message\n\n");
}


