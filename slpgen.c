
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "slplib.h"



#define PROG_NAME "SLPGen"
#define SPLIT_XOR "=+ "
#define SPLIT_AND "*"
#define TMP_FNAME "__((slpgen)).tmp"
#define BUF_SIZE 100



t_clause *remember(t_hashtable*, char*, int*);
int tokenize(const char*, const char*, char***);
void wipe_tokens(char**);
void fcpy(char*, char*, t_hashtable*, int, t_hashtable*, int, int);
void usage();


// TODO:
// check names for temp clauses.. (if known symbol, then print error msg)
// the *leftovers* buffer should not have fixed size ;-)




int main(int argc, char **argv) {
	int f, i, j, count, nx, ny, nt;
	int nxors, nands;
	bool xn_flg;
	t_hashtable *xs, *ys, *ts;
	t_clause *clause;
	FILE *pftemp, *pfout;
	char line[LINLEN];
	char copy[LINLEN];
	t_clause **leftovers;
	char **xors, **ands;
	char *tok1, *tok2, cname[11], *fout=NULL;
	char tname = 't';

	/* parse command line options (see the getopt tool) */
	if (argc < 3) {
		usage();
		exit(EXIT_SUCCESS);
	}

	while ((f=getopt(argc, argv, "o:t:h")) != -1) {
		switch(f) {
		case 'h':
			usage();
			exit(0);
		case 'o':
			fout = optarg;
			break;
		case 't':
			tname = optarg[0];
			break;
		default:
			printf("Unknown option '%c'\n", f);
			exit(ERR_PARSE);
		}
	}


	// init globals
	count=0;
	nx=0;
	ny=0;
	nt=0;
	xs=hashtable(IN_HASHSIZE);
	ys=hashtable(OUT_HASHSIZE);
	ts=hashtable(TMP_HASHSIZE);
	pftemp=fopen(TMP_FNAME, "w");
	if (!(xs && ys && ts && pftemp)) {
		fprintf(stderr, "--%s : init error. Program will now exit\n", PROG_NAME);
		if (xs)		wipe_hashtable(xs, TRUE);
		if (ys)		wipe_hashtable(ys, TRUE);
		if (ts)		wipe_hashtable(ts, TRUE);
		if (pftemp)	fclose(pftemp);
		exit(ERR_IO);
	}

	// prompt the user to enter equations
	printf("\n\nEnter equations (hit return or type end to finish):\n\n");
	while (fgets(line, LINLEN, stdin)) {

		// check stop condition
		if (streq(line, "\n") || streq(line, "end\n"))
			break;

		// init leftovers buffer and make disposable copy of line
		j=0;
		xn_flg = FALSE;
		leftovers = malloc(BUF_SIZE*sizeof(t_clause*));
		for (i=BUF_SIZE-1; i+1; i--)
			leftovers[i] = NULL;


		// remember output symbol if not already known
		tok1 = strpbrk(line, "=");
		if (!tok1) {
			fprintf(stderr, "--%s : illegal line\n", PROG_NAME);
			continue;
		}
		lincpy(tok1, copy, "\n ");
		*(tok1) = '\0';
		if (*(tok1--) == ' ')	*tok1 = '\0';
		lincpy(line, line, " ");
		tok2=line;
		clause = find_clause(ys, tok2);

		if (clause) {
			fprintf(stderr, "--%s : duplicate symbol (%s)\n", PROG_NAME, line);
			continue;
		}

		clause = remember(ys, tok2, &ny);


		// tokenize and proceed
		nxors = tokenize(copy, SPLIT_XOR, &xors);
		if (!xors) {
			fprintf(stderr, "--%s : memory allocation error\n", PROG_NAME, PROG_VERSION);
			exit(ERR_ALLOC);
		}


		for (i=0; (tok1=xors[i]); i++) {

			// set XNOR flag if necessary
			if (streq(tok1, "1")) {
				xn_flg = TRUE-xn_flg;
			}

			else

			{
				nands = tokenize(tok1, SPLIT_AND, &ands);
				if (!ands) {
					fprintf(stderr, "--%s : memory allocation error\n", PROG_NAME, PROG_VERSION);
					exit(ERR_ALLOC);
				}

				if (nands == 1) {
					// just remember the variable "as is"
					clause = remember(xs, tok1, &nx);
					leftovers[j++]=clause;
					nands--;
					continue;
				}

				if (nands >= 2) {
					tok1 = ands[--nands];
					clause = remember(xs, tok1, &nx);
					tok2 = ands[--nands];
					clause = remember(xs, tok2, &nx);

					// print the clause
					fprintf(pftemp, "t%d = %s x %s\n", count++, tok1, tok2);
				}

				while (nands) {
					tok1=ands[--nands];
					clause = remember(xs, tok1, &nx);
					fprintf(pftemp, "t%d = t%d x %s\n", count, count-1, tok1);
					count++;
				}

				// remember last t
				sprintf(cname, "t%d", count-1);
				clause = remember(ts, cname, &nt);
				leftovers[j++]=clause;

			} // end inner for loop

		} // end outer for loop


		// now j is the number of elements to XOR
		if (j > 1) {
			char *op = (xn_flg) ? "XNOR" : "+";

			fprintf(pftemp, "t%d = %s %s %s\n",
					count++,
					get_clause_name(leftovers[--j]),
					op,
					get_clause_name(leftovers[--j]));

			while (j) {
				fprintf(pftemp, "t%d = t%d + %s\n", count++, count-1, get_clause_name(leftovers[--j]));
			}

			// remember last variable
			sprintf(cname, "t%d", count-1);
			clause = remember(ts, cname, &nt);
			leftovers[0] = clause;
		}

		fprintf(pftemp, "%s = %s\n", line, get_clause_name(leftovers[0]));



		// clean up ...
		free(leftovers);
		wipe_tokens(xors);
		wipe_tokens(ands);

	} // end main loop


	// create output file and cleanup
	fclose(pftemp);
	fcpy(TMP_FNAME, fout, xs, nx, ys, ny, count);

	wipe_hashtable(xs, TRUE);
	wipe_hashtable(ys, TRUE);
	wipe_hashtable(ts, TRUE);

	return EXIT_SUCCESS;
}




