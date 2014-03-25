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


void scanMatrix(int);
void lowDepthGreedy(int, int);
void pickInputs(int, int*, int*, int);
int countRows(int, int, int);
int inferMaxDepth(int*, int);
float computeUpdatedNorm(int, int);
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


int main(int argc, char **argv) {

	int f;
	int verbose = 0;
	char *fin = NULL;
	char *fout = NULL;

	/* parse command line options (see the getopt tool) */
	if (argc < 3) {
		usage();
		exit(EXIT_SUCCESS);
	}

	while ((f=getopt(argc, argv, "f:o:vh")) != -1) {
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
			fprintf(stderr, "-- %s: Could not read file %s\n", PROG_NAME, fin);
			exit(ERR_IO);
		}
	}

	// Checks for the output file
	if (fout) {
		outFile = fopen(fout, "w");
		if (!outFile) {
			fprintf(stderr, "-- %s: Could not read file %s\n", PROG_NAME, fout);
			exit(ERR_IO);
		}
	}

	// Set on to parse inFile
	scanMatrix(verbose);

	// Run the heuristic
	lowDepthGreedy(k, verbose);

	// Print the outputs and deallocates memory
	printOutputs(outFile, numRows, numCols);
	printOutputs( stdout, numRows, numCols);


	// Clean up
	if (inFile) 	fclose(inFile);
	if (outFile)	fclose(outFile);
	cleanup();

	exit(0);

}


/**
 * Parses matrix file and creates data structures
 */
void scanMatrix(int verbose) {

	int r = -1;
	int c;
	int size;
	char line[LINLEN];
	char *token;

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
		if (verbose)
			printf("scanMatrix() :: hamming=%d, numRows=%d, numCols=%d\n", k, numRows, numCols);

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
						if (verbose)
							printf("scanMatrix() :: Setting bit r=%d c=%d H[%d]=%d\n", r, c, r, H[r]);
					}
				}

				// Infer maximum circuit depth
				k = inferMaxDepth(H, numRows);

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



void printFilePreamble(FILE *to, int numRows, int numCols) {

	if (!to)
		return;

	int m = 0;
	int n = 0;

	fprintf(to, "#\n# Boyar and Peralta's Low Depth Greedy heuristic\n#\n\n");
	fprintf(to, "%d inputs\n", numCols);
	while (n < numCols)		fprintf(to, "X%d ", n++);
	fprintf(to, "\n");
	fprintf(to, "%d outputs\n", numRows);
	while (m < numRows)		fprintf(to, "Y%d ", m++);
	fprintf(to, "\n\n");
	fprintf(to, "BEGIN\n");

}



void printOutputs(FILE *to, int numRows, int numCols) {

	if (!to)
		return;

	int size = numRows*numCols + numCols - numRows;
	int r;
	int c;

	for (r=0; r<numRows; r++) {
		for (c=0; c<size; c++) {
			if (to && getbit(columns[c], r)) {
				fprintf(to, "Y%d = X%d\n", r, c);
			}
		}
	}

	fprintf(to, "END\n");
}



/**
 * LowDepthGreedy heuristic. Input matrix is supposed
 * to have Hamming weight at most 2^k in every row
 */
void lowDepthGreedy(int k, int verbose) {

	// Prints preamble
	printFilePreamble(outFile, numRows, numCols);
	printFilePreamble( stdout, numRows, numCols);

	// pointer to the next slot in the array
	int s = numCols;

	// the current max depth
	int i = 0;

	// columns up to s-1 have depth at most i
	int ip = s-1;

	int d;
	int l;
	int j1;
	int j2;

	// The heuristic consists of k phases
	// starting from 0
	//for (; k>=0; k--) {
	while (i < k) {

		if (verbose)
			printf("lowDepthGreedy() :: Beginning phase %d [k=%d, s=%d, ip=%d]\n", i, k, s, ip);

		// The i-th phase terminates when there is
		// no more row with hamming weight greater
		// than 2^(k-i-1)
		while (findRowIndexMaxHamming(k-i-1, verbose) != -1) {

			l = j1 = j2 = -1;

			// At the beginning of the phase, we
			// first look for a row with hamming weight 2
			// and process that signal first
			if ((l = findRowIndexHamming2(ip, &j1, &j2, verbose)) != -1) {
				updateRows(j1, j2, s++, verbose);
				continue;

			}

			// Otherwise, find the two input variables
			// that occur most often in the current rows
			else
			{
				pickInputs(ip, &j1, &j2, verbose);
				if (j1 != -1 && j2 != -1)
					updateRows(j1, j2, s++, verbose);
			}

		}

		// End of i-th phase. Update counters
		for (d=0; d<numRows; d++) {
			H[d]+=deltaH[d];
			deltaH[d]=0;
		}
		ip = s-1;
		i++;

	}

}



