#! /usr/bin/env python


import random
import argparse


def __generateMatrixPool(n, m, b, N=1):
	"""Generates a collection of N random 0-1 matrices with the given params"""
	S = list()
	for _ in range(N):
		S += [ __generateMatrix(n, m, b) ]
	return S	


def __generateMatrix(m, n, b):
	"""Generates a nxm 0-1 matrix with bias b"""
	M = [ [ __getRandom(b) for i in range(n) ] for j in range(m) ]
	return M 


def __getRandom(b):
	assert float(b) > 0 and float(b) < 1
	r = random.uniform(0, 1)
	if r > float(b):
		return 0
	else:
		return 1


if __name__ == "__main__":

	#
	# TODO: pretty output to file/directory
	#

	parser = argparse.ArgumentParser(description="Generates a pool of arbitrarily-biased boolean matrices of given size")
	parser.add_argument('-n', help='Number of rows (outputs)', type=int, required=True, dest='n')
	parser.add_argument('-m', help='Number of columns (inputs)', type=int, required=True, dest='m')
	parser.add_argument('-b', help='Bias', type=float, default=0.5, dest='b')
	parser.add_argument('-N', help='Population size', type=int, default=1, dest='N')

	args = parser.parse_args() 

	S = __generateMatrixPool(args.n, args.m, args.b, args.N)
	print(S)	


