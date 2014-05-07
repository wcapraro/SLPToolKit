/*
 *  slp.h
 *
 *  Created on: 26/apr/2013
 *  Author: Wiliam Capraro - wiliam.capraro@studenti.unimi.it
 */

#ifndef SLP_H_
#define SLP_H_


#include <stdio.h>


typedef struct t_signal {
	char *signal;
	struct t_signal *next;
} t_signal;


typedef struct t_slp {
	int size;
	int inputs;
	int outputs;
	char *desc;
	t_signal *root_signal;
	t_signal *tail_signal;
} t_slp;


t_slp *new_slp();
int slp_size(t_slp*);
int slp_count_inputs(t_slp*);
int slp_count_outputs(t_slp*);
void slp_add_signal(t_slp*, char*);
void wipe_slp(t_slp*);
void slp_print(t_slp*, FILE*);



#endif /* PARSEUTILS_H_ */
