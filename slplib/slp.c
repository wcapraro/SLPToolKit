/*
 *  slp.c -- defines structures and functions for representing
 *  and manipulating straight-line programs as a sequence of
 *  clauses
 *
 *  Created on: 26/apr/2014
 *  Author: Wiliam Capraro - wiliam.capraro@studenti.unimi.it
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "slp.h"


void slp_print_signal(t_signal*, FILE*);
void slp_wipe_signal(t_signal*, FILE*);
void slp_for_each_signal(t_slp*, void (*f) (t_signal*, FILE*), FILE*);



t_slp *new_slp() {
	t_slp *slp = malloc(sizeof(t_slp));
	if (slp) {
		slp->size = 0;
		slp->inputs = 0;
		slp->outputs = 0;
		slp->desc = NULL;
		slp->root_signal = NULL;
		slp->tail_signal = NULL;
	}
	return (slp);
}



void wipe_slp(t_slp *slp) {
	if (slp) {
		slp_for_each_signal(slp, &slp_wipe_signal, NULL);
		free(slp);
	}
}



int slp_size(t_slp *slp) {
	if (slp)
		return slp->size-slp_count_outputs(slp);
	return (-1);
}


int slp_count_inputs(t_slp *slp) {
	if (slp)
		return (slp->inputs);
	return (-1);
}


int slp_count_outputs(t_slp *slp) {
	if (slp)
		return (slp->outputs);
	return (-1);
}


void slp_add_signal(t_slp *slp, char *signal) {

	t_signal *new_signal;

	if (slp) {

		// Create a new signal
		new_signal = malloc(sizeof(t_signal));
		if (!new_signal) {
			fprintf(stderr, "@! Could not allocate heap for new signal \'%s\'\n", signal);
			return;
		}
		new_signal->signal = signal;
		new_signal->next = NULL;

		if (slp->root_signal && slp->tail_signal && slp->size) {
			// SLP is not empty
			slp->tail_signal->next = new_signal;
			slp->tail_signal = new_signal;
			slp->size = 1+slp->size;
		}

		else {
			// SLP is currently empty
			slp->root_signal = new_signal;
			slp->tail_signal = new_signal;
			slp->size = 1;
		}
	}
}



void slp_print(t_slp *slp, FILE *to) {

	int _k;

	if (slp) {

		// Use stdout if no file pointer is specified
		to = (to) ? to : stdout;

		// Preamble
		fprintf(to, "\n#\n# %s\n#\n\n", slp->desc);
		fprintf(to, "%d gates\n", slp_size(slp));
		fprintf(to, "%d inputs\n", slp->inputs);
		for (_k=0; _k<slp->inputs; _k++)	fprintf(to, "X%d ", _k); fprintf(to, "\n");
		fprintf(to, "%d outputs\n", slp->outputs);
		for (_k=0; _k<slp->outputs; _k++)	fprintf(to, "Y%d ", _k); fprintf(to, "\n");

		// SLP
		fprintf(to, "\nBEGIN\n");
		slp_for_each_signal(slp, &slp_print_signal, to);
		fprintf(to, "END\n\n");
	}
}



void slp_for_each_signal(t_slp *slp, void (*func)(t_signal*, FILE*), FILE *to) {

	t_signal *cur;
	t_signal *next;

	if (slp && slp->size && slp->root_signal) {
		cur = slp->root_signal;
		next = cur->next;

		while (cur) {
			func(cur, to);
			cur = next;
			next = (cur) ? cur->next : cur;
		}
	}
}



void slp_wipe_signal(t_signal *sgn, FILE *to) {
	if (sgn) {
		if (sgn->signal)	free(sgn->signal);
		free (sgn);
	}
}



void slp_print_signal(t_signal *sgn, FILE *to) {
	if (sgn && sgn->signal) {
		fprintf(to, "%s\n", sgn->signal);
	}
}



