/*
 *  bastack.c
 *
 *  Created on: 16/mag/2013
 *  Revisited: 09/oct/2013
 *  Author: Wiliam Capraro - wiliam.capraro@studenti.unimi.it
 */


#include <stdio.h>
#include <stdlib.h>
#include "bitutils.h"
#include "bastack.h"



t_bastack *empty_stack() {
	t_bastack *stack = malloc(sizeof(t_bastack));
	if (stack) {
		stack->num_elements = 0;
		stack->xn_flg = FALSE;
		stack->and_flg = FALSE;
		stack->root = NULL;
		stack->last = NULL;
	} else {
		fprintf(stderr, "-- empty_stack() :: memory allocation failure\n");
	}
	return stack;
}



int size(t_bastack *stack) {
	if (stack) return stack->num_elements;
	return -1;
}



void wipeitem(t_stackitem *item) {
	if (item) {
		//printf("@@@ wipeitem(%x)\n", item);
		wipe(item->ba);
		free(item);
	}
}



void wipestack(t_bastack *stack) {
	if (stack) {
		//printf("@@@ wipestack %x\n", stack);
		t_stackitem *item = stack->last;
		t_stackitem *prev;
		while (item) {
			prev = item->prev;
			wipeitem(item);
			item = prev;
		}
	}
}



void push(t_bastack *stack, t_bitarray *ba) {
	if (stack && ba) {
		t_stackitem *node = (t_stackitem*)malloc(sizeof(t_stackitem));
		if (node) {
			node->next=NULL;
			node->ba=ba;
			if (size(stack)) {
				t_stackitem *last = stack->last;
				node->prev=last;
				last->next=node;
				stack->last=node;
			} else {
				// stack is currently empty
				stack->root=node;
				stack->last=node;
				node->prev=NULL;
				node->next=NULL;
			}
			stack->num_elements++;
		} else {
			// could not create node
			fprintf(stderr, "push() :: could not allocate memory for new node");
		}
	}
}



t_bitarray *pop(t_bastack *stack) {
	t_bitarray *ba = NULL;
	if (stack && size(stack) > 1) {
		t_stackitem *last = stack->last;
		t_stackitem *prev = last->prev;
		ba = last->ba;
		prev->next = NULL;
		free(last);
		stack->last = prev;
		stack->num_elements--;
	}
	else if (stack && size(stack) == 1) {
		t_stackitem *last = stack->last;
		ba = last->ba;
		stack->root = NULL;
		stack->last = NULL;
		stack->num_elements--;
		free(last);
	}
	return ba;
}


void stack_set_and_flg(t_bastack *stack, bool value) {
	if (stack) {
		stack->and_flg = value;
	}
}


void stack_set_xn_flg(t_bastack *stack, bool value) {
	if (stack) {
		stack->xn_flg = value;
	}
}


bool stack_is_and_flg_set(t_bastack *stack) {
	return (bool)( stack && stack->and_flg );
}


bool stack_is_xn_flg_set(t_bastack *stack) {
	return (bool)( stack && stack->xn_flg );
}