/*
 * Add a new signal corresponding to an XOR gate
 * of the given two inputs j1 and j2 and update all rows
 * of the matrix accordingly
 */
void updateRows(int j1, int j2, int s, int verbose) {

	int l;

	columns[s] = bitarray(numRows);

	if (outFile)	fprintf(outFile, "X%d = X%d + X%d\n", s, j1, j2);
	if (TRUE)		fprintf(stdout,  "X%d = X%d + X%d\n", s, j1, j2);

	for (l=0; l<numRows; l++) {

		if (getbit(columns[j1], l) && getbit(columns[j2], l)) {
			bitclr(columns[j1], l);
			bitclr(columns[j2], l);
			bitset(columns[s], l);
			H[l]--;
			H[l]--;
			deltaH[l]++;
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
		printf("findRowIndexMaxHamming() :: Looking for rows with Hamming weight > %d ... ", h);

	kk = 0;
	for (j=0; j<numRows && H; j++) {
		if (H[j] > h) {
			h = H[j];
			index = j;
			kk++;
		}
	}

	if (verbose)
		printf("%d\n", kk);

	return (index);

}



/**
 * Returns the value of the norm of H as if the gate
 * j1 XOR j2 was added to the matrix
 */
float computeUpdatedNorm(int j1, int j2) {

	int newH[numRows];
	int i;

	for (i=0; i<numRows; i++) {
		newH[i]=H[i];
		if (getbit(columns[j1], i) && getbit(columns[j2], i))
			newH[i]-=2;
	}

	return (norm2(newH, numRows));
}



/*
 * Choose two inputs j1 and j2 that occur most often
 * in the current rows, i.e which maximize
 * the set S = { r : M[r, j1] = M[r, j2] = 1 }
 */
void pickInputs(int limit, int* j1, int* j2, int verbose) {

	int buf1[numRows];
	int buf2[numRows];
	int i = 0;

	int jj1 = 0;
	int jj2 = 0;
	int count = 0;
	int c;

	// pre-compute the norm of H
	float normH = norm2(H, numRows);

	// choose a pair j1 and j2 that occur most
	// often in the current rows, resolving ties
	// by choosing the pair that maximizes the
	// Euclidean norm of H
	for (jj1=0; jj1<limit; jj1++)
		for (jj2=jj1+1; jj2<=limit; jj2++)

			if ((c = countRows(jj1, jj2, 0)) >= count)
			{
				if (c > count || computeUpdatedNorm(jj1, jj2) > normH)
				{
					count = c;
					i = 0;
				}

				// c == count and norm(H) == norm(newH)
				buf1[i] = jj1;
				buf2[i] = jj2;
				i++;
			}

	// end loop, set j1 and j2
	if (i>0) {
		(*j1) = buf1[0];
		(*j2) = buf2[0];
	}

	if (verbose)
		printf("pickInputs() :: I've picked inputs [j1=%d, j2=%d]\n", *j1, *j2);

}



/**
 * Returns the index of a row in the matrix
 * having hamming weight 2, taking into account
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
				printf("findRowIndexHamming2() :: Row %d has Hamming weight 2! [j1=%d, j2=%d]\n", t, *j1, *j2);

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
		printf("countRows() :: There are %d rows having bits %d=1 and %d=1\n", count, j1, j2);

	return (count);

}


/*
 * Computes the maximum circuit's depth
 * as max{log2(hamming(M_r) | r=0...numRows}
 */
int inferMaxDepth(int *H, int numRows) {

	int max = 0;
	int i;

	for (i=0; i<numRows; i++) {
		if (H[i] > max) {
			max = (int)ceil(log2(H[i]));
		}
	}

	return (max);
}



/*
 * Deallocates memory for data structures
 */
void cleanup() {

	while (columns && numRows--)	wipe(columns[numRows]);
	if (columns)					free(columns);
	if (H)							free(H);

}



/*
 * Simple usage message
 */
void usage() {
	printf("\n%s -- Boyar and Peralta's Heuristic for minimizing linear components' depth\n\n", PROG_NAME);
	printf("ldg [optlist]\n\n");
	printf("List of options:\n");
	printf("  -f <file>\tthe matrix file to use as input (mandatory)\n");
	printf("  -o <file>\tprint result to <file>\n");
	printf("  -v\t\tverbose\n");
	printf("  -h\t\tshows this message\n\n");
}


