/*
 * miscutils.h
 *
 *  Created on: 06/giu/2013
 *  Revisited: 09/oct/2013
 *  Author: Wiliam Capraro - wiliam.capraro@studenti.unimi.it
 */

#ifndef MISCUTILS_H_
#define MISCUTILS_H_


#define __DEBUG__

#ifdef __DEBUG__
#define dbgprint(x) fprintf(stderr, (x));
#endif


#define ERR_ARGS 1
#define ERR_IO 2
#define ERR_PARSE 3
#define ERR_ALLOC 4
#define PROG_VERSION "v0.2__09102013"


#endif /* MISCUTILS_H_ */
