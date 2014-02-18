/*
 *  parseutils.c
 *
 *  Created on: 01/giu/2013
 *  Revisited: 20/nov/2013
 *  Author: Wiliam Capraro - wiliam.capraro@studenti.unimi.it
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "bitutils.h"
#include "fsmparseutils.h"
#include "parseutils.h"
#include "miscutils.h"
#include "hashing.h"




/* used for string processing */
char MAP[256];



/*
 * Copies string src into dest, skipping chars in skip.
 * Returns the length of the destination string
 */
int lincpy(const char *src, char *dest, const char *skip) {

	int i, l=0;
	for (i=0; i<256; i++) 			MAP[i]=FALSE;
	for (i=0; i<strlen(skip); i++)	MAP[(int)skip[i]]=TRUE;

	for (i=0; i<strlen(src); i++)
		if (!MAP[(int)src[i]]) {
			dest[l++]=src[i];
			if (src[i] == COMMENT_CHAR) {
				dest[--l]='\0';
				break;
			}
		}

	if (dest[l] != '\0') {
		dest[l] = '\0';
	}

	return l;
}


/*
 * Reads each line from the input file, then malloc()s and populates main
 * data structures X, T, Y
 */
void compile(FILE *fin, t_clause ***X, t_clause ***T, t_clause ***Y, int *params, char *xs) {
	int r, value;
	int count = 0;
	t_fsm *fsmp = fsmparser();
	char line[LINLEN];
	char copy[LINLEN];
	char *token;
	char *op;
	char c;
	t_clause *cl;

	// used for keeping track of input and output symbols
	t_hashtable *inputs = hashtable(IN_HASHSIZE);
	t_hashtable *outputs = hashtable(OUT_HASHSIZE);
	t_hashtable *temps = hashtable(TMP_HASHSIZE);


	/* check hashtable pointers */
	if (!inputs || !outputs || !temps) {
		fprintf(stderr, "-- compile() : could not init hashtables\n");
		return;
	}

	/* process each line at a time */
	while (fgets(line, LINLEN, fin)) {
		int l = lincpy(line, copy, "\n\t\r;");

		if (l > 1) {

			/* initialize data structures */
			if (isdigit((c=copy[0]))) {

				token = strtok(copy, " ");
				value = atoi(token);
				token = strtok(NULL, " ");

				if (streq(token, "gates") || streq(token, "gate")) {
					*T = malloc(value*sizeof(t_clause*));
					params[2]=value;
					r = next(fsmp, LEN);
				} else if (streq(token, "inputs")) {
					*X = malloc(value*sizeof(t_clause*));
					params[0]=value;
					r = next(fsmp, IN);
				} else if (streq(token, "outputs")) {
					*Y = malloc(value*sizeof(t_clause*));
					params[1]=value;
					r = next(fsmp, OUT);
				} else
					r = next(fsmp, ERR);
			}

			/* line == 'begin' or line == 'end' */
			else if (streq(copy, "begin")) 		r = next(fsmp, PROG);
			else if (streq(copy, "end"))		r = next(fsmp, END);

			/* list of input symbols */
			else if (getstate(fsmp) == IN) {
				token=strtok(copy, " ");
				while (token)	{
					cl = clause(NULL, NULL, NULL, token, count);
					printf("0x%x @ %s ", cl, get_clause_name(cl));
					if (xs) {
						int xval = -48;
						bool b;
						xval += (count < strlen(xs)) ? xs[count] : 48;
						b = (!xval) ? FALSE : TRUE;
						set_value(cl, b);
						printf("= %d", b);
					}
					(*X)[count++] = cl;
					put_clause(inputs, cl);
					printf("\n");
					token = strtok(NULL, " ");
				}
				count=0;
			}

			/* list of output symbols */
			else if (getstate(fsmp) == OUT) {
				token = strtok(copy, " ");
				while (token) {
					cl = clause(NULL, NULL, NULL, token, count++);
					put_clause(outputs, cl);
					printf("0x%x @ %s\n", cl, get_clause_name(cl));
					token=strtok(NULL, " ");
				}
				count=0;
			}

			/* parse clauses */
			else if (getstate(fsmp) == PROG) {

				token = strtok(copy, "= ");
				t_clause *target = find_clause(outputs, token);
				char *lcname, *rcname, *op;
				t_clause *lcl, *rcl;
				lcname = strtok(NULL, "= ");
				op = strtok(NULL, "= ");
				rcname = strtok(NULL, "= ");

				/* composite output clause */
				if (op && lcname && rcname && target) {

					// find left and right operands
					//lcl = find_clause(inputs, lcname) ? find_clause(inputs, lcname) : find_clause(temps, lcname);	
					//rcl = find_clause(inputs, rcname) ? find_clause(inputs, rcname) : find_clause(temps, rcname);
					lcl = lookup(lcname, inputs, outputs, temps);
					rcl = lookup(rcname, inputs, outputs, temps);

					if (!lcl || !rcl) {
						r = next(fsmp, ERR);
						continue;
					}

					cl = clause(lcl, rcl, op, token, count);
					printf("0x%x @ %s = %s %s %s\n", target, get_clause_name(target), get_clause_name(lcl), op, get_clause_name(rcl));
					int num = enumerate(target);
					clause_copy(cl, target);
					(*Y)[num] = target;

				}

				/* simple output clause */
				else if (target && lcname) {

					//lcl = find_clause(inputs, lcname) ? find_clause(inputs, lcname) : find_clause(temps, lcname);
					lcl = lookup(lcname, inputs, outputs, temps);

					if (!lcl) {
						r = next(fsmp, ERR);
						continue;
					}

					printf("0x%x @ %s = %s\n", target, get_clause_name(target), get_clause_name(lcl));
					int num = enumerate(target);
					clause_copy(lcl, target);
					(*Y)[num] = target;
				}


				/* just a temp clause ?! */
				else

				{
					//char *lcname, *rcname, *op;
					//t_clause *lcl, *rcl;
					//lcname = strtok(NULL, "= ");
					//op = strtok(NULL, "= ");
					//rcname = strtok(NULL, "= ");

					//lcl = find_clause(inputs, lcname) ? find_clause(inputs, lcname) : find_clause(temps, lcname);
					//rcl = find_clause(inputs, rcname) ? find_clause(inputs, rcname) : find_clause(temps, rcname);
					lcl = lookup(lcname, inputs, outputs, temps);
					rcl = lookup(rcname, inputs, outputs, temps);


					if (!lcl || !rcl) {
						r = next(fsmp, ERR);
						continue;
					}

					cl = clause(lcl, rcl, op, token, count);
					put_clause(temps, cl);
					(*T)[count++]=cl;
					printf("0x%x @ %s\n", cl, get_clause_name(cl));
				}

			}

			else
			{
				/* invalid line */
				r = next(fsmp, ERR);
			}
		}

		else
		{
			/* l <= 1 means line is comment or blank */
			r = next(fsmp, IGN);
		}

		/* valid transaction? */
		if (!r || getstate(fsmp) == ERR) {
			fprintf(stderr, "compile() :: parsing error (r=%d\ts=%d)\n", r, getstate(fsmp));
			if (*T) 		free(*T);
			if (*Y) 		free(*Y);
			if (*X) 		free(*X);
			exit(1);
		}

	}


	// get rid of unnecessary hashtables
	// note that this won't affect clauses, which
	// remain available through X, Y, and T
	wipe_hashtable(inputs, FALSE);
	wipe_hashtable(outputs, FALSE);
	wipe_hashtable(temps, FALSE);
}


/*
 * Looks up a given clause name into the specified hashtables
 */
t_clause *lookup(char *cname, t_hashtable *xs, t_hashtable *ys, t_hashtable *ts) {
	t_clause *clause = NULL;
	clause = find_clause(xs, cname);
	if (!clause) {
		clause = find_clause(ys, cname);
		if (!clause)
			clause = find_clause(ts, cname);
	}
	return clause;
}


/*
 * Returns 1 iff str1 is equal to str2 (case insensitive)
 */
int streq(const char *str1, const char *str2) {
	return (!strcasecmp(str1, str2));
}


