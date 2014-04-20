#! /usr/bin/env python


from matrix import Matrix
import os
import argparse


def __generateMatrixPool(n, m, b, N=1):
	"""Generates a collection of N random 0-1 matrices with the given params"""
	S = list()
	count = 0
	while count < N:
		try:
			M = Matrix(n, m, b)
			if M.isValid() and not M in S:
				S += [M]
				count = count+1
		except AssertionError as err:
			print(err)
			continue

	return S



def __getMatrixFileName(n, m, bias, count, base=None, sep=os.path.sep):
	assert int(count) >= 0
	s = str(base)+sep if base else ""
	s += "%dx%d_%.1f_%.3d.mtx" % (n, m, bias, count)
	return s
				


if __name__ == "__main__":

	parser = argparse.ArgumentParser(description="Generates a pool of arbitrarily-biased boolean matrices of given size")
	parser.add_argument( 'n', help='Number of rows (outputs)', type=int)
	parser.add_argument( 'm', help='Number of columns (inputs)', type=int)
	parser.add_argument('-b', help='Bias', type=float, default=0.5, dest='b')
	parser.add_argument('-s', help='Population size', type=int, default=1, dest='N')
	parser.add_argument('-o', help='Output directory on the filesystem', type=str, default='mtx', dest='out')
	parser.add_argument('-x', help='Execute a command for each generated matrix', type=str, dest='cmd')

	args = parser.parse_args() 

	S = __generateMatrixPool(args.n, args.m, args.b, args.N)
	
	if args.out:
		if not os.path.exists(args.out) or os.path.isdir(args.out):
			os.mkdir(args.out)	
		for i, matrix in enumerate(S):
			filename = __getMatrixFileName(args.n, args.m, args.b, i, base=args.out)
			matrix.toFile(filename)
			if args.cmd:
				os.system(args.cmd.replace('?', filename))

	else:
		for matrix in S:
			print str(matrix), "\n" 	


