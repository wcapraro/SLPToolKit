#! /usr/bin/env python


import os
import argparse
from prettytable import PrettyTable
from prettytable import PLAIN_COLUMNS
from logger import Logger


def __checkPaths(infile):
	assert os.path.isfile(infile)


def __getOutPaths(infile):
	if infile:
		pth = os.path.sep.join(infile.split(os.path.sep)[:-2])
		fname = infile.split(os.path.sep)[-1]

		for outdir in ['Report__BP', 'Report__VP']:
			if not os.path.exists(os.path.sep.join([pth, outdir])):
				os.mkdir(os.path.sep.join([pth, outdir]))

		o1 = os.path.sep.join([pth, 'Report__BP', fname])
		o2 = os.path.sep.join([pth, 'Report__VP', fname])

		assert not os.path.exists(o1)
		assert not os.path.exists(o2)

		return (o1, o2)
	else:
		return (None, None)


def __reset(count):
	return (None, -1, -1, -1, -1, count+1)


if __name__ == "__main__":			

	parser = argparse.ArgumentParser(description =	"""Parser output files into a report""")
	parser.add_argument( 'infile',	help='Path on the filesystem of the result file', type=str)
	parser.add_argument( '-v', 		help='Verbose', type=bool, default=True)
	parser.add_argument( '-p', 		help='Use plain output', type=bool, default=False)
	args = parser.parse_args() 
	log = Logger()
	

	__checkPaths(args.infile)
	out1, out2 = __getOutPaths(args.infile)
	(slpname, size1, size2, depth1, depth2, count) = __reset(-1)
	headers = ['slp name', 'depth', 'size']
	table1 = PrettyTable(headers)
	table2 = PrettyTable(headers)
	table1.align["slp name"]="l"
	table2.align["slp name"]="l"
	
	if (args.p):
		table1.set_style(PLAIN_COLUMNS)
		table2.set_style(PLAIN_COLUMNS)
	
	
	with open(args.infile, 'r') as fh:
		for line in fh:
			if "Gates" in line:
				s = int(line[line.find(':')+1:].strip())
				if line.startswith("B-P"):
					size1 = s
				else:
					size2 = s
			elif line.startswith(">>"):
				d = int(line[line.find('=')+1:line.find('@')-1])
				slpname = "{0:0=3d}__{1:s}".format(count, line[line.find('@')+2:-1])
				if depth1 < 0:
					depth1 = d
				else:
					depth2 = d
			if slpname and size1 != -1 and size2 != -1 and depth1 != -1 and depth2 != -1:
				table1.add_row([slpname, depth1, size1])
				table2.add_row([slpname, depth2, size2])
				(slpname, size1, size2, depth1, depth2, count) = __reset(count)
	fh.close()

	
	# pretty print the tables
	table1.sortby = "slp name"
	table2.sortby = "slp name"

	if (out1):
		with open(out1, "w") as fout:
			fout.write(str(table1))
		fout.close()
	if (out2):
		with open(out2, "w") as fout:
			fout.write(str(table2))
		fout.close()
		


	

	
	