/*
 * Given an input string and separators, generates an array of
 * tokens. Each token is a dynamically allocated, NULL-terminated
 * string. The last element of the array is a sentinel NULL
 * pointer. The returned array (and all the strings in it) must
 * be deallocated by the caller.
 *
 * In case of errors, NULL is returned.
 */
int tokenize(const char *input, const char *sep, char ***dest) {
	char* dup;
	char** toks = 0;
	char* cur_tok;
	int size = 2;
	int ntok = 0;
	int i;

	if (!(dup = strdup(input)))
		return -1;

	if (!(toks = malloc(size * sizeof(*toks)))) {
		fprintf(stderr, "-- tokenize() : memory allocation error\n");
		return -1;
	}

	cur_tok = strtok(dup, sep);

	// while we have more tokens to process...
	while (cur_tok)
	{
		// we should still have 2 empty elements in the array,
		if (ntok > size - 2)
		{
			char** newtoks;
			size *= 2;

			newtoks = realloc(toks, size * sizeof(*toks));

			if (!newtoks){
				fprintf(stderr, "-- tokenize() : memory allocation error\n");
				free(dup);
				for (i = 0; i < ntok; ++i)
					free(toks[i]);
				free(toks);
				return -1;
			}

			toks = newtoks;
		}

		// now the array is definitely large enough, so we just
		// copy the new token into it.
		toks[ntok] = strdup(cur_tok);

		if (!toks[ntok]) {
			fprintf(stderr, "-- tokenize() : memory allocation error\n");
			free(dup);
			for (i = 0; i < ntok; ++i)
				free(toks[i]);
			free(toks);
			return -1;
		}

		ntok++;
		cur_tok = strtok(0, sep);
	}

	free(dup);
	toks[ntok] = 0;

	// assign token array to dest and return ntok
	(*dest) = toks;
	return ntok;

}



t_clause *remember(t_hashtable *map, char *cname, int *counter) {
	t_clause *cl = NULL;
	if (map && cname) {
		cl = find_clause(map, cname);
		if (!cl) {
			cl = clause(NULL, NULL, NULL, cname, 0);
			put_clause(map, cl);
			(*counter)++;
		}
	}
	return cl;
}



void wipe_tokens(char **tokens) {
	int i=0;
	if (tokens) {
		while (tokens[i])
			free(tokens[i++]);
		free(tokens);
	}
}


/*
 * Writes the program to the output file
 */
void fcpy(char *fin, char *fout, t_hashtable *xs, int nx, t_hashtable *ys, int ny, int nt) {
	FILE *src, *dest;
	t_clause **clauses;
	char l[LINLEN];
	if (fin && fout && xs && ys) {
		src = fopen(fin, "r");
		dest = fopen(fout, "w");

		if (!src || !dest) {
			fprintf(stderr, "--fcpy() : could not open file\n");
			return;
		}

		// print preamble to destination file
		fprintf(dest, "#\n# Generated with %s %s\n#\n\n", PROG_NAME, PROG_VERSION);
		fprintf(dest, "%d gates\n", nt);
		fprintf(dest, "%d inputs\n", nx);
		clauses = get_clauses(xs);
		while (nx)	fprintf(dest, "%s ", get_clause_name(clauses[--nx])); free(clauses);
		fprintf(dest, "\n%d outputs\n", ny);
		clauses = get_clauses(ys);
		while (ny)	{
			//printf(">> clause @%d --> \"%s\"\n", clauses[ny-1], clauses[ny-1]->name);
			fprintf(dest, "%s ", get_clause_name(clauses[--ny]));
		}
		free(clauses);
		fprintf(dest, "\n\nBEGIN\n");

		while (fgets(l, LINLEN, src)) {
			fprintf(dest, l);
		}

		fprintf(dest, "END\n\n");
		fclose(src);
		fclose(dest);
		remove(TMP_FNAME);
	}
}


/*
 * Simple usage message
 */
void usage() {
	printf("\n%s %s -- Straight-line program generator\n\n", PROG_NAME, PROG_VERSION);
	printf("slpgen [optlist]\n\n");
	printf("List of options:\n");
	printf("  -o <file>\tprint output to <file> (mandatory)\n");
	printf("  -t <char>\tuse <char> to name temporary clauses\n");
	printf("  -h\t\tshows this message\n\n");
}


