#! /usr/bin/env python


import os
import re
import argparse
import fsutils as fs
from itertools import izip_longest as zzip
from prettytable import PrettyTable
from prettytable import PLAIN_COLUMNS
from logger import Logger


# Column names for tables
COL_SIZE = 'Size'
COL_H1 = 'First Heuristic'
COL_H2 = 'Second Heuristic'
COL_WIN1 = 'First wins'
COL_WIN2 = 'Second wins'
COL_TIES = 'Ties'
COL_AVG1 = 'First Avg'
COL_AVG2 = 'Second Avg'
COL_AVG_DIFF = 'Delta Avg'
COL_BEST = 'Best improve'
COL_WORST = 'Worst improve'
COL_SLP = 'SLP Name'
COL_DELTA = 'Delta'
COL_COUNT = 'Count'


def __checkPaths(repo1, repo2, outfile):
	assert os.path.isfile(repo1)
	assert os.path.isfile(repo2)
	if outfile:
		assert not os.path.exists(outfile)


def __isValidLine(toks1, toks2, relaxSameName=False):
	if toks1[0].strip().endswith('.slp'):
		if toks1[0] == toks2[0] or relaxSameName:
			try:
				int(toks1[1])+int(toks1[2])+int(toks2[1])+int(toks2[2])
				return True
			except Exception:
				pass
	return False



def __addToDict(a_dict, key, value):
	a_dict[key]=a_dict[key]+value


def __detectNames(f1, f2):
	n1 = COL_H1 if not args.n1 else args.n1
	n2 = COL_H2 if not args.n2 else args.n2
	return n1, n2


def __detectSize(f1):
	if (os.path.isfile(f1)):
		times = re.findall('\d\dx\d\d', f1)
		return times[0] if times else '?x?'



if __name__ == "__main__":

	parser = argparse.ArgumentParser(description =	"""
													Examines the two specified report files and 
													prints some useful information and statistics
													"""
	)
	parser.add_argument( 'report1', 	help='Path to file to use as first report' ,  type=str)
	parser.add_argument( 'report2', 	help='Path to file to use as second report',  type=str)
	parser.add_argument( '-o', 			help='Name of the output file (if any)', type=str)
	parser.add_argument( '-s', 			help='Perform size analysis', type=bool, default=False)
	parser.add_argument( '-d', 			help='Perform depth analysis', type=bool, default=False)
	parser.add_argument( '-t', 			help='Produce statistical data', type=bool, default=True)
	parser.add_argument( '-p', 			help='Plain output', type=bool, default=False)
	parser.add_argument( '-n1',			help='Name of the first heuristic', type=str)
	parser.add_argument( '-n2',			help='Name of the second heuristic', type=str)
	args = parser.parse_args() 

	# Check and open report files
	__checkPaths(args.report1, args.report2, args.o)
	handle1 = open(args.report1, "r")
	handle2 = open(args.report2, "r")
	
	(name1, name2) = __detectNames(args.report1, args.report2)
	size = __detectSize(args.report1)

	# Some debugging aid ;)
	log = Logger()

	# Tables
	cols = [COL_SLP, name1, name2, COL_DELTA]
	dpthTable = PrettyTable(cols)
	sizeTable = PrettyTable(cols)
	
	# Stats dict
	stats = {	
				COL_COUNT:0,
				COL_WIN1:0,
				COL_WIN2:0,
				COL_TIES:0,
				COL_AVG1:0.0,
				COL_AVG2:0.0,
				COL_AVG_DIFF:0.0,
				COL_BEST:0.0,
				COL_WORST:0.0
	}

	# Parse report
	for a, b in zzip(handle1, handle2):
		
		# examine each string and check for validity
		tokens1 = [s for s in re.split('[| ]+', a) if s and s != '\n']
		tokens2 = [s for s in re.split('[| ]+', b) if s and s != '\n']
		if (__isValidLine(tokens1, tokens2, relaxSameName=True)):			
			name = tokens1[0]
			d1 = int(tokens1[1])
			d2 = int(tokens2[1])
			s1 = int(tokens1[2])
			s2 = int(tokens2[2])
			dpthTable.add_row([name, d1, d2, d2-d1])
			sizeTable.add_row([name, s1, s2, s2-s1])
			
			# counters
			__addToDict(stats, COL_COUNT, 1)
			if (d1 != d2):
				winner = COL_WIN1 if d1<d2 else COL_WIN2
				__addToDict(stats, winner, 1)
			else:
				__addToDict(stats, COL_TIES, 1)

			# delta
			delta = d2-d1
			if (delta < stats[COL_WORST]):
				stats[COL_WORST]=delta
			elif (delta > stats[COL_BEST]):
				stats[COL_BEST]=delta

			# average
			__addToDict(stats, COL_AVG1, d1)
			__addToDict(stats, COL_AVG2, d2)

	# end for loop
	try:
		stats[COL_AVG1]=stats[COL_AVG1]/stats[COL_COUNT]
		stats[COL_AVG2]=stats[COL_AVG2]/stats[COL_COUNT]
		stats[COL_AVG_DIFF]=stats[COL_AVG1]-stats[COL_AVG2]
	except Exception as e:
		log.err(e)
	
	# create a table for stats
	statTab = PrettyTable()
	statTab.add_column(COL_SIZE, [size])
	statTab.add_column(COL_WIN1, [stats[COL_WIN1]])	
	statTab.add_column(COL_WIN2, [stats[COL_WIN2]])	
	statTab.add_column(COL_TIES, [stats[COL_TIES]])	
	statTab.add_column(COL_AVG1, [stats[COL_AVG1]])	
	statTab.add_column(COL_AVG2, [stats[COL_AVG2]])	
	statTab.add_column(COL_AVG_DIFF, [stats[COL_AVG_DIFF]])	
	statTab.add_column(COL_BEST, [stats[COL_BEST]])	
	statTab.add_column(COL_WORST, [stats[COL_WORST]])	

	if (args.p):
		dpthTable.set_style(PLAIN_COLUMNS)
		sizeTable.set_style(PLAIN_COLUMNS)
		statTab.set_style(PLAIN_COLUMNS)
	
	# output
	if args.o:
		with open(args.o, 'w') as fout:
			if (args.d):
				fout.write(log.info('DEPTH ANALYSIS\n\n', prefix='\n\n>>'))
				fout.write(str(dpthTable))
			if (args.s):
				fout.write(log.info('SIZE ANALYSIS\n\n', prefix='\n\n>>'))
				fout.write(str(sizeTable))
			fout.write(log.info('STATISTICS\n\n', prefix='\n\n>>'))
			fout.write(str(statTab))
		fout.close()

	else:
		print statTab

	
# TODO: infer size, column names, heur names? + plain print option	

	

	
	
