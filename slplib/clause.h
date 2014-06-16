/*
 * clause.h
 *
 *  Created on: 13/mag/2013
 *  Author: Wiliam Capraro - wiliam.capraro@studenti.unimi.it
 */

#ifndef CLAUSE_H_
#define CLAUSE_H_


#include "bitutils.h"


typedef struct t_clause {
	struct t_clause *children[2];
	char *operator;
	char *name;
	bool value;
	unsigned int number;
} t_clause;



t_clause *empty_clause();
t_clause *clause(t_clause*, t_clause*, char*, char*, int);
t_clause *get_child(t_clause*, int);
int set_child(t_clause*, t_clause*, int);
bool get_value(t_clause*);
void set_value(t_clause*, bool);
int has_value(t_clause*);
char *get_operator(t_clause*);
char *get_clause_name(t_clause*);
void clause_copy(t_clause*, t_clause*);
int isinput(t_clause*);
void print_clause(t_clause*);
void evaluate(t_clause*);
int enumerate(t_clause*);
void wipe_clause(t_clause*);
void reset_clause(t_clause*);


#endif /* CLAUSE_H_ */
