#
# SLPToolKit Makefile
#

# the compiler to use
CC=gcc

# flags
CFLAGS=-g -I -Wall 

# output directory
BIN_DIR=bin

#libraries
SLPLIBS=\
slplib/bitutils.o \
slplib/bastack.o \
slplib/clause.o \
slplib/parseutils.o \
slplib/fsmparseutils.o \
slplib/hashing.o \
slplib/vectors.o \



#
# TARGETS
#

all: slpsolve slpchk slpgen slpdpth ldg



# compile library objects
slplib/%.o: slplib/%.c
	$(CC) $(CFGAGS) -c -o $@ $< 
	


slpsolve: slpsolve.c slplib.h $(SLPLIBS)
	$(CC) $(CFLAGS) slpsolve.c slplib.h $(SLPLIBS) -lm -o $(BIN_DIR)/slpsolve
	
	

slpchk: slpchk.c slplib.h $(SLPLIBS)
	$(CC) $(CFLAGS) slpchk.c slplib.h $(SLPLIBS) -lm -o $(BIN_DIR)/slpchk



slpgen: slpgen.c slplib.h $(SLPLIBS)
	$(CC) $(CFLAGS) slpgen.c slplib.h $(SLPLIBS) -lm -o $(BIN_DIR)/slpgen
	


slpdpth: slpdpth.c slplib.h $(SLPLIBS)
	$(CC) $(CFLAGS) slpdpth.c slplib.h $(SLPLIBS) -lm -o $(BIN_DIR)/slpdpth
	
	
	
ldg: ldg.c slplib.h $(SLPLIBS)
	$(CC) $(CFLAGS) ldg.c slplib.h $(SLPLIBS) -lm -o $(BIN_DIR)/ldg

	
clean: 
	rm -rf *.o 
	rm -f $(SLPLIBS) 
	rm -f $(BIN_DIR)/slpchk $(BIN_DIR)/slpsolve $(BIN_DIR)/slpgen $(BIN_DIR)/slpdpth $(BIN_DIR)/ldg

