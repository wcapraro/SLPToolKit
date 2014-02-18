/*
 * fsmparseutils.c - simple FSM-based parser for straight line programs
 *
 *  Created on: 01/giu/2013
 *  Author: Wiliam Capraro - wiliam.capraro@studenti.unimi.it
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "fsmparseutils.h"
#include "parseutils.h"




int fsm[NUM_STATES][NUM_STATES];


void init_fsm() {
	int i, j;
	for (i=0; i<NUM_STATES; i++)
		for (j=0; j<NUM_STATES; j++) {
			fsm[i][j]=0;
			if (i != END || i != ERR)
				if (j == IGN || j == ERR)
					fsm[i][j]=1;
		}

	fsm[STT][LEN]=1;
	fsm[LEN][IN]=1;
	fsm[IN][OUT]=1;
	fsm[OUT][PROG]=1;
	fsm[IGN][LEN]=1;
	fsm[IGN][IN]=1;
	fsm[IGN][OUT]=1;
	fsm[IGN][PROG]=1;
	fsm[IGN][END]=1;
	fsm[PROG][PROG]=1;
	fsm[PROG][END]=1;
}



t_fsm *fsmparser() {
	t_fsm *f = malloc(sizeof(t_fsm));
	if (f) {
		f->state=STT;
	}
	else {
		fprintf(stderr, "fsm() :: memory allocation error");
	}
	init_fsm();
	return f;
}



int next(t_fsm *f, int s) {
	if (f && fsm[f->state][s]) {
		f->state = s;
		return 1;
	}
	return 0;
}


int getstate(t_fsm *f) {
	if (f)
		return f->state;
	return -1;
}


void fsmfree(t_fsm *f) {
	if (f) free(f);
}



