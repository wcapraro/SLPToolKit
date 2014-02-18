/*
 * 	ldg.c - Boyar and Peralta's Low Depth Greedy heuristic
 *
 * 	Created on: 18/02/2014
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


t_bitarray** scanMatrix(FILE*, int, int);
void lowDepthGreedy(t_bitarray**, int, int);
void usage();


int numRows = 0;
int numCols = 0;
int maxDepth = 0;


int main(int argc, char **argv) {
	int f;
	int hamming = 0;
	char *fin = NULL;

	FILE *matrix;

	/* parse command line options (see the getopt tool) */
	if (argc < 3) {
		usage();
		exit(EXIT_SUCCESS);
	}

	while ((f=getopt(argc, argv, "w:m:h")) != -1) {
		switch(f) {
		case 'h':
			usage();
			exit(0);
		case 'm':
			fin = optarg;
			break;
		case 'w':
			hamming = atoi(optarg);
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

	// Proceed, parse matrix
	t_bitarray **signals = scanMatrix(matrix, hamming, TRUE);

	// Run the heuristic
	lowDepthGreedy(signals, numRows, numCols);

	int q;

	for (q=0; q<numCols; q++) {
		ba_print(signals[q]);
		printf("\n");
		wipe(signals[q]);
	}
	free(signals);


	if (matrix) {
		fclose(matrix);
	}

	exit(0);

}


/**
 * Scan matrix file and returns an array of bitarrays
 */
t_bitarray **scanMatrix(FILE *file, int hamming, int verbose) {

	int r = -1;
	int c;
	int size;
	char line[LINLEN];
	char *token;
	t_bitarray **signals = NULL;

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
		if (!hamming)
			hamming = numCols;

		if (verbose)
			printf("scanMatrix() :: hamming=%d, numRows=%d, numCols=%d\n",hamming, numRows, numCols);

		// Create main data structure. We're gonna use an array of
		// bitarrays of size numCols + log2(hamming)
		size = numCols + ceil(log2(hamming));
		signals = malloc(size*sizeof(t_bitarray*));

		if (signals)
		{
			while (size--) {
				signals[size] = bitarray(numRows);
			}

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
					if (token && atoi(token) && verbose) {
						printf("scanMatrix() :: Setting bit r=%d c=%d value=%s\n", r, c, token);
						bitset(signals[c], r);
					}
				}

			}
		}
		else
		{
			fprintf(stderr, "scanMatrix() :: Allocation error\n");
		}
	}
	else
	{
		fprintf(stderr, "scanMatrix() :: parsing error: hamming=%d, numRows=%d, numCols=%d\n", hamming, numRows, numCols);
	}

	return (signals);

}



/**
 * LowDepthGreedy
 */
void lowDepthGreedy(t_bitarray **signals, int m, int n, int k) {

	// pointer to the next slot in the array
	int s = n;

	// the current max depth
	int i = 0;

	// columns up to s-1 have depth at most i
	int ip = s-1;




}



/*
 * Simple usage message
 */
void usage() {
	printf("\n%s -- Boyar and Peralta's Heuristic for minimizing linear components' depth\n\n", PROG_NAME);
	printf("ldg [optlist]\n\n");
	printf("List of options:\n");
	printf("  -m <file>\tthe matrix file to use as input (mandatory)\n");
	printf("  -w <int>\tmaximum hamming weight for all rows, or maximum depth allowed (default is ?)\n");
	printf("  -h\t\tshows this message\n\n");
}


