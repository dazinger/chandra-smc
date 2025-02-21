# Makefile for SMC
#
# Code from Mark A. Sheldon, UMass Lowell, Fall 2011
#
# Edited by Zinger, Spring 2011
 
EXECS = SMC

all: $(EXECS)

SMC: trees.o data.o SMC.o
	$(CC) $(CFLAGS) -o $@ $^

SMC.o: SMC.c

trees.o: globals.h trees.h trees.c

data.o: trees.h data.h data.c

clean:
	rm -f $(EXECS) *.o *~
