#! /usr/bin/env python


import os
import argparse
from prettytable import PrettyTable
from logger import Logger



def __prepareDirectories(testset, heuristic, indirname="mtx", outdirname="slp"):
	reportfile = "%s.report" % outdirname.split(os.sep)[-1]
	out_path = os.sep.join([testset.strip(os.sep), outdirname])
	mtx_path = os.sep.join([testset.strip(os.sep), indirname])
	report_path = os.sep.join([testset.strip(os.sep), reportfile])
	assert os.path.isdir(testset)
	assert os.path.isdir(mtx_path)
	assert not os.path.exists(out_path)
	os.mkdir(out_path)
	if os.path.isfile(report_path):
		os.remove(report_path)
	return (mtx_path, out_path, report_path)


def __processLine(line, find_char="=", end_char=","):
	stt_idx = 0
	end_idx = 0
	values = []
	while len(values) < 2 and stt_idx != -1:
		stt_idx = line.find(find_char, stt_idx) + 1
		end_idx = line.find(end_char, stt_idx)
		values.append(int(line[stt_idx:end_idx]))
		stt_idx = end_idx
	return values



if __name__ == "__main__":

	parser = argparse.ArgumentParser(description =	"""
													Runs a specified heuristic on a specified testset and 
													produces report information that can be saved to
													a file.
													"""
	)
	parser.add_argument( 'heur',	help='Path of the executable to use as heuristic', type=str)
	parser.add_argument( 'ts', 		help='Path on the filesystem of the testset root',  type=str)
	parser.add_argument('-hargs',	help='Heuristic arguments', type=str)
	parser.add_argument('-dn', 		help='Name of the output subdirectory for the testet', type=str, required=True)
	parser.add_argument('-bp', 		help='Path on the filesystem to binaries', type=str, default='bin')
	parser.add_argument('-v', 		help='Verbose', type=bool, default=True)

	args = parser.parse_args() 
	
	log = Logger()
	if (args.v):
		print log.header('Running {0:s} on testset {1:s}'.format(args.heur, args.ts))


	# get everything ready...
	(mtxdir, outdir, reportfile) = __prepareDirectories(args.ts, args.heur, outdirname=args.dn)
	heur = args.heur
	opts = "" if not args.hargs else args.hargs
	slpdpth = os.sep.join([args.bp.strip(os.sep), "slpdpth"])


	# walk through each matrix of the testset and
	# compile the corresponding slp
	matrices = [x for x in os.listdir(mtxdir) if x.endswith(".mtx")]
	for mtx in matrices:
		mtxfile = os.sep.join([mtxdir, mtx])
		slpfile = os.sep.join([outdir, mtx.replace('.mtx', '.slp', 1)])
		exec_cmd = "%s -f %s %s -o %s > /dev/null" % (heur, mtxfile, opts, slpfile)
		if not os.system(exec_cmd):
			print log.info(slpfile)
		else:
			raise Exception("Could not create file " + slpfile + ". Aborting...")


	# generate report
	table = PrettyTable(['slp name', 'depth', 'size'])
	table.align["slp name"]="l"
	slps = [x for x in os.listdir(outdir) if x.endswith(".slp")]
	for slp in slps:
		slpfile = os.sep.join([outdir.strip(os.sep), slp])
		exec_cmd = "%s -f %s" % (slpdpth, slpfile)
		line = os.popen(exec_cmd, "r").readline()
		if line:
			depth, size = __processLine(line)
			table.add_row([slp, depth, size])
		else:
			raise Exception("Could not process SLP: " + slp + ". Aborting")

	
	# pretty print the table
	table.sortby = "slp name"

	with open(reportfile, "w") as rf:
		rf.write(str(table))
	rf.close()
	
	print log.info("Output saved to file {0:s}".format(reportfile))

	

	
	

