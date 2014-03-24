#! /usr/bin/env python 


import random


class Matrix:
	

	def __init__(self, nrows, ncols, bias):
		assert int(nrows) > 0
		assert int(ncols) > 0
		assert float(bias) > 0 and float(bias) < 1
		self.__nrows = int(nrows)
		self.__ncols = int(ncols)
		self.__bias = float(bias)
		self.__M = Matrix.__generateMatrix(self.__nrows, self.__ncols, self.__bias)



	def isValid(self):
		for row in self.__M:
			if sum(row) == 0:
				return False
			for another_row in self.__M:
				if row == another_row and row is not another_row:
					return False
		return True


	
	def hamming(self):
		return max(sum(self.__M[i]) for i in range(self.__nrows))



	def __print_size(self):
		return "%d %d\n" % (self.__nrows, self.__ncols)



	def __print_row(self, index):
		if (index < 0):
			s = " ".join(["x"+str(i) for i in range(self.__ncols)])+"\n"
		else:
			s = "y"+str(index)
			for bit in self.__M[index]:
				s+=" "+str(bit)
			s+="\n"
		return s



	def __str__(self):
		s = self.__print_size()
		for row in range(-1, self.__nrows):
			s += self.__print_row(row)
		return s



	def toFile(self, filename, mode="a"):
		if filename and self.isValid():
			f = open(filename, mode, buffering=1)
			f.write(str(self))
			f.close()


	@staticmethod
	def __generateMatrix(nrows, ncols, bias):
		S = list()
		for _ in range(nrows):
			S += [Matrix.__generateRow(ncols, bias)]
		return (S)


	@staticmethod
	def __generateRow(ncols, bias):
		row = [ 0 if random.uniform(0, 1) > bias else 1 for _ in range(ncols)]
		return row
		


if __name__ == "__main__":
	m = Matrix(10, 10, 0.6)
	print ""
	print(m)
	print "Hamming = %d" % m.hamming()
	print "Is valid = %r" % m.isValid()
	
			
		
