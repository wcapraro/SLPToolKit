#! /usr/bin/env python


import os
import re



def checkDirectoryExists(dirpath, create=False):
	""" Checks whether the specified directory exists """
	try:
		assert os.path.isdir(dirpath)
		return True
	except AssertionError:
		if create:
			os.mkdir(dirpath)
			return True
	return False



def checkFileExists(filepath):
	""" Checks whether the specified file exists """
	try:
		assert os.path.isfile(filepath)
		return True
	except:
		pass
	return False



def buildPath(iterable):
	""" Builds a path string based on the list provided """
	if iterable[0]:
		iterable = [""]+iterable
	return os.path.sep.join(iterable)



def getFileNameFromPath(filepath):
	""" Returns the file name for the file path provided """
	if checkFileExists(filepath):
		return filepath.split(os.path.sep)[-1]
	return ""



def getLastPathSegment(filepath):
	""" Gets the last directory path segment for the provided path """
	if (checkFileExists(filepath)):
		dirpath=buildPath(filepath.split(os.path.sep)[:-1])
	else:
		dirpath=filepath
	return dirpath.split(os.path.sep)[-1]
			



if __name__ == "__main__":

	testpath = buildPath(["home", "wcapraro", "Desktop", "Visconti", "README"])
	print("Path: "+testpath)
	print("Filename: "+getFileNameFromPath(testpath))
	print("Dir: " + getLastPathSegment(testpath))
	print("File exists: "+str(checkFileExists(testpath)))
	print("Dir exists: " +str(checkDirectoryExists(testpath)))
	

	
	

