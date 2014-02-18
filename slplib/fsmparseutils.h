/*
 * fsmparseutils.h
 *
 *  Created on: 01/giu/2013
 *  Author: Wiliam Capraro - wiliam.capraro@studenti.unimi.it
 */

#ifndef FSMPARSEUTILS_H_
#define FSMPARSEUTILS_H_

#define COMMENT_CHAR '#'
#define INPUT_CHAR 'x'
#define OUTPUT_CHAR 'y'
#define TEMP_CHAR 't'

#define STT 0
#define LEN 1
#define IN 2
#define OUT 3
#define IGN 4
#define PROG 5
#define END 6
#define ERR 7
#define NUM_STATES 8


typedef struct {
	int state;
} t_fsm;


t_fsm *fsmparser();
int next(t_fsm*, int);
int getstate(t_fsm*);
void fsmfree(t_fsm*);


#endif /* FSMPARSEUTILS_H_ */
