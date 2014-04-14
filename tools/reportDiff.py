#! /usr/bin/env python


import os
import argparse
from itertools import izip_longest as zzip


def __checkDirectories(testset):
	assert os.path.isdir(testset)
	return [x for x in os.listdir(testset) if x.startswith("report")]

if __name__ == "__main__":

	parser = argparse.ArgumentParser(description="Examines each available report in the given testes directory "
				"and prints useful information")
	parser.add_argument( 'testset', 	help='Path on the filesystem of the testes root',  type=str)
	parser.add_argument( '-o', 			help='Name of the output file (if any)', type=str)
	args = parser.parse_args() 

	reports = __checkDirectories(args.testset)
	handles = [open(os.sep.join([args.testset.strip(os.sep), x]), "r") for x in reports]
	
	diff_count = 0
	for a, b in zzip(handles[0], handles[1]):
		da = a.split()[2]
		db = b.split()[2]
		sa = a.split()[3]
		sb = b.split()[3]

		if (da != db):
			print "+1" if int(da) > int(db) else "-1"
			diff_count = 1+diff_count
		print diff_count
	

	

	
	

