/*
 * bitutils.c - memory-efficient representation of bit arrays
 * and operations on them
 *
 *  Created on: 14/mag/2013
 *   Author: Wiliam Capraro - wiliam.capraro@studenti.unimi.it
 */


#include <stdio.h>
#include <stdlib.h>
#include "bitutils.h"



t_bitarray *bitarray(int size) {
	int i;
	int s = (size + 7) / 8;
	t_bitarray *ba = NULL;
	bool *data = (bool*)malloc(s*sizeof(char));
	if (data) {
		for (i=0; i<s; i++)
			data[i]=FALSE;
		ba = (t_bitarray*)malloc(sizeof(t_bitarray));
		if (ba) {
			ba->data=data;
			ba->size=size;
		} else {
			free(data);
		}
	}
	return ba;
}



void bitclr(t_bitarray *ba, int pos) {
	if (ba && pos >= 0 && pos < ba->size) {
		int slot = pos / 8;
		int offset = pos % 8;
		ba->data[slot] &= ~( TRUE << (offset));
	} else {
		fprintf(stderr, "-- bitclr():: illegal argument(s)\n");
	}
}



void bitset(t_bitarray *ba, int pos) {
	if (ba && pos >= 0 && pos < ba->size) {
		int slot = pos / 8;
		int offset = pos % 8;
		ba->data[slot] |= ( TRUE << (offset));
	} else {
		fprintf(stderr, "-- bitset():: illegal argument(s)\n");
	}
}



void bittgl(t_bitarray *ba, int pos) {
	if (ba && pos >= 0 && pos < ba->size) {
		int slot = pos / 8;
		int offset = pos % 8;
		ba->data[slot] ^= ( TRUE << (offset));
	} else {
		fprintf(stderr, "-- bittgl():: illegal argument(s)\n");
	}
}



void ba_print(t_bitarray *ba) {
	int i;
	if (ba) {
		for (i=0; i<ba->size; i++) {
			printf("%d", getbit(ba, i) == TRUE);
		}
	}
}


t_bitarray *ba_and(t_bitarray *a, t_bitarray *b) {
	t_bitarray *res = NULL;
	if (a && b && a->size == b->size) {
		printf("## AND ");
		ba_print(a);
		printf(", ");
		ba_print(b);
		printf("\n");
		res = bitarray(a->size);
		int l = (a->size + 7) / 8;
		int i;
		for (i=0; i<l; i++) {
			res->data[i]=a->data[i] & b->data[i];
		}
	}
	return res;
}



t_bitarray *ba_xor(t_bitarray *a, t_bitarray *b) {
	t_bitarray *res = NULL;
	if (a && b && a->size == b->size) {
		res = bitarray(a->size);
		int l = (a->size + 7) / 8;
		int i;
		for (i=0; i<l; i++) {
			res->data[i]=a->data[i] ^ b->data[i];
		}
	}
	return res;
}



t_bitarray *ba_or(t_bitarray *a, t_bitarray *b) {
	t_bitarray *res = NULL;
	if (a && b && a->size == b->size) {
		res = bitarray(a->size);
		int l = (a->size + 7) / 8;
		int i;
		for (i=0; i<l; i++) {
			res->data[i]=a->data[i] | b->data[i];
		}
	}
	return res;
}



int hamming(t_bitarray *ba) {
	int i, l=0;
	if (ba) {
		for (i=0; i<ba->size; i++) {
			if (getbit(ba, i))
				l++;
		}
	}
	return l;
}


bool getbit(t_bitarray *ba, int pos) {
	bool result;
	if (ba && pos >= 0 && pos < ba->size) {
			int slot = pos / 8;
			int offset = pos % 8;
			result = ba->data[slot];
			result &= ( TRUE << (offset));
			result = result >> offset;
		} else {
			fprintf(stderr, "-- getbit():: illegal argument(s)\n");
		}
	return result;
}



void wipe(t_bitarray *ba) {
	if (ba) {
		free(ba->data);
		free(ba);
	}
}
