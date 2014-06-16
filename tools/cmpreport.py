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
COL_BEST = 'Improvement'
COL_WORST = 'Deterioration'
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


def __applyTableStyle(table, style):
	if table and style:
		if style == 'ascii':
			pass
		elif style == 'plain':
			table.set_style(PLAIN_COLUMNS)


if __name__ == "__main__":

	parser = argparse.ArgumentParser(description =	"""
													Examines the two specified report files and 
													prints some useful information and statistics
													"""
	)
	parser.add_argument( 'report1', help='Path to file to use as first report' ,  type=str)
	parser.add_argument( 'report2', help='Path to file to use as second report',  type=str)
	parser.add_argument( '-o', 		help='Name of the output file', type=str)
	parser.add_argument( '-s', 		help='What test to perform', choices=['depth', 'size'], dest='test', default='depth')
	parser.add_argument( '-t',		help='Selects the style to apply for the output', choices=['plain', 'ascii'], default='ascii', dest='style')
	parser.add_argument( '-So',		help='Just return statistics', action='store_true', default=False, dest='statsonly')
	parser.add_argument( '-n1',		help='Name of the first heuristic', type=str)
	parser.add_argument( '-n2',		help='Name of the second heuristic', type=str)
	parser.add_argument( '-q', 		help='Keep quiet', action='store_true', default=False)
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
				COL_BEST:'-',
				COL_WORST:'-'
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
			dpthTable.add_row([name, d1, d2, "{0:+d}".format(d1-d2)])
			sizeTable.add_row([name, s1, s2, "{0:+d}".format(s1-s2)])

			k1 = s1 if args.test == "size" else d1
			k2 = s2 if args.test == "size" else d2
			
			# counters
			__addToDict(stats, COL_COUNT, 1)
			if (k1 != k2):
				winner = COL_WIN1 if k1<k2 else COL_WIN2
				__addToDict(stats, winner, 1)
			else:
				__addToDict(stats, COL_TIES, 1)

			# delta
			delta = k1-k2
			if delta < 0:
				if stats[COL_BEST] == '-':
					stats[COL_BEST]=0
				stats[COL_BEST]=min([stats[COL_BEST], delta])
			elif delta > 0:
				if stats[COL_WORST] == '-':
					stats[COL_WORST] = 0
				stats[COL_WORST]=max([stats[COL_WORST], delta])

			# average
			__addToDict(stats, COL_AVG1, k1)
			__addToDict(stats, COL_AVG2, k2)

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
	statTab.add_column(COL_BEST, [stats[COL_BEST] if stats[COL_BEST] == '-' else "{0:+d}".format(stats[COL_BEST])])	
	statTab.add_column(COL_WORST, [stats[COL_WORST] if stats[COL_WORST] == '-' else "{0:+d}".format(stats[COL_WORST])])	

	# apply styles to tables
	__applyTableStyle(dpthTable, args.style)
	__applyTableStyle(sizeTable, args.style)
	__applyTableStyle(statTab, args.style)
		
	
	# output
	if args.o:
		with open(args.o, 'w') as fout:
			if (args.test == "depth" and not args.statsonly):
				fout.write(log.info('DEPTH ANALYSIS\n\n', prefix='\n\n>>'))
				fout.write(str(dpthTable))
			elif (args.test == "size" and not args.statsonly):
				fout.write(log.info('SIZE ANALYSIS\n\n', prefix='\n\n>>'))
				fout.write(str(sizeTable))
			fout.write(log.info('STATISTICS\n\n', prefix='\n\n>>'))
			fout.write(str(statTab))
		fout.close()

	if not args.q:
		__applyTableStyle(statTab, 'plain')
		statTab.header=False
		print statTab

	

	
	

