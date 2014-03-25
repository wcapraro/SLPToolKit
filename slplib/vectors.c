/*
 *  vectors.c - Common operations on integer vectors
 *
 *  Created on: 25/mar/2014
 *  Author: Wiliam Capraro - wiliam.capraro@studenti.unimi.it
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "bitutils.h"
#include "fsmparseutils.h"
#include "parseutils.h"
#include "miscutils.h"
#include "hashing.h"


/**
 * Yields the p-norm of integer vector v of
 * size s
 */
float norm(int[] v, int s, int p) {
	
	int i;
	float n = 0f;

	for (i=0; i<s; i++) {
		n += math.pow(math.abs(v[i]), p);	
	}

	n = math.pow(n, 1.0/p);
	return (n);
}



/**
 * Yields the Euclidean norm of vector v of size s
 */
float norm2(int[] v, int s) {
	return (norm(v, s, 2))
}


/*
 * TODO: for each candidate x, compute the new H(x) vector. then pass <x[], H(x)[]> as arguments to the tie-break 
 * function . Tie-break((int*)[], int count, int[] newH, )
 */





