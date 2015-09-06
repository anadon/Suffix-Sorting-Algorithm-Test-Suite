#!/bin/sh
#
# Makefile for building a suffix array test program.  Unknown licensing
# status.
#

##GLOBAL VARIABLES######################################################

CFLAGS = -O3 -march=native -Wall

SOURCE = tryall.c drittel.c

OBJECT = tryall.o drittel.o

EXEC = KS


##STANDARD BUILD########################################################

all : $(OBJECT)
	g++ $(CFLAGS) $(OBJECT) -o $(EXEC)

%.o : %.c $(SOURCE)
	g++ $(CFLAGS) $< -c -o $@

clean :
	rm -f $(EXEC) $(OBJECT)
