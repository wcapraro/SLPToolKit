#! /usr/bin/env python


import os
import re
import argparse
from itertools import izip_longest as zzip
from prettytable import PrettyTable
from logger import Logger


def __checkPaths(repo1, repo2, outfile):
	assert os.path.isfile(repo1)
	assert os.path.isfile(repo2)
	if outfile:
		assert not os.path.exists(outfile)


def __isValidLine(toks1, toks2):
	if toks1[0].strip().endswith('.slp'):
		if toks1[0] == toks2[0]:
			try:
				int(toks1[1])+int(toks1[2])+int(toks2[1])+int(toks2[2])
				return True
			except Exception:
				pass
	#Logger().err("Illegal line: " + str(toks1))
	return False


if __name__ == "__main__":

	parser = argparse.ArgumentParser(description =	"""
													Examines the two specified report files and 
													prints some useful information and statistics
													"""
	)
	parser.add_argument( 'report1', 	help='Path to file to use as first report' ,  type=str)
	parser.add_argument( 'report2', 	help='Path to file to use as second report',  type=str)
	parser.add_argument( '-o', 			help='Name of the output file (if any)', type=str)
	args = parser.parse_args() 

	__checkPaths(args.report1, args.report2, args.o)
	handle1 = open(args.report1, "r")
	handle2 = open(args.report2, "r")
	name1 = args.report1.split(os.sep)[-1]
	name2 = args.report2.split(os.sep)[-1]
	log = Logger()
	cols = ['slp name', name1, name2, 'delta']
	dpthTable = PrettyTable(cols)
	sizeTable = PrettyTable(cols)
	stats = {	
				'read_lines':0,
				'dpth_mismatch':0, 
				'avg_dpth_1':0.0, 
				'avg_size_1':0.0, 
				'avg_dpth_2':0.0, 
				'avg_size_2':0.0,
				'avg_delta_dpth':0.0,
				'avg_delta_size':0.0,
				'max_delta_dpth':0,
				'max_delta_size':0
	}

	for a, b in zzip(handle1, handle2):
		
		# examine each string and check for validity
		tokens1 = [s for s in re.split('[| ]+', a) if s and s != '\n']
		tokens2 = [s for s in re.split('[| ]+', b) if s and s != '\n']
		if (__isValidLine(tokens1, tokens2)):			
			name = tokens1[0]
			d1 = int(tokens1[1])
			d2 = int(tokens2[1])
			s1 = int(tokens1[2])
			s2 = int(tokens2[2])
			dpthTable.add_row([name, d1, d2, d1-d2])
			sizeTable.add_row([name, s1, s2, s1-s2])
			stats['read_lines']=stats['read_lines']+1
			stats['avg_dpth_1']=stats['avg_dpth_1']+d1
			stats['avg_dpth_2']=stats['avg_dpth_2']+d2
			stats['avg_size_1']=stats['avg_size_1']+s1
			stats['avg_size_2']=stats['avg_size_2']+s2
			stats['avg_delta_dpth']=stats['avg_delta_dpth']+abs(d1-d2)
			stats['avg_delta_size']=stats['avg_delta_size']+abs(s1-s2)
			if (d1 != d2):
				stats['dpth_mismatch']=stats['dpth_mismatch']+1
			if (abs(d1-d2)) > stats['max_delta_dpth']:
				stats['max_delta_dpth'] = abs(d1-d2)
			if (abs(s1-s2)) > stats['max_delta_size']:
				stats['max_delta_size'] = abs(s1-s2)

	# end loop
	stats['avg_dpth_1']=stats['avg_dpth_1']/stats['read_lines']
	stats['avg_dpth_2']=stats['avg_dpth_2']/stats['read_lines']
	stats['avg_size_1']=stats['avg_size_1']/stats['read_lines']
	stats['avg_size_2']=stats['avg_size_2']/stats['read_lines']
	stats['avg_delta_dpth']=stats['avg_delta_dpth']/stats['read_lines']
	stats['avg_delta_size']=stats['avg_delta_size']/stats['read_lines']

	statTab = PrettyTable()
	statTab.add_column('', [	
			'Average depth', 'Average size', 'Average delta depth', 'Average delta size', 
			'Max delta depth', 'Max delta size', 'Depth mismatches'
		])
	statTab.add_column(name1, [
			stats['avg_dpth_1'], stats['avg_size_1'], stats['avg_delta_dpth'], stats['avg_delta_size'], 
			stats['max_delta_dpth'], stats['max_delta_size'], stats['dpth_mismatch']
		])
	statTab.add_column(name2, [
			stats['avg_dpth_2'], stats['avg_size_2'], stats['avg_delta_dpth'], stats['avg_delta_size'], 
			stats['max_delta_dpth'], stats['max_delta_size'], stats['dpth_mismatch']
		])
	statTab.align['']='l'

	
	# output
	if args.o:
		with open(args.o, 'w') as fout:
			fout.write(log.info('DEPTH ANALYSIS\n\n', prefix='\n\n>>'))
			fout.write(str(dpthTable))
			fout.write(log.info('SIZE ANALYSIS\n\n', prefix='\n\n>>'))
			fout.write(str(sizeTable))
			fout.write(log.info('STATISTICS\n\n', prefix='\n\n>>'))
			fout.write(str(statTab))
		fout.close()

	else:
		print statTab

	
	

	

	
	

