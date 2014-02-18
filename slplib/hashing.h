/*
 * hashing.h
 *
 *  Created on: 18/giu/2013
 *  Revisited: 09/oct/2013
 *  Author: Wiliam Capraro - wiliam.capraro@studenti.unimi.it
 */

#ifndef HASHING_H_
#define HASHING_H_

#include "clause.h"



typedef struct {
	t_clause *clause;
	struct t_hashslot *next;
} t_hashslot;



typedef struct {
	int size;
	t_hashslot **content;
} t_hashtable;



t_hashslot *hashslot(t_clause*);
t_hashtable *hashtable(int);
void wipe_hashtable(t_hashtable*, bool deep);
void put_clause(t_hashtable*, t_clause*);
t_clause *find_clause(t_hashtable*, char*);
t_clause **get_clauses(t_hashtable*);
unsigned int hash(char*, int);



#endif /* HASHING_H_ */
