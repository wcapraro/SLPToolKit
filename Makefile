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
slplib/slp.o \


#
# TARGETS 
#

all: slpsolve slpchk slpgen slpdpth ldg ildg xldg



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
	
	

ildg: ildg.c slplib.h $(SLPLIBS)
	$(CC) $(CFLAGS) ildg.c slplib.h $(SLPLIBS) -lm -o $(BIN_DIR)/ildg
	
	

xldg: xldg.c slplib.h $(SLPLIBS)
	$(CC) $(CFLAGS) xldg.c slplib.h $(SLPLIBS) -lm -o $(BIN_DIR)/xldg

	
clean: 
	rm -rf *.o 
	rm -f $(SLPLIBS) 
	rm -rf $(BIN_DIR)/*

