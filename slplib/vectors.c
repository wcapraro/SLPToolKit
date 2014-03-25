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
#include <math.h>


/**
 * Yields the p-norm of integer vector v of
 * size s
 */
float norm(int v[], int s, int p) {
	
	int i;
	float n = 0.0f;

	for (i=0; i<s; i++) {
		n += pow(abs(v[i]), p);
	}

	n = pow(n, 1.0/p);
	return (n);
}



/**
 * Yields the Euclidean norm of vector v of size s
 */
float norm2(int v[], int s) {
	return (norm(v, s, 2));
}



/**
 * Returns the sum of the components of the
 * given vector
 */
int vsum(int v[], int s) {
	int i;
	int z;
	for(i=0,z=0; i<s; z+=v[i++]);
	return (z);
}








