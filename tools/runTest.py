#! /usr/bin/env python


import subprocess as sub
import os
import argparse



def __prepareDirectories(testset, indirname="mtx", outdirname="slp", tmpfile="__tmp__.out"):
	out_path = os.sep.join([testset.strip(os.sep), outdirname])
	mtx_path = os.sep.join([testset.strip(os.sep), indirname])
	tmp_path = os.sep.join([testset.strip(os.sep), tmpfile])
	assert os.path.isdir(testset)
	assert os.path.isdir(mtx_path)
	assert not os.path.exists(out_path)
	os.mkdir(out_path)
	if os.path.isfile(tmp_path):
		os.remove(tmp_path)
	return (mtx_path, out_path, tmp_path)


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


def __generateReportFile(outfile, depths, sizes):
	with open(outfile, "w") as f:
		for n,key in enumerate(sorted(depths.keys())):
			f.write("%d\t%s\t%d\t%d\n" % (n, key, depths[key], sizes[key]))	
		f.close()


def avg(seq):
	return sum(seq)/len(seq)



if __name__ == "__main__":

	parser = argparse.ArgumentParser(description="Runs a heuristic on a testset and prints a report")
	parser.add_argument( 'heuristic',	help='Path of the executable to use as heuristic', type=str)
	parser.add_argument( 'testset', 	help='Path on the filesystem of the testes root',  type=str)
	parser.add_argument('-dn', 	help='Name of the output subdirectory for the testet', type=str, default='slp')
	parser.add_argument('-bp', 	help='Path on the filesystem to binaries', type=str, default='bin')

	args = parser.parse_args() 

	print "Running", args.heuristic, "on testset", args.testset

	# stats
	depths = dict()
	sizes = dict()

	# get everything ready...
	(mtxdir, outdir, tmpfile) = __prepareDirectories(args.testset)
	heur = args.heuristic
	slpdpth = os.sep.join([args.bp.strip(os.sep), "slpdpth"])

	# walk through each matrix of the testset
	matrices = [x for x in os.listdir(mtxdir) if x.endswith(".mtx")]
	for mtx in matrices:
		mtxfile = os.sep.join([mtxdir, mtx])
		slpfile = os.sep.join([outdir, mtx.replace('.mtx', '.slp', 1)])
		exec_cmd = "%s -f %s -o %s > /dev/null" % (heur, mtxfile, slpfile)
		if not os.system(exec_cmd):
			print "+@", slpfile
		else:
			raise Exception("Could not create file " + slpfile + ". Aborting...")

	# here if slps compiled correctly
	slps = [x for x in os.listdir(outdir) if x.endswith(".slp")]
	for slp in slps:
		slpfile = os.sep.join([outdir.strip(os.sep), slp])
		exec_cmd = "%s -f %s" % (slpdpth, slpfile)
		line = os.popen(exec_cmd, "r").readline()
		if line:
			depth, size = __processLine(line)
			depths[slp] = depth
			sizes[slp] = size
		else:
			raise Exception("Could not process SLP: " + slp + ". Aborting")

	# generate report
	__generateReportFile(tmpfile, depths, sizes)

	

	
	

