#! /usr/bin/env python 


class Logger:

	def __mkStr(self, line, prefix):
		return "%s %s" % (prefix, line)
	
	def info(self, line, prefix="@+"):
		return self.__mkStr(line, prefix)

	def debug(self, line, prefix="@-"):
		return self.__mkStr(line, prefix)

	def err(self, line, prefix="@#"):
		return self.__mkStr(line, prefix)

	def header(self, line, breakline=True):
		if breakline:
			print ""
		print 70*'+'
		print '{0:s} {1:^66s} {2:s}'.format("+", line, "+")
		print 70*'+'
		if breakline:
			print ""

	
if __name__ == "__main__":

	log = Logger()
	print log.header("Logger test code")
	print log.info("This is an info")
	print log.debug("This is a debug string")
	print log.err("THIS IS AN ERROR")
	
			
		
