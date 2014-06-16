/*
 *  clause.c -- defines structures and functions for representing and
 *  manipulating clauses over GF(2) using the basis (x, +, XNOR)
 *
 *  Created on: 13/mag/2013
 *  Revisited: 20/nov/2013
 *  Author: Wiliam Capraro - wiliam.capraro@studenti.unimi.it
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitutils.h"
#include "parseutils.h"
#include "clause.h"


#define SPLIT_CHARS "\t\n =;"



/**
 * Returns an empty clause, or NULL in case of memory allocation failure
 */
t_clause *empty_clause() {
	t_clause *c = malloc(sizeof(t_clause));
	if (c)
	{
		c->children[0]=NULL;
		c->children[1]=NULL;
		c->operator=NULL;
		c->name=NULL;
		c->value=INVALID_BOOL_VALUE;
		c->number=NULL;
	}
	return c;
}




/**
 * Returns the clause represented by the given parameters, or
 * NULL in case of memory allocation failure
 */
t_clause *clause(t_clause *ch1, t_clause *ch2, char *op, char *cname, int num) {
	t_clause *c = empty_clause();
	if (c) {
		c->children[0]=ch1;
		c->children[1]=ch2;
		c->operator = (op) ? strdup(op) : NULL;
		c->name=strdup(cname);
		c->number=num;
	}

	return c;
}




t_clause *get_child(t_clause *clause, int index) {
	if (!clause || index < 0 || index > 1)
		return NULL;
	else
		return clause->children[index];
}



int set_child(t_clause *clause, t_clause *child, int index) {
	if (!clause || index < 0 || index > 1)
		return 0;
	else {
		clause->children[index]=child;
		return 1;
	}
}



void set_value(t_clause *clause, bool b) {
	if (clause)
	{
		clause->value = b;
	}
}



bool get_value(t_clause *clause) {
	bool b = INVALID_BOOL_VALUE;
	if (clause) {
		b = clause->value;
	}
	return b;
}



int has_value(t_clause *clause) {
	if (clause) {
		return get_value(clause) != INVALID_BOOL_VALUE;
	}
	return 0;
}



char *get_operator(t_clause *clause) {
	char *op = NULL;
	if (clause) {
		op = clause->operator;
	}
	return op;
}



char *get_clause_name(t_clause *clause) {
	char *cname = NULL;
	if (clause) {
		cname = clause->name;
	}
	return cname;
}



void clause_copy(t_clause *src, t_clause *dest) {
	if (src && dest) {
		dest->children[0] = src->children[0];
		dest->children[1] = src->children[1];
		free(dest->operator);
		dest->operator = (src->operator)? strdup(src->operator) : NULL;
		dest->value = src->value;
		if (isinput(src)) {
			dest->number = src->number;
			dest->children[0] = src;
		}
	}
}



/**
 * Returns 1 iff the clause c in an input clause
 */
int isinput(t_clause *c) {
	return (c && !c->operator);
}




/**
 * (Recursively) prints a clause
 */
void print_clause(t_clause *c) {
	if (c) {
		if (!isinput(c)) {
			t_clause *ch1=c->children[0];
			t_clause *ch2=c->children[1];
			printf("%s ( ", c->operator);
			print_clause(ch1);
			printf(", ");
			print_clause(ch2);
			printf(" )");
		} else {
			printf("%s", c->name);
		}
	}
}



/*
 * (Recursively) evaluates a clause. Assumes each input variable
 * has a valid value associated with it.
 */
void evaluate(t_clause *clause) {
	bool r = INVALID_BOOL_VALUE;
	bool a, b;
	char *op;

	if (clause && !isinput(clause))
	{
		// the operator?
		op = clause->operator;

		// avoid evaluating sub-clauses multiple times
		if (!has_value(get_child(clause, 0)))	evaluate(get_child(clause, 0));
		if (!has_value(get_child(clause, 1)))	evaluate(get_child(clause, 1));
		a = get_value(get_child(clause, 0));
		b = get_value(get_child(clause, 1));

		// evaluation logic
		if (!strcasecmp(op, XOR)) r = a ^ b;
		if (!strcasecmp(op, XNOR)) r = !(a ^ b);
		if (!strcasecmp(op, AND)) r = a & b;

		// store the value
		set_value(clause, r);
	}
}



int enumerate(t_clause *clause) {
	int n = -1;
	if (clause) {
		n = clause->number;
	}
	return n;
}



/*
 * Destroys a clause
 */
void wipe_clause(t_clause *clause) {
	if (clause) {
		//printf("wipe_clause(%x)\n", clause);
		if (clause->operator) free(clause->operator);
		if (clause->name) free(clause->name);
		free(clause);
	}
}


/*
 * Resets one clause's value
 */
void reset_clause(t_clause *clause) {
	if (clause && clause->value != INVALID_BOOL_VALUE) {
		clause->value = INVALID_BOOL_VALUE;
	}
}




