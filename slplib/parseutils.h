/*
 *  parseutils.h
 *
 *  Created on: 13/mag/2013
 *  Revisited: 20/nov/2013
 *  Author: Wiliam Capraro - wiliam.capraro@studenti.unimi.it
 */

#ifndef PARSEUTILS_H_
#define PARSEUTILS_H_


#include <stdio.h>
#include "clause.h"
#include "hashing.h"



#define AND "x"
#define XOR "+"
#define XNOR "XNOR"
#define LINLEN 1000
#define TOKLEN 50
#define OPLEN 5
#define IN_HASHSIZE 64
#define OUT_HASHSIZE 64
#define TMP_HASHSIZE 128


/* function prototypes */
int streq(const char*, const char*);
void compile(FILE*, t_clause***, t_clause***, t_clause***, int*, char*);
t_clause *lookup(char*, t_hashtable*, t_hashtable*, t_hashtable*);
int lincpy(const char*, char*, const char*);



#endif /* PARSEUTILS_H_ */
