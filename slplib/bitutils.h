/*
 * bitutils.h
 *
 *  Created on: 14/mag/2013
 *   Author: Wiliam Capraro - wiliam.capraro@studenti.unimi.it
 */

#ifndef BITUTILS_H_
#define BITUTILS_H_


#define bool unsigned short
#define TRUE (bool)1
#define FALSE (bool)0
#define INVALID_BOOL_VALUE (bool)11



typedef struct {
	int size;
	bool *data;
} t_bitarray;



t_bitarray *bitarray(int);
void bitclr(t_bitarray*, int);
void bitset(t_bitarray*, int);
void bittgl(t_bitarray*, int);
bool getbit(t_bitarray*, int);
t_bitarray *ba_and(t_bitarray*, t_bitarray*);
t_bitarray *ba_xor(t_bitarray*, t_bitarray*);
t_bitarray *ba_or(t_bitarray*, t_bitarray*);
int hamming(t_bitarray*);
void ba_print(t_bitarray*);
void wipe(t_bitarray*);


#endif /* BITUTILS_H_ */
