/*
 *  hashing.c
 *
 *  Created on: 18/giu/2013
 *  Author: Wiliam Capraro - wiliam.capraro@studenti.unimi.it
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "hashing.h"



/*
 * Returns a reference to a new hashtable with the specified
 * size, or NULL if some error occurs;
 */
t_hashtable *hashtable(int size) {
	t_hashtable *tab;
	tab = malloc(sizeof(t_hashtable));
	if (tab) {
		tab->size = size;
		tab->content = malloc(size*sizeof(t_hashslot*));
		if (!tab->content) {
			free(tab);
			tab = NULL;
		} else {
			while (size--) {
				tab->content[size] = NULL;
			}
		}
	}
	return tab;
}



/*
 * Returns a reference to a new slot containing the specified
 * clause, or NULL if some error occurs
 */
t_hashslot *hashslot(t_clause *clause) {
	t_hashslot *slot;
	slot = malloc(sizeof(t_hashslot));
	if (slot) {
		slot->next = NULL;
		slot->clause = clause;
	}
	return slot;
}



/*
 * Destroys the hashmap and dealloc()s memory. It does not
 * dealloc() memory for clauses unless deep evaluates to TRUE
 */
void wipe_hashtable(t_hashtable *map, bool deep) {
	int i=0;
	t_hashslot *sl, *nextsl;
	if (map) {
		for (i=0; i < map->size; i++) {
			sl = map->content[i];
			if (!sl)	
				continue;
			nextsl = sl->next;
			while (nextsl) {
				if (deep)
					wipe_clause(sl->clause);
				free(sl);
				sl = nextsl;
				nextsl = sl->next;
			}
			if (deep)
				wipe_clause(sl->clause);
			free(sl);
		}
		free(map->content);
	}
}



/*
 * Inserts the clause in the map if not alredy present
 */
void put_clause(t_hashtable *map, t_clause *clause) {
	char *cname;
	int key;
	t_hashslot *ptr;
	if (map && clause) {
		cname = get_clause_name(clause);
		key = hash(cname, map->size);
		//printf(">> putting clause %s (hash %d)\n", cname, key);
		ptr = map->content[key];
		while (ptr) {
			// pointer-based comparison
			if (ptr->clause == clause)		return;
			ptr = ptr->next;
		}

		// here if map does not contain clause
		ptr = hashslot(clause);
		if (!ptr) {
			fprintf(stderr, "-- insert_clause() : could not create slot\n");
		}
		ptr->next = map->content[key];
		map->content[key] = ptr;
	}
}



/*
 * Return a pointer to the clause with the given name in
 * the map, if it is present, or NULL otherwise
 */
t_clause *find_clause(t_hashtable *map, char *cname) {
	int key;
	t_hashslot *ptr;
	if (map) {
		key = hash(cname, map->size);
		ptr = map->content[key];
		while (ptr) {
			if (streq(get_clause_name(ptr->clause),cname))
				break;
			ptr = ptr->next;
		}
		return ptr ? ptr->clause : NULL ;
	}
	return NULL;
}



/*
 * Return an array of pointer to the clauses contained in the
 * map (or NULL if map is empty). The last item in the array is
 * NULL.
 */
t_clause **get_clauses(t_hashtable *map) {
	t_clause **clauses = NULL;
	int i, s;
	t_hashslot *ptr;
	if (map && map->size) {
		i = 0;
		s = map->size;
		clauses = malloc((s+1)*sizeof(t_clause*));
		if (clauses) {
			clauses[s] = NULL;
			while (s) {
				ptr = map->content[--s];
				while (ptr) {
					clauses[i++] = ptr->clause;
					//printf("## get_clauses() : found \"%s\" @ %d\n", get_clause_name(ptr->clause), ptr->clause);
					ptr = ptr->next;
				}
			}
		}
	}
	return clauses;
}




/*
 * Dan Bernstein's djb2 hash function, truncated according to size
 */
unsigned int hash(char *str, int size) {
	unsigned int hash = 5381;
	int c;

	while ((c = *str++))
		hash = ((hash << 5) + hash) + c;

	hash &= (unsigned int)pow(2, size)-1;
	return hash % size;
}



