
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "slplib.h"



#define PROG_NAME "LowDepthGreedy"


void lowDepthGreedy(t_bitarray**, int, int, int);
void usage();


int main(int argc, char **argv) {
	int f;
	int hamming;
	char *fin=NULL;

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

	// 1. Parse the input matrix file and build bitarrays columnwise...
	// note that we'll have n+w  bitarrays at most!

	// 2. given M, m, n and w, implement the LowDepthGreedy procedure



}



void lowDepthGreedy(t_bitarray **signals, int nRown, int nCols) {
	return;
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


