/*
 * bastack.h
 *
 *  Created on: 16/mag/2013
 *   Author: Wiliam Capraro - wiliam.capraro@studenti.unimi.it
 */


#include <stdio.h>
#include <stdlib.h>
#include "bitutils.h"


typedef struct {
	struct t_stackitem *prev;
	struct t_stackitem *next;
	t_bitarray *ba;
} t_stackitem;



typedef struct {
	int num_elements;
	bool xn_flg, and_flg;
	t_stackitem *root;
	t_stackitem *last;
} t_bastack;


t_bastack *empty_stack();
int size(t_bastack*);
void wipeitem(t_stackitem*);
void wipestack(t_bastack*);
void push(t_bastack*, t_bitarray*);
t_bitarray *pop(t_bastack*);
void stack_set_and_flg(t_bastack*, bool);
void stack_set_xn_flg(t_bastack*, bool);
bool stack_is_and_flg_set(t_bastack*);
bool stack_is_xn_flg_set(t_bastack*);
